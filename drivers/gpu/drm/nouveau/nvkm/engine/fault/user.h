#ifndef __NVKM_FAULT_USER_H__
#define __NVKM_FAULT_USER_H__
#include "priv.h"

int nvkm_ufault_new(struct nvkm_fault *, const struct nvkm_oclass *,
		    void *, u32, struct nvkm_object **);
#endif
