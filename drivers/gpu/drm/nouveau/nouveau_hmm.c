/*
 * Copyright (C) 2018 Red Hat All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Author: Jérôme Glisse, Ben Skeggs
 */
#include <nvif/class.h>
#include <nvif/clb069.h>
#include "nouveau_hmm.h"
#include "nouveau_drv.h"
#include "nouveau_bo.h"
#include <nvkm/subdev/mmu.h>
#include <linux/sched/mm.h>
#include <linux/mm.h>

struct fault_entry {
	u32 instlo;
	u32 insthi;
	u32 addrlo;
	u32 addrhi;
	u32 timelo;
	u32 timehi;
	u32 rsvd;
	u32 info;
};

#define NV_PFAULT_ACCESS_R 0 /* read */
#define NV_PFAULT_ACCESS_W 1 /* write */
#define NV_PFAULT_ACCESS_A 2 /* atomic */
#define NV_PFAULT_ACCESS_P 3 /* prefetch */

static inline u64
fault_entry_addr(const struct fault_entry *fe)
{
	return ((u64)fe->addrhi << 32) | (fe->addrlo & PAGE_MASK);
}

static inline unsigned
fault_entry_access(const struct fault_entry *fe)
{
	return ((u64)fe->info >> 16) & 7;
}

struct nouveau_vmf {
	struct vm_area_struct *vma;
	struct nouveau_cli *cli;
	uint64_t *pages;;
	u64 npages;
	u64 start;
};

static void
nouveau_hmm_fault_signal(struct nouveau_cli *cli,
			 struct fault_entry *fe,
			 bool success)
{
	u32 gpc, isgpc, client;

	if (!(fe->info & 0x80000000))
		return;

	gpc    = (fe->info & 0x1f000000) >> 24;
	isgpc  = (fe->info & 0x00100000) >> 20;
	client = (fe->info & 0x00007f00) >> 8;
	fe->info &= 0x7fffffff;

	/* Only report error right away */
	if (success)
		return;

	/*
	 * Looks like maybe a "free flush slots" counter, the
	 * faster you write to 0x100cbc to more it decreases.
	 */
	while (!(nvif_rd32(&cli->device.object, 0x100c80) & 0x00ff0000)) {
		msleep(1);
	}

	nvif_wr32(&cli->device.object, 0x100cbc, 0x80000000 |
		  (3 << 3) | (client << 9) |
		  (gpc << 15) | (isgpc << 20));

	/* Wait for flush to be queued? */
	while (!(nvif_rd32(&cli->device.object, 0x100c80) & 0x00008000)) {
		msleep(1);
	}
}

static const uint64_t hmm_pfn_flags[HMM_PFN_FLAG_MAX] = {
	/* FIXME find a way to build time check order */
	NV_HMM_PAGE_FLAG_V, /* HMM_PFN_VALID */
	NV_HMM_PAGE_FLAG_W, /* HMM_PFN_WRITE */
	0, /* HMM_PFN_DEVICE_PRIVATE */
};

static const uint64_t hmm_pfn_values[HMM_PFN_VALUE_MAX] = {
	/* FIXME find a way to build time check order */
	NV_HMM_PAGE_VALUE_E, /* HMM_PFN_ERROR */
	NV_HMM_PAGE_VALUE_N, /* HMM_PFN_NONE */
	NV_HMM_PAGE_VALUE_S, /* HMM_PFN_SPECIAL */
};

static int
nouveau_hmm_handle_fault(struct nouveau_vmf *vmf)
{
	struct nouveau_hmm *hmm = &vmf->cli->hmm;
	struct hmm_range range;
	int ret;

	range.vma = vmf->vma;
	range.start = vmf->start;
	range.end = vmf->start + (vmf->npages << PAGE_SHIFT);
	range.pfns = vmf->pages;
	range.pfn_shift = NV_HMM_PAGE_PFN_SHIFT;
	range.flags = hmm_pfn_flags;
	range.values = hmm_pfn_values;

	ret = hmm_vma_fault(&range, true);
	if (ret)
		return ret;

	mutex_lock(&hmm->mutex);
	if (!hmm_vma_range_done(&range)) {
		mutex_unlock(&hmm->mutex);
		return -EAGAIN;
	}

	nvif_vmm_hmm_map(&vmf->cli->vmm.vmm, vmf->start,
			 vmf->npages, (u64 *)vmf->pages);
	mutex_unlock(&hmm->mutex);
	return 0;
}

