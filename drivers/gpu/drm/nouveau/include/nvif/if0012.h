/* SPDX-License-Identifier: MIT */
#ifndef __NVIF_IF0012_H__
#define __NVIF_IF0012_H__

union nvif_outp_args {
	struct nvif_outp_v0 {
		__u8 version;
		__u8 id;	/* DCB device index. */
#define NVIF_OUTP_V0_DAC  0x00
#define NVIF_OUTP_V0_SOR  0x01
#define NVIF_OUTP_V0_PIOR 0x02
		__u8 type;
#define NVIF_OUTP_V0_RGB_CRT 0x00
#define NVIF_OUTP_V0_TMDS    0x01
#define NVIF_OUTP_V0_LVDS    0x02
#define NVIF_OUTP_V0_DP      0x03
		__u8 proto;
		__u8 heads;
		__u8 ddc;
		__u8 conn;
		__u8 pad07;
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
				u8  link_bw;
				u8  mst;
			} dp;
		};
	} v0;
};

#define NVIF_OUTP_V0_DETECT          0x00
#define NVIF_OUTP_V0_ACQUIRE         0x02
#define NVIF_OUTP_V0_RELEASE         0x03

#define NVIF_OUTP_V0_LOAD_DETECT     0x10

#define NVIF_OUTP_V0_INFOFRAME       0x40
#define NVIF_OUTP_V0_HDA_ELD         0x41
#define NVIF_OUTP_V0_BL_GET          0x42
#define NVIF_OUTP_V0_BL_SET          0x43

#define NVIF_OUTP_V0_DP_AUX_PWR      0x50
#define NVIF_OUTP_V0_DP_RETRAIN      0x52
#define NVIF_OUTP_V0_DP_MST_VCPI     0x56

union nvif_outp_detect_args {
	struct nvif_outp_detect_v0 {
		__u8 version;
#define NVIF_OUTP_DETECT_V0_NOT_PRESENT 0x00
#define NVIF_OUTP_DETECT_V0_PRESENT     0x01
#define NVIF_OUTP_DETECT_V0_UNKNOWN     0x02
		__u8 status;
	} v0;
};

union nvif_outp_load_detect_args {
	struct nvif_outp_load_detect_v0 {
		__u8  version;
		__u8  load;
		__u8  pad02[2];
		__u32 data; /*TODO: move vbios loadval parsing into nvkm */
	} v0;
};

union nvif_outp_acquire_args {
	struct nvif_outp_acquire_v0 {
		__u8 version;
#define NVIF_OUTP_ACQUIRE_V0_RGB_CRT 0x00
#define NVIF_OUTP_ACQUIRE_V0_TV      0x01
#define NVIF_OUTP_ACQUIRE_V0_TMDS    0x02
#define NVIF_OUTP_ACQUIRE_V0_LVDS    0x03
#define NVIF_OUTP_ACQUIRE_V0_DP      0x04
		__u8 proto;
		__u8 or;
		__u8 link;
		__u8 pad04[4];
		union {
			struct {
				__u8 head;
				__u8 hdmi;
				__u8 hdmi_max_ac_packet;
				__u8 hdmi_rekey;
#define NVIF_OUTP_ACQUIRE_V0_TMDS_HDMI_SCDC_SCRAMBLE (1 << 0)
#define NVIF_OUTP_ACQUIRE_V0_TMDS_HDMI_SCDC_DIV_BY_4 (1 << 1)
				__u8 hdmi_scdc;
				__u8 hdmi_hda;
				__u8 pad06[2];
			} tmds;
			struct {
				__u8 dual;
				__u8 bpc8;
				__u8 pad02[6];
			} lvds;
			struct {
				__u8 link_nr; /* 0 = highest possible. */
				__u8 link_bw; /* 0 = highest possible, DP BW code otherwise. */
				__u8 hda;
				__u8 mst;
				__u8 pad04[4];
				__u8 dpcd[16];
			} dp;
		};
	} v0;
};

union nvif_outp_release_args {
	struct nvif_outp_release_vn {
	} vn;
};

union nvif_outp_infoframe_args {
	struct nvif_outp_infoframe_v0 {
		__u8 version;
#define NVIF_OUTP_INFOFRAME_V0_AVI 0
#define NVIF_OUTP_INFOFRAME_V0_VSI 1
		__u8 type;
		__u8 head;
		__u8 pad03[5];
		__u8 data[];
	} v0;
};

union nvif_outp_hda_eld_args {
	struct nvif_outp_hda_eld_v0 {
		__u8  version;
		__u8  head;
		__u8  pad02[6];
		__u8  data[];
	} v0;
};

union nvif_outp_bl_get_args {
	struct nvif_outp_bl_get_v0 {
		__u8  version;
		__u8  level;
	} v0;
};

union nvif_outp_bl_set_args {
	struct nvif_outp_bl_set_v0 {
		__u8  version;
		__u8  level;
	} v0;
};

union nvif_outp_dp_aux_pwr_args {
	struct nvif_outp_dp_aux_pwr_v0 {
		__u8 version;
		__u8 state;
		__u8 pad02[6];
	} v0;
};

union nvif_outp_dp_retrain_args {
	struct nvif_outp_dp_retrain_vn {
	} vn;
};

union nvif_outp_dp_mst_vcpi_args {
	struct nvif_outp_dp_mst_vcpi_v0 {
		__u8  version;
		__u8  head;
		__u8  start_slot;
		__u8  num_slots;
		__u16 pbn;
		__u16 aligned_pbn;
	} v0;
};
#endif
