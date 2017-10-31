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
#include "user.h"

#include <core/object.h>
#include <core/memory.h>
#include <subdev/bar.h>

#include <nvif/clb069.h>
#include <nvif/unpack.h>

static int
nvkm_ufault_map(struct nvkm_object *object, void *argv, u32 argc,
		enum nvkm_object_map *type, u64 *addr, u64 *size)
{
	struct nvkm_fault *fault = nvkm_fault(object->engine);
	struct nvkm_device *device = fault->engine.subdev.device;
	*type = NVKM_OBJECT_MAP_IO;
	*addr = device->func->resource_addr(device, 3) + fault->vma->addr;
	*size = nvkm_memory_size(fault->mem);
	return 0;
}

static int
nvkm_ufault_ntfy(struct nvkm_object *object, u32 type,
		 struct nvkm_event **pevent)
{
	struct nvkm_fault *fault = nvkm_fault(object->engine);
	if (type == NVB069_VN_NTFY_FAULT) {
		*pevent = &fault->event;
		return 0;
	}
	return -EINVAL;
}

static int
nvkm_ufault_fini(struct nvkm_object *object, bool suspend)
{
	struct nvkm_fault *fault = nvkm_fault(object->engine);
	fault->func->fini(fault);
	return 0;
}

static int
nvkm_ufault_init(struct nvkm_object *object)
{
	struct nvkm_fault *fault = nvkm_fault(object->engine);
	fault->func->init(fault);
	return 0;
}

static void *
nvkm_ufault_dtor(struct nvkm_object *object)
{
	struct nvkm_fault *fault = nvkm_fault(object->engine);
	struct nvkm_vmm *bar2 = nvkm_bar_bar2_vmm(fault->engine.subdev.device);

	mutex_lock(&fault->engine.subdev.mutex);
	if (fault->user == object)
		fault->user = NULL;
	mutex_unlock(&fault->engine.subdev.mutex);

	nvkm_vmm_put(bar2, &fault->vma);
	nvkm_memory_unref(&fault->mem);
	return object;
}

static const struct nvkm_object_func
nvkm_ufault = {
	.dtor = nvkm_ufault_dtor,
	.init = nvkm_ufault_init,
	.fini = nvkm_ufault_fini,
	.ntfy = nvkm_ufault_ntfy,
	.map = nvkm_ufault_map,
};

int
nvkm_ufault_new(struct nvkm_fault *fault, const struct nvkm_oclass *oclass,
		void *argv, u32 argc, struct nvkm_object **pobject)
{
	union {
		struct nvb069_vn vn;
	} *args = argv;
	struct nvkm_subdev *subdev = &fault->engine.subdev;
	struct nvkm_device *device = subdev->device;
	struct nvkm_vmm *bar2 = nvkm_bar_bar2_vmm(device);
	u32 size = fault->func->size(fault);
	int ret = -ENOSYS;

	if ((ret = nvif_unvers(ret, &argv, &argc, args->vn)))
		return ret;

	ret = nvkm_object_new_(&nvkm_ufault, oclass, NULL, 0, pobject);
	if (ret)
		return ret;

	ret = nvkm_memory_new(device, NVKM_MEM_TARGET_INST, size,
			      0x1000, false, &fault->mem);
	if (ret)
		return ret;

	ret = nvkm_vmm_get(bar2, 12, nvkm_memory_size(fault->mem), &fault->vma);
	if (ret)
		return ret;

	ret = nvkm_memory_map(fault->mem, 0, bar2, fault->vma, NULL, 0);
	if (ret)
		return ret;

	mutex_lock(&subdev->mutex);
	if (!fault->user)
		fault->user = *pobject;
	else
		ret = -EBUSY;
	mutex_unlock(&subdev->mutex);
	return 0;
}
