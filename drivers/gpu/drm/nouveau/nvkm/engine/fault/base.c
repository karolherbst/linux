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
#include "user.h"

#include <core/client.h>
#include <core/notify.h>

static int
nvkm_fault_ntfy_ctor(struct nvkm_object *object, void *data, u32 size,
		     struct nvkm_notify *notify)
{
	if (size == 0) {
		notify->size  = 0;
		notify->types = 1;
		notify->index = 0;
		return 0;
	}
	return -ENOSYS;
}

static const struct nvkm_event_func
nvkm_fault_ntfy = {
	.ctor = nvkm_fault_ntfy_ctor,
};

static int
nvkm_fault_class_new(struct nvkm_device *device,
		     const struct nvkm_oclass *oclass, void *data, u32 size,
		     struct nvkm_object **pobject)
{
	struct nvkm_fault *fault = nvkm_fault(device->fault);
	if (!oclass->client->super)
		return -EACCES;
	return nvkm_ufault_new(fault, oclass, data, size, pobject);
}

static const struct nvkm_device_oclass
nvkm_fault_class = {
	.ctor = nvkm_fault_class_new,
};

static int
nvkm_fault_class_get(struct nvkm_oclass *oclass, int index,
		     const struct nvkm_device_oclass **class)
{
	struct nvkm_fault *fault = nvkm_fault(oclass->engine);
	if (index == 0) {
		oclass->base.oclass = fault->func->oclass;
		oclass->base.minver = -1;
		oclass->base.maxver = -1;
		*class = &nvkm_fault_class;
	}
	return 1;
}

static void
nvkm_fault_intr(struct nvkm_engine *engine)
{
	struct nvkm_fault *fault = nvkm_fault(engine);
	nvkm_event_send(&fault->event, 1, 0, NULL, 0);
}

static void *
nvkm_fault_dtor(struct nvkm_engine *engine)
{
	struct nvkm_fault *fault = nvkm_fault(engine);
	nvkm_event_fini(&fault->event);
	return fault;
}

static const struct nvkm_engine_func
nvkm_fault = {
	.dtor = nvkm_fault_dtor,
	.intr = nvkm_fault_intr,
	.base.sclass = nvkm_fault_class_get,
};

int
nvkm_fault_new_(const struct nvkm_fault_func *func, struct nvkm_device *device,
		int index, struct nvkm_engine **pengine)
{
	struct nvkm_fault *fault;
	int ret;

	if (!(fault = kzalloc(sizeof(*fault), GFP_KERNEL)))
		return -ENOMEM;
	*pengine = &fault->engine;
	fault->func = func;

	ret = nvkm_engine_ctor(&nvkm_fault, device, index, true,
			       &fault->engine);
	if (ret)
		return ret;

	return nvkm_event_init(&nvkm_fault_ntfy, 1, 1, &fault->event);
}
