/*! \file gsmtapv3.h
 * GSMTAP header, pseudo-header in front of the non-IP cellular payload.
 * GSMTAP is a generic header format for cellular protocol captures.
 * It could be carried over various transport layer protocols, including
 * UDP with the IANA-assigned UDP port number 4729. It carries
 * payload in various formats of cellular interfaces.
 *
 * Example programs generating GSMTAP data are airprobe
 * (http://airprobe.org/) or OsmocomBB (http://bb.osmocom.org/)
 */

#pragma once

#include <stdint.h>

/* ====== DO NOT MAKE UNAPPROVED MODIFICATIONS HERE ===== */

/* The GSMTAPv3 format definition is maintained in libosmocore,
 * specifically the latest version can always be obtained from
 * - TBD -
 *
 * If you want to introduce new protocol/burst/channel types or extend
 * GSMTAP in any way, please contact the GSMTAP maintainer at either the
 * public openbsc@lists.osmocom.org mailing list, or privately at
 * Harald Welte <laforge@gnumonks.org>.
 *
 * Your cooperation ensures that all projects will use the same GSMTAP
 * definitions and remain compatible with each other.
 */

#define GSMTAPV3_VERSION		0x03

/* 0x00, 0x01: Common and non-3GPP protocols */
#define GSMTAPV3_TYPE_OSMOCORE_LOG	0x0000	/* libosmocore logging */
#define GSMTAPV3_TYPE_SIM			0x0001	/* ISO 7816 smartcard interface */
#define GSMTAPV3_TYPE_BASEBAND_DIAG	0x0002	/* Baseband diagnostic data */
#define GSMTAPV3_TYPE_SIGNAL_STATUS_REPORT	0x0003	/* Radio signal status report, exact data TBD */
#define GSMTAPV3_TYPE_TETRA_I1		0x0004	/* TETRA air interface */
#define GSMTAPV3_TYPE_TETRA_I1_BURST	0x0005	/* TETRA air interface */
#define GSMTAPV3_TYPE_GMR1_UM		0x0006	/* GMR-1 L2 packets */
#define GSMTAPV3_TYPE_E1T1			0x0007	/* E1/T1 Lines */
#define GSMTAPV3_TYPE_WMX_BURST		0x0008	/* WiMAX burst, shall we deprecate? */
#define GSMTAPV3_TYPE_DECT		0x0009  /* DECT frames */
#define GSMTAPV3_TYPE_QCOM_MSM		0x0010  /* Qualcomm Modem related protocol: QMI */

/* 0x02: GSM */
#define GSMTAPV3_TYPE_UM			0x0200
#define GSMTAPV3_TYPE_UM_BURST		0x0201	/* raw burst bits */
#define GSMTAPV3_TYPE_GB_RLCMAC		0x0202	/* GPRS Gb interface: RLC/MAC */
#define GSMTAPV3_TYPE_GB_LLC		0x0203	/* GPRS Gb interface: LLC */
#define GSMTAPV3_TYPE_GB_SNDCP		0x0204	/* GPRS Gb interface: SNDCP */
#define GSMTAPV3_TYPE_ABIS			0x0205
#define GSMTAPV3_TYPE_RLP			0x0206	/* GSM RLP frames, as per 3GPP TS 24.022 */

/* 0x03: UMTS/WCDMA */
#define GSMTAPV3_TYPE_UMTS_MAC		0x0300	/* UMTS MAC PDU with context, as per 3GPP TS 25.321 */
#define GSMTAPV3_TYPE_UMTS_RLC		0x0301	/* UMTS RLC PDU with context, as per 3GPP TS 25.322 */
#define GSMTAPV3_TYPE_UMTS_PDCP		0x0302	/* UMTS PDCP PDU with context, as per 3GPP TS 25.323 */
#define GSMTAPV3_TYPE_UMTS_RRC		0x0303	/* UMTS RRC PDU, as per 3GPP TS 25.331 */

