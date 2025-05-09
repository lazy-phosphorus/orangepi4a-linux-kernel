/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner SoCs hdmi2.0 driver.
 *
 * Copyright (C) 2016 Allwinner.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#ifndef _DW_FRAME_COMPOSER_H
#define _DW_FRAME_COMPOSER_H

#include "dw_dev.h"

typedef struct fc_spd_info {
	const u8 *vName;
	u8 vLength;
	const u8 *pName;
	u8 pLength;
	u8 code;
	u8 autoSend;
} fc_spd_info_t;

typedef struct channel_count {
	unsigned char channel_allocation;
	unsigned char channel_count;
} channel_count_t;

typedef union iec {
	u32 frequency;
	u8 sample_size;
} iec_t;

typedef struct iec_sampling_freq {
	iec_t iec;
	u8 value;
} iec_params_t;

/*****************************************************************************
 *                                                                           *
 *                          Frame Composer Registers                         *
 *                                                                           *
 *****************************************************************************/

/* Frame Composer Input Video Configuration and HDCP Keepout Register */
#define FC_INVIDCONF  0x00004000
#define FC_INVIDCONF_IN_I_P_MASK  0x00000001 /* Input video mode: 1b: Interlaced 0b: Progressive */
#define FC_INVIDCONF_R_V_BLANK_IN_OSC_MASK  0x00000002 /* Used for CEA861-D modes with fractional Vblank (for example, modes 5, 6, 7, 10, 11, 20, 21, and 22) */
#define FC_INVIDCONF_DVI_MODEZ_MASK  0x00000008 /* Active low 0b: DVI mode selected 1b: HDMI mode selected */
#define FC_INVIDCONF_DE_IN_POLARITY_MASK  0x00000010 /* Data enable input polarity 1b: Active high 0b: Active low */
#define FC_INVIDCONF_HSYNC_IN_POLARITY_MASK  0x00000020 /* Hsync input polarity 1b: Active high 0b: Active low */
#define FC_INVIDCONF_VSYNC_IN_POLARITY_MASK  0x00000040 /* Vsync input polarity 1b: Active high 0b: Active low */
#define FC_INVIDCONF_HDCP_KEEPOUT_MASK  0x00000080 /* Start/stop HDCP keepout window generation 1b: Active */

/* Frame Composer Input Video HActive Pixels Register 0 */
#define FC_INHACTIV0  0x00004004
#define FC_INHACTIV0_H_IN_ACTIV_MASK  0x000000FF /* Input video Horizontal active pixel region width */

/* Frame Composer Input Video HActive Pixels Register 1 */
#define FC_INHACTIV1  0x00004008
#define FC_INHACTIV1_H_IN_ACTIV_MASK  0x0000000F /* Input video Horizontal active pixel region width */
#define FC_INHACTIV1_H_IN_ACTIV_12_MASK  0x00000010 /* Input video Horizontal active pixel region width (0 */
#define FC_INHACTIV1_H_IN_ACTIV_13_MASK  0x00000020 /* Input video Horizontal active pixel region width (0 */

/* Frame Composer Input Video HBlank Pixels Register 0 */
#define FC_INHBLANK0  0x0000400C
#define FC_INHBLANK0_H_IN_BLANK_MASK  0x000000FF /* Input video Horizontal blanking pixel region width */

/* Frame Composer Input Video HBlank Pixels Register 1 */
#define FC_INHBLANK1  0x00004010
#define FC_INHBLANK1_H_IN_BLANK_MASK  0x00000003 /* Input video Horizontal blanking pixel region width this bit field holds bits 9:8 of number of Horizontal blanking pixels */
#define FC_INHBLANK1_H_IN_BLANK_12_MASK  0x0000001C /* Input video Horizontal blanking pixel region width If configuration parameter DWC_HDMI_TX_14 = True (1), this bit field holds bit 12:10 of number of horizontal blanking pixels */

/* Frame Composer Input Video VActive Pixels Register 0 */
#define FC_INVACTIV0  0x00004014
#define FC_INVACTIV0_V_IN_ACTIV_MASK  0x000000FF /* Input video Vertical active pixel region width */

/* Frame Composer Input Video VActive Pixels Register 1 */
#define FC_INVACTIV1  0x00004018
#define FC_INVACTIV1_V_IN_ACTIV_MASK  0x00000007 /* Input video Vertical active pixel region width */
#define FC_INVACTIV1_V_IN_ACTIV_12_11_MASK  0x00000018 /* Input video Vertical active pixel region width */

/* Frame Composer Input Video VBlank Pixels Register */
#define FC_INVBLANK  0x0000401C
#define FC_INVBLANK_V_IN_BLANK_MASK  0x000000FF /* Input video Vertical blanking pixel region width */

/* Frame Composer Input Video HSync Front Porch Register 0 */
#define FC_HSYNCINDELAY0  0x00004020
#define FC_HSYNCINDELAY0_H_IN_DELAY_MASK  0x000000FF /* Input video Hsync active edge delay */

/* Frame Composer Input Video HSync Front Porch Register 1 */
#define FC_HSYNCINDELAY1  0x00004024
#define FC_HSYNCINDELAY1_H_IN_DELAY_MASK  0x00000007 /* Input video Horizontal active edge delay */
#define FC_HSYNCINDELAY1_H_IN_DELAY_12_MASK  0x00000018 /* Input video Horizontal active edge delay */

/* Frame Composer Input Video HSync Width Register 0 */
#define FC_HSYNCINWIDTH0  0x00004028
#define FC_HSYNCINWIDTH0_H_IN_WIDTH_MASK  0x000000FF /* Input video Hsync active pulse width */

/* Frame Composer Input Video HSync Width Register 1 */
#define FC_HSYNCINWIDTH1  0x0000402C
#define FC_HSYNCINWIDTH1_H_IN_WIDTH_MASK  0x00000001 /* Input video Hsync active pulse width */
#define FC_HSYNCINWIDTH1_H_IN_WIDTH_9_MASK  0x00000002 /* Input video Hsync active pulse width */

/* Frame Composer Input Video VSync Front Porch Register */
#define FC_VSYNCINDELAY  0x00004030
#define FC_VSYNCINDELAY_V_IN_DELAY_MASK  0x000000FF /* Input video Vsync active edge delay */

/* Frame Composer Input Video VSync Width Register */
#define FC_VSYNCINWIDTH  0x00004034
#define FC_VSYNCINWIDTH_V_IN_WIDTH_MASK  0x0000003F /* Input video Vsync active pulse width */

/* Frame Composer Input Video Refresh Rate Register 0 */
#define FC_INFREQ0  0x00004038
#define FC_INFREQ0_INFREQ_MASK  0x000000FF /* Video refresh rate in Hz*1E3 format */

/* Frame Composer Input Video Refresh Rate Register 1 */
#define FC_INFREQ1  0x0000403C
#define FC_INFREQ1_INFREQ_MASK  0x000000FF /* Video refresh rate in Hz*1E3 format */

/* Frame Composer Input Video Refresh Rate Register 2 */
#define FC_INFREQ2  0x00004040
#define FC_INFREQ2_INFREQ_MASK  0x0000000F /* Video refresh rate in Hz*1E3 format */

/* Frame Composer Control Period Duration Register */
#define FC_CTRLDUR  0x00004044
#define FC_CTRLDUR_CTRLPERIODDURATION_MASK  0x000000FF /* Configuration of the control period minimum duration (minimum of 12 pixel clock cycles; refer to HDMI 1 */

/* Frame Composer Extended Control Period Duration Register */
#define FC_EXCTRLDUR  0x00004048
#define FC_EXCTRLDUR_EXCTRLPERIODDURATION_MASK  0x000000FF /* Configuration of the extended control period minimum duration (minimum of 32 pixel clock cycles; refer to HDMI 1 */

/* Frame Composer Extended Control Period Maximum Spacing Register */
#define FC_EXCTRLSPAC  0x0000404C
#define FC_EXCTRLSPAC_EXCTRLPERIODSPACING_MASK  0x000000FF /* Configuration of the maximum spacing between consecutive extended control periods (maximum of 50ms; refer to the applicable HDMI specification) */

/* Frame Composer Channel 0 Non-Preamble Data Register */
#define FC_CH0PREAM  0x00004050
#define FC_CH0PREAM_CH0_PREAMBLE_FILTER_MASK  0x000000FF /* When in control mode, configures 8 bits that fill the channel 0 data lines not used to transmit the preamble (for more clarification, refer to the HDMI 1 */

/* Frame Composer Channel 1 Non-Preamble Data Register */
#define FC_CH1PREAM  0x00004054
#define FC_CH1PREAM_CH1_PREAMBLE_FILTER_MASK  0x0000003F /* When in control mode, configures 6 bits that fill the channel 1 data lines not used to transmit the preamble (for more clarification, refer to the HDMI 1 */