static int
nouveau_hmm_rpfb_process(struct nvif_notify *ntfy)
{
	struct nouveau_hmm *hmm = container_of(ntfy, typeof(*hmm), pending);
	struct nouveau_cli *cli = container_of(hmm, typeof(*cli), hmm);
	u32 get = nvif_rd32(&cli->device.object, 0x002a7c);
	u32 put = nvif_rd32(&cli->device.object, 0x002a80);
	struct fault_entry *fe = (void *)hmm->rpfb.map.ptr;
	u32 processed = 0, mapped = 0, restart = 0, next;
	u32 max = (u32)(hmm->rpfb.map.size >> 5);

	if (get > put) {
		restart = put;
		put = max;
	}

restart:
	for (; hmm->enabled && (get != put); get = next) {
		/* FIXME something else than a 16 pages window ... */
		const u64 max_pages = 16;
		const u64 range_mask = (max_pages << PAGE_SHIFT) - 1;
		u64 addr, start, end, i;
		struct nouveau_vmf vmf;
		u64 pages[16] = {0};
		int ret;

		if (!(fe[get].info & 0x80000000)) {
			processed++; next = get + 1;
			continue;
		}

		start = fault_entry_addr(&fe[get]) & (~range_mask);
		end = start + range_mask + 1;

		for (next = get; next < put; ++next) {
			unsigned access;

			if (!(fe[next].info & 0x80000000)) {
				continue;
			}

			addr = fault_entry_addr(&fe[next]);
			if (addr < start || addr >= end) {
				break;
			}

			i = (addr - start) >> PAGE_SHIFT;
			access = fault_entry_access(&fe[next]);
			pages[i] = (access == NV_PFAULT_ACCESS_W) ?
				NV_HMM_PAGE_FLAG_V |
				NV_HMM_PAGE_FLAG_W :
				NV_HMM_PAGE_FLAG_V;
		}

again:
		down_read(&hmm->mm->mmap_sem);
		vmf.vma = find_vma_intersection(hmm->mm, start, end);
		if (vmf.vma == NULL) {
			up_read(&hmm->mm->mmap_sem);
			for (i = 0; i < max_pages; ++i) {
				pages[i] = NV_HMM_PAGE_VALUE_E;
			}
			goto signal;
		}

		/* Mark error */
		for (addr = start, i = 0; addr < vmf.vma->vm_start;
		     addr += PAGE_SIZE, ++i) {
			pages[i] = NV_HMM_PAGE_VALUE_E;
		}
		for (addr = end - PAGE_SIZE, i = max_pages - 1;
		     addr >= vmf.vma->vm_end; addr -= PAGE_SIZE, --i) {
			pages[i] = NV_HMM_PAGE_VALUE_E;
		}
		vmf.start = max_t(u64, start, vmf.vma->vm_start);
		end = min_t(u64, end, vmf.vma->vm_end);

		vmf.cli = cli;
		vmf.pages = &pages[(vmf.start - start) >> PAGE_SHIFT];
		vmf.npages = (end - vmf.start) >> PAGE_SHIFT;
		ret = nouveau_hmm_handle_fault(&vmf);
		switch (ret) {
		case -EAGAIN:
			up_read(&hmm->mm->mmap_sem);
			/* fallthrough */
		case -EBUSY:
			/* Try again */
			goto again;
		default:
			up_read(&hmm->mm->mmap_sem);
			break;
		}

	signal:
		for (; get < next; ++get) {
			bool success;

			if (!(fe[get].info & 0x80000000)) {
				continue;
			}

			addr = fault_entry_addr(&fe[get]);
			i = (addr - start) >> PAGE_SHIFT;
			success = (pages[i] != NV_HMM_PAGE_VALUE_E);
			nouveau_hmm_fault_signal(cli, &fe[get], success);
			mapped += success;
		}
	}

	if (mapped) {
		/*
		 * Looks like maybe a "free flush slots" counter, the
		 * faster you write to 0x100cbc to more it decreases.
		 */
		while (!(nvif_rd32(&cli->device.object, 0x100c80) & 0x00ff0000)) {
			msleep(1);
		}

		nvif_wr32(&cli->device.object, 0x100cbc, 0x80000000 |
			  (1 << 3) | (1 << 1) | (1 << 0));

		/* Wait for flush to be queued? */
		while (!(nvif_rd32(&cli->device.object, 0x100c80) & 0x00008000)) {
			msleep(1);
		}
	}

	nvif_wr32(&cli->device.object, 0x002a7c, get - 1);
	if (restart) {
		put = restart;
		restart = 0;
		get = 0;
		goto restart;
	}

	return hmm->enabled ? NVIF_NOTIFY_KEEP : NVIF_NOTIFY_DROP;
}