/* 0x04: LTE */
#define GSMTAPV3_TYPE_LTE_MAC		0x0400	/* LTE MAC PDU with context, as per 3GPP TS 36.321 */ 
#define GSMTAPV3_TYPE_LTE_RLC		0x0401	/* LTE RLC PDU with context, as per 3GPP TS 36.322 */ 
#define GSMTAPV3_TYPE_LTE_PDCP		0x0402	/* LTE PDCP PDU with context, as per 3GPP TS 36.323 */ 
#define GSMTAPV3_TYPE_LTE_RRC		0x0403	/* LTE RRC PDU, as per 3GPP TS 36.331 */
#define GSMTAPV3_TYPE_NAS_EPS		0x0404	/* EPS Non-Access Stratum, as per 3GPP TS 24.301 */

/* 0x05: NR */
#define GSMTAPV3_TYPE_NR_MAC		0x0500	/* NR MAC PDU with context, as per 3GPP TS 38.321 */ 
#define GSMTAPV3_TYPE_NR_RLC		0x0501	/* NR RLC PDU with context, as per 3GPP TS 38.322 */ 
#define GSMTAPV3_TYPE_NR_PDCP		0x0502	/* NR PDCP PDU with context, as per 3GPP TS 38.323 */ 
#define GSMTAPV3_TYPE_NR_RRC		0x0503	/* NR RRC PDU, as per 3GPP TS 38.331 */
#define GSMTAPV3_TYPE_NAS_5GS		0x0504	/* 5GS Non-Access Stratum, as per 3GPP TS 24.501 */

/* ====== DO NOT MAKE UNAPPROVED MODIFICATIONS HERE ===== */

/* sub-types for TYPE_SIM (0x0001) */
#define GSMTAPV3_SIM_APDU			0x0001 /* APDU data (complete APDU) */
#define GSMTAPV3_SIM_ATR			0x0002 /* card ATR data */
#define GSMTAPV3_SIM_PPS_REQ		0x0003 /* PPS request data */
#define GSMTAPV3_SIM_PPS_RSP		0x0004 /* PPS response data */
#define GSMTAPV3_SIM_TPDU_HDR		0x0005 /* TPDU command header */
#define GSMTAPV3_SIM_TPDU_CMD		0x0006 /* TPDU command body */
#define GSMTAPV3_SIM_TPDU_RSP		0x0007 /* TPDU response body */
#define GSMTAPV3_SIM_TPDU_SW		0x0008 /* TPDU response trailer */

/* ====== DO NOT MAKE UNAPPROVED MODIFICATIONS HERE ===== */

/* sub-types for TYPE_BASEBAND_DIAG (0x0002) */
#define GSMTAPV3_BASEBAND_DIAG_QUALCOMM		0x0001 /* Qualcomm DIAG */
#define GSMTAPV3_BASEBAND_DIAG_SAMSUNG		0x0002 /* Samsung SDM */
#define GSMTAPV3_BASEBAND_DIAG_MEDIATEK		0x0003
#define GSMTAPV3_BASEBAND_DIAG_UNISOC		0x0004
#define GSMTAPV3_BASEBAND_DIAG_HISILICON	0x0005
#define GSMTAPV3_BASEBAND_DIAG_INTEL		0x0006
#define GSMTAPV3_BASEBAND_DIAG_QMI		0x0012 /* Qualcomm MSM Interface */

/* ====== DO NOT MAKE UNAPPROVED MODIFICATIONS HERE ===== */

/* sub-types for TYPE_TETRA_AIR (0x0004, 0x0005) */
#define GSMTAPV3_TETRA_BSCH			0x0001
#define GSMTAPV3_TETRA_AACH			0x0002
#define GSMTAPV3_TETRA_SCH_HU		0x0003
#define GSMTAPV3_TETRA_SCH_HD		0x0004
#define GSMTAPV3_TETRA_SCH_F		0x0005
#define GSMTAPV3_TETRA_BNCH			0x0006
#define GSMTAPV3_TETRA_STCH			0x0007
#define GSMTAPV3_TETRA_TCH_F		0x0008
#define GSMTAPV3_TETRA_DMO_SCH_S	0x0009
#define GSMTAPV3_TETRA_DMO_SCH_H	0x000a
#define GSMTAPV3_TETRA_DMO_SCH_F	0x000b
#define GSMTAPV3_TETRA_DMO_STCH		0x000c
#define GSMTAPV3_TETRA_DMO_TCH		0x000d

