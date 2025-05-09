// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2017 Realtek Corporation */

/* ************************************************************
 * include files
 * ************************************************************ */

#include "mp_precomp.h"
#include "phydm_precomp.h"

/* ******************************************************
 * when antenna test utility is on or some testing need to disable antenna diversity
 * call this function to disable all ODM related mechanisms which will switch antenna.
 * ****************************************************** */
#if (defined(CONFIG_SMART_ANTENNA))
#if (defined(CONFIG_CUMITEK_SMART_ANTENNA))
void
phydm_cumitek_smt_ant_mapping_table_8822b(
	void		*p_dm_void,
	u8		*table_path_a,
	u8		*table_path_b
)
{
	struct	PHY_DM_STRUCT		*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32		path_a_0to3_idx = 0;
	u32		path_b_0to3_idx = 0;
	u32		path_a_4to7_idx = 0;
	u32		path_b_4to7_idx = 0;

	path_a_0to3_idx = ((table_path_a[3] & 0xf) << 24) | ((table_path_a[2] & 0xf) << 16)
						| ((table_path_a[1] & 0xf) << 8) | (table_path_a[0] & 0xf);

	path_b_0to3_idx = ((table_path_b[3] & 0xf) << 28) | ((table_path_b[2] & 0xf) << 20)
						| ((table_path_b[1] & 0xf) << 12) | ((table_path_b[0] & 0xf) << 4);

	path_a_4to7_idx = ((table_path_a[7] & 0xf) << 24) | ((table_path_a[6] & 0xf) << 16)
						| ((table_path_a[5] & 0xf) << 8) | (table_path_a[4] & 0xf);

	path_b_4to7_idx = ((table_path_b[7] & 0xf) << 28) | ((table_path_b[6] & 0xf) << 20)
						| ((table_path_b[5] & 0xf) << 12) | ((table_path_b[4] & 0xf) << 4);


	/*PHYDM_DBG(p_dm, DBG_SMT_ANT, ("mapping table{A, B} = {0x%x, 0x%x}\n", path_a_0to3_idx, path_b_0to3_idx));*/

	/*pathA*/
	odm_set_bb_reg(p_dm, 0xca4, MASKDWORD, path_a_0to3_idx); /*ant map 1*/
	odm_set_bb_reg(p_dm, 0xca8, MASKDWORD, path_a_4to7_idx); /*ant map 2*/

	/*pathB*/
	odm_set_bb_reg(p_dm, 0xea4, MASKDWORD, path_b_0to3_idx); /*ant map 1*/
	odm_set_bb_reg(p_dm, 0xea8, MASKDWORD, path_b_4to7_idx); /*ant map 2*/

}


void
phydm_cumitek_smt_ant_init_8822b(
	void		*p_dm_void
)
{
	struct	PHY_DM_STRUCT		*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct	smt_ant				*p_smtant_table = &(p_dm->smtant_table);
	struct	smt_ant_cumitek		*p_cumi_smtant_table = &(p_dm->smtant_table.cumi_smtant_table);
	u32		value32;

	PHYDM_DBG(p_dm, DBG_SMT_ANT, ("[8822B Cumitek SmtAnt Int]\n"));

	/*========= MAC GPIO setting =================================*/

	/* Pin, pin_name, RFE_CTRL_NUM*/

	/* A0, 55, 5G_TRSW, 3*/
	/* A1, 52, 5G_TRSW, 0*/
	/* A2, 25, 5G_TRSW, 8*/

	/* B0, 16, 5G_TRSW, 4*/
	/* B1, 13, 5G_TRSW, 11*/
	/* B2, 24, 5G_TRSW, 9*/

	/*for RFE_CTRL 8 & 9*/
	odm_set_mac_reg(p_dm, 0x4c, BIT(24) | BIT(23), 2);
	odm_set_mac_reg(p_dm, 0x44, BIT(27) | BIT(26), 0);

	/*for RFE_CTRL 0*/
	odm_set_mac_reg(p_dm, 0x4c, BIT(25), 0);
	odm_set_mac_reg(p_dm, 0x64, BIT(29), 1);

	/*for RFE_CTRL 2 & 3*/
	odm_set_mac_reg(p_dm, 0x4c, BIT(26), 0);
	odm_set_mac_reg(p_dm, 0x64, BIT(28), 1);

	/*for RFE_CTRL 11*/
	odm_set_mac_reg(p_dm, 0x40, BIT(3), 1);


	/*0x604[25]=1 : 2bit mode for pathA&B&C&D*/
	/*0x604[25]=0 : 3bit mode for pathA&B*/
	p_smtant_table->tx_desc_mode = 0;
	odm_set_mac_reg(p_dm, 0x604, BIT(25), (u32)p_smtant_table->tx_desc_mode);

	/*========= BB RFE setting =================================*/
	/*========= BB SmtAnt setting =================================*/
	odm_set_mac_reg(p_dm, 0x668, BIT(3), 1);
	odm_set_bb_reg(p_dm, 0x804, BIT(4), 0); /*lathch antsel*/
	odm_set_bb_reg(p_dm, 0x818, 0xf00000, 0); /*keep tx by rx*/
	odm_set_bb_reg(p_dm, 0x900, BIT(19), 0); /*fast train*/
	odm_set_bb_reg(p_dm, 0x900, BIT(18), 1); /*1: by TXDESC*/

	/*pathA*/
	odm_set_bb_reg(p_dm, 0xca4, MASKDWORD, 0x03020100); /*ant map 1*/
	odm_set_bb_reg(p_dm, 0xca8, MASKDWORD, 0x07060504); /*ant map 2*/
	odm_set_bb_reg(p_dm, 0xcac, BIT(9), 0); /*keep antsel map by GNT_BT*/

	/*pathB*/
	odm_set_bb_reg(p_dm, 0xea4, MASKDWORD, 0x30201000); /*ant map 1*/
	odm_set_bb_reg(p_dm, 0xea8, MASKDWORD, 0x70605040); /*ant map 2*/
	odm_set_bb_reg(p_dm, 0xeac, BIT(9), 0); /*keep antsel map by GNT_BT*/
}

void
phydm_cumitek_smt_ant_init_8197f(
	void		*p_dm_void
)
{
	struct	PHY_DM_STRUCT	*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct	smt_ant			*p_smtant_table = &(p_dm->smtant_table);
	struct	smt_ant_cumitek	*p_cumi_smtant_table = &(p_dm->smtant_table.cumi_smtant_table);
	u32		value32;

	PHYDM_DBG(p_dm, DBG_SMT_ANT, ("[8197F Cumitek SmtAnt Int]\n"));

	/*GPIO setting*/


}

void
phydm_cumitek_smt_tx_ant_update(
	void		*p_dm_void,
	u8		tx_ant_idx_path_a,
	u8		tx_ant_idx_path_b,
	u32		mac_id
)
{
	struct	PHY_DM_STRUCT	*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct	smt_ant			*p_smtant_table = &(p_dm->smtant_table);
	struct	smt_ant_cumitek	*p_cumi_smtant_table = &(p_dm->smtant_table.cumi_smtant_table);

	PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[Cumitek] Set TX-ANT[%d] = (( A:0x%x ,  B:0x%x ))\n",
		mac_id, tx_ant_idx_path_a, tx_ant_idx_path_b));

	/*path-A*/
	p_cumi_smtant_table->tx_ant_idx[0][mac_id] = tx_ant_idx_path_a; /*fill this value into TXDESC*/

	/*path-B*/
	p_cumi_smtant_table->tx_ant_idx[1][mac_id] = tx_ant_idx_path_b; /*fill this value into TXDESC*/
}

void
phydm_cumitek_smt_rx_default_ant_update(
	void		*p_dm_void,
	u8		rx_ant_idx_path_a,
	u8		rx_ant_idx_path_b
)
{
	struct	PHY_DM_STRUCT	*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct	smt_ant			*p_smtant_table = &(p_dm->smtant_table);
	struct	smt_ant_cumitek	*p_cumi_smtant_table = &(p_dm->smtant_table.cumi_smtant_table);

	PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[Cumitek] Set RX-ANT = (( A:0x%x, B:0x%x ))\n",
		rx_ant_idx_path_a, rx_ant_idx_path_b));

	/*path-A*/
	if (p_cumi_smtant_table->rx_default_ant_idx[0] != rx_ant_idx_path_a) {
		p_cumi_smtant_table->rx_default_ant_idx[0] = rx_ant_idx_path_a;
	}

	/*path-B*/
	if (p_cumi_smtant_table->rx_default_ant_idx[1] != rx_ant_idx_path_b) {
		p_cumi_smtant_table->rx_default_ant_idx[1] = rx_ant_idx_path_b;
	}

}

