/* SPDX-License-Identifier: MIT */
#ifndef __NVKM_FB_PRIV_H__
#define __NVKM_FB_PRIV_H__
#define nvkm_fb(p) container_of((p), struct nvkm_fb, subdev)
#include <subdev/fb.h>
#include <subdev/therm.h>
struct nvkm_bios;

struct nvkm_fb_func {
	void *(*dtor)(struct nvkm_fb *);
	u32 (*tags)(struct nvkm_fb *);
	int (*oneinit)(struct nvkm_fb *);
	void (*init)(struct nvkm_fb *);
	void (*init_remapper)(struct nvkm_fb *);
	int (*init_page)(struct nvkm_fb *);
	void (*init_unkn)(struct nvkm_fb *);
	void (*intr)(struct nvkm_fb *);

	struct nvkm_fb_func_sysmem {
		void (*flush_page_init)(struct nvkm_fb *);
	} sysmem;

	struct nvkm_fb_func_vidmem {
		enum nvkm_ram_type (*type)(struct nvkm_fb *);
		u64 (*size)(struct nvkm_fb *, u64 *lower, u64 *ubase, u64 *usize);
		u32 (*rblock)(struct nvkm_fb *);
		u64 upper;
		u32 (*probe_fbp)(struct nvkm_fb *, int fbp, int *pltcs);
		u32 (*probe_fbp_amount)(struct nvkm_fb *, u32 fbpao, int fbp, int *pltcs);
		u32 (*probe_fbpa_amount)(struct nvkm_device *, int fbpa);
	} vidmem;

	struct {
		bool (*scrub_required)(struct nvkm_fb *);
		int (*scrub)(struct nvkm_fb *);
	} vpr;

	struct {
		int regions;
		void (*init)(struct nvkm_fb *, int i, u32 addr, u32 size,
			     u32 pitch, u32 flags, struct nvkm_fb_tile *);
		void (*comp)(struct nvkm_fb *, int i, u32 size, u32 flags,
			     struct nvkm_fb_tile *);
		void (*fini)(struct nvkm_fb *, int i, struct nvkm_fb_tile *);
		void (*prog)(struct nvkm_fb *, int i, struct nvkm_fb_tile *);
	} tile;

	int (*ram_new)(struct nvkm_fb *, struct nvkm_ram **);

	u8 default_bigpage;
	const struct nvkm_therm_clkgate_pack *clkgate_pack;
};

int nvkm_fb_ctor(const struct nvkm_fb_func *, struct nvkm_device *device,
		 enum nvkm_subdev_type type, int inst, struct nvkm_fb *);
int nvkm_fb_new_(const struct nvkm_fb_func *, struct nvkm_device *device,
		 enum nvkm_subdev_type type, int inst, struct nvkm_fb **);
int nvkm_fb_bios_memtype(struct nvkm_bios *);

u64 nv10_fb_vidmem_size(struct nvkm_fb *, u64 *, u64 *, u64 *);
void nv10_fb_tile_init(struct nvkm_fb *, int i, u32 addr, u32 size,
		       u32 pitch, u32 flags, struct nvkm_fb_tile *);
void nv10_fb_tile_fini(struct nvkm_fb *, int i, struct nvkm_fb_tile *);
void nv10_fb_tile_prog(struct nvkm_fb *, int, struct nvkm_fb_tile *);

enum nvkm_ram_type nv1a_fb_vidmem_type(struct nvkm_fb *);

u32 nv20_fb_tags(struct nvkm_fb *);
void nv20_fb_tile_init(struct nvkm_fb *, int i, u32 addr, u32 size,
		       u32 pitch, u32 flags, struct nvkm_fb_tile *);
void nv20_fb_tile_fini(struct nvkm_fb *, int i, struct nvkm_fb_tile *);
void nv20_fb_tile_prog(struct nvkm_fb *, int, struct nvkm_fb_tile *);

void nv30_fb_init(struct nvkm_fb *);
void nv30_fb_tile_init(struct nvkm_fb *, int i, u32 addr, u32 size,
		       u32 pitch, u32 flags, struct nvkm_fb_tile *);

void nv40_fb_tile_comp(struct nvkm_fb *, int i, u32 size, u32 flags,
		       struct nvkm_fb_tile *);

void nv41_fb_init(struct nvkm_fb *);
enum nvkm_ram_type nv41_fb_vidmem_type(struct nvkm_fb *);
void nv41_fb_tile_prog(struct nvkm_fb *, int, struct nvkm_fb_tile *);

void nv44_fb_init(struct nvkm_fb *);
void nv44_fb_tile_prog(struct nvkm_fb *, int, struct nvkm_fb_tile *);

void nv46_fb_tile_init(struct nvkm_fb *, int i, u32 addr, u32 size,
		       u32 pitch, u32 flags, struct nvkm_fb_tile *);

enum nvkm_ram_type nv50_fb_vidmem_type(struct nvkm_fb *);
u64 nv50_fb_vidmem_size(struct nvkm_fb *, u64 *, u64 *, u64 *);
u32 nv50_fb_vidmem_rblock(struct nvkm_fb *);

int gf100_fb_oneinit(struct nvkm_fb *);
int gf100_fb_init_page(struct nvkm_fb *);
void gf100_fb_sysmem_flush_page_init(struct nvkm_fb *);
enum nvkm_ram_type gf100_fb_vidmem_type(struct nvkm_fb *);
u64 gf100_fb_vidmem_size(struct nvkm_fb *, u64 *, u64 *, u64 *);
u32 gf100_fb_vidmem_probe_fbp(struct nvkm_fb *, int, int *);
u32 gf100_fb_vidmem_probe_fbpa_amount(struct nvkm_device *, int);

u32 gf108_fb_vidmem_probe_fbp_amount(struct nvkm_fb *, u32, int, int *);

u32 gm107_fb_vidmem_probe_fbp(struct nvkm_fb *, int, int *);

u32 gm200_fb_vidmem_probe_fbp_amount(struct nvkm_fb *, u32, int, int *);
int gm200_fb_init_page(struct nvkm_fb *);

u32 gp100_fb_vidmem_probe_fbpa_amount(struct nvkm_device *, int);
void gp100_fb_init_remapper(struct nvkm_fb *);
void gp100_fb_init_unkn(struct nvkm_fb *);

int gp102_fb_new_(const struct nvkm_fb_func *, struct nvkm_device *, enum nvkm_subdev_type, int,
		  struct nvkm_fb **);
bool gp102_fb_vpr_scrub_required(struct nvkm_fb *);
int gp102_fb_vpr_scrub(struct nvkm_fb *);

int gv100_fb_init_page(struct nvkm_fb *);

bool tu102_fb_vpr_scrub_required(struct nvkm_fb *);
#endif
