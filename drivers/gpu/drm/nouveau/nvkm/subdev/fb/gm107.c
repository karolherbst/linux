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

u32
gm107_fb_vidmem_probe_fbp(struct nvkm_fb *fb, int fbp, int *pltcs)
{
	u32 fbpao = nvkm_rd32(fb->subdev.device, 0x021c14);

	return fb->func->vidmem.probe_fbp_amount(fb, fbpao, fbp, pltcs);
}

static const struct nvkm_fb_func
gm107_fb = {
	.dtor = gf100_fb_dtor,
	.oneinit = gf100_fb_oneinit,
	.init = gf100_fb_init,
	.init_page = gf100_fb_init_page,
	.intr = gf100_fb_intr,
	.vidmem.type = gf100_fb_vidmem_type,
	.vidmem.size = gf100_fb_vidmem_size,
	.vidmem.upper = 0x1000000000ULL,
	.vidmem.probe_fbp = gm107_fb_vidmem_probe_fbp,
	.vidmem.probe_fbp_amount = gf108_fb_vidmem_probe_fbp_amount,
	.vidmem.probe_fbpa_amount = gf100_fb_vidmem_probe_fbpa_amount,
	.ram_new = gk104_ram_new,
	.default_bigpage = 17,
};

int
gm107_fb_new(struct nvkm_device *device, enum nvkm_subdev_type type, int inst, struct nvkm_fb **pfb)
{
	return gf100_fb_new_(&gm107_fb, device, type, inst, pfb);
}
