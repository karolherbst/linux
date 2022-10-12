/* SPDX-License-Identifier: MIT */
#ifndef __NVKM_FB_RAM_PRIV_H__
#define __NVKM_FB_RAM_PRIV_H__
#include "priv.h"

int  nvkm_ram_ctor(const struct nvkm_ram_func *, struct nvkm_fb *, u32 rsvd_head, u32 rsvd_tail,
		   struct nvkm_ram *);
int  nvkm_ram_new_(const struct nvkm_ram_func *, struct nvkm_fb *, u32 rsvd_head, u32 rsvd_tail,
		   struct nvkm_ram **);
void nvkm_ram_del(struct nvkm_ram **);
int  nvkm_ram_init(struct nvkm_ram *);

int  nv50_ram_ctor(const struct nvkm_ram_func *, struct nvkm_fb *,
		   struct nvkm_ram *);

int gf100_ram_new_(const struct nvkm_ram_func *, struct nvkm_fb *,
		   struct nvkm_ram **);
int gf100_ram_init(struct nvkm_ram *);
int gf100_ram_calc(struct nvkm_ram *, u32);
int gf100_ram_prog(struct nvkm_ram *);
void gf100_ram_tidy(struct nvkm_ram *);

int gk104_ram_new_(const struct nvkm_ram_func *, struct nvkm_fb *,
		   struct nvkm_ram **);
void *gk104_ram_dtor(struct nvkm_ram *);
int gk104_ram_init(struct nvkm_ram *);
int gk104_ram_calc(struct nvkm_ram *, u32);
int gk104_ram_prog(struct nvkm_ram *);
void gk104_ram_tidy(struct nvkm_ram *);

/* RAM type-specific MR calculation routines */
int nvkm_sddr2_calc(struct nvkm_ram *);
int nvkm_sddr3_calc(struct nvkm_ram *);
int nvkm_gddr3_calc(struct nvkm_ram *);
int nvkm_gddr5_calc(struct nvkm_ram *, bool nuts);

int nv04_ram_new(struct nvkm_fb *, struct nvkm_ram **);
int nv20_ram_new(struct nvkm_fb *, struct nvkm_ram **);
int nv40_ram_new(struct nvkm_fb *, struct nvkm_ram **);
int nv44_ram_new(struct nvkm_fb *, struct nvkm_ram **);
int nv50_ram_new(struct nvkm_fb *, struct nvkm_ram **);
int gt215_ram_new(struct nvkm_fb *, struct nvkm_ram **);
int mcp77_ram_new(struct nvkm_fb *, struct nvkm_ram **);
int gf100_ram_new(struct nvkm_fb *, struct nvkm_ram **);
int gk104_ram_new(struct nvkm_fb *, struct nvkm_ram **);
int gp100_ram_new(struct nvkm_fb *, struct nvkm_ram **);
#endif