void
phydm_cumitek_smt_ant_debug(
	void		*p_dm_void,
	char		input[][16],
	u32		*_used,
	char		*output,
	u32		*_out_len,
	u32		input_num
)
{
	struct PHY_DM_STRUCT		*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct	smt_ant			*p_smtant_table = &(p_dm->smtant_table);
	struct	smt_ant_cumitek	*p_cumi_smtant_table = &(p_dm->smtant_table.cumi_smtant_table);
	u32			used = *_used;
	u32			out_len = *_out_len;
	char			help[] = "-h";
	u32			dm_value[10] = {0};
	u8			i;

	PHYDM_SSCANF(input[1], DCMD_DECIMAL, &dm_value[0]);

	if (strcmp(input[1], help) == 0) {
		PHYDM_SNPRINTF((output + used, out_len - used, "{1} {PathA rx_ant_idx} {pathB rx_ant_idx}\n"));
		PHYDM_SNPRINTF((output + used, out_len - used, "{2} {PathA tx_ant_idx} {pathB tx_ant_idx} {macid}\n"));
		PHYDM_SNPRINTF((output + used, out_len - used, "{3} {PathA mapping table} {PathB mapping table}\n"));
		PHYDM_SNPRINTF((output + used, out_len - used, "{4} {txdesc_mode 0:3bit, 1:2bit}\n"));

	} else if (dm_value[0] == 1) { /*fix rx_idle pattern*/

		PHYDM_SSCANF(input[2], DCMD_DECIMAL, &dm_value[1]);
		PHYDM_SSCANF(input[3], DCMD_DECIMAL, &dm_value[2]);

		phydm_cumitek_smt_rx_default_ant_update(p_dm, (u8)dm_value[1], (u8)dm_value[2]);
		PHYDM_SNPRINTF((output + used, out_len - used, "RX Ant{A, B}={%d, %d}\n", dm_value[1], dm_value[2]));

	} else if (dm_value[0] == 2) { /*fix tx pattern*/


		for (i = 1; i < 4; i++) {
			if (input[i + 1])
				PHYDM_SSCANF(input[i + 1], DCMD_DECIMAL, &dm_value[i]);
		}

		PHYDM_SNPRINTF((output + used, out_len - used, "STA[%d] TX Ant{A, B}={%d, %d}\n",dm_value[3], dm_value[1], dm_value[2]));
		phydm_cumitek_smt_tx_ant_update(p_dm, (u8)dm_value[1], (u8)dm_value[2], (u8)dm_value[3]);

	} else if (dm_value[0] == 3) {

		u8 table_path_a[8] = {0};
		u8 table_path_b[8] = {0};

		for (i = 1; i < 4; i++) {
			if (input[i + 1])
				PHYDM_SSCANF(input[i + 1], DCMD_HEX, &dm_value[i]);
		}

		PHYDM_SNPRINTF((output + used, out_len - used, "Set Path-AB mapping table={%d, %d}\n", dm_value[1], dm_value[2]));

		for (i = 0; i <8; i++) {
			table_path_a[i] = (u8)((dm_value[1] >> (4 * i)) & 0xf);
			table_path_b[i] = (u8)((dm_value[2] >> (4 * i)) & 0xf);
		}

		PHYDM_SNPRINTF((output + used, out_len - used, "Ant_Table_A[7:0]={0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x}\n",
			table_path_a[7], table_path_a[6], table_path_a[5], table_path_a[4], table_path_a[3], table_path_a[2], table_path_a[1], table_path_a[0]));
		PHYDM_SNPRINTF((output + used, out_len - used, "Ant_Table_B[7:0]={0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x}\n",
			table_path_b[7], table_path_b[6], table_path_b[5], table_path_b[4], table_path_b[3], table_path_b[2], table_path_b[1], table_path_b[0]));

		phydm_cumitek_smt_ant_mapping_table_8822b(p_dm, &(table_path_a[0]), &(table_path_b[0]));
	}else if (dm_value[0] == 4) {

		p_smtant_table->tx_desc_mode = (u8)dm_value[1];
		odm_set_mac_reg(p_dm, 0x604, BIT(25), (u32)p_smtant_table->tx_desc_mode);
	}
	*_used = used;
	*_out_len = out_len;
}

#endif

#if (defined(CONFIG_HL_SMART_ANTENNA))
#ifdef CONFIG_HL_SMART_ANTENNA_TYPE2


u32
phydm_construct_hb_rfu_codeword_type2(
	void		*p_dm_void,
	u32		beam_set_idx
)
{
	struct PHY_DM_STRUCT	*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct smt_ant_honbo		*pdm_sat_table = &(p_dm->dm_sat_table);
	u32		sync_codeword = 0x7f;
	u32		codeword = 0;
	u32		data_tmp = 0;
	u32		i;

	for (i = 0; i < pdm_sat_table->ant_num_total; i++) {

		if (*p_dm->p_band_type == ODM_BAND_5G)
			data_tmp = pdm_sat_table->rfu_codeword_table_5g[beam_set_idx][i];
		else
			data_tmp = pdm_sat_table->rfu_codeword_table_2g[beam_set_idx][i];

		codeword |= (data_tmp << (i * pdm_sat_table->rfu_each_ant_bit_num));
	}

	codeword = (codeword<<8) | sync_codeword;

	return codeword;
}

void
phydm_update_beam_pattern_type2(
	void		*p_dm_void,
	u32		codeword,
	u32		codeword_length
)
{
	struct PHY_DM_STRUCT		*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct smt_ant_honbo			*pdm_sat_table = &(p_dm->dm_sat_table);
	u8			i;
	bool			beam_ctrl_signal;
	u32			one = 0x1;
	u32			reg44_tmp_p, reg44_tmp_n, reg44_ori;
	u8			devide_num = 4;

	PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Set codeword = ((0x%x))\n", codeword));

	reg44_ori = odm_get_mac_reg(p_dm, 0x44, MASKDWORD);
	reg44_tmp_p = reg44_ori;
	/*PHYDM_DBG(p_dm, DBG_ANT_DIV, ("reg44_ori =0x%x\n", reg44_ori));*/

	/*devide_num = (pdm_sat_table->rfu_protocol_type == 2) ? 8 : 4;*/

	for (i = 0; i <= (codeword_length - 1); i++) {
		beam_ctrl_signal = (bool)((codeword & BIT(i)) >> i);

		if (p_dm->debug_components & DBG_ANT_DIV) {

			if (i == (codeword_length - 1)) {
				dbg_print("%d ]\n", beam_ctrl_signal);
				/**/
			} else if (i == 0) {
				dbg_print("Start sending codeword[1:%d] ---> [ %d ", codeword_length, beam_ctrl_signal);
				/**/
			} else if ((i % devide_num) == (devide_num-1)) {
				dbg_print("%d  |  ", beam_ctrl_signal);
				/**/
			} else {
				dbg_print("%d ", beam_ctrl_signal);
				/**/
			}
		}
	}
}

void
phydm_update_rx_idle_beam_type2(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT				*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct phydm_fat_struct		*p_dm_fat_table = &p_dm->dm_fat_table;
	struct smt_ant_honbo	*pdm_sat_table = &(p_dm->dm_sat_table);
	u32			i;

	pdm_sat_table->update_beam_codeword = phydm_construct_hb_rfu_codeword_type2(p_dm, pdm_sat_table->rx_idle_beam_set_idx);
	PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[ Update Rx-Idle-Beam ] BeamSet idx = ((%d))\n", pdm_sat_table->rx_idle_beam_set_idx));

	odm_schedule_work_item(&pdm_sat_table->hl_smart_antenna_workitem);
	/*odm_stall_execution(1);*/

	pdm_sat_table->pre_codeword = pdm_sat_table->update_beam_codeword;
}