/* ====== DO NOT MAKE UNAPPROVED MODIFICATIONS HERE ===== */

/* sub-types for TYPE_GMR1_UM (0x0006) */
#define GSMTAPV3_GMR1_BCCH		0x0001
#define GSMTAPV3_GMR1_CCCH		0x0002	/* either AGCH or PCH */
#define GSMTAPV3_GMR1_PCH		0x0003
#define GSMTAPV3_GMR1_AGCH		0x0004
#define GSMTAPV3_GMR1_BACH		0x0005
#define GSMTAPV3_GMR1_RACH		0x0006
#define GSMTAPV3_GMR1_CBCH		0x0007
#define GSMTAPV3_GMR1_SDCCH		0x0008
#define GSMTAPV3_GMR1_TACCH		0x0009
#define GSMTAPV3_GMR1_GBCH		0x000a

#define GSMTAPV3_GMR1_SACCH		0x0001	/* to be combined with _TCH{6,9}   */
#define GSMTAPV3_GMR1_FACCH		0x0002	/* to be combines with _TCH{3,6,9} */
#define GSMTAPV3_GMR1_DKAB		0x0003	/* to be combined with _TCH3 */
#define GSMTAPV3_GMR1_TCH3		0x0100
#define GSMTAPV3_GMR1_TCH6		0x0200
#define GSMTAPV3_GMR1_TCH9		0x0300

/* ====== DO NOT MAKE UNAPPROVED MODIFICATIONS HERE ===== */

/* sub-types for TYPE_E1T1 (0x0007) */
#define GSMTAPV3_E1T1_LAPD		0x0001	/* Q.921 LAPD */
#define GSMTAPV3_E1T1_FR		0x0002	/* Frame Relay */
#define GSMTAPV3_E1T1_RAW		0x0003	/* raw/transparent B-channel */
#define GSMTAPV3_E1T1_TRAU16	0x0004	/* 16k TRAU frames; sub-slot 0-3 */
#define GSMTAPV3_E1T1_TRAU8		0x0005	/* 8k TRAU frames; sub-slot 0-7 */

/* ====== DO NOT MAKE UNAPPROVED MODIFICATIONS HERE ===== */

/* sub-types for TYPE_WMX_BURST (0x0008) */
#define GSMTAPV3_WMX_BURST_CDMA_CODE	0x0001	/* WiMAX CDMA Code Attribute burst */
#define GSMTAPV3_WMX_BURST_FCH			0x0002	/* WiMAX FCH burst */
#define GSMTAPV3_WMX_BURST_FFB			0x0003	/* WiMAX Fast Feedback burst */
#define GSMTAPV3_WMX_BURST_PDU			0x0004	/* WiMAX PDU burst */
#define GSMTAPV3_WMX_BURST_HACK			0x0005	/* WiMAX HARQ ACK burst */
#define GSMTAPV3_WMX_BURST_PHY_ATTRIBUTES	0x0006	/* WiMAX PHY Attributes burst */

/* ====== DO NOT MAKE UNAPPROVED MODIFICATIONS HERE ===== */

/* sub-types for TYPE_UM (0x0200) */
#define GSMTAPV3_UM_CHANNEL_UNKNOWN	0x0000
#define GSMTAPV3_UM_CHANNEL_BCCH		0x0001
#define GSMTAPV3_UM_CHANNEL_CCCH		0x0002
#define GSMTAPV3_UM_CHANNEL_RACH		0x0003
#define GSMTAPV3_UM_CHANNEL_AGCH		0x0004
#define GSMTAPV3_UM_CHANNEL_PCH			0x0005
#define GSMTAPV3_UM_CHANNEL_SDCCH		0x0006
#define GSMTAPV3_UM_CHANNEL_SDCCH4		0x0007
#define GSMTAPV3_UM_CHANNEL_SDCCH8		0x0008
#define GSMTAPV3_UM_CHANNEL_FACCH_F		0x0009
#define GSMTAPV3_UM_CHANNEL_FACCH_H		0x000a
#define GSMTAPV3_UM_CHANNEL_PACCH		0x000b
#define GSMTAPV3_UM_CHANNEL_CBCH52		0x000c
#define GSMTAPV3_UM_CHANNEL_PDCH		0x000d
#define GSMTAPV3_UM_CHANNEL_PTCCH		0x000e
#define GSMTAPV3_UM_CHANNEL_CBCH51		0x000f
#define GSMTAPV3_UM_CHANNEL_VOICE_F		0x0010	/* voice codec payload (FR/EFR/AMR) */
#define GSMTAPV3_UM_CHANNEL_VOICE_H		0x0011	/* voice codec payload (HR/AMR) */

