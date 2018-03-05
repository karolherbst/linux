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
#ifndef NOUVEAU_HMM_H
#define NOUVEAU_HMM_H
#include <nvif/object.h>
#include <nvif/notify.h>
#include <nouveau_vmm.h>
#include <linux/hmm.h>

#if defined(CONFIG_HMM_MIRROR) && defined(CONFIG_DEVICE_PRIVATE)

struct nouveau_hmm {
	struct nouveau_vmm_hole hole;
	struct nvif_object rpfb;
	struct nvif_notify pending;
	struct task_struct *task;
	struct hmm_mirror mirror;
	struct mm_struct *mm;
	struct mutex mutex;
	bool enabled;
};

void nouveau_vmm_hmm_release(struct hmm_mirror *mirror);
void nouveau_hmm_fini(struct nouveau_cli *cli);
int nouveau_hmm_init(struct nouveau_cli *cli);

#else /* defined(CONFIG_HMM_MIRROR) && defined(CONFIG_DEVICE_PRIVATE) */

struct nouveau_hmm {
};

static inline void nouveau_hmm_fini(struct nouveau_cli *cli)
{
}

static inline void nouveau_hmm_init(struct nouveau_cli *cli)
{
	return -EINVAL;
}

#endif /* defined(CONFIG_HMM_MIRROR) && defined(CONFIG_DEVICE_PRIVATE) */
#endif /* NOUVEAU_HMM_H */