void
phydm_hl_smart_ant_debug_type2(
	void		*p_dm_void,
	char		input[][16],
	u32		*_used,
	char		*output,
	u32		*_out_len,
	u32		input_num
)
{
	struct PHY_DM_STRUCT		*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct smt_ant_honbo			*pdm_sat_table = &(p_dm->dm_sat_table);
	u32			used = *_used;
	u32			out_len = *_out_len;
	u32			one = 0x1;
	u32			codeword_length = pdm_sat_table->rfu_codeword_total_bit_num;
	u32			beam_ctrl_signal, i;
	u8			devide_num = 4;
	char			help[] = "-h";
	u32			dm_value[10] = {0};

	PHYDM_SSCANF(input[1], DCMD_DECIMAL, &dm_value[0]);
	PHYDM_SSCANF(input[2], DCMD_DECIMAL, &dm_value[1]);
	PHYDM_SSCANF(input[3], DCMD_DECIMAL, &dm_value[2]);
	PHYDM_SSCANF(input[4], DCMD_DECIMAL, &dm_value[3]);
	PHYDM_SSCANF(input[5], DCMD_DECIMAL, &dm_value[4]);


	if (strcmp(input[1], help) == 0) {
		PHYDM_SNPRINTF((output + used, out_len - used, " 1 {fix_en} {codeword(Hex)}\n"));
		PHYDM_SNPRINTF((output + used, out_len - used, " 3 {Fix_training_num_en} {Per_beam_training_pkt_num} {Decision_holding_period}\n"));
		PHYDM_SNPRINTF((output + used, out_len - used, " 5 {0:show, 1:2G, 2:5G} {beam_num} {idxA(Hex)} {idxB(Hex)}\n"));
		PHYDM_SNPRINTF((output + used, out_len - used, " 7 {0:show, 1:2G, 2:5G} {total_beam_set_num}\n"));
		PHYDM_SNPRINTF((output + used, out_len - used, " 8 {0:show, 1:set} {RFU delay time(us)}\n"));

	} else if (dm_value[0] == 1) { /*fix beam pattern*/

		pdm_sat_table->fix_beam_pattern_en = dm_value[1];

		if (pdm_sat_table->fix_beam_pattern_en == 1) {

			PHYDM_SSCANF(input[3], DCMD_HEX, &dm_value[2]);
			pdm_sat_table->fix_beam_pattern_codeword = dm_value[2];

			if (pdm_sat_table->fix_beam_pattern_codeword  > (one << codeword_length)) {

				PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[ SmartAnt ] Codeword overflow, Current codeword is ((0x%x)), and should be less than ((%d))bit\n",
					pdm_sat_table->fix_beam_pattern_codeword, codeword_length));

				(pdm_sat_table->fix_beam_pattern_codeword) &= 0xffffff;

				PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[ SmartAnt ] Auto modify to (0x%x)\n", pdm_sat_table->fix_beam_pattern_codeword));
			}

			pdm_sat_table->update_beam_codeword = pdm_sat_table->fix_beam_pattern_codeword;

			/*---------------------------------------------------------*/
			PHYDM_SNPRINTF((output + used, out_len - used, "Fix Beam Pattern\n"));

			/*devide_num = (pdm_sat_table->rfu_protocol_type == 2) ? 8 : 4;*/

			for (i = 0; i <= (codeword_length - 1); i++) {
				beam_ctrl_signal = (bool)((pdm_sat_table->update_beam_codeword & BIT(i)) >> i);

				if (i == (codeword_length - 1)) {
					PHYDM_SNPRINTF((output + used, out_len - used, "%d]\n", beam_ctrl_signal));
					/**/
				} else if (i == 0) {
					PHYDM_SNPRINTF((output + used, out_len - used, "Send Codeword[1:%d] to RFU -> [%d", pdm_sat_table->rfu_codeword_total_bit_num, beam_ctrl_signal));
					/**/
				} else if ((i % devide_num) == (devide_num-1)) {
					PHYDM_SNPRINTF((output + used, out_len - used, "%d|", beam_ctrl_signal));
					/**/
				} else {
					PHYDM_SNPRINTF((output + used, out_len - used, "%d", beam_ctrl_signal));
					/**/
				}
			}
			/*---------------------------------------------------------*/

			odm_schedule_work_item(&pdm_sat_table->hl_smart_antenna_workitem);
		} else if (pdm_sat_table->fix_beam_pattern_en == 0)
			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] Smart Antenna: Enable\n"));

	} else if (dm_value[0] == 2) { /*set latch time*/

		pdm_sat_table->latch_time = dm_value[1];
		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[ SmartAnt ]  latch_time =0x%x\n", pdm_sat_table->latch_time));
	} else if (dm_value[0] == 3) {

		pdm_sat_table->fix_training_num_en = dm_value[1];

		if (pdm_sat_table->fix_training_num_en == 1) {
			pdm_sat_table->per_beam_training_pkt_num = (u8)dm_value[2];
			pdm_sat_table->decision_holding_period = (u8)dm_value[3];

			PHYDM_SNPRINTF((output + used, out_len - used, "[SmtAnt] Fix_train_en = (( %d )), train_pkt_num = (( %d )), holding_period = (( %d )),\n",
				pdm_sat_table->fix_training_num_en, pdm_sat_table->per_beam_training_pkt_num, pdm_sat_table->decision_holding_period));

		} else if (pdm_sat_table->fix_training_num_en == 0) {
			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ]  AUTO per_beam_training_pkt_num\n"));
			/**/
		}
	} else if (dm_value[0] == 5) {	/*set beam set table*/

		PHYDM_SSCANF(input[4], DCMD_HEX, &dm_value[3]);
		PHYDM_SSCANF(input[5], DCMD_HEX, &dm_value[4]);

		if (dm_value[1] == 1) { /*2G*/
			if (dm_value[2] < SUPPORT_BEAM_SET_PATTERN_NUM) {
				pdm_sat_table->rfu_codeword_table_2g[dm_value[2] ][0] = (u8)dm_value[3];
				pdm_sat_table->rfu_codeword_table_2g[dm_value[2] ][1] = (u8)dm_value[4];
				PHYDM_SNPRINTF((output + used, out_len - used, "[SmtAnt] Set 2G Table[%d] = [A:0x%x, B:0x%x]\n",dm_value[2], dm_value[3], dm_value[4]));
			}

		} else if (dm_value[1] == 2) { /*5G*/
			if (dm_value[2] < SUPPORT_BEAM_SET_PATTERN_NUM) {
				pdm_sat_table->rfu_codeword_table_5g[dm_value[2] ][0] = (u8)dm_value[3];
				pdm_sat_table->rfu_codeword_table_5g[dm_value[2] ][1] = (u8)dm_value[4];
				PHYDM_SNPRINTF((output + used, out_len - used, "[SmtAnt] Set5G Table[%d] = [A:0x%x, B:0x%x]\n",dm_value[2], dm_value[3], dm_value[4]));
			}
		} else  if (dm_value[1] == 0) {
			PHYDM_SNPRINTF((output + used, out_len - used, "[SmtAnt] 2G Beam Table==============>\n"));
			for (i = 0; i < pdm_sat_table->total_beam_set_num_2g; i++) {
				PHYDM_SNPRINTF((output + used, out_len - used, "2G Table[%d] = [A:0x%x, B:0x%x]\n",
					i, pdm_sat_table->rfu_codeword_table_2g[i][0], pdm_sat_table->rfu_codeword_table_2g[i][1]));
			}
			PHYDM_SNPRINTF((output + used, out_len - used, "[SmtAnt] 5G Beam Table==============>\n"));
			for (i = 0; i < pdm_sat_table->total_beam_set_num_5g; i++) {
				PHYDM_SNPRINTF((output + used, out_len - used, "5G Table[%d] = [A:0x%x, B:0x%x]\n",
					i, pdm_sat_table->rfu_codeword_table_5g[i][0], pdm_sat_table->rfu_codeword_table_5g[i][1]));
			}
		}

	} else if (dm_value[0] == 7) {

		if (dm_value[1] == 1) {

			pdm_sat_table->total_beam_set_num_2g = (u8)(dm_value[2]);
			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] total_beam_set_num_2g = ((%d))\n", pdm_sat_table->total_beam_set_num_2g));

		} else if (dm_value[1] == 2) {

			pdm_sat_table->total_beam_set_num_5g = (u8)(dm_value[2]);
			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] total_beam_set_num_5g = ((%d))\n", pdm_sat_table->total_beam_set_num_5g));
		} else if (dm_value[1] == 0) {
			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] Show total_beam_set_num{2g,5g} = {%d,%d}\n",
				pdm_sat_table->total_beam_set_num_2g, pdm_sat_table->total_beam_set_num_5g));
		}

	} else if (dm_value[0] == 8) {

			if (dm_value[1] == 1) {
				pdm_sat_table->rfu_protocol_delay_time = (u16)(dm_value[2]);
				PHYDM_SNPRINTF((output + used, out_len - used, "[SmtAnt] Set rfu_protocol_delay_time = ((%d))\n", pdm_sat_table->rfu_protocol_delay_time));
			} else if (dm_value[1] == 0) {
				PHYDM_SNPRINTF((output + used, out_len - used, "[SmtAnt] Read rfu_protocol_delay_time = ((%d))\n", pdm_sat_table->rfu_protocol_delay_time));
			}
	}

	*_used = used;
	*_out_len = out_len;
}

void
phydm_set_rfu_beam_pattern_type2(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT				*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct smt_ant_honbo	*pdm_sat_table = &(p_dm->dm_sat_table);

	if (p_dm->ant_div_type != HL_SW_SMART_ANT_TYPE2)
		return;

	PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Training beam_set index = (( 0x%x ))\n", pdm_sat_table->fast_training_beam_num));
	pdm_sat_table->update_beam_codeword = phydm_construct_hb_rfu_codeword_type2(p_dm, pdm_sat_table->fast_training_beam_num);

	odm_schedule_work_item(&pdm_sat_table->hl_smart_antenna_workitem);
}