#define GSMTAPV3_UM_CHANNEL_ACCH		0x0100

/* GPRS Coding Scheme CS1..4 */
#define GSMTAPV3_UM_GPRS_CS_BASE		0x0200
#define GSMTAPV3_UM_GPRS_CS(N)	(GSMTAP_GPRS_CS_BASE + N)
/* (E) GPRS Coding Scheme MCS0..9 */
#define GSMTAPV3_UM_GPRS_MCS_BASE		0x0300
#define GSMTAPV3_UM_GPRS_MCS(N)	(GSMTAP_GPRS_MCS_BASE + N)

/* ====== DO NOT MAKE UNAPPROVED MODIFICATIONS HERE ===== */

/* sub-types for TYPE_UM_BURST (0x0201) */
#define GSMTAPV3_UM_BURST_FCCH			0x0001
#define GSMTAPV3_UM_BURST_PARTIAL_SCH	0x0002
#define GSMTAPV3_UM_BURST_SCH			0x0003
#define GSMTAPV3_UM_BURST_CTS_SCH		0x0004
#define GSMTAPV3_UM_BURST_COMPACT_SCH	0x0005
#define GSMTAPV3_UM_BURST_NORMAL		0x0006
#define GSMTAPV3_UM_BURST_DUMMY			0x0007
#define GSMTAPV3_UM_BURST_ACCESS		0x0008
#define GSMTAPV3_UM_BURST_NONE			0x0009

/* ====== DO NOT MAKE UNAPPROVED MODIFICATIONS HERE ===== */

/* sub-types for TYPE_UMTS_RRC (0x0303) */
#define GSMTAPV3_UMTS_RRC_DL_DCCH			0x0001
#define GSMTAPV3_UMTS_RRC_UL_DCCH			0x0002
#define GSMTAPV3_UMTS_RRC_DL_CCCH			0x0003
#define GSMTAPV3_UMTS_RRC_UL_CCCH			0x0004
#define GSMTAPV3_UMTS_RRC_PCCH				0x0005
#define GSMTAPV3_UMTS_RRC_DL_SHCCH			0x0006
#define GSMTAPV3_UMTS_RRC_UL_SHCCH			0x0007
#define GSMTAPV3_UMTS_RRC_BCCH_FACH			0x0008
#define GSMTAPV3_UMTS_RRC_BCCH_BCH			0x0009
#define GSMTAPV3_UMTS_RRC_BCCH_BCH2			0x000a
#define GSMTAPV3_UMTS_RRC_MCCH				0x000b
#define GSMTAPV3_UMTS_RRC_MSCH				0x000c