/* Frame Composer Channel 2 Non-Preamble Data Register */
#define FC_CH2PREAM  0x00004058
#define FC_CH2PREAM_CH2_PREAMBLE_FILTER_MASK  0x0000003F /* When in control mode, configures 6 bits that fill the channel 2 data lines not used to transmit the preamble (for more clarification, refer to the HDMI 1 */

/* Frame Composer AVI Packet Configuration Register 3 */
#define FC_AVICONF3  0x0000405C
#define FC_AVICONF3_CN_MASK  0x00000003 /* IT content type according to CEA the specification */
#define FC_AVICONF3_YQ_MASK  0x0000000C /* YCC Quantization range according to the CEA specification */

/* Frame Composer GCP Packet Configuration Register */
#define FC_GCP  0x00004060
#define FC_GCP_CLEAR_AVMUTE_MASK  0x00000001 /* Value of "clear_avmute" in the GCP packet */
#define FC_GCP_SET_AVMUTE_MASK  0x00000002 /* Value of "set_avmute" in the GCP packet Once the AVmute is set, the frame composer schedules the GCP packet with AVmute set in the packet scheduler to be sent once (may only be transmitted between the active edge of VSYNC and 384 pixels following this edge) */
#define FC_GCP_DEFAULT_PHASE_MASK  0x00000004 /* Value of "default_phase" in the GCP packet */

/* Frame Composer AVI Packet Configuration Register 0 */
#define FC_AVICONF0  0x00004064
#define FC_AVICONF0_RGC_YCC_INDICATION_MASK  0x00000003 /* Y1,Y0 RGB or YCC indicator */
#define FC_AVICONF0_BAR_INFORMATION_MASK  0x0000000C /* Bar information data valid */
#define FC_AVICONF0_SCAN_INFORMATION_MASK  0x00000030 /* Scan information */
#define FC_AVICONF0_ACTIVE_FORMAT_PRESENT_MASK  0x00000040 /* Active format present */
#define FC_AVICONF0_RGC_YCC_INDICATION_2_MASK  0x00000080 /* Y2, Bit 2 of rgc_ycc_indication */

/* Frame Composer AVI Packet Configuration Register 1 */
#define FC_AVICONF1  0x00004068
#define FC_AVICONF1_ACTIVE_ASPECT_RATIO_MASK  0x0000000F /* Active aspect ratio */
#define FC_AVICONF1_PICTURE_ASPECT_RATIO_MASK  0x00000030 /* Picture aspect ratio */
#define FC_AVICONF1_COLORIMETRY_MASK  0x000000C0 /* Colorimetry */

/* Frame Composer AVI Packet Configuration Register 2 */
#define FC_AVICONF2  0x0000406C
#define FC_AVICONF2_NON_UNIFORM_PICTURE_SCALING_MASK  0x00000003 /* Non-uniform picture scaling */
#define FC_AVICONF2_QUANTIZATION_RANGE_MASK  0x0000000C /* Quantization range */
#define FC_AVICONF2_EXTENDED_COLORIMETRY_MASK  0x00000070 /* Extended colorimetry */
#define FC_AVICONF2_IT_CONTENT_MASK  0x00000080 /* IT content */

/* Frame Composer AVI Packet VIC Register */
#define FC_AVIVID  0x00004070
#define FC_AVIVID_FC_AVIVID_MASK  0x0000007F /* Configures the AVI InfoFrame Video Identification code */
#define FC_AVIVID_FC_AVIVID_7_MASK  0x00000080 /* Bit 7 of fc_avivid register */

#define FC_AVIETB0  0x4074
#define FC_AVIETB1  0x4078
#define FC_AVISBB0  0x407C
#define FC_AVISBB1  0x4080
#define FC_AVIELB0  0x4084
#define FC_AVIELB1  0x4088
#define FC_AVISRB0  0x408C
#define FC_AVISRB1  0x4090

/* Frame Composer AUD Packet Configuration Register 0 */
#define FC_AUDICONF0  0x00004094
#define FC_AUDICONF0_CT_MASK  0x0000000F /* Coding Type */
#define FC_AUDICONF0_CC_MASK  0x00000070 /* Channel count */

/* Frame Composer AUD Packet Configuration Register 1 */
#define FC_AUDICONF1  0x00004098
#define FC_AUDICONF1_SF_MASK  0x00000007 /* Sampling frequency */
#define FC_AUDICONF1_SS_MASK  0x00000030 /* Sampling size */

/* Frame Composer AUD Packet Configuration Register 2 */
#define FC_AUDICONF2  0x0000409C
#define FC_AUDICONF2_CA_MASK  0x000000FF /* Channel allocation */

/* Frame Composer AUD Packet Configuration Register 0 */
#define FC_AUDICONF3  0x000040A0
#define FC_AUDICONF3_LSV_MASK  0x0000000F /* Level shift value (for down mixing) */
#define FC_AUDICONF3_DM_INH_MASK  0x00000010 /* Down mix enable */
#define FC_AUDICONF3_LFEPBL_MASK  0x00000060 /* LFE playback information LFEPBL1, LFEPBL0 LFE playback level as compared to the other channels */

/* Frame Composer VSI Packet Data IEEE Register 0 */
#define FC_VSDIEEEID0  0x000040A4
#define FC_VSDIEEEID0_IEEE_MASK  0x000000FF /* This register configures the Vendor Specific InfoFrame IEEE registration identifier */

/* Frame Composer VSI Packet Data Size Register */
#define FC_VSDSIZE  0x000040A8
#define FC_VSDSIZE_VSDSIZE_MASK  0x0000001F /* Packet size as described in the HDMI Vendor Specific InfoFrame (from the HDMI specification) */

/* Frame Composer VSI Packet Data IEEE Register 1 */
#define FC_VSDIEEEID1  0x000040C0
#define FC_VSDIEEEID1_IEEE_MASK  0x000000FF /* This register configures the Vendor Specific InfoFrame IEEE registration identifier */

/* Frame Composer VSI Packet Data IEEE Register 2 */
#define FC_VSDIEEEID2  0x000040C4
#define FC_VSDIEEEID2_IEEE_MASK  0x000000FF /* This register configures the Vendor Specific InfoFrame IEEE registration identifier */

#define  FC_VSDPAYLOAD0  0x40C8

#define FC_SPDVENDORNAME0 0x4128
#define  FC_SPDPRODUCTNAME0 0x4148

/* Frame Composer SPD Packet Data Source Product Descriptor Register */
#define FC_SPDDEVICEINF  0x00004188
#define FC_SPDDEVICEINF_FC_SPDDEVICEINF_MASK  0x000000FF /* Frame Composer SPD Packet Data Source Product Descriptor Register */

/* Frame Composer Audio Sample Flat and Layout Configuration Register */
#define FC_AUDSCONF  0x0000418C
#define FC_AUDSCONF_AUD_PACKET_LAYOUT_MASK  0x00000001 /* Set the audio packet layout to be sent in the packet: 1b: layout 1 0b: layout 0 If DWC_HDMI_TX_20 is defined and register field fc_multistream_ctrl */
#define FC_AUDSCONF_AUD_PACKET_SAMPFLT_MASK  0x000000F0 /* Set the audio packet sample flat value to be sent on the packet */

/* Frame Composer Audio Sample Flat and Layout Configuration Register */
#define FC_AUDSSTAT  0x00004190
#define FC_AUDSSTAT_PACKET_SAMPPRS_MASK  0x0000000F /* Shows the data sample present indication of the last Audio sample packet sent by the HDMI TX Controller */

/* Frame Composer Audio Sample Validity Flag Register */
#define FC_AUDSV  0x00004194
#define FC_AUDSV_V0L_MASK  0x00000001 /* Set validity bit "V" for Channel 0, Left */
#define FC_AUDSV_V1L_MASK  0x00000002 /* Set validity bit "V" for Channel 1, Left */
#define FC_AUDSV_V2L_MASK  0x00000004 /* Set validity bit "V" for Channel 2, Left */
#define FC_AUDSV_V3L_MASK  0x00000008 /* Set validity bit "V" for Channel 3, Left */
#define FC_AUDSV_V0R_MASK  0x00000010 /* Set validity bit "V" for Channel 0, Right */
#define FC_AUDSV_V1R_MASK  0x00000020 /* Set validity bit "V" for Channel 1, Right */
#define FC_AUDSV_V2R_MASK  0x00000040 /* Set validity bit "V" for Channel 2, Right */
#define FC_AUDSV_V3R_MASK  0x00000080 /* Set validity bit "V" for Channel 3, Right */