void
phydm_fast_ant_training_hl_smart_antenna_type2(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT				*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct smt_ant_honbo	*pdm_sat_table = &(p_dm->dm_sat_table);
	struct phydm_fat_struct		*p_dm_fat_table	 = &(p_dm->dm_fat_table);
	struct _sw_antenna_switch_				*p_dm_swat_table = &p_dm->dm_swat_table;
	u32		codeword = 0;
	u8		i = 0, j=0;
	u8		avg_rssi_tmp;
	u8		avg_rssi_tmp_ma;
	u8		max_beam_ant_rssi = 0;
	u8		rssi_target_beam = 0, target_beam_max_rssi = 0;
	u8		evm1ss_target_beam = 0, evm2ss_target_beam = 0;
	u32		target_beam_max_evm1ss = 0, target_beam_max_evm2ss = 0;
	u32		beam_tmp;
	u8		per_beam_val_diff_tmp = 0, training_pkt_num_offset;
	u32		avg_evm2ss[2] = {0}, avg_evm2ss_sum = 0;
	u32		avg_evm1ss = 0;
	u32		beam_path_evm_2ss_cnt_all = 0; /*sum of all 2SS-pattern cnt*/
	u32		beam_path_evm_1ss_cnt_all = 0; /*sum of all 1SS-pattern cnt*/
	u8		decision_type;

	if (!p_dm->is_linked) {
		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[No Link!!!]\n"));

		if (p_dm_fat_table->is_become_linked) {

			pdm_sat_table->decision_holding_period = 0;
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Link->no Link\n"));
			p_dm_fat_table->fat_state = FAT_BEFORE_LINK_STATE;
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("change to (( %d )) FAT_state\n", p_dm_fat_table->fat_state));
			p_dm_fat_table->is_become_linked = p_dm->is_linked;
		}
		return;

	} else {
		if (!p_dm_fat_table->is_become_linked) {

			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[Linked !!!]\n"));

			p_dm_fat_table->fat_state = FAT_PREPARE_STATE;
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("change to (( %d )) FAT_state\n", p_dm_fat_table->fat_state));

			/*pdm_sat_table->fast_training_beam_num = 0;*/
			/*phydm_set_rfu_beam_pattern_type2(p_dm);*/

			p_dm_fat_table->is_become_linked = p_dm->is_linked;
		}
	}


	/*PHYDM_DBG(p_dm, DBG_ANT_DIV, ("HL Smart ant Training: state (( %d ))\n", p_dm_fat_table->fat_state));*/

	/* [DECISION STATE] */
	/*=======================================================================================*/
	if (p_dm_fat_table->fat_state == FAT_DECISION_STATE) {

		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[ 3. In Decision state]\n"));

		/*compute target beam in each antenna*/

		for (j = 0; j < (pdm_sat_table->total_beam_set_num); j++) {

			/*[Decision1: RSSI]-------------------------------------------------------------------*/
			if (pdm_sat_table->statistic_pkt_cnt[j] == 0) {	/*if new RSSI = 0 -> MA_RSSI-=2*/
				avg_rssi_tmp = pdm_sat_table->beam_set_avg_rssi_pre[j];
				avg_rssi_tmp = (avg_rssi_tmp >= 2) ? (avg_rssi_tmp - 2) : avg_rssi_tmp;
				avg_rssi_tmp_ma = avg_rssi_tmp;
			} else {
				avg_rssi_tmp = (u8)((pdm_sat_table->beam_set_rssi_avg_sum[j]) / (pdm_sat_table->statistic_pkt_cnt[j]));
				avg_rssi_tmp_ma = (avg_rssi_tmp + pdm_sat_table->beam_set_avg_rssi_pre[j]) >> 1;
			}

			pdm_sat_table->beam_set_avg_rssi_pre[j] = avg_rssi_tmp;

			if (avg_rssi_tmp > target_beam_max_rssi) {
				rssi_target_beam = j;
				target_beam_max_rssi = avg_rssi_tmp;
			}

			/*[Decision2: EVM 2ss]-------------------------------------------------------------------*/
			if (pdm_sat_table->beam_path_evm_2ss_cnt[j] != 0) {
				avg_evm2ss[0] =  pdm_sat_table->beam_path_evm_2ss_sum[j][0] / pdm_sat_table->beam_path_evm_2ss_cnt[j];
				avg_evm2ss[1] =  pdm_sat_table->beam_path_evm_2ss_sum[j][1] / pdm_sat_table->beam_path_evm_2ss_cnt[j];
				avg_evm2ss_sum = avg_evm2ss[0] + avg_evm2ss[1];
				beam_path_evm_2ss_cnt_all += pdm_sat_table->beam_path_evm_2ss_cnt[j];

				pdm_sat_table->beam_set_avg_evm_2ss_pre[j] = (u8)avg_evm2ss_sum;
			}

			if (avg_evm2ss_sum > target_beam_max_evm2ss) {
				evm2ss_target_beam = j;
				target_beam_max_evm2ss = avg_evm2ss_sum;
			}

			/*[Decision3: EVM 1ss]-------------------------------------------------------------------*/
			if (pdm_sat_table->beam_path_evm_1ss_cnt[j] != 0) {
				avg_evm1ss =  pdm_sat_table->beam_path_evm_1ss_sum[j] / pdm_sat_table->beam_path_evm_1ss_cnt[j];
				beam_path_evm_1ss_cnt_all += pdm_sat_table->beam_path_evm_1ss_cnt[j];

				pdm_sat_table->beam_set_avg_evm_1ss_pre[j] = (u8)avg_evm1ss;
			}

			if (avg_evm1ss > target_beam_max_evm1ss) {
				evm1ss_target_beam = j;
				target_beam_max_evm1ss = avg_evm1ss;
			}

			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Beam[%d] Pkt_cnt=(( %d )), avg{MA,rssi}={%d, %d}, EVM1={%d}, EVM2={%d, %d, %d}\n",
				j, pdm_sat_table->statistic_pkt_cnt[j], avg_rssi_tmp_ma, avg_rssi_tmp, avg_evm1ss, avg_evm2ss[0], avg_evm2ss[1], avg_evm2ss_sum));

			/*reset counter value*/
			pdm_sat_table->beam_set_rssi_avg_sum[j] = 0;
			pdm_sat_table->beam_path_rssi_sum[j][0] = 0;
			pdm_sat_table->beam_path_rssi_sum[j][1] = 0;
			pdm_sat_table->statistic_pkt_cnt[j] = 0;

			pdm_sat_table->beam_path_evm_2ss_sum[j][0] = 0;
			pdm_sat_table->beam_path_evm_2ss_sum[j][1] = 0;
			pdm_sat_table->beam_path_evm_2ss_cnt[j] = 0;

			pdm_sat_table->beam_path_evm_1ss_sum[j] = 0;
			pdm_sat_table->beam_path_evm_1ss_cnt[j] = 0;
		}

		/*[Joint Decision]-------------------------------------------------------------------*/
		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("--->1.[RSSI]      Target Beam(( %d )) RSSI_max=((%d))\n", rssi_target_beam, target_beam_max_rssi));
		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("--->2.[Evm2SS] Target Beam(( %d )) EVM2SS_max=((%d))\n", evm2ss_target_beam, target_beam_max_evm2ss));
		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("--->3.[Evm1SS] Target Beam(( %d )) EVM1SS_max=((%d))\n", evm1ss_target_beam, target_beam_max_evm1ss));

		if (target_beam_max_rssi <= 10) {
			pdm_sat_table->rx_idle_beam_set_idx = rssi_target_beam;
			decision_type = 1;
		} else {
			if (beam_path_evm_2ss_cnt_all != 0) {
				pdm_sat_table->rx_idle_beam_set_idx = evm2ss_target_beam;
				decision_type = 2;
			} else if (beam_path_evm_1ss_cnt_all != 0) {
				pdm_sat_table->rx_idle_beam_set_idx = evm1ss_target_beam;
				decision_type = 3;
			} else {
				pdm_sat_table->rx_idle_beam_set_idx = rssi_target_beam;
				decision_type = 1;
			}
		}

		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("---> Decision_type=((%d)), Final Target Beam(( %d ))\n", decision_type, pdm_sat_table->rx_idle_beam_set_idx));

		/*Calculate packet counter offset*/
		for (j = 0; j < (pdm_sat_table->total_beam_set_num); j++) {

			if (decision_type == 1) {
				per_beam_val_diff_tmp = target_beam_max_rssi - pdm_sat_table->beam_set_avg_rssi_pre[j];

			} else if (decision_type == 2) {
				per_beam_val_diff_tmp = ((u8)target_beam_max_evm2ss - pdm_sat_table->beam_set_avg_evm_2ss_pre[j]) >> 1;
			} else if (decision_type == 3) {
				per_beam_val_diff_tmp = (u8)target_beam_max_evm1ss - pdm_sat_table->beam_set_avg_evm_1ss_pre[j];
			}
			pdm_sat_table->beam_set_train_val_diff[j] = per_beam_val_diff_tmp;
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Beam_Set[%d]: diff= ((%d))\n", j, per_beam_val_diff_tmp));
		}

		/*set beam in each antenna*/
		phydm_update_rx_idle_beam_type2(p_dm);
		p_dm_fat_table->fat_state = FAT_PREPARE_STATE;

	}
	/* [TRAINING STATE] */
	else if (p_dm_fat_table->fat_state == FAT_TRAINING_STATE) {
		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[ 2. In Training state]\n"));

		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("curr_beam_idx = (( %d )), pre_beam_idx = (( %d ))\n",
			pdm_sat_table->fast_training_beam_num, pdm_sat_table->pre_fast_training_beam_num));

		if (pdm_sat_table->fast_training_beam_num > pdm_sat_table->pre_fast_training_beam_num)

			pdm_sat_table->force_update_beam_en = 0;

		else {

			pdm_sat_table->force_update_beam_en = 1;

			pdm_sat_table->pkt_counter = 0;
			beam_tmp = pdm_sat_table->fast_training_beam_num;
			if (pdm_sat_table->fast_training_beam_num >= ((u32)pdm_sat_table->total_beam_set_num - 1)) {

				PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[Timeout Update]  Beam_num (( %d )) -> (( decision ))\n", pdm_sat_table->fast_training_beam_num));
				p_dm_fat_table->fat_state = FAT_DECISION_STATE;
				phydm_fast_ant_training_hl_smart_antenna_type2(p_dm);

			} else {
				pdm_sat_table->fast_training_beam_num++;

				PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[Timeout Update]  Beam_num (( %d )) -> (( %d ))\n", beam_tmp, pdm_sat_table->fast_training_beam_num));
				phydm_set_rfu_beam_pattern_type2(p_dm);
				p_dm_fat_table->fat_state = FAT_TRAINING_STATE;

			}
		}
		pdm_sat_table->pre_fast_training_beam_num = pdm_sat_table->fast_training_beam_num;
		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Update Pre_Beam =(( %d ))\n", pdm_sat_table->pre_fast_training_beam_num));
	}
	/*  [Prepare state] */
	/*=======================================================================================*/
	else if (p_dm_fat_table->fat_state == FAT_PREPARE_STATE) {

		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("\n\n[ 1. In Prepare state]\n"));

		if (p_dm->pre_traffic_load == (p_dm->traffic_load)) {
			if (pdm_sat_table->decision_holding_period != 0) {
				PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Holding_period = (( %d )), return!!!\n", pdm_sat_table->decision_holding_period));
				pdm_sat_table->decision_holding_period--;
				return;
			}
		}

		/* Set training packet number*/
		if (pdm_sat_table->fix_training_num_en == 0) {

			switch (p_dm->traffic_load) {

			case TRAFFIC_HIGH:
				pdm_sat_table->per_beam_training_pkt_num = 8;
				pdm_sat_table->decision_holding_period = 2;
				break;
			case TRAFFIC_MID:
				pdm_sat_table->per_beam_training_pkt_num = 6;
				pdm_sat_table->decision_holding_period = 3;
				break;
			case TRAFFIC_LOW:
				pdm_sat_table->per_beam_training_pkt_num = 3; /*ping 60000*/
				pdm_sat_table->decision_holding_period = 4;
				break;
			case TRAFFIC_ULTRA_LOW:
				pdm_sat_table->per_beam_training_pkt_num = 1;
				pdm_sat_table->decision_holding_period = 6;
				break;
			default:
				break;
			}
		}

		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("TrafficLoad = (( %d )), Fix_beam = (( %d )), per_beam_training_pkt_num = (( %d )), decision_holding_period = ((%d))\n",
			p_dm->traffic_load, pdm_sat_table->fix_training_num_en, pdm_sat_table->per_beam_training_pkt_num, pdm_sat_table->decision_holding_period));

		/*Beam_set number*/
		if (*p_dm->p_band_type == ODM_BAND_5G) {
			pdm_sat_table->total_beam_set_num = pdm_sat_table->total_beam_set_num_5g;
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("5G beam_set num = ((%d))\n", pdm_sat_table->total_beam_set_num));
		} else {
			pdm_sat_table->total_beam_set_num = pdm_sat_table->total_beam_set_num_2g;
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("2G beam_set num = ((%d))\n", pdm_sat_table->total_beam_set_num));
		}

		for (j = 0; j < (pdm_sat_table->total_beam_set_num); j++) {

			training_pkt_num_offset = pdm_sat_table->beam_set_train_val_diff[j];

			if ((pdm_sat_table->per_beam_training_pkt_num) > training_pkt_num_offset)
				pdm_sat_table->beam_set_train_cnt[j] = pdm_sat_table->per_beam_training_pkt_num - training_pkt_num_offset;
			else
				pdm_sat_table->beam_set_train_cnt[j] = 1;

			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Beam_Set[ %d ] training_pkt_offset = ((%d)), training_pkt_num = ((%d))\n",
				j, pdm_sat_table->beam_set_train_val_diff[j], pdm_sat_table->beam_set_train_cnt[j]));
		}

		pdm_sat_table->pre_beacon_counter = pdm_sat_table->beacon_counter;
		pdm_sat_table->update_beam_idx = 0;
		pdm_sat_table->pkt_counter = 0;

		pdm_sat_table->fast_training_beam_num = 0;
		phydm_set_rfu_beam_pattern_type2(p_dm);
		pdm_sat_table->pre_fast_training_beam_num = pdm_sat_table->fast_training_beam_num;
		p_dm_fat_table->fat_state = FAT_TRAINING_STATE;
	}

}