/* sub-types for individual UMTS RRC message */
#define GSMTAPV3_UMTS_RRC_HandoverToUTRANCommand		0x0101
#define GSMTAPV3_UMTS_RRC_InterRATHandoverInfo			0x0102
#define GSMTAPV3_UMTS_RRC_SystemInformation_BCH			0x0103
#define GSMTAPV3_UMTS_RRC_System_Information_Container	0x0104
#define GSMTAPV3_UMTS_RRC_UE_RadioAccessCapabilityInfo	0x0105
#define GSMTAPV3_UMTS_RRC_MasterInformationBlock		0x0106
#define GSMTAPV3_UMTS_RRC_SysInfoType1					0x0107
#define GSMTAPV3_UMTS_RRC_SysInfoType2					0x0108
#define GSMTAPV3_UMTS_RRC_SysInfoType3					0x0109
#define GSMTAPV3_UMTS_RRC_SysInfoType4					0x010a
#define GSMTAPV3_UMTS_RRC_SysInfoType5					0x010b
#define GSMTAPV3_UMTS_RRC_SysInfoType5bis				0x010c
#define GSMTAPV3_UMTS_RRC_SysInfoType6					0x010d
#define GSMTAPV3_UMTS_RRC_SysInfoType7					0x010e
#define GSMTAPV3_UMTS_RRC_SysInfoType8					0x010f
#define GSMTAPV3_UMTS_RRC_SysInfoType9					0x0110
#define GSMTAPV3_UMTS_RRC_SysInfoType10					0x0111
#define GSMTAPV3_UMTS_RRC_SysInfoType11					0x0112
#define GSMTAPV3_UMTS_RRC_SysInfoType11bis				0x0113
#define GSMTAPV3_UMTS_RRC_SysInfoType12					0x0114
#define GSMTAPV3_UMTS_RRC_SysInfoType13					0x0115
#define GSMTAPV3_UMTS_RRC_SysInfoType13_1				0x0116
#define GSMTAPV3_UMTS_RRC_SysInfoType13_2				0x0117
#define GSMTAPV3_UMTS_RRC_SysInfoType13_3				0x0118
#define GSMTAPV3_UMTS_RRC_SysInfoType13_4				0x0119
#define GSMTAPV3_UMTS_RRC_SysInfoType14					0x011a
#define GSMTAPV3_UMTS_RRC_SysInfoType15					0x011b
#define GSMTAPV3_UMTS_RRC_SysInfoType15bis				0x011c
#define GSMTAPV3_UMTS_RRC_SysInfoType15_1				0x011d
#define GSMTAPV3_UMTS_RRC_SysInfoType15_1bis			0x011e
#define GSMTAPV3_UMTS_RRC_SysInfoType15_2				0x011f
#define GSMTAPV3_UMTS_RRC_SysInfoType15_2bis			0x0120
#define GSMTAPV3_UMTS_RRC_SysInfoType15_2ter			0x0121
#define GSMTAPV3_UMTS_RRC_SysInfoType15_3				0x0122
#define GSMTAPV3_UMTS_RRC_SysInfoType15_3bis			0x0123
#define GSMTAPV3_UMTS_RRC_SysInfoType15_4				0x0124
#define GSMTAPV3_UMTS_RRC_SysInfoType15_5				0x0125
#define GSMTAPV3_UMTS_RRC_SysInfoType15_6				0x0126
#define GSMTAPV3_UMTS_RRC_SysInfoType15_7				0x0127
#define GSMTAPV3_UMTS_RRC_SysInfoType15_8				0x0128
#define GSMTAPV3_UMTS_RRC_SysInfoType16					0x0129
#define GSMTAPV3_UMTS_RRC_SysInfoType17					0x012a
#define GSMTAPV3_UMTS_RRC_SysInfoType18					0x012b
#define GSMTAPV3_UMTS_RRC_SysInfoType19					0x012c
#define GSMTAPV3_UMTS_RRC_SysInfoType20					0x012d
#define GSMTAPV3_UMTS_RRC_SysInfoType21					0x012e
#define GSMTAPV3_UMTS_RRC_SysInfoType22					0x012f
#define GSMTAPV3_UMTS_RRC_SysInfoTypeSB1				0x0130
#define GSMTAPV3_UMTS_RRC_SysInfoTypeSB2				0x0131
#define GSMTAPV3_UMTS_RRC_ToTargetRNC_Container			0x0132
#define GSMTAPV3_UMTS_RRC_TargetRNC_ToSourceRNC_Container	0x0133

/* ====== DO NOT MAKE UNAPPROVED MODIFICATIONS HERE ===== */

