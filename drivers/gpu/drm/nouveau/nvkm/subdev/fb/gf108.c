/*
 * Copyright 2017 Red Hat Inc.
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
 * Authors: Ben Skeggs <bskeggs@redhat.com>
 */
#include "gf100.h"
#include "ram.h"

u32
gf108_fb_vidmem_probe_fbp_amount(struct nvkm_fb *fb, u32 fbpao, int fbp, int *pltcs)
{
	struct nvkm_device *device = fb->subdev.device;
	u32 fbpt  = nvkm_rd32(device, 0x022438);
	u32 fbpat = nvkm_rd32(device, 0x02243c);
	u32 fbpas = fbpat / fbpt;
	u32 fbpa  = fbp * fbpas;
	u32 size  = 0;

	while (fbpas--) {
		if (!(fbpao & BIT(fbpa)))
			size += fb->func->vidmem.probe_fbpa_amount(device, fbpa);
		fbpa++;
	}

	*pltcs = 1;
	return size;
}

static const struct nvkm_fb_func
gf108_fb = {
	.dtor = gf100_fb_dtor,
	.oneinit = gf100_fb_oneinit,
	.init = gf100_fb_init,
	.init_page = gf100_fb_init_page,
	.intr = gf100_fb_intr,
	.vidmem.type = gf100_fb_vidmem_type,
	.vidmem.size = gf100_fb_vidmem_size,
	.vidmem.upper = 0x0200000000ULL,
	.vidmem.probe_fbp = gf100_fb_vidmem_probe_fbp,
	.vidmem.probe_fbp_amount = gf108_fb_vidmem_probe_fbp_amount,
	.vidmem.probe_fbpa_amount = gf100_fb_vidmem_probe_fbpa_amount,
	.ram_new = gf100_ram_new,
	.default_bigpage = 17,
};

int
gf108_fb_new(struct nvkm_device *device, enum nvkm_subdev_type type, int inst, struct nvkm_fb **pfb)
{
	return gf100_fb_new_(&gf108_fb, device, type, inst, pfb);
}