/* Frame Composer Audio Sample User Flag Register */
#define FC_AUDSU  0x00004198
#define FC_AUDSU_U0L_MASK  0x00000001 /* Set user bit "U" for Channel 0, Left */
#define FC_AUDSU_U1L_MASK  0x00000002 /* Set user bit "U" for Channel 1, Left */
#define FC_AUDSU_U2L_MASK  0x00000004 /* Set user bit "U" for Channel 2, Left */
#define FC_AUDSU_U3L_MASK  0x00000008 /* Set user bit "U" for Channel 3, Left */
#define FC_AUDSU_U0R_MASK  0x00000010 /* Set user bit "U" for Channel 0, Right */
#define FC_AUDSU_U1R_MASK  0x00000020 /* Set user bit "U" for Channel 1, Right */
#define FC_AUDSU_U2R_MASK  0x00000040 /* Set user bit "U" for Channel 2, Right */
#define FC_AUDSU_U3R_MASK  0x00000080 /* Set user bit "U" for Channel 3, Right */

/* Frame Composer Audio Sample Channel Status Configuration Register 0 */
#define FC_AUDSCHNL0  0x0000419C
#define FC_AUDSCHNL0_OIEC_COPYRIGHT_MASK  0x00000001 /* IEC Copyright indication */
#define FC_AUDSCHNL0_OIEC_CGMSA_MASK  0x00000030 /* CGMS-A */

/* Frame Composer Audio Sample Channel Status Configuration Register 1 */
#define FC_AUDSCHNL1  0x000041A0
#define FC_AUDSCHNL1_OIEC_CATEGORYCODE_MASK  0x000000FF /* Category code */

/* Frame Composer Audio Sample Channel Status Configuration Register 2 */
#define FC_AUDSCHNL2  0x000041A4
#define FC_AUDSCHNL2_OIEC_SOURCENUMBER_MASK  0x0000000F /* Source number */
#define FC_AUDSCHNL2_OIEC_PCMAUDIOMODE_MASK  0x00000070 /* PCM audio mode */

/* Frame Composer Audio Sample Channel Status Configuration Register 3 */
#define FC_AUDSCHNL3  0x000041A8
#define FC_AUDSCHNL3_OIEC_CHANNELNUMCR0_MASK  0x0000000F /* Channel number for first right sample */
#define FC_AUDSCHNL3_OIEC_CHANNELNUMCR1_MASK  0x000000F0 /* Channel number for second right sample */

/* Frame Composer Audio Sample Channel Status Configuration Register 4 */
#define FC_AUDSCHNL4  0x000041AC
#define FC_AUDSCHNL4_OIEC_CHANNELNUMCR2_MASK  0x0000000F /* Channel number for third right sample */
#define FC_AUDSCHNL4_OIEC_CHANNELNUMCR3_MASK  0x000000F0 /* Channel number for fourth right sample */

/* Frame Composer Audio Sample Channel Status Configuration Register 5 */
#define FC_AUDSCHNL5  0x000041B0
#define FC_AUDSCHNL5_OIEC_CHANNELNUMCL0_MASK  0x0000000F /* Channel number for first left sample */
#define FC_AUDSCHNL5_OIEC_CHANNELNUMCL1_MASK  0x000000F0 /* Channel number for second left sample */

/* Frame Composer Audio Sample Channel Status Configuration Register 6 */
#define FC_AUDSCHNL6  0x000041B4
#define FC_AUDSCHNL6_OIEC_CHANNELNUMCL2_MASK  0x0000000F /* Channel number for third left sample */
#define FC_AUDSCHNL6_OIEC_CHANNELNUMCL3_MASK  0x000000F0 /* Channel number for fourth left sample */

/* Frame Composer Audio Sample Channel Status Configuration Register 7 */
#define FC_AUDSCHNL7  0x000041B8
#define FC_AUDSCHNL7_OIEC_SAMPFREQ_MASK  0x0000000F /* Sampling frequency */
#define FC_AUDSCHNL7_OIEC_CLKACCURACY_MASK  0x00000030 /* Clock accuracy */
#define FC_AUDSCHNL7_OIEC_SAMPFREQ_EXT_MASK  0x000000C0 /* Sampling frequency (channel status bits 31 and 30) */

/* Frame Composer Audio Sample Channel Status Configuration Register 8 */
#define FC_AUDSCHNL8  0x000041BC
#define FC_AUDSCHNL8_OIEC_WORDLENGTH_MASK  0x0000000F /* Word length configuration */
#define FC_AUDSCHNL8_OIEC_ORIGSAMPFREQ_MASK  0x000000F0 /* Original sampling frequency */

/* Frame Composer Number of High Priority Packets Attended Configuration Register */
#define FC_CTRLQHIGH  0x000041CC
#define FC_CTRLQHIGH_ONHIGHATTENDED_MASK  0x0000001F /* Configures the number of high priority packets or audio sample packets consecutively attended before checking low priority queue status */

/* Frame Composer Number of Low Priority Packets Attended Configuration Register */
#define FC_CTRLQLOW  0x000041D0
#define FC_CTRLQLOW_ONLOWATTENDED_MASK  0x0000001F /* Configures the number of low priority packets or null packets consecutively attended before checking high priority queue status or audio samples availability */

/* ACP: Audio content protection */
/* Frame Composer ACP Packet Type Configuration Register 0 */
#define FC_ACP0  0x000041D4
#define FC_ACP0_ACPTYPE_MASK  0x000000FF /* Configures the ACP packet type */

/* Frame Composer ACP Packet Body Configuration Register 16 */
#define FC_ACP16  0x00004208
#define FC_ACP16_FC_ACP16_MASK  0x000000FF /* Frame Composer ACP Packet Body Configuration Register 16 */

/* Frame Composer ACP Packet Body Configuration Register 15 */
#define FC_ACP15  0x0000420C
#define FC_ACP15_FC_ACP15_MASK  0x000000FF /* Frame Composer ACP Packet Body Configuration Register 15 */

/* Frame Composer ACP Packet Body Configuration Register 14 */
#define FC_ACP14  0x00004210
#define FC_ACP14_FC_ACP14_MASK  0x000000FF /* Frame Composer ACP Packet Body Configuration Register 14 */

/* Frame Composer ACP Packet Body Configuration Register 13 */
#define FC_ACP13  0x00004214
#define FC_ACP13_FC_ACP13_MASK  0x000000FF /* Frame Composer ACP Packet Body Configuration Register 13 */

/* Frame Composer ACP Packet Body Configuration Register 12 */
#define FC_ACP12  0x00004218
#define FC_ACP12_FC_ACP12_MASK  0x000000FF /* Frame Composer ACP Packet Body Configuration Register 12 */

/* Frame Composer ACP Packet Body Configuration Register 11 */
#define FC_ACP11  0x0000421C
#define FC_ACP11_FC_ACP11_MASK  0x000000FF /* Frame Composer ACP Packet Body Configuration Register 11 */

/* Frame Composer ACP Packet Body Configuration Register 10 */
#define FC_ACP10  0x00004220
#define FC_ACP10_FC_ACP10_MASK  0x000000FF /* Frame Composer ACP Packet Body Configuration Register 10 */

/* Frame Composer ACP Packet Body Configuration Register 9 */
#define FC_ACP9  0x00004224
#define FC_ACP9_FC_ACP9_MASK  0x000000FF /* Frame Composer ACP Packet Body Configuration Register 9 */

/* Frame Composer ACP Packet Body Configuration Register 8 */
#define FC_ACP8  0x00004228
#define FC_ACP8_FC_ACP8_MASK  0x000000FF /* Frame Composer ACP Packet Body Configuration Register 8 */

/* Frame Composer ACP Packet Body Configuration Register 7 */
#define FC_ACP7  0x0000422C
#define FC_ACP7_FC_ACP7_MASK  0x000000FF /* Frame Composer ACP Packet Body Configuration Register 7 */

/* Frame Composer ACP Packet Body Configuration Register 6 */
#define FC_ACP6  0x00004230
#define FC_ACP6_FC_ACP6_MASK  0x000000FF /* Frame Composer ACP Packet Body Configuration Register 6 */

/* Frame Composer ACP Packet Body Configuration Register 5 */
#define FC_ACP5  0x00004234
#define FC_ACP5_FC_ACP5_MASK  0x000000FF /* Frame Composer ACP Packet Body Configuration Register 5 */

/* Frame Composer ACP Packet Body Configuration Register 4 */
#define FC_ACP4  0x00004238
#define FC_ACP4_FC_ACP4_MASK  0x000000FF /* Frame Composer ACP Packet Body Configuration Register 4 */

