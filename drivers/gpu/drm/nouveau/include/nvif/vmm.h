#ifndef __NVIF_VMM_H__
#define __NVIF_VMM_H__
#include <nvif/object.h>
struct nvif_mem;
struct nvif_mmu;

enum nvif_vmm_get {
	ADDR,
	PTES,
	LAZY
};

struct nvif_vma {
	u64 addr;
	u64 size;
};

struct nvif_vmm {
	struct nvif_object object;
	u64 start;
	u64 limit;

	struct {
		u8 shift;
		bool sparse:1;
		bool vram:1;
		bool host:1;
		bool comp:1;
	} *page;
	int page_nr;
};

int nvif_vmm_init(struct nvif_mmu *, s32 oclass, u64 addr, u64 size,
		  void *argv, u32 argc, struct nvif_vmm *);
void nvif_vmm_fini(struct nvif_vmm *);
int nvif_vmm_get(struct nvif_vmm *, enum nvif_vmm_get, bool sparse,
		 u8 page, u8 align, u64 size, struct nvif_vma *);
void nvif_vmm_put(struct nvif_vmm *, struct nvif_vma *);
int nvif_vmm_map(struct nvif_vmm *, u64 addr, u64 size, void *argv, u32 argc,
		 struct nvif_mem *, u64 offset);
int nvif_vmm_unmap(struct nvif_vmm *, u64);
int nvif_vmm_hmm_init(struct nvif_vmm *vmm, u64 hstart, u64 hend);
void nvif_vmm_hmm_fini(struct nvif_vmm *vmm, u64 hstart, u64 hend);
int nvif_vmm_hmm_map(struct nvif_vmm *vmm, u64 addr, u64 npages, u64 *pages);
int nvif_vmm_hmm_unmap(struct nvif_vmm *vmm, u64 addr, u64 npages);
#endif
