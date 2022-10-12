/* SPDX-License-Identifier: MIT */
#ifndef __NVIF_OUTP_H__
#define __NVIF_OUTP_H__
#include <nvif/object.h>
#include <nvif/if0012.h>
struct nvif_disp;

struct nvif_outp {
	struct nvif_object object;
	u32 id;

	struct {
		enum {
			NVIF_OUTP_DAC,
			NVIF_OUTP_SOR,
			NVIF_OUTP_PIOR,
		} type;

		enum {
			NVIF_OUTP_RGB_CRT,
			NVIF_OUTP_TV,
			NVIF_OUTP_TMDS,
			NVIF_OUTP_LVDS,
			NVIF_OUTP_DP,
		} proto;

		u8 heads;
#define NVIF_OUTP_DDC_INVALID 0xff
		u8 ddc;
		u8 conn;

		union {
			struct {
				u32 freq_max;
			} rgb_crt;
			struct {
				u8  dual;
			} tmds;
			struct {
				u32 dual;
				u8  bpc8;
			} lvds;
			struct {
				u8  aux;
				u8  link_nr;
				u32 link_bw;
				bool mst;
			} dp;
		};
	} info;

	struct {
		int id;
		int link;
	} or;
};

int nvif_outp_ctor(struct nvif_disp *, const char *name, int id, struct nvif_outp *);
void nvif_outp_dtor(struct nvif_outp *);

enum nvif_outp_detect_status {
	NOT_PRESENT,
	PRESENT,
	UNKNOWN,
};

enum nvif_outp_detect_status nvif_outp_detect(struct nvif_outp *);

int nvif_outp_load_detect(struct nvif_outp *, u32 loadval);
int nvif_outp_acquire_rgb_crt(struct nvif_outp *);
int nvif_outp_acquire_tmds(struct nvif_outp *, int head,
			   bool hdmi, u8 max_ac_packet, u8 rekey, u8 scdc, bool hda);
int nvif_outp_acquire_lvds(struct nvif_outp *, bool dual, bool bpc8);
int nvif_outp_acquire_dp(struct nvif_outp *, u8 dpcd[16],
			 int link_nr, int link_bw, bool hda, bool mst);
void nvif_outp_release(struct nvif_outp *);
int nvif_outp_infoframe(struct nvif_outp *, u8 type, struct nvif_outp_infoframe_v0 *, u32 size);
int nvif_outp_hda_eld(struct nvif_outp *, int head, void *data, u32 size);
int nvif_outp_dp_aux_xfer(struct nvif_outp *, u8 type, u8 *size, u32 addr, u8 *data);
int nvif_outp_dp_aux_pwr(struct nvif_outp *, bool enable);
int nvif_outp_dp_retrain(struct nvif_outp *);
int nvif_outp_dp_mst_id_get(struct nvif_outp *, u32 *id);
int nvif_outp_dp_mst_id_put(struct nvif_outp *, u32 id);
int nvif_outp_dp_mst_vcpi(struct nvif_outp *, int head,
			  u8 start_slot, u8 num_slots, u16 pbn, u16 aligned_pbn);
int nvif_outp_bl_get(struct nvif_outp *);
int nvif_outp_bl_set(struct nvif_outp *, int level);
#endif