/* Frame Composer ACP Packet Body Configuration Register 3 */
#define FC_ACP3  0x0000423C
#define FC_ACP3_FC_ACP3_MASK  0x000000FF /* Frame Composer ACP Packet Body Configuration Register 3 */

/* Frame Composer ACP Packet Body Configuration Register 2 */
#define FC_ACP2  0x00004240
#define FC_ACP2_FC_ACP2_MASK  0x000000FF /* Frame Composer ACP Packet Body Configuration Register 2 */

/* Frame Composer ACP Packet Body Configuration Register 1 */
#define FC_ACP1  0x00004244
#define FC_ACP1_FC_ACP1_MASK  0x000000FF /* Frame Composer ACP Packet Body Configuration Register 1 */

/* Frame Composer ISRC1 Packet Status, Valid, and Continue Configuration Register */
#define FC_ISCR1_0  0x00004248
#define FC_ISCR1_0_ISRC_CONT_MASK  0x00000001 /* ISRC1 Indication of packet continuation (ISRC2 will be transmitted) */
#define FC_ISCR1_0_ISRC_VALID_MASK  0x00000002 /* ISRC1 Valid control signal */
#define FC_ISCR1_0_ISRC_STATUS_MASK  0x0000001C /* ISRC1 Status signal */

/* Frame Composer ISRC1 Packet Body Register 16 */
#define FC_ISCR1_16  0x0000424C
#define FC_ISCR1_16_FC_ISCR1_16_MASK  0x000000FF /* Frame Composer ISRC1 Packet Body Register 16; configures ISRC1 packet body of the ISRC1 packet */

/* Frame Composer ISRC1 Packet Body Register 15 */
#define FC_ISCR1_15  0x00004250
#define FC_ISCR1_15_FC_ISCR1_15_MASK  0x000000FF /* Frame Composer ISRC1 Packet Body Register 15 */

/* Frame Composer ISRC1 Packet Body Register 14 */
#define FC_ISCR1_14  0x00004254
#define FC_ISCR1_14_FC_ISCR1_14_MASK  0x000000FF /* Frame Composer ISRC1 Packet Body Register 14 */

/* Frame Composer ISRC1 Packet Body Register 13 */
#define FC_ISCR1_13  0x00004258
#define FC_ISCR1_13_FC_ISCR1_13_MASK  0x000000FF /* Frame Composer ISRC1 Packet Body Register 13 */

/* Frame Composer ISRC1 Packet Body Register 12 */
#define FC_ISCR1_12  0x0000425C
#define FC_ISCR1_12_FC_ISCR1_12_MASK  0x000000FF /* Frame Composer ISRC1 Packet Body Register 12 */

/* Frame Composer ISRC1 Packet Body Register 11 */
#define FC_ISCR1_11  0x00004260
#define FC_ISCR1_11_FC_ISCR1_11_MASK  0x000000FF /* Frame Composer ISRC1 Packet Body Register 11 */

/* Frame Composer ISRC1 Packet Body Register 10 */
#define FC_ISCR1_10  0x00004264
#define FC_ISCR1_10_FC_ISCR1_10_MASK  0x000000FF /* Frame Composer ISRC1 Packet Body Register 10 */

/* Frame Composer ISRC1 Packet Body Register 9 */
#define FC_ISCR1_9  0x00004268
#define FC_ISCR1_9_FC_ISCR1_9_MASK  0x000000FF /* Frame Composer ISRC1 Packet Body Register 9 */

/* Frame Composer ISRC1 Packet Body Register 8 */
#define FC_ISCR1_8  0x0000426C
#define FC_ISCR1_8_FC_ISCR1_8_MASK  0x000000FF /* Frame Composer ISRC1 Packet Body Register 8 */

/* Frame Composer ISRC1 Packet Body Register 7 */
#define FC_ISCR1_7  0x00004270
#define FC_ISCR1_7_FC_ISCR1_7_MASK  0x000000FF /* Frame Composer ISRC1 Packet Body Register 7 */

/* Frame Composer ISRC1 Packet Body Register 6 */
#define FC_ISCR1_6  0x00004274
#define FC_ISCR1_6_FC_ISCR1_6_MASK  0x000000FF /* Frame Composer ISRC1 Packet Body Register 6 */

/* Frame Composer ISRC1 Packet Body Register 5 */
#define FC_ISCR1_5  0x00004278
#define FC_ISCR1_5_FC_ISCR1_5_MASK  0x000000FF /* Frame Composer ISRC1 Packet Body Register 5 */

/* Frame Composer ISRC1 Packet Body Register 4 */
#define FC_ISCR1_4  0x0000427C
#define FC_ISCR1_4_FC_ISCR1_4_MASK  0x000000FF /* Frame Composer ISRC1 Packet Body Register 4 */

/* Frame Composer ISRC1 Packet Body Register 3 */
#define FC_ISCR1_3  0x00004280
#define FC_ISCR1_3_FC_ISCR1_3_MASK  0x000000FF /* Frame Composer ISRC1 Packet Body Register 3 */

/* Frame Composer ISRC1 Packet Body Register 2 */
#define FC_ISCR1_2  0x00004284
#define FC_ISCR1_2_FC_ISCR1_2_MASK  0x000000FF /* Frame Composer ISRC1 Packet Body Register 2 */

/* Frame Composer ISRC1 Packet Body Register 1 */
#define FC_ISCR1_1  0x00004288
#define FC_ISCR1_1_FC_ISCR1_1_MASK  0x000000FF /* Frame Composer ISRC1 Packet Body Register 1 */

/* Frame Composer ISRC2 Packet Body Register 15 */
#define FC_ISCR2_15  0x0000428C
#define FC_ISCR2_15_FC_ISCR2_15_MASK  0x000000FF /* Frame Composer ISRC2 Packet Body Register 15; configures the ISRC2 packet body of the ISRC2 packet */

/* Frame Composer ISRC2 Packet Body Register 14 */
#define FC_ISCR2_14  0x00004290
#define FC_ISCR2_14_FC_ISCR2_14_MASK  0x000000FF /* Frame Composer ISRC2 Packet Body Register 14 */

/* Frame Composer ISRC2 Packet Body Register 13 */
#define FC_ISCR2_13  0x00004294
#define FC_ISCR2_13_FC_ISCR2_13_MASK  0x000000FF /* Frame Composer ISRC2 Packet Body Register 13 */

/* Frame Composer ISRC2 Packet Body Register 12 */
#define FC_ISCR2_12  0x00004298
#define FC_ISCR2_12_FC_ISCR2_12_MASK  0x000000FF /* Frame Composer ISRC2 Packet Body Register 12 */

/* Frame Composer ISRC2 Packet Body Register 11 */
#define FC_ISCR2_11  0x0000429C
#define FC_ISCR2_11_FC_ISCR2_11_MASK  0x000000FF /* Frame Composer ISRC2 Packet Body Register 11 */

/* Frame Composer ISRC2 Packet Body Register 10 */
#define FC_ISCR2_10  0x000042A0
#define FC_ISCR2_10_FC_ISCR2_10_MASK  0x000000FF /* Frame Composer ISRC2 Packet Body Register 10 */

/* Frame Composer ISRC2 Packet Body Register 9 */
#define FC_ISCR2_9  0x000042A4
#define FC_ISCR2_9_FC_ISCR2_9_MASK  0x000000FF /* Frame Composer ISRC2 Packet Body Register 9 */

/* Frame Composer ISRC2 Packet Body Register 8 */
#define FC_ISCR2_8  0x000042A8
#define FC_ISCR2_8_FC_ISCR2_8_MASK  0x000000FF /* Frame Composer ISRC2 Packet Body Register 8 */

/* Frame Composer ISRC2 Packet Body Register 7 */
#define FC_ISCR2_7  0x000042AC
#define FC_ISCR2_7_FC_ISCR2_7_MASK  0x000000FF /* Frame Composer ISRC2 Packet Body Register 7 */

/* Frame Composer ISRC2 Packet Body Register 6 */
#define FC_ISCR2_6  0x000042B0
#define FC_ISCR2_6_FC_ISCR2_6_MASK  0x000000FF /* Frame Composer ISRC2 Packet Body Register 6 */

/* Frame Composer ISRC2 Packet Body Register 5 */
#define FC_ISCR2_5  0x000042B4
#define FC_ISCR2_5_FC_ISCR2_5_MASK  0x000000FF /* Frame Composer ISRC2 Packet Body Register 5 */

/* Frame Composer ISRC2 Packet Body Register 4 */
#define FC_ISCR2_4  0x000042B8
#define FC_ISCR2_4_FC_ISCR2_4_MASK  0x000000FF /* Frame Composer ISRC2 Packet Body Register 4 */