void
phydm_process_rssi_for_hb_smtant_type2(
	void		*p_dm_void,
	void		*p_phy_info_void,
	void		*p_pkt_info_void,
	u8		rssi_avg
)
{
	struct PHY_DM_STRUCT				*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct phydm_phyinfo_struct			*p_phy_info = (struct phydm_phyinfo_struct *)p_phy_info_void;
	struct phydm_perpkt_info_struct				*p_pktinfo = (struct phydm_perpkt_info_struct *)p_pkt_info_void;
	struct phydm_fat_struct		*p_dm_fat_table = &p_dm->dm_fat_table;
	struct smt_ant_honbo	*pdm_sat_table = &(p_dm->dm_sat_table);
	u8		train_pkt_number;
	u32		beam_tmp;
	u8		is_cck_rate;
	u8		rx_power_ant0 = p_phy_info->rx_mimo_signal_strength[0];
	u8		rx_power_ant1 = p_phy_info->rx_mimo_signal_strength[1];
	u8		rx_evm_ant0 = p_phy_info->rx_mimo_evm_dbm[0];
	u8		rx_evm_ant1 = p_phy_info->rx_mimo_evm_dbm[1];
	u8		rate_ss = phydm_rate_to_num_ss(p_dm, p_pktinfo->data_rate);

	is_cck_rate = (p_pktinfo->data_rate <= ODM_RATE11M) ? true : false;


	/*[Beacon]*/
	if (p_pktinfo->is_packet_beacon) {

		pdm_sat_table->beacon_counter++;
		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("MatchBSSID_beacon_counter = ((%d))\n", pdm_sat_table->beacon_counter));

		if (pdm_sat_table->beacon_counter >= pdm_sat_table->pre_beacon_counter + 2) {

			pdm_sat_table->update_beam_idx++;
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("pre_beacon_counter = ((%d)), pkt_counter = ((%d)), update_beam_idx = ((%d))\n",
				pdm_sat_table->pre_beacon_counter, pdm_sat_table->pkt_counter, pdm_sat_table->update_beam_idx));

			pdm_sat_table->pre_beacon_counter = pdm_sat_table->beacon_counter;
			pdm_sat_table->pkt_counter = 0;
		}
	}
	/*[data]*/
	else if (p_pktinfo->is_packet_to_self) {

		if (pdm_sat_table->pkt_skip_statistic_en == 0) {

			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("ID[%d] pkt_cnt=((%d)): Beam_set = ((%d)), RSSI{A,B,avg} = {%d, %d, %d}\n",
				p_pktinfo->station_id, pdm_sat_table->pkt_counter,  pdm_sat_table->fast_training_beam_num, rx_power_ant0, rx_power_ant1, rssi_avg));

			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Rate_ss = ((%d)), EVM{A,B} = {%d, %d}, RX Rate =", rate_ss,  rx_evm_ant0, rx_evm_ant1));
			phydm_print_rate(p_dm, p_dm->rx_rate, DBG_ANT_DIV);


			if (pdm_sat_table->pkt_counter >= 1)  /*packet skip count*/
			{
				pdm_sat_table->beam_set_rssi_avg_sum[pdm_sat_table->fast_training_beam_num] += rssi_avg;
				pdm_sat_table->statistic_pkt_cnt[pdm_sat_table->fast_training_beam_num]++;

				pdm_sat_table->beam_path_rssi_sum[pdm_sat_table->fast_training_beam_num][0] += rx_power_ant0;
				pdm_sat_table->beam_path_rssi_sum[pdm_sat_table->fast_training_beam_num][1] += rx_power_ant1;

				if (rate_ss == 2) {
					pdm_sat_table->beam_path_evm_2ss_sum[pdm_sat_table->fast_training_beam_num][0] += rx_evm_ant0;
					pdm_sat_table->beam_path_evm_2ss_sum[pdm_sat_table->fast_training_beam_num][1] += rx_evm_ant1;
					pdm_sat_table->beam_path_evm_2ss_cnt[pdm_sat_table->fast_training_beam_num]++;
				} else {
					pdm_sat_table->beam_path_evm_1ss_sum[pdm_sat_table->fast_training_beam_num] += rx_evm_ant0;
					pdm_sat_table->beam_path_evm_1ss_cnt[pdm_sat_table->fast_training_beam_num]++;
				}
			}

			pdm_sat_table->pkt_counter++;

			train_pkt_number = pdm_sat_table->beam_set_train_cnt[pdm_sat_table->fast_training_beam_num];

			if (pdm_sat_table->pkt_counter >= train_pkt_number) {

				pdm_sat_table->update_beam_idx++;
				PHYDM_DBG(p_dm, DBG_ANT_DIV, ("pre_beacon_counter = ((%d)), Update_new_beam = ((%d))\n",
					pdm_sat_table->pre_beacon_counter, pdm_sat_table->update_beam_idx));

				pdm_sat_table->pre_beacon_counter = pdm_sat_table->beacon_counter;
				pdm_sat_table->pkt_counter = 0;
			}
		}
	}

	if (pdm_sat_table->update_beam_idx > 0) {

		pdm_sat_table->update_beam_idx = 0;

		if (pdm_sat_table->fast_training_beam_num >= ((u32)pdm_sat_table->total_beam_set_num - 1)) {

			p_dm_fat_table->fat_state = FAT_DECISION_STATE;

			odm_schedule_work_item(&pdm_sat_table->hl_smart_antenna_decision_workitem);
		} else {
			beam_tmp = pdm_sat_table->fast_training_beam_num;
			pdm_sat_table->fast_training_beam_num++;
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Update Beam_num (( %d )) -> (( %d ))\n", beam_tmp, pdm_sat_table->fast_training_beam_num));
			phydm_set_rfu_beam_pattern_type2(p_dm);
			pdm_sat_table->pre_fast_training_beam_num = pdm_sat_table->fast_training_beam_num;

			p_dm_fat_table->fat_state = FAT_TRAINING_STATE;
		}
	}

}
#endif

#if (defined(CONFIG_HL_SMART_ANTENNA_TYPE1))

void
phydm_hl_smart_ant_type1_init_8821a(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct smt_ant_honbo			*pdm_sat_table = &(p_dm->dm_sat_table);
	struct phydm_fat_struct			*p_dm_fat_table = &p_dm->dm_fat_table;
	u32			value32;

	PHYDM_DBG(p_dm, DBG_ANT_DIV, ("***8821A SmartAnt_Init => ant_div_type=[Hong-Lin Smart ant Type1]\n"));

	/*GPIO setting*/
	odm_set_mac_reg(p_dm, 0x64, BIT(18), 0);
	odm_set_mac_reg(p_dm, 0x44, BIT(27) | BIT(26), 0);
	odm_set_mac_reg(p_dm, 0x44, BIT(19) | BIT(18), 0x3);	/*enable_output for P_GPIO[3:2]*/
	/*odm_set_mac_reg(p_dm, 0x44, BIT(11)|BIT(10), 0);*/ /*output value*/
	odm_set_mac_reg(p_dm, 0x40, BIT(1) | BIT(0), 0);		/*GPIO function*/

	/*Hong_lin smart antenna HW setting*/
	pdm_sat_table->rfu_codeword_total_bit_num  = 24;/*max=32*/
	pdm_sat_table->rfu_each_ant_bit_num = 4;
	pdm_sat_table->beam_patten_num_each_ant = 4;

	pdm_sat_table->latch_time = 100; /*mu sec*/
	pdm_sat_table->pkt_skip_statistic_en = 0;

	pdm_sat_table->ant_num = 1;/*max=8*/
	pdm_sat_table->ant_num_total = NUM_ANTENNA_8821A;
	pdm_sat_table->first_train_ant = MAIN_ANT;

	pdm_sat_table->rfu_codeword_table[0] = 0x0;
	pdm_sat_table->rfu_codeword_table[1] = 0x4;
	pdm_sat_table->rfu_codeword_table[2] = 0x8;
	pdm_sat_table->rfu_codeword_table[3] = 0xc;

	pdm_sat_table->rfu_codeword_table_5g[0] = 0x1;
	pdm_sat_table->rfu_codeword_table_5g[1] = 0x2;
	pdm_sat_table->rfu_codeword_table_5g[2] = 0x4;
	pdm_sat_table->rfu_codeword_table_5g[3] = 0x8;

	pdm_sat_table->fix_beam_pattern_en  = 0;
	pdm_sat_table->decision_holding_period = 0;

	/*beam training setting*/
	pdm_sat_table->pkt_counter = 0;
	pdm_sat_table->per_beam_training_pkt_num = 10;

	/*set default beam*/
	pdm_sat_table->fast_training_beam_num = 0;
	pdm_sat_table->pre_fast_training_beam_num = pdm_sat_table->fast_training_beam_num;
	phydm_set_all_ant_same_beam_num(p_dm);

	p_dm_fat_table->fat_state = FAT_BEFORE_LINK_STATE;

	odm_set_bb_reg(p_dm, 0xCA4, MASKDWORD, 0x01000100);
	odm_set_bb_reg(p_dm, 0xCA8, MASKDWORD, 0x01000100);

	/*[BB] FAT setting*/
	odm_set_bb_reg(p_dm, 0xc08, BIT(18) | BIT(17) | BIT(16), pdm_sat_table->ant_num);
	odm_set_bb_reg(p_dm, 0xc08, BIT(31), 0); /*increase ant num every FAT period 0:+1, 1+2*/
	odm_set_bb_reg(p_dm, 0x8c4, BIT(2) | BIT(1), 1); /*change cca antenna timming threshold if no CCA occurred: 0:200ms / 1:100ms / 2:no use / 3: 300*/
	odm_set_bb_reg(p_dm, 0x8c4, BIT(0), 1); /*FAT_watchdog_en*/

	value32 = odm_get_mac_reg(p_dm,  0x7B4, MASKDWORD);
	odm_set_mac_reg(p_dm, 0x7b4, MASKDWORD, value32 | (BIT(16) | BIT(17)));	/*Reg7B4[16]=1 enable antenna training */
	/*Reg7B4[17]=1 enable  match MAC addr*/
	odm_set_mac_reg(p_dm, 0x7b4, 0xFFFF, 0);/*Match MAC ADDR*/
	odm_set_mac_reg(p_dm, 0x7b0, MASKDWORD, 0);

}

