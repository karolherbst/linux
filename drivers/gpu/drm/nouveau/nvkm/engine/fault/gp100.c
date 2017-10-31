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
 */
#include "priv.h"

#include <nvif/class.h>

static void
gp100_fault_fini(struct nvkm_fault *fault)
{
	struct nvkm_device *device = fault->engine.subdev.device;
	nvkm_mask(device, 0x002a70, 0x00000001, 0x00000000);
}

static void
gp100_fault_init(struct nvkm_fault *fault)
{
	struct nvkm_device *device = fault->engine.subdev.device;
	nvkm_wr32(device, 0x002a74, upper_32_bits(fault->vma->addr));
	nvkm_wr32(device, 0x002a70, lower_32_bits(fault->vma->addr));
	nvkm_mask(device, 0x002a70, 0x00000001, 0x00000001);
}

static u32
gp100_fault_size(struct nvkm_fault *fault)
{
	return nvkm_rd32(fault->engine.subdev.device, 0x002a78) * 32;
}

static const struct nvkm_fault_func
gp100_fault = {
	.size = gp100_fault_size,
	.init = gp100_fault_init,
	.fini = gp100_fault_fini,
	.oclass = MAXWELL_FAULT_BUFFER_A,
};

int
gp100_fault_new(struct nvkm_device *device, int index,
		struct nvkm_engine **pengine)
{
	return nvkm_fault_new_(&gp100_fault, device, index, pengine);
}