/* Frame Composer ISRC2 Packet Body Register 3 */
#define FC_ISCR2_3  0x000042BC
#define FC_ISCR2_3_FC_ISCR2_3_MASK  0x000000FF /* Frame Composer ISRC2 Packet Body Register 3 */

/* Frame Composer ISRC2 Packet Body Register 2 */
#define FC_ISCR2_2  0x000042C0
#define FC_ISCR2_2_FC_ISCR2_2_MASK  0x000000FF /* Frame Composer ISRC2 Packet Body Register 2 */

/* Frame Composer ISRC2 Packet Body Register 1 */
#define FC_ISCR2_1  0x000042C4
#define FC_ISCR2_1_FC_ISCR2_1_MASK  0x000000FF /* Frame Composer ISRC2 Packet Body Register 1 */

/* Frame Composer ISRC2 Packet Body Register 0 */
#define FC_ISCR2_0  0x000042C8
#define FC_ISCR2_0_FC_ISCR2_0_MASK  0x000000FF /* Frame Composer ISRC2 Packet Body Register 0 */

/* Frame Composer Data Island Auto Packet Scheduling Register 0 Configures the Frame Composer RDRB(1)/Manual(0) data island packet insertion for SPD, VSD, ISRC2, ISRC1 and ACP packets */
#define FC_DATAUTO0  0x000042CC
#define FC_DATAUTO0_ACP_AUTO_MASK  0x00000001 /* Enables ACP automatic packet scheduling */
#define FC_DATAUTO0_ISCR1_AUTO_MASK  0x00000002 /* Enables ISRC1 automatic packet scheduling */
#define FC_DATAUTO0_ISCR2_AUTO_MASK  0x00000004 /* Enables ISRC2 automatic packet scheduling */
#define FC_DATAUTO0_VSD_AUTO_MASK  0x00000008 /* Enables VSD automatic packet scheduling */
#define FC_DATAUTO0_SPD_AUTO_MASK  0x00000010 /* Enables SPD automatic packet scheduling */

/* Frame Composer Data Island Auto Packet Scheduling Register 1 Configures the Frame Composer (FC) RDRB frame interpolation for SPD, VSD, ISRC2, ISRC1 and ACP packet insertion on data island when FC is on RDRB mode for the listed packets */
#define FC_DATAUTO1  0x000042D0
#define FC_DATAUTO1_AUTO_FRAME_INTERPOLATION_MASK  0x0000000F /* Packet frame interpolation for automatic packet scheduling */

/* Frame Composer Data Island Auto packet scheduling Register 2 Configures the Frame Composer (FC) RDRB line interpolation and number of packets in frame for SPD, VSD, ISRC2, ISRC1 and ACP packet insertion on data island when FC is on RDRB mode for the listed packets */
#define FC_DATAUTO2  0x000042D4
#define FC_DATAUTO2_AUTO_LINE_SPACING_MASK  0x0000000F /* Packets line spacing, for automatic packet scheduling */
#define FC_DATAUTO2_AUTO_FRAME_PACKETS_MASK  0x000000F0 /* Packets per frame, for automatic packet scheduling */

/* Frame Composer Data Island Manual Packet Request Register Requests to the Frame Composer the data island packet insertion for NULL, SPD, VSD, ISRC2, ISRC1 and ACP packets when FC_DATAUTO0 bit is in manual mode for the packet requested */
#define FC_DATMAN  0x000042D8
#define FC_DATMAN_ACP_TX_MASK  0x00000001 /* ACP packet */
#define FC_DATMAN_ISCR1_TX_MASK  0x00000002 /* ISRC1 packet */
#define FC_DATMAN_ISCR2_TX_MASK  0x00000004 /* ISRC2 packet */
#define FC_DATMAN_VSD_TX_MASK  0x00000008 /* VSD packet */
#define FC_DATMAN_SPD_TX_MASK  0x00000010 /* SPD packet */
#define FC_DATMAN_NULL_TX_MASK  0x00000020 /* Null packet */

/* Frame Composer Data Island Auto Packet Scheduling Register 3 Configures the Frame Composer Automatic(1)/RDRB(0) data island packet insertion for AVI, GCP, AUDI and ACR packets */
#define FC_DATAUTO3  0x000042DC
#define FC_DATAUTO3_ACR_AUTO_MASK  0x00000001 /* Enables ACR packet insertion */
#define FC_DATAUTO3_AUDI_AUTO_MASK  0x00000002 /* Enables AUDI packet insertion */
#define FC_DATAUTO3_GCP_AUTO_MASK  0x00000004 /* Enables GCP packet insertion */
#define FC_DATAUTO3_AVI_AUTO_MASK  0x00000008 /* Enables AVI packet insertion */
#define FC_DATAUTO3_AMP_AUTO_MASK  0x00000010 /* Enables AMP packet insertion */
#define FC_DATAUTO3_NVBI_AUTO_MASK  0x00000020 /* Enables NTSC VBI packet insertion */

/* Frame Composer Round Robin ACR Packet Insertion Register 0 Configures the Frame Composer (FC) RDRB frame interpolation for ACR packet insertion on data island when FC is on RDRB mode for this packet */
#define FC_RDRB0  0x000042E0
#define FC_RDRB0_ACRFRAMEINTERPOLATION_MASK  0x0000000F /* ACR Frame interpolation */

/* Frame Composer Round Robin ACR Packet Insertion Register 1 Configures the Frame Composer (FC) RDRB line interpolation and number of packets in frame for the ACR packet insertion on data island when FC is on RDRB mode this packet */
#define FC_RDRB1  0x000042E4
#define FC_RDRB1_ACRPACKETLINESPACING_MASK  0x0000000F /* ACR packet line spacing */
#define FC_RDRB1_ACRPACKETSINFRAME_MASK  0x000000F0 /* ACR packets in frame */

/* Frame Composer Round Robin AUDI Packet Insertion Register 2 Configures the Frame Composer (FC) RDRB frame interpolation for AUDI packet insertion on data island when FC is on RDRB mode for this packet */
#define FC_RDRB2  0x000042E8
#define FC_RDRB2_AUDIFRAMEINTERPOLATION_MASK  0x0000000F /* Audio frame interpolation */

/* Frame Composer Round Robin AUDI Packet Insertion Register 3 Configures the Frame Composer (FC) RDRB line interpolation and number of packets in frame for the AUDI packet insertion on data island when FC is on RDRB mode this packet */
#define FC_RDRB3  0x000042EC
#define FC_RDRB3_AUDIPACKETLINESPACING_MASK  0x0000000F /* Audio packets line spacing */
#define FC_RDRB3_AUDIPACKETSINFRAME_MASK  0x000000F0 /* Audio packets per frame */

/* Frame Composer Round Robin GCP Packet Insertion Register 4 Configures the Frame Composer (FC) RDRB frame interpolation for GCP packet insertion on data island when FC is on RDRB mode for this packet */
#define FC_RDRB4  0x000042F0
#define FC_RDRB4_GCPFRAMEINTERPOLATION_MASK  0x0000000F /* Frames interpolated between GCP packets */

/* Frame Composer Round Robin GCP Packet Insertion Register 5 Configures the Frame Composer (FC) RDRB line interpolation and number of packets in frame for the GCP packet insertion on data island when FC is on RDRB mode this packet */
#define FC_RDRB5  0x000042F4
#define FC_RDRB5_GCPPACKETLINESPACING_MASK  0x0000000F /* GCP packets line spacing */
#define FC_RDRB5_GCPPACKETSINFRAME_MASK  0x000000F0 /* GCP packets per frame */

/* Frame Composer Round Robin AVI Packet Insertion Register 6 Configures the Frame Composer (FC) RDRB frame interpolation for AVI packet insertion on data island when FC is on RDRB mode for this packet */
#define FC_RDRB6  0x000042F8
#define FC_RDRB6_AVIFRAMEINTERPOLATION_MASK  0x0000000F /* Frames interpolated between AVI packets */

/* Frame Composer Round Robin AVI Packet Insertion Register 7 Configures the Frame Composer (FC) RDRB line interpolation and number of packets in frame for the AVI packet insertion on data island when FC is on RDRB mode this packet */
#define FC_RDRB7  0x000042FC
#define FC_RDRB7_AVIPACKETLINESPACING_MASK  0x0000000F /* AVI packets line spacing */
#define FC_RDRB7_AVIPACKETSINFRAME_MASK  0x000000F0 /* AVI packets per frame */

/* Frame Composer Round Robin AMP Packet Insertion Register 8 */
#define FC_RDRB8  0x00004300
#define FC_RDRB8_AMPFRAMEINTERPOLATION_MASK  0x0000000F /* AMP frame interpolation */