u32
phydm_construct_hl_beam_codeword(
	void		*p_dm_void,
	u32		*beam_pattern_idx,
	u32		ant_num
)
{
	struct PHY_DM_STRUCT	*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct smt_ant_honbo		*pdm_sat_table = &(p_dm->dm_sat_table);
	u32		codeword = 0;
	u32		data_tmp;
	u32		i;
	u32		break_counter = 0;

	if (ant_num < 8) {
		for (i = 0; i < (pdm_sat_table->ant_num_total); i++) {
			/*PHYDM_DBG(p_dm,DBG_ANT_DIV, ("beam_pattern_num[%x] = %x\n",i,beam_pattern_num[i] ));*/
			if ((i < (pdm_sat_table->first_train_ant - 1)) || (break_counter >= (pdm_sat_table->ant_num))) {
				data_tmp = 0;
				/**/
			} else {

				break_counter++;

				if (beam_pattern_idx[i] == 0) {

					if (*p_dm->p_band_type == ODM_BAND_5G)
						data_tmp = pdm_sat_table->rfu_codeword_table_5g[0];
					else
						data_tmp = pdm_sat_table->rfu_codeword_table[0];

				} else if (beam_pattern_idx[i] == 1) {


					if (*p_dm->p_band_type == ODM_BAND_5G)
						data_tmp = pdm_sat_table->rfu_codeword_table_5g[1];
					else
						data_tmp = pdm_sat_table->rfu_codeword_table[1];

				} else if (beam_pattern_idx[i] == 2) {

					if (*p_dm->p_band_type == ODM_BAND_5G)
						data_tmp = pdm_sat_table->rfu_codeword_table_5g[2];
					else
						data_tmp = pdm_sat_table->rfu_codeword_table[2];

				} else if (beam_pattern_idx[i] == 3) {

					if (*p_dm->p_band_type == ODM_BAND_5G)
						data_tmp = pdm_sat_table->rfu_codeword_table_5g[3];
					else
						data_tmp = pdm_sat_table->rfu_codeword_table[3];
				}
			}


			codeword |= (data_tmp << (i * 4));

		}
	}

	return codeword;
}

void
phydm_update_beam_pattern(
	void		*p_dm_void,
	u32		codeword,
	u32		codeword_length
)
{
	struct PHY_DM_STRUCT		*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct smt_ant_honbo			*pdm_sat_table = &(p_dm->dm_sat_table);
	u8			i;
	bool			beam_ctrl_signal;
	u32			one = 0x1;
	u32			reg44_tmp_p, reg44_tmp_n, reg44_ori;
	u8			devide_num = 4;

	PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[ SmartAnt ] Set Beam Pattern =0x%x\n", codeword));

	reg44_ori = odm_get_mac_reg(p_dm, 0x44, MASKDWORD);
	reg44_tmp_p = reg44_ori;
	/*PHYDM_DBG(p_dm, DBG_ANT_DIV, ("reg44_ori =0x%x\n", reg44_ori));*/

	devide_num = (pdm_sat_table->rfu_protocol_type == 2) ? 6 : 4;

	for (i = 0; i <= (codeword_length - 1); i++) {
		beam_ctrl_signal = (bool)((codeword & BIT(i)) >> i);

		if (p_dm->debug_components & DBG_ANT_DIV) {

			if (i == (codeword_length - 1)) {
				dbg_print("%d ]\n", beam_ctrl_signal);
				/**/
			} else if (i == 0) {
				dbg_print("Send codeword[1:%d] ---> [ %d ", codeword_length, beam_ctrl_signal);
				/**/
			} else if ((i % devide_num) == (devide_num-1)) {
				dbg_print("%d  |  ", beam_ctrl_signal);
				/**/
			} else {
				dbg_print("%d ", beam_ctrl_signal);
				/**/
			}
		}
	}
}

void
phydm_update_rx_idle_beam(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct phydm_fat_struct			*p_dm_fat_table = &p_dm->dm_fat_table;
	struct smt_ant_honbo			*pdm_sat_table = &(p_dm->dm_sat_table);
	u32			i;

	pdm_sat_table->update_beam_codeword = phydm_construct_hl_beam_codeword(p_dm, &(pdm_sat_table->rx_idle_beam[0]), pdm_sat_table->ant_num);
	PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Set target beam_pattern codeword = (( 0x%x ))\n", pdm_sat_table->update_beam_codeword));

	for (i = 0; i < (pdm_sat_table->ant_num); i++) {
		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[ Update Rx-Idle-Beam ] RxIdleBeam[%d] =%d\n", i, pdm_sat_table->rx_idle_beam[i]));
		/**/
	}

	odm_schedule_work_item(&pdm_sat_table->hl_smart_antenna_workitem);

	pdm_sat_table->pre_codeword = pdm_sat_table->update_beam_codeword;
}

void
phydm_hl_smart_ant_debug(
	void		*p_dm_void,
	char		input[][16],
	u32		*_used,
	char		*output,
	u32		*_out_len,
	u32		input_num
)
{
	struct PHY_DM_STRUCT		*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct smt_ant_honbo			*pdm_sat_table = &(p_dm->dm_sat_table);
	u32			used = *_used;
	u32			out_len = *_out_len;
	u32			one = 0x1;
	u32			codeword_length = pdm_sat_table->rfu_codeword_total_bit_num;
	u32			beam_ctrl_signal, i;
	u8			devide_num = 4;

	if (dm_value[0] == 1) { /*fix beam pattern*/

		pdm_sat_table->fix_beam_pattern_en = dm_value[1];

		if (pdm_sat_table->fix_beam_pattern_en == 1) {

			pdm_sat_table->fix_beam_pattern_codeword = dm_value[2];

			if (pdm_sat_table->fix_beam_pattern_codeword  > (one << codeword_length)) {

				PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[ SmartAnt ] Codeword overflow, Current codeword is ((0x%x)), and should be less than ((%d))bit\n",
					pdm_sat_table->fix_beam_pattern_codeword, codeword_length));

				(pdm_sat_table->fix_beam_pattern_codeword) &= 0xffffff;

				PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[ SmartAnt ] Auto modify to (0x%x)\n", pdm_sat_table->fix_beam_pattern_codeword));
			}

			pdm_sat_table->update_beam_codeword = pdm_sat_table->fix_beam_pattern_codeword;

			/*---------------------------------------------------------*/
			PHYDM_SNPRINTF((output + used, out_len - used, "Fix Beam Pattern\n"));

			devide_num = (pdm_sat_table->rfu_protocol_type == 2) ? 6 : 4;

			for (i = 0; i <= (codeword_length - 1); i++) {
				beam_ctrl_signal = (bool)((pdm_sat_table->update_beam_codeword & BIT(i)) >> i);

				if (i == (codeword_length - 1)) {
					PHYDM_SNPRINTF((output + used, out_len - used, "%d]\n", beam_ctrl_signal));
					/**/
				} else if (i == 0) {
					PHYDM_SNPRINTF((output + used, out_len - used, "Send Codeword[1:24] to RFU -> [%d", beam_ctrl_signal));
					/**/
				} else if ((i % devide_num) == (devide_num-1)) {
					PHYDM_SNPRINTF((output + used, out_len - used, "%d|", beam_ctrl_signal));
					/**/
				} else {
					PHYDM_SNPRINTF((output + used, out_len - used, "%d", beam_ctrl_signal));
					/**/
				}
			}
			/*---------------------------------------------------------*/


			odm_schedule_work_item(&pdm_sat_table->hl_smart_antenna_workitem);
		} else if (pdm_sat_table->fix_beam_pattern_en == 0)
			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] Smart Antenna: Enable\n"));
	} else if (dm_value[0] == 2) { /*set latch time*/

		pdm_sat_table->latch_time = dm_value[1];
		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[ SmartAnt ]  latch_time =0x%x\n", pdm_sat_table->latch_time));
	} else if (dm_value[0] == 3) {

		pdm_sat_table->fix_training_num_en = dm_value[1];

		if (pdm_sat_table->fix_training_num_en == 1) {
			pdm_sat_table->per_beam_training_pkt_num = (u8)dm_value[2];
			pdm_sat_table->decision_holding_period = (u8)dm_value[3];

			PHYDM_SNPRINTF((output + used, out_len - used, "[SmartAnt][Dbg] Fix_train_en = (( %d )), train_pkt_num = (( %d )), holding_period = (( %d )),\n",
				pdm_sat_table->fix_training_num_en, pdm_sat_table->per_beam_training_pkt_num, pdm_sat_table->decision_holding_period));

		} else if (pdm_sat_table->fix_training_num_en == 0) {
			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ]  AUTO per_beam_training_pkt_num\n"));
			/**/
		}
	} else if (dm_value[0] == 4) {

		if (dm_value[1] == 1) {
			pdm_sat_table->ant_num = 1;
			pdm_sat_table->first_train_ant = MAIN_ANT;

		} else if (dm_value[1] == 2) {
			pdm_sat_table->ant_num = 1;
			pdm_sat_table->first_train_ant = AUX_ANT;

		} else if (dm_value[1] == 3) {
			pdm_sat_table->ant_num = 2;
			pdm_sat_table->first_train_ant = MAIN_ANT;
		}

		PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ]  Set ant Num = (( %d )), first_train_ant = (( %d ))\n",
			pdm_sat_table->ant_num, (pdm_sat_table->first_train_ant - 1)));
	} else if (dm_value[0] == 5) {

		if (dm_value[1] <= 3) {
			pdm_sat_table->rfu_codeword_table[dm_value[1]] = dm_value[2];
			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] Set Beam_2G: (( %d )), RFU codeword table = (( 0x%x ))\n",
					dm_value[1], dm_value[2]));
		} else {
			for (i = 0; i < 4; i++) {
				PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] Show Beam_2G: (( %d )), RFU codeword table = (( 0x%x ))\n",
					i, pdm_sat_table->rfu_codeword_table[i]));
			}
		}
	} else if (dm_value[0] == 6) {

		if (dm_value[1] <= 3) {
			pdm_sat_table->rfu_codeword_table_5g[dm_value[1]] = dm_value[2];
			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] Set Beam_5G: (( %d )), RFU codeword table = (( 0x%x ))\n",
					dm_value[1], dm_value[2]));
		} else {
			for (i = 0; i < 4; i++) {
				PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] Show Beam_5G: (( %d )), RFU codeword table = (( 0x%x ))\n",
					i, pdm_sat_table->rfu_codeword_table_5g[i]));
			}
		}
	} else if (dm_value[0] == 7) {

		if (dm_value[1] <= 4) {

			pdm_sat_table->beam_patten_num_each_ant = dm_value[1];
			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] Set Beam number = (( %d ))\n",
				pdm_sat_table->beam_patten_num_each_ant));
		} else {

			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] Show Beam number = (( %d ))\n",
				pdm_sat_table->beam_patten_num_each_ant));
		}
	}
	*_used = used;
	*_out_len = out_len;
}