/* sub-types for TYPE_LTE_RRC (0x0403) */
#define GSMTAPV3_LTE_RRC_BCCH_BCH			0x0001
#define GSMTAPV3_LTE_RRC_BCCH_BCH_MBMS		0x0002
#define GSMTAPV3_LTE_RRC_BCCH_DL_SCH		0x0003
#define GSMTAPV3_LTE_RRC_BCCH_DL_SCH_BR		0x0004
#define GSMTAPV3_LTE_RRC_BCCH_DL_SCH_MBMS	0x0005
#define GSMTAPV3_LTE_RRC_MCCH				0x0006
#define GSMTAPV3_LTE_RRC_PCCH				0x0007
#define GSMTAPV3_LTE_RRC_DL_CCCH			0x0008
#define GSMTAPV3_LTE_RRC_DL_DCCH			0x0009
#define GSMTAPV3_LTE_RRC_UL_CCCH			0x000a
#define GSMTAPV3_LTE_RRC_UL_DCCH			0x000b
#define GSMTAPV3_LTE_RRC_SC_MCCH			0x000c

#define GSMTAPV3_LTE_RRC_SBCCH_SL_BCH		0x0101
#define GSMTAPV3_LTE_RRC_SBCCH_SL_BCH_V2X	0x0102

#define GSMTAPV3_LTE_RRC_BCCH_BCH_NB		0x0200
#define GSMTAPV3_LTE_RRC_BCCH_BCH_TDD_NB	0x0201
#define GSMTAPV3_LTE_RRC_BCCH_DL_SCH_NB		0x0202
#define GSMTAPV3_LTE_RRC_PCCH_NB			0x0203
#define GSMTAPV3_LTE_RRC_DL_CCCH_NB			0x0204
#define GSMTAPV3_LTE_RRC_DL_DCCH_NB			0x0205
#define GSMTAPV3_LTE_RRC_UL_CCCH_NB			0x0205
#define GSMTAPV3_LTE_RRC_SC_MCCH_NB			0x0206
#define GSMTAPV3_LTE_RRC_UL_DCCH_NB			0x0207

/* ====== DO NOT MAKE UNAPPROVED MODIFICATIONS HERE ===== */

/* sub-types for TYPE_NR_RRC (0x0503) */
#define GSMTAPV3_NR_RRC_BCCH_BCH		0x0001
#define GSMTAPV3_NR_RRC_BCCH_DL_SCH		0x0002
#define GSMTAPV3_NR_RRC_DL_CCCH			0x0003
#define GSMTAPV3_NR_RRC_DL_DCCH			0x0004
#define GSMTAPV3_NR_RRC_MCCH			0x0005
#define GSMTAPV3_NR_RRC_PCCH			0x0006
#define GSMTAPV3_NR_RRC_UL_CCCH			0x0007
#define GSMTAPV3_NR_RRC_UL_CCCH1		0x0008
#define GSMTAPV3_NR_RRC_UL_DCCH			0x0009

#define GSMTAPV3_NR_RRC_SBCCH_SL_BCH	0x0101
#define GSMTAPV3_NR_RRC_SCCH			0x0102

/* ====== DO NOT MAKE UNAPPROVED MODIFICATIONS HERE ===== */

/* sub-types for TYPE_LTE_NAS and TYPE_NR_NAS (0x0404, 0x0504) */
#define GSMTAPV3_NAS_EPS_PLAIN		0x0000
#define GSMTAPV3_NAS_EPS_SEC_HEADER	0x0001
#define GSMTAPV3_NAS_5GS_PLAIN		GSMTAPV3_NAS_EPS_PLAIN
#define GSMTAPV3_NAS_5GS_SEC_HEADER	GSMTAPV3_NAS_EPS_SEC_HEADER

/* ====== DO NOT MAKE UNAPPROVED MODIFICATIONS HERE ===== */

/* IANA-assigned well-known UDP port for GSMTAP messages */
#define GSMTAPV3_UDP_PORT			4729

/* ====== DO NOT MAKE UNAPPROVED MODIFICATIONS HERE ===== */

/*! Structure of the GSMTAP pseudo-header */
struct gsmtap_hdr_v3 {
	uint8_t version;	/*!< version, set to 0x03 */
	uint8_t res;		/*!< reserved for future use (RFU). Padding. */
	uint16_t hdr_len;	/*!< length (including metadata) in number of 32bit words */

	uint16_t type;		/*!< see GSMTAPV3_TYPE */
	uint16_t sub_type;	/*!< type of burst/channel, see above */

	uint8_t metadata[0];	/*!< type-specific metadata structure */
} __attribute__((packed));