/* Frame Composer Round Robin AMP Packet Insertion Register 9 */
#define FC_RDRB9  0x00004304
#define FC_RDRB9_AMPPACKETLINESPACING_MASK  0x0000000F /* AMP packets line spacing */
#define FC_RDRB9_AMPPACKETSINFRAME_MASK  0x000000F0 /* AMP packets per frame */

/* Frame Composer Round Robin NTSC VBI Packet Insertion Register 10 */
#define FC_RDRB10  0x00004308
#define FC_RDRB10_NVBIFRAMEINTERPOLATION_MASK  0x0000000F /* NTSC VBI frame interpolation */

/* Frame Composer Round Robin NTSC VBI Packet Insertion Register 11 */
#define FC_RDRB11  0x0000430C
#define FC_RDRB11_NVBIPACKETLINESPACING_MASK  0x0000000F /* NTSC VBI packets line spacing */
#define FC_RDRB11_NVBIPACKETSINFRAME_MASK  0x000000F0 /* NTSC VBI packets per frame */

/* Frame Composer Packet Interrupt Mask Register 0 */
#define FC_MASK0  0x00004348
#define FC_MASK0_NULL_MASK  0x00000001 /* Mask bit for FC_INT0 */
#define FC_MASK0_ACR_MASK  0x00000002 /* Mask bit for FC_INT0 */
#define FC_MASK0_AUDS_MASK  0x00000004 /* Mask bit for FC_INT0 */
#define FC_MASK0_NVBI_MASK  0x00000008 /* Mask bit for FC_INT0 */
#define FC_MASK0_MAS_MASK  0x00000010 /* Mask bit for FC_INT0 */
#define FC_MASK0_HBR_MASK  0x00000020 /* Mask bit for FC_INT0 */
#define FC_MASK0_ACP_MASK  0x00000040 /* Mask bit for FC_INT0 */
#define FC_MASK0_AUDI_MASK  0x00000080 /* Mask bit for FC_INT0 */

/* Frame Composer Packet Interrupt Mask Register 1 */
#define FC_MASK1  0x00004358
#define FC_MASK1_GCP_MASK  0x00000001 /* Mask bit for FC_INT1 */
#define FC_MASK1_AVI_MASK  0x00000002 /* Mask bit for FC_INT1 */
#define FC_MASK1_AMP_MASK  0x00000004 /* Mask bit for FC_INT1 */
#define FC_MASK1_SPD_MASK  0x00000008 /* Mask bit for FC_INT1 */
#define FC_MASK1_VSD_MASK  0x00000010 /* Mask bit for FC_INT1 */
#define FC_MASK1_ISCR2_MASK  0x00000020 /* Mask bit for FC_INT1 */
#define FC_MASK1_ISCR1_MASK  0x00000040 /* Mask bit for FC_INT1 */
#define FC_MASK1_GMD_MASK  0x00000080 /* Mask bit for FC_INT1 */

/* Frame Composer High/Low Priority Overflow Interrupt Mask Register 2 */
#define FC_MASK2  0x00004368
#define FC_MASK2_HIGHPRIORITY_OVERFLOW_MASK  0x00000001 /* Mask bit for FC_INT2 */
#define FC_MASK2_LOWPRIORITY_OVERFLOW_MASK  0x00000002 /* Mask bit for FC_INT2 */

/* Frame Composer Pixel Repetition Configuration Register */
#define FC_PRCONF  0x00004380
#define FC_PRCONF_OUTPUT_PR_FACTOR_MASK  0x0000000F /* Configures the video pixel repetition ratio to be sent on the AVI InfoFrame */
#define FC_PRCONF_INCOMING_PR_FACTOR_MASK  0x000000F0 /* Configures the input video pixel repetition */

/* Frame Composer Scrambler Control */
#define FC_SCRAMBLER_CTRL  0x00004384
#define FC_SCRAMBLER_CTRL_SCRAMBLER_ON_MASK  0x00000001 /* When set (1'b1), this field activates the HDMI 2 */
#define FC_SCRAMBLER_CTRL_SCRAMBLER_UCP_LINE_MASK  0x00000010 /* Debug register */

/* Frame Composer Multi-Stream Audio Control */
#define FC_MULTISTREAM_CTRL  0x00004388
#define FC_MULTISTREAM_CTRL_FC_MAS_PACKET_EN_MASK  0x00000001 /* This field, when set (1'b1), activates the HDMI 2 */

/* Frame Composer Packet Transmission Control */
#define FC_PACKET_TX_EN  0x0000438C
#define FC_PACKET_TX_EN_ACR_TX_EN_MASK  0x00000001 /* ACR packet transmission control 1b: Transmission enabled 0b: Transmission disabled */
#define FC_PACKET_TX_EN_GCP_TX_EN_MASK  0x00000002 /* GCP transmission control 1b: Transmission enabled 0b: Transmission disabled */
#define FC_PACKET_TX_EN_AVI_TX_EN_MASK  0x00000004 /* AVI packet transmission control 1b: Transmission enabled 0b: Transmission disabled */
#define FC_PACKET_TX_EN_AUDI_TX_EN_MASK  0x00000008 /* AUDI packet transmission control 1b: Transmission enabled 0b: Transmission disabled */
#define FC_PACKET_TX_EN_AUT_TX_EN_MASK  0x00000010 /* ACP, SPD, VSIF, ISRC1, and SRC2 packet transmission control 1b: Transmission enabled 0b: Transmission disabled */
#define FC_PACKET_TX_EN_AMP_TX_EN_MASK  0x00000020 /* AMP transmission control 1b: Transmission enabled 0b: Transmission disabled */
#define FC_PACKET_TX_EN_NVBI_TX_EN_MASK  0x00000040 /* NTSC VBI transmission control 1b: Transmission enabled 0b: Transmission disabled */
#define FC_PACKET_TX_EN_DRM_TX_EN_MASK  0x00000080/* DRM transmission control */

/* Frame Composer Active Space Control */
#define FC_ACTSPC_HDLR_CFG  0x000043A0
#define FC_ACTSPC_HDLR_CFG_ACTSPC_HDLR_EN_MASK  0x00000001 /* Active Space Handler Control 1b: Fixed active space value mode enabled */
#define FC_ACTSPC_HDLR_CFG_ACTSPC_HDLR_TGL_MASK  0x00000002 /* Active Space handler control 1b: Active space 1 value is different from Active Space 2 value */

/* Frame Composer Input Video 2D VActive Pixels Register 0 */
#define FC_INVACT_2D_0  0x000043A4
#define FC_INVACT_2D_0_FC_INVACT_2D_0_MASK  0x000000FF /* 2D Input video vertical active pixel region width */

/* Frame Composer Input Video VActive pixels Register 1 */
#define FC_INVACT_2D_1  0x000043A8
#define FC_INVACT_2D_1_FC_INVACT_2D_1_MASK  0x0000000F /* 2D Input video vertical active pixel region width */

/* Frame Composer GMD Packet Status Register Gamut metadata packet status bit information for no_current_gmd, next_gmd_field, gmd_packet_sequence and current_gamut_seq_num */
#define FC_GMD_STAT  0x00004400
#define FC_GMD_STAT_IGMDCURRENT_GAMUT_SEQ_NUM_MASK  0x0000000F /* Gamut scheduling: Current Gamut packet sequence number */
#define FC_GMD_STAT_IGMDPACKET_SEQ_MASK  0x00000030 /* Gamut scheduling: Gamut packet sequence */
#define FC_GMD_STAT_IGMDDNEXT_FIELD_MASK  0x00000040 /* Gamut scheduling: Gamut Next field */
#define FC_GMD_STAT_IGMDNO_CRNT_GBD_MASK  0x00000080 /* Gamut scheduling: No current gamut data */

/* Frame Composer GMD Packet Enable Register This register enables Gamut metadata (GMD) packet transmission */
#define FC_GMD_EN  0x00004404
#define FC_GMD_EN_GMDENABLETX_MASK  0x00000001 /* Gamut Metadata packet transmission enable (1b) */

/* Frame Composer GMD Packet Update Register This register performs an GMD packet content update according to the configured packet body (FC_GMD_PB0 to FC_GMD_PB27) and packet header (FC_GMD_HB) */
#define FC_GMD_UP  0x00004408
#define FC_GMD_UP_GMDUPDATEPACKET_MASK  0x00000001 /* Gamut Metadata packet update */

/* Frame Composer GMD Packet Schedule Configuration Register This register configures the number of GMD packets to be inserted per frame (starting always in the line where the active Vsync appears) and the line spacing between the transmitted GMD packets */
#define FC_GMD_CONF  0x0000440C
#define FC_GMD_CONF_GMDPACKETLINESPACING_MASK  0x0000000F /* Number of line spacing between the transmitted GMD packets */
#define FC_GMD_CONF_GMDPACKETSINFRAME_MASK  0x000000F0 /* Number of GMD packets per frame or video field (profile P0) */