void
phydm_set_all_ant_same_beam_num(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct smt_ant_honbo			*pdm_sat_table = &(p_dm->dm_sat_table);

	if (p_dm->ant_div_type == HL_SW_SMART_ANT_TYPE1) { /*2ant for 8821A*/

		pdm_sat_table->rx_idle_beam[0] = pdm_sat_table->fast_training_beam_num;
		pdm_sat_table->rx_idle_beam[1] = pdm_sat_table->fast_training_beam_num;
	}

	pdm_sat_table->update_beam_codeword = phydm_construct_hl_beam_codeword(p_dm, &(pdm_sat_table->rx_idle_beam[0]), pdm_sat_table->ant_num);

	PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[ SmartAnt ] Set all ant beam_pattern: codeword = (( 0x%x ))\n", pdm_sat_table->update_beam_codeword));

	odm_schedule_work_item(&pdm_sat_table->hl_smart_antenna_workitem);
}

void
odm_fast_ant_training_hl_smart_antenna_type1(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT	*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct smt_ant_honbo		*pdm_sat_table = &(p_dm->dm_sat_table);
	struct phydm_fat_struct		*p_dm_fat_table	 = &(p_dm->dm_fat_table);
	struct _sw_antenna_switch_		*p_dm_swat_table = &p_dm->dm_swat_table;
	u32		codeword = 0, i, j;
	u32		target_ant;
	u32		avg_rssi_tmp, avg_rssi_tmp_ma;
	u32		target_ant_beam_max_rssi[SUPPORT_RF_PATH_NUM] = {0};
	u32		max_beam_ant_rssi = 0;
	u32		target_ant_beam[SUPPORT_RF_PATH_NUM] = {0};
	u32		beam_tmp;
	u8		next_ant;
	u32		rssi_sorting_seq[SUPPORT_BEAM_PATTERN_NUM] = {0};
	u32		rank_idx_seq[SUPPORT_BEAM_PATTERN_NUM] = {0};
	u32		rank_idx_out[SUPPORT_BEAM_PATTERN_NUM] = {0};
	u8		per_beam_rssi_diff_tmp = 0, training_pkt_num_offset;
	u32		break_counter = 0;
	u32		used_ant;


	if (!p_dm->is_linked) {
		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[No Link!!!]\n"));

		if (p_dm_fat_table->is_become_linked) {

			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Link->no Link\n"));
			p_dm_fat_table->fat_state = FAT_BEFORE_LINK_STATE;
			odm_ant_div_on_off(p_dm, ANTDIV_OFF);
			odm_tx_by_tx_desc_or_reg(p_dm, TX_BY_REG);
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("change to (( %d )) FAT_state\n", p_dm_fat_table->fat_state));

			p_dm_fat_table->is_become_linked = p_dm->is_linked;
		}
		return;

	} else {
		if (!p_dm_fat_table->is_become_linked) {

			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[Linked !!!]\n"));

			p_dm_fat_table->fat_state = FAT_PREPARE_STATE;
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("change to (( %d )) FAT_state\n", p_dm_fat_table->fat_state));

			/*pdm_sat_table->fast_training_beam_num = 0;*/
			/*phydm_set_all_ant_same_beam_num(p_dm);*/

			p_dm_fat_table->is_become_linked = p_dm->is_linked;
		}
	}

	if (!*p_dm_fat_table->p_force_tx_ant_by_desc) {
		if (p_dm->is_one_entry_only)
			odm_tx_by_tx_desc_or_reg(p_dm, TX_BY_REG);
		else
			odm_tx_by_tx_desc_or_reg(p_dm, TX_BY_DESC);
	}

	/*PHYDM_DBG(p_dm, DBG_ANT_DIV, ("HL Smart ant Training: state (( %d ))\n", p_dm_fat_table->fat_state));*/

	/* [DECISION STATE] */
	/*=======================================================================================*/
	if (p_dm_fat_table->fat_state == FAT_DECISION_STATE) {

		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[ 3. In Decision state]\n"));
		phydm_fast_training_enable(p_dm, FAT_OFF);

		break_counter = 0;
		/*compute target beam in each antenna*/
		for (i = (pdm_sat_table->first_train_ant - 1); i < pdm_sat_table->ant_num_total; i++) {
			for (j = 0; j < (pdm_sat_table->beam_patten_num_each_ant); j++) {

				if (pdm_sat_table->pkt_rssi_cnt[i][j] == 0) {
					avg_rssi_tmp = pdm_sat_table->pkt_rssi_pre[i][j];
					avg_rssi_tmp = (avg_rssi_tmp >= 2) ? (avg_rssi_tmp - 2) : avg_rssi_tmp;
					avg_rssi_tmp_ma = avg_rssi_tmp;
				} else {
					avg_rssi_tmp = (pdm_sat_table->pkt_rssi_sum[i][j]) / (pdm_sat_table->pkt_rssi_cnt[i][j]);
					avg_rssi_tmp_ma = (avg_rssi_tmp + pdm_sat_table->pkt_rssi_pre[i][j]) >> 1;
				}

				rssi_sorting_seq[j] = avg_rssi_tmp;
				pdm_sat_table->pkt_rssi_pre[i][j] = avg_rssi_tmp;

				PHYDM_DBG(p_dm, DBG_ANT_DIV, ("ant[%d], Beam[%d]: pkt_cnt=(( %d )), avg_rssi_MA=(( %d )), avg_rssi=(( %d ))\n",
					i, j, pdm_sat_table->pkt_rssi_cnt[i][j], avg_rssi_tmp_ma, avg_rssi_tmp));

				if (avg_rssi_tmp > target_ant_beam_max_rssi[i]) {
					target_ant_beam[i] = j;
					target_ant_beam_max_rssi[i] = avg_rssi_tmp;
				}

				/*reset counter value*/
				pdm_sat_table->pkt_rssi_sum[i][j] = 0;
				pdm_sat_table->pkt_rssi_cnt[i][j] = 0;

			}
			pdm_sat_table->rx_idle_beam[i] = target_ant_beam[i];
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("---------> Target of ant[%d]: Beam_num-(( %d )) RSSI= ((%d))\n",
				i,  target_ant_beam[i], target_ant_beam_max_rssi[i]));

			/*sorting*/
			/*
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[Pre]rssi_sorting_seq = [%d, %d, %d, %d]\n", rssi_sorting_seq[0], rssi_sorting_seq[1], rssi_sorting_seq[2], rssi_sorting_seq[3]));
			*/

			/*phydm_seq_sorting(p_dm, &rssi_sorting_seq[0], &rank_idx_seq[0], &rank_idx_out[0], SUPPORT_BEAM_PATTERN_NUM);*/

			/*
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[Post]rssi_sorting_seq = [%d, %d, %d, %d]\n", rssi_sorting_seq[0], rssi_sorting_seq[1], rssi_sorting_seq[2], rssi_sorting_seq[3]));
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[Post]rank_idx_seq = [%d, %d, %d, %d]\n", rank_idx_seq[0], rank_idx_seq[1], rank_idx_seq[2], rank_idx_seq[3]));
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[Post]rank_idx_out = [%d, %d, %d, %d]\n", rank_idx_out[0], rank_idx_out[1], rank_idx_out[2], rank_idx_out[3]));
			*/

			if (target_ant_beam_max_rssi[i] > max_beam_ant_rssi) {
				target_ant = i;
				max_beam_ant_rssi = target_ant_beam_max_rssi[i];
				/*PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Target of ant = (( %d )) max_beam_ant_rssi = (( %d ))\n",
					target_ant,  max_beam_ant_rssi));*/
			}
			break_counter++;
			if (break_counter >= (pdm_sat_table->ant_num))
				break;
		}

