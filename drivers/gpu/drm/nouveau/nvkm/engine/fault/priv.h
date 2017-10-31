#ifndef __NVKM_FAULT_PRIV_H__
#define __NVKM_FAULT_PRIV_H__
#define nvkm_fault(p) container_of((p), struct nvkm_fault, engine)
#include <engine/fault.h>

#include <core/event.h>
#include <subdev/mmu.h>

struct nvkm_fault {
	const struct nvkm_fault_func *func;
	struct nvkm_engine engine;

	struct nvkm_event event;

	struct nvkm_object *user;
	struct nvkm_memory *mem;
	struct nvkm_vma *vma;
};

struct nvkm_fault_func {
	u32 (*size)(struct nvkm_fault *);
	void (*init)(struct nvkm_fault *);
	void (*fini)(struct nvkm_fault *);
	s32 oclass;
};

int nvkm_fault_new_(const struct nvkm_fault_func *, struct nvkm_device *,
		    int, struct nvkm_engine **);
#endif