/* Frame Composer GMD Packet Profile and Gamut Sequence Configuration Register This register configures the GMD packet header affected_gamut_seq_num and gmd_profile bits */
#define FC_GMD_HB  0x00004410
#define FC_GMD_HB_GMDAFFECTED_GAMUT_SEQ_NUM_MASK  0x0000000F /* Affected gamut sequence number */
#define FC_GMD_HB_GMDGBD_PROFILE_MASK  0x00000070 /* GMD profile bits */

#define FC_GMD_PB0  0x4414
#define FC_GMD_PB27 0x4480

/* Frame Composer AMP Packet Header Register 1 */
#define FC_AMP_HB1  0x000044A0
#define FC_AMP_HB1_FC_AMP_HB0_MASK  0x000000FF /* Frame Composer AMP Packet Header Register 1 */

/* Frame Composer AMP Packet Header Register 2 */
#define FC_AMP_HB2  0x000044A4
#define FC_AMP_HB2_FC_AMP_HB1_MASK  0x000000FF /* Frame Composer AMP Packet Header Register 2 */

#define FC_AMP_PB   0x000044A8

/* Frame Composer NTSC VBI Packet Header Register 1 */
#define FC_NVBI_HB1  0x00004520
#define FC_NVBI_HB1_FC_NVBI_HB0_MASK  0x000000FF /* Frame Composer NTSC VBI Packet Header Register 1 */

/* Frame Composer NTSC VBI Packet Header Register 2 */
#define FC_NVBI_HB2  0x00004524
#define FC_NVBI_HB2_FC_NVBI_HB1_MASK  0x000000FF /* Frame Composer NTSC VBI Packet Header Register 2 */

/* Frame Composer video/audio Force Enable Register This register allows to force the controller to output audio and video data the values configured in the FC_DBGAUD and FC_DBGTMDS registers */
#define FC_DBGFORCE  0x00004800
#define FC_DBGFORCE_FORCEVIDEO_MASK  0x00000001 /* Force fixed video output with FC_DBGTMDSx register contents */
#define FC_DBGFORCE_FORCEAUDIO_MASK  0x00000010 /* Force fixed audio output with FC_DBGAUDxCHx register contents */

/* Frame Composer Audio Data Channel 0 Register 0 Configures the audio fixed data to be used in channel 0 when in fixed audio selection */
#define FC_DBGAUD0CH0  0x00004804
#define FC_DBGAUD0CH0_FC_DBGAUD0CH0_MASK  0x000000FF /* Frame Composer Audio Data Channel 0 Register 0 */

/* Frame Composer Audio Data Channel 0 Register 1 Configures the audio fixed data to be used in channel 0 when in fixed audio selection */
#define FC_DBGAUD1CH0  0x00004808
#define FC_DBGAUD1CH0_FC_DBGAUD1CH0_MASK  0x000000FF /* Frame Composer Audio Data Channel 0 Register 1 */

/* Frame Composer Audio Data Channel 0 Register 2 Configures the audio fixed data to be used in channel 0 when in fixed audio selection */
#define FC_DBGAUD2CH0  0x0000480C
#define FC_DBGAUD2CH0_FC_DBGAUD2CH0_MASK  0x000000FF /* Frame Composer Audio Data Channel 0 Register 2 */

/* Frame Composer Audio Data Channel 1 Register 0 Configures the audio fixed data to be used in channel 1 when in fixed audio selection */
#define FC_DBGAUD0CH1  0x00004810
#define FC_DBGAUD0CH1_FC_DBGAUD0CH1_MASK  0x000000FF /* Frame Composer Audio Data Channel 1 Register 0 */

/* Frame Composer Audio Data Channel 1 Register 1 Configures the audio fixed data to be used in channel 1 when in fixed audio selection */
#define FC_DBGAUD1CH1  0x00004814
#define FC_DBGAUD1CH1_FC_DBGAUD1CH1_MASK  0x000000FF /* Frame Composer Audio Data Channel 1 Register 1 */

/* Frame Composer Audio Data Channel 1 Register 2 Configures the audio fixed data to be used in channel 1 when in fixed audio selection */
#define FC_DBGAUD2CH1  0x00004818
#define FC_DBGAUD2CH1_FC_DBGAUD2CH1_MASK  0x000000FF /* Frame Composer Audio Data Channel 1 Register 2 */

/* Frame Composer Audio Data Channel 2 Register 0 Configures the audio fixed data to be used in channel 2 when in fixed audio selection */
#define FC_DBGAUD0CH2  0x0000481C
#define FC_DBGAUD0CH2_FC_DBGAUD0CH2_MASK  0x000000FF /* Frame Composer Audio Data Channel 2 Register 0 */

/* Frame Composer Audio Data Channel 2 Register 1 Configures the audio fixed data to be used in channel 2 when in fixed audio selection */
#define FC_DBGAUD1CH2  0x00004820
#define FC_DBGAUD1CH2_FC_DBGAUD1CH2_MASK  0x000000FF /* Frame Composer Audio Data Channel 2 Register 1 */

/* Frame Composer Audio Data Channel 2 Register 2 Configures the audio fixed data to be used in channel 2 when in fixed audio selection */
#define FC_DBGAUD2CH2  0x00004824
#define FC_DBGAUD2CH2_FC_DBGAUD2CH2_MASK  0x000000FF /* Frame Composer Audio Data Channel 2 Register 2 */

/* Frame Composer Audio Data Channel 3 Register 0 Configures the audio fixed data to be used in channel 3 when in fixed audio selection */
#define FC_DBGAUD0CH3  0x00004828
#define FC_DBGAUD0CH3_FC_DBGAUD0CH3_MASK  0x000000FF /* Frame Composer Audio Data Channel 3 Register 0 */

/* Frame Composer Audio Data Channel 3 Register 1 Configures the audio fixed data to be used in channel 3 when in fixed audio selection */
#define FC_DBGAUD1CH3  0x0000482C
#define FC_DBGAUD1CH3_FC_DBGAUD1CH3_MASK  0x000000FF /* Frame Composer Audio Data Channel 3 Register 1 */

/* Frame Composer Audio Data Channel 3 Register 2 Configures the audio fixed data to be used in channel 3 when in fixed audio selection */
#define FC_DBGAUD2CH3  0x00004830
#define FC_DBGAUD2CH3_FC_DBGAUD2CH3_MASK  0x000000FF /* Frame Composer Audio Data Channel 3 Register 2 */

/* Frame Composer Audio Data Channel 4 Register 0 Configures the audio fixed data to be used in channel 4 when in fixed audio selection */
#define FC_DBGAUD0CH4  0x00004834
#define FC_DBGAUD0CH4_FC_DBGAUD0CH4_MASK  0x000000FF /* Frame Composer Audio Data Channel 4 Register 0 */

/* Frame Composer Audio Data Channel 4 Register 1 Configures the audio fixed data to be used in channel 4 when in fixed audio selection */
#define FC_DBGAUD1CH4  0x00004838
#define FC_DBGAUD1CH4_FC_DBGAUD1CH4_MASK  0x000000FF /* Frame Composer Audio Data Channel 4 Register 1 */

/* Frame Composer Audio Data Channel 4 Register 2 Configures the audio fixed data to be used in channel 4 when in fixed audio selection */
#define FC_DBGAUD2CH4  0x0000483C
#define FC_DBGAUD2CH4_FC_DBGAUD2CH4_MASK  0x000000FF /* Frame Composer Audio Data Channel 4 Register 2 */

/* Frame Composer Audio Data Channel 5 Register 0 Configures the audio fixed data to be used in channel 5 when in fixed audio selection */
#define FC_DBGAUD0CH5  0x00004840
#define FC_DBGAUD0CH5_FC_DBGAUD0CH5_MASK  0x000000FF /* Frame Composer Audio Data Channel 5 Register 0 */

/* Frame Composer Audio Data Channel 5 Register 1 Configures the audio fixed data to be used in channel 5 when in fixed audio selection */
#define FC_DBGAUD1CH5  0x00004844
#define FC_DBGAUD1CH5_FC_DBGAUD1CH5_MASK  0x000000FF /* Frame Composer Audio Data Channel 5 Register 1 */

/* Frame Composer Audio Data Channel 5 Register 2 Configures the audio fixed data to be used in channel 5 when in fixed audio selection */
#define FC_DBGAUD2CH5  0x00004848
#define FC_DBGAUD2CH5_FC_DBGAUD2CH5_MASK  0x000000FF /* Frame Composer Audio Data Channel 5 Register 2 */