#ifdef CONFIG_FAT_PATCH
		break_counter = 0;
		for (i = (pdm_sat_table->first_train_ant - 1); i < pdm_sat_table->ant_num_total; i++) {
			for (j = 0; j < (pdm_sat_table->beam_patten_num_each_ant); j++) {

				per_beam_rssi_diff_tmp = (u8)(max_beam_ant_rssi - pdm_sat_table->pkt_rssi_pre[i][j]);
				pdm_sat_table->beam_train_rssi_diff[i][j] = per_beam_rssi_diff_tmp;

				PHYDM_DBG(p_dm, DBG_ANT_DIV, ("ant[%d], Beam[%d]: RSSI_diff= ((%d))\n",
						i,  j, per_beam_rssi_diff_tmp));
			}
			break_counter++;
			if (break_counter >= (pdm_sat_table->ant_num))
				break;
		}
#endif

		if (target_ant == 0)
			target_ant = MAIN_ANT;
		else if (target_ant == 1)
			target_ant = AUX_ANT;

		if (pdm_sat_table->ant_num > 1) {
			/* [ update RX ant ]*/
			odm_update_rx_idle_ant(p_dm, (u8)target_ant);

			/* [ update TX ant ]*/
			odm_update_tx_ant(p_dm, (u8)target_ant, (p_dm_fat_table->train_idx));
		}

		/*set beam in each antenna*/
		phydm_update_rx_idle_beam(p_dm);

		odm_ant_div_on_off(p_dm, ANTDIV_ON);
		p_dm_fat_table->fat_state = FAT_PREPARE_STATE;
		return;

	}
	/* [TRAINING STATE] */
	else if (p_dm_fat_table->fat_state == FAT_TRAINING_STATE) {
		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[ 2. In Training state]\n"));

		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("fat_beam_n = (( %d )), pre_fat_beam_n = (( %d ))\n",
			pdm_sat_table->fast_training_beam_num, pdm_sat_table->pre_fast_training_beam_num));

		if (pdm_sat_table->fast_training_beam_num > pdm_sat_table->pre_fast_training_beam_num)

			pdm_sat_table->force_update_beam_en = 0;

		else {

			pdm_sat_table->force_update_beam_en = 1;

			pdm_sat_table->pkt_counter = 0;
			beam_tmp = pdm_sat_table->fast_training_beam_num;
			if (pdm_sat_table->fast_training_beam_num >= (pdm_sat_table->beam_patten_num_each_ant - 1)) {

				PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[Timeout Update]  Beam_num (( %d )) -> (( decision ))\n", pdm_sat_table->fast_training_beam_num));
				p_dm_fat_table->fat_state = FAT_DECISION_STATE;
				odm_fast_ant_training_hl_smart_antenna_type1(p_dm);

			} else {
				pdm_sat_table->fast_training_beam_num++;

				PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[Timeout Update]  Beam_num (( %d )) -> (( %d ))\n", beam_tmp, pdm_sat_table->fast_training_beam_num));
				phydm_set_all_ant_same_beam_num(p_dm);
				p_dm_fat_table->fat_state = FAT_TRAINING_STATE;

			}
		}
		pdm_sat_table->pre_fast_training_beam_num = pdm_sat_table->fast_training_beam_num;
		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("[prepare state] Update Pre_Beam =(( %d ))\n", pdm_sat_table->pre_fast_training_beam_num));
	}
	/*  [Prepare state] */
	/*=======================================================================================*/
	else if (p_dm_fat_table->fat_state == FAT_PREPARE_STATE) {

		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("\n\n[ 1. In Prepare state]\n"));

		if (p_dm->pre_traffic_load == (p_dm->traffic_load)) {
			if (pdm_sat_table->decision_holding_period != 0) {
				PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Holding_period = (( %d )), return!!!\n", pdm_sat_table->decision_holding_period));
				pdm_sat_table->decision_holding_period--;
				return;
			}
		}


		/* Set training packet number*/
		if (pdm_sat_table->fix_training_num_en == 0) {

			switch (p_dm->traffic_load) {

			case TRAFFIC_HIGH:
				pdm_sat_table->per_beam_training_pkt_num = 8;
				pdm_sat_table->decision_holding_period = 2;
				break;
			case TRAFFIC_MID:
				pdm_sat_table->per_beam_training_pkt_num = 6;
				pdm_sat_table->decision_holding_period = 3;
				break;
			case TRAFFIC_LOW:
				pdm_sat_table->per_beam_training_pkt_num = 3; /*ping 60000*/
				pdm_sat_table->decision_holding_period = 4;
				break;
			case TRAFFIC_ULTRA_LOW:
				pdm_sat_table->per_beam_training_pkt_num = 1;
				pdm_sat_table->decision_holding_period = 6;
				break;
			default:
				break;
			}
		}
		PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Fix_training_en = (( %d )), training_pkt_num_base = (( %d )), holding_period = ((%d))\n",
			pdm_sat_table->fix_training_num_en, pdm_sat_table->per_beam_training_pkt_num, pdm_sat_table->decision_holding_period));


#ifdef CONFIG_FAT_PATCH
		break_counter = 0;
		for (i = (pdm_sat_table->first_train_ant - 1); i < pdm_sat_table->ant_num_total; i++) {
			for (j = 0; j < (pdm_sat_table->beam_patten_num_each_ant); j++) {

				per_beam_rssi_diff_tmp = pdm_sat_table->beam_train_rssi_diff[i][j];
				training_pkt_num_offset = per_beam_rssi_diff_tmp;

				if ((pdm_sat_table->per_beam_training_pkt_num) > training_pkt_num_offset)
					pdm_sat_table->beam_train_cnt[i][j] = pdm_sat_table->per_beam_training_pkt_num - training_pkt_num_offset;
				else
					pdm_sat_table->beam_train_cnt[i][j] = 1;


				PHYDM_DBG(p_dm, DBG_ANT_DIV, ("ant[%d]: Beam_num-(( %d ))  training_pkt_num = ((%d))\n",
					i,  j, pdm_sat_table->beam_train_cnt[i][j]));
			}
			break_counter++;
			if (break_counter >= (pdm_sat_table->ant_num))
				break;
		}


		phydm_fast_training_enable(p_dm, FAT_OFF);
		pdm_sat_table->pre_beacon_counter = pdm_sat_table->beacon_counter;
		pdm_sat_table->update_beam_idx = 0;

		if (*p_dm->p_band_type == ODM_BAND_5G) {
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Set 5G ant\n"));
			/*used_ant = (pdm_sat_table->first_train_ant == MAIN_ANT) ? AUX_ANT : MAIN_ANT;*/
			used_ant = pdm_sat_table->first_train_ant;
		} else {
			PHYDM_DBG(p_dm, DBG_ANT_DIV, ("Set 2.4G ant\n"));
			used_ant = pdm_sat_table->first_train_ant;
		}

		odm_update_rx_idle_ant(p_dm, (u8)used_ant);

#else
		/* Set training MAC addr. of target */
		odm_set_next_mac_addr_target(p_dm);
		phydm_fast_training_enable(p_dm, FAT_ON);
#endif

		odm_ant_div_on_off(p_dm, ANTDIV_OFF);
		pdm_sat_table->pkt_counter = 0;
		pdm_sat_table->fast_training_beam_num = 0;
		phydm_set_all_ant_same_beam_num(p_dm);
		pdm_sat_table->pre_fast_training_beam_num = pdm_sat_table->fast_training_beam_num;
		p_dm_fat_table->fat_state = FAT_TRAINING_STATE;
	}

}

#endif /*#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1*/


#endif/*#ifdef CONFIG_HL_SMART_ANTENNA*/



void
phydm_smt_ant_config(
	void		*p_dm_void
)
{
	struct	PHY_DM_STRUCT		*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct	smt_ant				*p_smtant_table = &(p_dm->smtant_table);

	#if (defined(CONFIG_CUMITEK_SMART_ANTENNA))

		p_dm->support_ability |= ODM_BB_SMT_ANT;
		p_smtant_table->smt_ant_vendor = SMTANT_CUMITEK;
		p_smtant_table->smt_ant_type = 1;
	#elif (defined(CONFIG_HL_SMART_ANTENNA))

		p_dm->support_ability |= ODM_BB_SMT_ANT;
		p_smtant_table->smt_ant_vendor = SMTANT_HON_BO;

		#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1
		p_smtant_table->smt_ant_type = 1;
		#endif

		#ifdef CONFIG_HL_SMART_ANTENNA_TYPE2
		p_smtant_table->smt_ant_type = 2;
		#endif
	#endif

	PHYDM_DBG(p_dm, DBG_SMT_ANT, ("[SmtAnt Config] Vendor=((%d)), Smt_ant_type =((%d))\n",
		p_smtant_table->smt_ant_vendor, p_smtant_table->smt_ant_type));
}
#endif


void
phydm_smt_ant_init(
	void		*p_dm_void
)
{
#if (defined(CONFIG_SMART_ANTENNA))
	struct	PHY_DM_STRUCT		*p_dm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct	smt_ant				*p_smtant_table = &(p_dm->smtant_table);

	phydm_smt_ant_config(p_dm);


	if (p_smtant_table->smt_ant_vendor == SMTANT_CUMITEK) {
	} else if (p_smtant_table->smt_ant_vendor == SMTANT_HON_BO) {

		#if (defined(CONFIG_HL_SMART_ANTENNA))
		#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1
		if (p_dm->support_ic_type == ODM_RTL8821) {
			phydm_hl_smart_ant_type1_init_8821a(p_dm);
			/**/
		}
		#endif

		#ifdef CONFIG_HL_SMART_ANTENNA_TYPE2
		if (p_dm->support_ic_type == ODM_RTL8822B) {
			phydm_hl_smart_ant_type2_init_8822b(p_dm);
			/**/
		}
		#endif
		#endif/*#if (defined(CONFIG_HL_SMART_ANTENNA))*/
	}
#endif
}


