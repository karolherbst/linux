/*
 * Copyright 2012 Red Hat Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Ben Skeggs
 */
#include "gf100.h"
#include "ram.h"

#include <core/memory.h>
#include <core/option.h>
#include <subdev/therm.h>

u32
gf100_fb_vidmem_probe_fbpa_amount(struct nvkm_device *device, int fbpa)
{
	return nvkm_rd32(device, 0x11020c + (fbpa * 0x1000));
}

static u32
gf100_fb_vidmem_probe_fbp_amount(struct nvkm_fb *fb, u32 fbpao, int fbp, int *pltcs)
{
	if (!(fbpao & BIT(fbp))) {
		*pltcs = 1;
		return fb->func->vidmem.probe_fbpa_amount(fb->subdev.device, fbp);
	}

	return 0;
}

u32
gf100_fb_vidmem_probe_fbp(struct nvkm_fb *fb, int fbp, int *pltcs)
{
	u32 fbpao = nvkm_rd32(fb->subdev.device, 0x022554);

	return fb->func->vidmem.probe_fbp_amount(fb, fbpao, fbp, pltcs);
}

u64
gf100_fb_vidmem_size(struct nvkm_fb *fb, u64 *plower, u64 *pubase, u64 *pusize)
{
	struct nvkm_subdev *subdev = &fb->subdev;
	struct nvkm_device *device = subdev->device;
	u32 fbps = nvkm_rd32(device, 0x022438);
	u64 total = 0, lcomm = ~0;
	int fbp, ltcs, ltcn = 0;

	nvkm_debug(subdev, "%d FBP(s)\n", fbps);
	for (fbp = 0; fbp < fbps; fbp++) {
		u32 size = fb->func->vidmem.probe_fbp(fb, fbp, &ltcs);
		if (size) {
			nvkm_debug(subdev, "FBP %d: %4d MiB, %d LTC(s)\n", fbp, size, ltcs);
			lcomm  = min(lcomm, (u64)(size / ltcs) << 20);
			total += (u64)size << 20;
			ltcn  += ltcs;
		} else {
			nvkm_debug(subdev, "FBP %d: disabled\n", fbp);
		}
	}

	if (plower) {
		*plower = lcomm * ltcn;
		*pubase = lcomm + fb->func->vidmem.upper;
		*pusize = total - *plower;

		nvkm_debug(subdev, "Lower: %4lld MiB @ %010llx\n", *plower >> 20, 0ULL);
		nvkm_debug(subdev, "Upper: %4lld MiB @ %010llx\n", *pusize >> 20, *pubase);
		nvkm_debug(subdev, "Total: %4lld MiB\n", total >> 20);
	}

	return total;
}

enum nvkm_ram_type
gf100_fb_vidmem_type(struct nvkm_fb *fb)
{
	return nvkm_fb_bios_memtype(fb->subdev.device->bios);
}

void
gf100_fb_intr(struct nvkm_fb *base)
{
	struct gf100_fb *fb = gf100_fb(base);
	struct nvkm_subdev *subdev = &fb->base.subdev;
	struct nvkm_device *device = subdev->device;
	u32 intr = nvkm_rd32(device, 0x000100);
	if (intr & 0x08000000)
		nvkm_debug(subdev, "PFFB intr\n");
	if (intr & 0x00002000)
		nvkm_debug(subdev, "PBFB intr\n");
}

int
gf100_fb_oneinit(struct nvkm_fb *base)
{
	struct gf100_fb *fb = gf100_fb(base);
	struct nvkm_device *device = fb->base.subdev.device;
	int ret, size = 1 << (fb->base.page ? fb->base.page : 17);

	size = nvkm_longopt(device->cfgopt, "MmuDebugBufferSize", size);
	size = max(size, 0x1000);

	ret = nvkm_memory_new(device, NVKM_MEM_TARGET_INST, size, 0x1000,
			      true, &fb->base.mmu_rd);
	if (ret)
		return ret;

	ret = nvkm_memory_new(device, NVKM_MEM_TARGET_INST, size, 0x1000,
			      true, &fb->base.mmu_wr);
	if (ret)
		return ret;

	return 0;
}

int
gf100_fb_init_page(struct nvkm_fb *fb)
{
	struct nvkm_device *device = fb->subdev.device;
	switch (fb->page) {
	case 16: nvkm_mask(device, 0x100c80, 0x00000001, 0x00000001); break;
	case 17: nvkm_mask(device, 0x100c80, 0x00000001, 0x00000000); break;
	default:
		return -EINVAL;
	}
	return 0;
}

void
gf100_fb_sysmem_flush_page_init(struct nvkm_fb *fb)
{
	nvkm_wr32(fb->subdev.device, 0x100c10, fb->sysmem.flush_page_addr >> 8);
}

void
gf100_fb_init(struct nvkm_fb *base)
{
	struct gf100_fb *fb = gf100_fb(base);
	struct nvkm_device *device = fb->base.subdev.device;

	if (base->func->clkgate_pack) {
		nvkm_therm_clkgate_init(device->therm,
					base->func->clkgate_pack);
	}
}

void *
gf100_fb_dtor(struct nvkm_fb *base)
{
	struct gf100_fb *fb = gf100_fb(base);

	return fb;
}

int
gf100_fb_new_(const struct nvkm_fb_func *func, struct nvkm_device *device,
	      enum nvkm_subdev_type type, int inst, struct nvkm_fb **pfb)
{
	struct gf100_fb *fb;

	if (!(fb = kzalloc(sizeof(*fb), GFP_KERNEL)))
		return -ENOMEM;
	nvkm_fb_ctor(func, device, type, inst, &fb->base);
	*pfb = &fb->base;

	return 0;
}

static const struct nvkm_fb_func
gf100_fb = {
	.dtor = gf100_fb_dtor,
	.oneinit = gf100_fb_oneinit,
	.init = gf100_fb_init,
	.init_page = gf100_fb_init_page,
	.intr = gf100_fb_intr,
	.sysmem.flush_page_init = gf100_fb_sysmem_flush_page_init,
	.vidmem.type = gf100_fb_vidmem_type,
	.vidmem.size = gf100_fb_vidmem_size,
	.vidmem.upper = 0x0200000000ULL,
	.vidmem.probe_fbp = gf100_fb_vidmem_probe_fbp,
	.vidmem.probe_fbp_amount = gf100_fb_vidmem_probe_fbp_amount,
	.vidmem.probe_fbpa_amount = gf100_fb_vidmem_probe_fbpa_amount,
	.ram_new = gf100_ram_new,
	.default_bigpage = 17,
};

int
gf100_fb_new(struct nvkm_device *device, enum nvkm_subdev_type type, int inst, struct nvkm_fb **pfb)
{
	return gf100_fb_new_(&gf100_fb, device, type, inst, pfb);
}