/* Frame Composer Audio Data Channel 6 Register 0 Configures the audio fixed data to be used in channel 6 when in fixed audio selection */
#define FC_DBGAUD0CH6  0x0000484C
#define FC_DBGAUD0CH6_FC_DBGAUD0CH6_MASK  0x000000FF /* Frame Composer Audio Data Channel 6 Register 0 */

/* Frame Composer Audio Data Channel 6 Register 1 Configures the audio fixed data to be used in channel 6 when in fixed audio selection */
#define FC_DBGAUD1CH6  0x00004850
#define FC_DBGAUD1CH6_FC_DBGAUD1CH6_MASK  0x000000FF /* Frame Composer Audio Data Channel 6 Register 1 */

/* Frame Composer Audio Data Channel 6 Register 2 Configures the audio fixed data to be used in channel 6 when in fixed audio selection */
#define FC_DBGAUD2CH6  0x00004854
#define FC_DBGAUD2CH6_FC_DBGAUD2CH6_MASK  0x000000FF /* Frame Composer Audio Data Channel 6 Register 2 */

/* Frame Composer Audio Data Channel 7 Register 0 Configures the audio fixed data to be used in channel 7 when in fixed audio selection */
#define FC_DBGAUD0CH7  0x00004858
#define FC_DBGAUD0CH7_FC_DBGAUD0CH7_MASK  0x000000FF /* Frame Composer Audio Data Channel 7 Register 0 */

/* Frame Composer Audio Data Channel 7 Register 1 Configures the audio fixed data to be used in channel 7 when in fixed audio selection */
#define FC_DBGAUD1CH7  0x0000485C
#define FC_DBGAUD1CH7_FC_DBGAUD1CH7_MASK  0x000000FF /* Frame Composer Audio Data Channel 7 Register 1 */

/* Frame Composer Audio Data Channel 7 Register 2 Configures the audio fixed data to be used in channel 7 when in fixed audio selection */
#define FC_DBGAUD2CH7  0x00004860
#define FC_DBGAUD2CH7_FC_DBGAUD2CH7_MASK  0x000000FF /* Frame Composer Audio Data Channel 7 Register 2 */

#define FC_DBGTMDS0  0x4864
#define FC_DBGTMDS1  0x4868
#define FC_DBGTMDS2  0x486C

/* Frame Composer DRM Packet */
#define FC_DRM_UP	 0x459C
#define FC_DRM_UP_DRMPACKETUPDATE_MASK    0x00000001

/* Frame Composer DRM Packet Header Rigister Array */
#define FC_DRM_HB0	 0x45A0
#define FC_DRM_HB1	 0x45A4
#define FC_DRM_UP_FC_DRM_HB_MASK          0x000000FF

#define FC_DRM_PB0   0x45A8
#define FC_DRM_PB1	 0x45AC
#define FC_DRM_PB2	 0x45B0
#define FC_DRM_PB3	 0x45B4
#define FC_DRM_PB4	 0x45B8
#define FC_DRM_PB5	 0x45BC
#define FC_DRM_PB6	 0x45C0
#define FC_DRM_PB7	 0x45C4
#define FC_DRM_PB8	 0x45C8
#define FC_DRM_PB9	 0x45CC
#define FC_DRM_PB10	 0x45D0
#define FC_DRM_PB11	 0x45D4
#define FC_DRM_PB12	 0x45D8
#define FC_DRM_PB13	 0x45DC
#define FC_DRM_PB14	 0x45E0
#define FC_DRM_PB15	 0x45E4
#define FC_DRM_PB16	 0x45E8
#define FC_DRM_PB17	 0x45EC
#define FC_DRM_PB18	 0x45F0
#define FC_DRM_PB19	 0x45F4
#define FC_DRM_PB20	 0x45F8
#define FC_DRM_PB21	 0x45FC
#define FC_DRM_PB22	 0x4600
#define FC_DRM_PB23	 0x4604
#define FC_DRM_PB24	 0x4608
#define FC_DRM_PB25	 0x460C
#define FC_DRM_PB26	 0x4610


/**
 * @desc: set audio mute state
*/
void fc_audio_mute(dw_hdmi_dev_t *dev);

/**
 * @desc: get audio current mute state
*/
u8 dw_fc_audio_get_mute(dw_hdmi_dev_t *dev);

/**
 * @desc: get audio current packet layout
*/
u8 dw_fc_audio_get_packet_layout(dw_hdmi_dev_t *dev);

/**
 * @desc: get audio current channel count
*/
u8 dw_fc_audio_get_channel_count(dw_hdmi_dev_t *dev);

/**
 * @desc: set audio infoframe config
*/
void dw_fc_audio_packet_config(dw_hdmi_dev_t *dev, dw_audio_param_t *audio);

/**
 * @desc: set audio config
*/
void dw_fc_audio_config(dw_hdmi_dev_t *dev, dw_audio_param_t *audio);

/**
 * @desc: set video hdcp keepout
*/
void dw_fc_video_set_hdcp_keepout(dw_hdmi_dev_t *dev, u8 bit);

/**
 * @desc: set video config
*/
int dw_fc_video_config(dw_hdmi_dev_t *dev, dw_video_param_t *video);

/**
 * @desc: set video mode. hdmi or dvi
 * @bit: 1 - hdmi mode
 *       0 - dvi mode
*/
void dw_fc_video_set_tmds_mode(dw_hdmi_dev_t *dev, u8 bit);

/**
 * @desc: get video current mode
 * @return: 1 - hdmi mode
 *          0 - dvi mode
*/
u8 dw_fc_video_get_tmds_mode(dw_hdmi_dev_t *dev);

/**
 * @desc: video config scramble
 * @state: 1 - enable scramble
 *         0 - disable scramble
*/
void dw_fc_video_set_scramble(dw_hdmi_dev_t *dev, u8 state);

/**
 * @desc: video get current scramble state
 * @return: 1 - scramble is enable
 *          0 - scramble is disable
*/
u8 dw_fc_video_get_scramble(dw_hdmi_dev_t *dev);

/**
 * @desc: force audio output
*/
void dw_fc_audio_force(dw_hdmi_dev_t *dev, u8 bit);

/**
 * @desc: both force audio and video output
*/
void dw_fc_force_output(dw_hdmi_dev_t *dev, int enable);

/**
 * @desc: packets configure is the same as infoframe configure
*/
int dw_fc_packet_config(dw_hdmi_dev_t *dev, dw_video_param_t *video,
						dw_product_param_t *prod);

/**
 * @desc: packet config set avmute
*/
void dw_gcp_set_avmute(dw_hdmi_dev_t *dev, u8 enable);

/**
 * @desc: get current avmute state
*/
u8 dw_gcp_get_avmute(dw_hdmi_dev_t *dev);

void dw_drm_packet_clear(dw_fc_drm_pb_t *pb);

void dw_drm_packet_up(dw_hdmi_dev_t *dev, dw_fc_drm_pb_t *pb);

void dw_drm_packet_disabled(dw_hdmi_dev_t *dev);

u8 dw_avi_get_colori_metry(dw_hdmi_dev_t *dev);

void dw_avi_set_colori_metry(dw_hdmi_dev_t *dev, u8 metry, u8 ex_metry);

void dw_avi_set_scan_info(dw_hdmi_dev_t *dev, u8 left);

u8 dw_avi_get_rgb_ycc(dw_hdmi_dev_t *dev);

u8 dw_avi_get_video_code(dw_hdmi_dev_t *dev);

void dw_avi_set_video_code(dw_hdmi_dev_t *dev, u8 data);

void dw_avi_set_aspect_ratio(dw_hdmi_dev_t *dev, u8 left);

void dw_avi_set_quantization_range(dw_hdmi_dev_t *dev, u8 range);

/*
* get vsif data
* data[0]: hdmi_format filed in vsif
* data[1]: hdmi_vic or 3d strcture filed in vsif
*/
void dw_vsif_get_hdmi_vic(dw_hdmi_dev_t *dev, u8 *data);

/*
* set vsif data
* data[0]: hdmi_format filed in vsif
* data[1]: hdmi_vic or 3d strcture filed in vsif
*/
void dw_vsif_set_hdmi_vic(dw_hdmi_dev_t *dev, u8 *data);

u32 dw_fc_audio_get_sample_freq(dw_hdmi_dev_t *dev);

u8 dw_fc_audio_get_word_length(dw_hdmi_dev_t *dev);

void dw_fc_audio_set_mute(dw_hdmi_dev_t *dev, u8 mute);

int dw_fc_iteration_process(dw_hdmi_dev_t *dev);

#endif /* _DW_FRAME_COMPOSER_H */