static void
nouveau_vmm_sync_pagetables(struct hmm_mirror *mirror,
			    enum hmm_update_type update,
			    unsigned long start,
			    unsigned long end)
{
	struct nouveau_hmm *hmm;
	struct nouveau_cli *cli;

	hmm = container_of(mirror, struct nouveau_hmm, mirror);
	if (!hmm->hole.vma || hmm->hole.start == hmm->hole.end)
		return;

	/* Ignore area inside hole */
	end = min(end, TASK_SIZE);
	if (start >= hmm->hole.start && end <= hmm->hole.end)
		return;
	if (start < hmm->hole.start && end > hmm->hole.start) {
		nouveau_vmm_sync_pagetables(mirror, update, start,
					    hmm->hole.start);
		start = hmm->hole.end;
	} else if (start < hmm->hole.end && start >= hmm->hole.start) {
		start = hmm->hole.end;
	}
	if (end <= start)
		return;

	cli = container_of(hmm, struct nouveau_cli, hmm);
	mutex_lock(&hmm->mutex);
	nvif_vmm_hmm_unmap(&cli->vmm.vmm, start, (end - start) >> PAGE_SHIFT);
	mutex_unlock(&hmm->mutex);
}

void
nouveau_vmm_hmm_release(struct hmm_mirror *mirror)
{
	struct nouveau_hmm *hmm;
	struct nouveau_cli *cli;

	hmm = container_of(mirror, struct nouveau_hmm, mirror);
	if (!hmm->enabled)
		return;

	hmm->enabled = false;
	nvif_notify_fini(&hmm->pending);
	nvif_object_fini(&hmm->rpfb);
	cli = container_of(hmm, struct nouveau_cli, hmm);

	nvif_vmm_hmm_fini(&cli->vmm.vmm, hmm->hole.start, hmm->hole.end);
	nouveau_vmm_sync_pagetables(&hmm->mirror, HMM_UPDATE_INVALIDATE,
				    PAGE_SIZE, TASK_SIZE);
}

static const struct hmm_mirror_ops nouveau_hmm_mirror_ops = {
	.sync_cpu_device_pagetables	= &nouveau_vmm_sync_pagetables,
	.release			= &nouveau_vmm_hmm_release,
};

void
nouveau_hmm_fini(struct nouveau_cli *cli)
{
	nouveau_vmm_hmm_release(&cli->hmm.mirror);
	hmm_mirror_unregister(&cli->hmm.mirror);
}

int
nouveau_hmm_init(struct nouveau_cli *cli)
{
	struct mm_struct *mm = get_task_mm(current);
	static const struct nvif_mclass rpfbs[] = {
		{ MAXWELL_FAULT_BUFFER_A, -1 },
		{}
	};
	bool super;
	int ret;

	if (cli->hmm.mm)
		return 0;

	mutex_init(&cli->hmm.mutex);

	down_write(&mm->mmap_sem);
	mutex_lock(&cli->hmm.mutex);
	cli->hmm.mirror.ops = &nouveau_hmm_mirror_ops;
	ret = hmm_mirror_register(&cli->hmm.mirror, mm);
	if (!ret)
		cli->hmm.mm = mm;
	mutex_unlock(&cli->hmm.mutex);
	up_write(&mm->mmap_sem);
	mmput(mm);
	if (ret)
		return ret;

	/* Allocate replayable fault buffer. */
	ret = nvif_mclass(&cli->device.object, rpfbs);
	if (ret < 0) {
		hmm_mirror_unregister(&cli->hmm.mirror);
		cli->hmm.mm = NULL;
		return ret;
	}

	super = cli->base.super;
	cli->base.super = true;
	ret = nvif_object_init(&cli->device.object, 0,
			       rpfbs[ret].oclass,
			       NULL, 0, &cli->hmm.rpfb);
	if (ret) {
		hmm_mirror_unregister(&cli->hmm.mirror);
		cli->base.super = super;
		cli->hmm.mm = NULL;
		return ret;
	}
	nvif_object_map(&cli->hmm.rpfb, NULL, 0);

	/* Request notification of pending replayable faults. */
	ret = nvif_notify_init(&cli->hmm.rpfb, nouveau_hmm_rpfb_process,
			       true, NVB069_VN_NTFY_FAULT, NULL, 0, 0,
			       &cli->hmm.pending);
	cli->base.super = super;
	if (ret)
		goto error_notify;

	ret = nvif_notify_get(&cli->hmm.pending);
	if (ret)
		goto error_notify_get;

	cli->hmm.mm = current->mm;
	cli->hmm.task = current;
	cli->hmm.enabled = true;
	return 0;

error_notify_get:
	nvif_notify_fini(&cli->hmm.pending);
error_notify:
	nvif_object_fini(&cli->hmm.rpfb);
	hmm_mirror_unregister(&cli->hmm.mirror);
	cli->hmm.mm = NULL;
	return ret;
}
