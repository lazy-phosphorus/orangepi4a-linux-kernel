/* SPDX-License-Identifier: GPL-2.0-or-later */
/*******************************************************************************
 * Allwinner SoCs hdmi2.0 driver.
 *
 * Copyright (C) 2016 Allwinner.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 ******************************************************************************/
#include <linux/delay.h>
#include "include/ESMHost.h"
#include "include/esm_hdcp_hdmi_image_read.h"

ESM_STATUS ESM_LoadPairing(esm_instance_t *esm, uint8_t *PairData, uint32_t BufferSize)
{
	ESM_STATUS err;

	if ((esm == NULL) || (esm->driver == NULL)
			|| (esm->driver->data_set == NULL)
			|| (esm->driver->data_write == NULL)) {
		hdmi_inf("ESM_HL_NO_INSTANCE");
		return ESM_HL_NO_INSTANCE;
	}

	if ((!esm->esm_pair_off) && (!esm->esm_pair_size)) {
		hdmi_inf("esm pair offset or size is zero\n");
		return -1;
	}

	if (BufferSize > esm->esm_pair_size) {
		hdmi_inf("ESM_HL_INVALID_PARAMETERS\n");
		return ESM_HL_INVALID_PARAMETERS;
	}
	err = esm->driver->data_set(esm->driver->instance, esm->esm_pair_off,
		0, esm->esm_pair_size);
	if (err != ESM_HL_DRIVER_SUCCESS) {
		hdmi_inf("Data SET Failed %d\n", err);
		return ESM_HL_DRIVER_DATA_SET_FAILED;
	}

	if (PairData && BufferSize) {
		err = esm->driver->data_write(esm->driver->instance,
			esm->esm_pair_off, PairData, BufferSize);
		if (err != ESM_HL_DRIVER_SUCCESS) {
			hdmi_inf("Data WRITE Failed %d\n", err);
			return ESM_HL_DRIVER_DATA_WRITE_FAILED;
		}
	}

	return ESM_HL_SUCCESS;
}

ESM_STATUS ESM_SavePairing(esm_instance_t *esm, uint8_t *PairData, uint32_t *BufferSize)
{
	ESM_STATUS err;

	if ((esm == NULL) || (esm->driver == NULL)
			|| (esm->driver->data_read == NULL)) {
		return ESM_HL_NO_INSTANCE;
	}

	if (!PairData || !BufferSize)
		return ESM_HL_INVALID_PARAMETERS;

	if (*BufferSize < esm->esm_pair_size) {
		hdmi_inf("BufferSize:%d esm_pair_size:%d\n",
			*BufferSize, esm->esm_pair_size);
		return ESM_HL_BUFFER_TOO_SMALL;
	}

	*BufferSize = esm->esm_pair_size;
	err = esm->driver->data_read(esm->driver->instance, esm->esm_pair_off,
		       PairData, esm->esm_pair_size);
	if (err != ESM_HL_DRIVER_SUCCESS) {
		hdmi_inf("Data READ Failed %d\n", err);
		return ESM_HL_DRIVER_DATA_READ_FAILED;
	}

	return ESM_HL_SUCCESS;
}

ESM_STATUS ESM_EnablePairing(esm_instance_t *esm, uint32_t OnOff)
{
	uint32_t req = 0;
	uint32_t resp = 0;
	uint32_t req_param = 0;
	ESM_STATUS err;

	if (esm == NULL)
		return ESM_HL_NO_INSTANCE;

	req_param = OnOff;

		req =  ESM_HDCP_HDMI_TX_CMD_AKE_PAIRING_REQ;
		resp = ESM_HDCP_HDMI_TX_CMD_AKE_PAIRING_RESP;

	err = esm_hostlib_mb_cmd(esm, req, 1, &req_param, resp, 1,
			&esm->status, CMD_DEFAULT_TIMEOUT);
	if (err != ESM_HL_SUCCESS) {
		hdmi_inf("MB Failed %d", err);
		return ESM_HL_MB_FAILED;
	}

	if (esm->status != ESM_SUCCESS) {
		hdmi_inf("status %d", esm->status);
		return ESM_HL_FAILED;
	}

	return ESM_HL_SUCCESS;
}


ELP_STATUS esm_image_read(uint8_t *config_table,
				uint32_t config_table_size,
				uint8_t app_id,
				uint8_t *ctrl,
				esm_controls *sys_c,
				void *app_c,
				uint32_t fw_version,
				uint32_t *offset)
{
	esm_hdmi_hdcp_controls *hdcp_c;
	uint8_t *pc;

	/* Parameter checks. */
	ESM_MACRO_ASSERT(offset != 0);
	ESM_MACRO_ASSERT(config_table != 0);
	ESM_MACRO_ASSERT(config_table_size > 0);

	if (ESM_APP_CONFIG_MAX_SIZE > ESM_CONFIG_MAX_SIZE)
		return ESM_FAILED;

	hdcp_c = (esm_hdmi_hdcp_controls *)app_c;
	pc = config_table;

	sys_c->c_int.ext_mem_topo_seed_size = 0;

	sys_c->cookie = (uint32_t)ELP_BYTE_GET(pc, sizeof(sys_c->cookie));
	pc += sizeof(sys_c->cookie);
	if (sys_c->cookie != ESM_CONFIG_COOKIE)
		return ESM_CONFIG_BAD_COOKIE;

	sys_c->fw_version = (uint32_t)ELP_BYTE_GET(pc, sizeof(sys_c->fw_version));
	pc += sizeof(sys_c->fw_version);
	/* This is for host lib. It does not know initialy what is ESM firmware app. */
	if (fw_version != ESM_HDCP_HDMI_UNKNOWN_ID)
		if (sys_c->fw_version != fw_version)
			return ESM_CONFIG_FW_SYS_VERSION_NOT_SUPPORTED;

	sys_c->config_version = (uint32_t)ELP_BYTE_GET(pc, sizeof(sys_c->config_version));
	pc += sizeof(sys_c->config_version);

	/* This is for host lib. It does not know initialy what is ESM firmware app. */
	if (app_id != ESM_HDCP_HDMI_UNKNOWN_ID) {
		if (ESM_GET_CONFIG_VERSION(sys_c->config_version) != ESM_HDCP_HDMI_CONFIG_VERSION)
			return ESM_CONFIG_VERSION_NOT_SUPPORTED;

		if (ESM_GET_APP_ID(sys_c->config_version) != app_id)
			return ESM_APP_ID_NOT_SUPPORTED;
	}

	if (ctrl != 0)
		(*ctrl) = ESM_GET_CONFIG_CTRL(sys_c->config_version);

	sys_c->watchdog_mode = (uint8_t)ELP_BYTE_GET(pc, sizeof(sys_c->watchdog_mode));
	pc += sizeof(sys_c->watchdog_mode);
	if (sys_c->watchdog_mode >= ESM_SYSTEM_WATCHDOG_MODE_MAX)
		return ESM_CONFIG_WD_ENUM_NOT_SUPPORTED;

	sys_c->logging_mode = (uint8_t)ELP_BYTE_GET(pc, sizeof(sys_c->logging_mode));
	pc += sizeof(sys_c->logging_mode);
	if (sys_c->logging_mode >= ESM_SYSTEM_LOGGING_MODE_MAX)
		return ESM_CONFIG_WD_ENUM_NOT_SUPPORTED;

	sys_c->logging_level = (uint8_t)ELP_BYTE_GET(pc, sizeof(sys_c->logging_level));
	pc += sizeof(sys_c->logging_level);
	if (sys_c->logging_level  >= ESM_SYSTEM_LOGGING_LEVEL_MAX)
		return ESM_CONFIG_LOGGING_LEVEL_NOT_SUPPORTED;

	sys_c->shared_timer_int_interval = (uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->shared_timer_int_interval));
	pc += sizeof(sys_c->shared_timer_int_interval);

	sys_c->reseed_timeout = (uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->reseed_timeout));
	pc += sizeof(sys_c->reseed_timeout);

	sys_c->comm_timeout = (uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->comm_timeout));
	pc += sizeof(sys_c->comm_timeout);

	sys_c->i2c_freq = (uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->i2c_freq));
	pc += sizeof(sys_c->i2c_freq);

	sys_c->cee_state_wait_time = (uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->cee_state_wait_time));
	pc += sizeof(sys_c->cee_state_wait_time);

	sys_c->cpu_freq = (uint32_t)ELP_BYTE_GET(pc, sizeof(sys_c->cpu_freq));
	pc += sizeof(sys_c->cpu_freq);

	sys_c->c_int.except_mode = (uint8_t)ELP_BYTE_GET(pc, sizeof(sys_c->c_int.except_mode));
	pc += sizeof(sys_c->c_int.except_mode);

	sys_c->c_int.ext_mem_total_size = (uint32_t)ELP_BYTE_GET(pc, sizeof(sys_c->c_int.ext_mem_total_size));
	pc += sizeof(sys_c->c_int.ext_mem_total_size);

	if (sys_c->c_int.ext_mem_total_size >= ESM_SYSTEM_MAX_SIZE)
		return ESM_CONFIG_MEM_SIZE_NOT_SUPPORTED;

	sys_c->c_int.ext_mem_cfgdata_size = (uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->c_int.ext_mem_cfgdata_size));
	pc += sizeof(sys_c->c_int.ext_mem_cfgdata_size);

	sys_c->c_int.ext_mem_cpireg_size = (uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->c_int.ext_mem_cpireg_size));
	pc += sizeof(sys_c->c_int.ext_mem_cpireg_size);

	sys_c->c_int.ext_mem_mb_size = (uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->c_int.ext_mem_mb_size));
	pc += sizeof(sys_c->c_int.ext_mem_mb_size);
	if (sys_c->c_int.ext_mem_mb_size >= ESM_SYSTEM_MB_MAX_SIZE)
		return ESM_CONFIG_MB_SIZE_NOT_SUPPORTED;

	sys_c->c_int.ext_mem_except_size = (uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->c_int.ext_mem_except_size));
	pc += sizeof(sys_c->c_int.ext_mem_except_size);
	if ((sys_c->c_int.ext_mem_except_size >= ESM_SYSTEM_EXCEPT_MAX_SIZE) ||
		(sys_c->c_int.ext_mem_except_size < (sys_c->c_int.except_mode * sizeof(esm_exception_info))))
		return ESM_CONFIG_EXCEPT_SIZE_NOT_SUPPORTED;

	sys_c->c_int.ext_mem_srm_size = (uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->c_int.ext_mem_srm_size));
	pc += sizeof(sys_c->c_int.ext_mem_srm_size);
	if (sys_c->c_int.ext_mem_srm_size >= ESM_SYSTEM_SRM_MAX_SIZE)
		return ESM_CONFIG_SRM_SIZE_NOT_SUPPORTED;

	sys_c->c_int.ext_mem_topo_size = (uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->c_int.ext_mem_topo_size));
	pc += sizeof(sys_c->c_int.ext_mem_topo_size);
	if (sys_c->c_int.ext_mem_topo_size >= ESM_SYSTEM_TOPO_MAX_SIZE)
		return ESM_CONFIG_TOPO_SIZE_NOT_SUPPORTED;


	sys_c->c_int.ext_mem_pairing_size = (uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->c_int.ext_mem_pairing_size));
	pc += sizeof(sys_c->c_int.ext_mem_pairing_size);
	if (sys_c->c_int.ext_mem_pairing_size >= ESM_SYSTEM_PAIRING_MAX_SIZE)
		return ESM_CONFIG_PAIRING_SIZE_NOT_SUPPORTED;

	sys_c->c_int.ext_mem_logging_size = (uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->c_int.ext_mem_logging_size));
	pc += sizeof(sys_c->c_int.ext_mem_logging_size);
	if (sys_c->c_int.ext_mem_logging_size >= ESM_SYSTEM_LOGGING_MAX_SIZE)
		return ESM_CONFIG_LOGGING_SIZE_NOT_SUPPORTED;
	sys_c->c_int.topo_slots = (uint8_t)ELP_BYTE_GET(pc, sizeof(sys_c->c_int.topo_slots));
	pc += sizeof(sys_c->c_int.topo_slots);

	sys_c->c_int.ext_mem_topo_seed_size = ELP_BYTE_GET(pc, sizeof(sys_c->c_int.ext_mem_topo_seed_size));
	pc += sizeof(sys_c->c_int.ext_mem_topo_seed_size);
#if (ESM_HDCP_APP_MODE == ESM_HDCP_RP_TX_APP) || (ESM_HDCP_APP_MODE == ESM_HDCP_RP_RX_APP) || (ESM_HDCP_APP_MODE == ESM_HDCP_RP_APP)
	if (sys_c->c_int.ext_mem_topo_seed_size > ESM_HDCP_HDMI_TOPO_INTEGRITY_KEY_SIZE)
		return ESM_CONFIG_TOPO_SEED_SIZE_NOT_SUPPORTED;
#endif

#ifdef ESM_BUILD_SC
	/* Secure communication. */
	sys_c->sc_int.ext_mem_sc_maxdataregion_size =
		(uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->sc_int.ext_mem_sc_maxdataregion_size));
	pc += sizeof(sys_c->sc_int.ext_mem_sc_maxdataregion_size);

	sys_c->sc_int.ext_mem_sc_maxkeyregion_size =
		(uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->sc_int.ext_mem_sc_maxkeyregion_size));
	pc += sizeof(sys_c->sc_int.ext_mem_sc_maxkeyregion_size);
#endif
#ifdef ESM_BUILD_SAUTH
	/* Secure authentication. */
	sys_c->sa_int.ext_mem_sa_maxdataregion_size =
		(uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->sa_int.ext_mem_sa_maxdataregion_size));
	pc += sizeof(sys_c->sa_int.ext_mem_sa_maxdataregion_size);

	sys_c->sa_int.ext_mem_sa_maxkeyregion_size =
		(uint16_t)ELP_BYTE_GET(pc, sizeof(sys_c->sa_int.ext_mem_sa_maxkeyregion_size));
	pc += sizeof(sys_c->sa_int.ext_mem_sa_maxkeyregion_size);
#endif

/* Ensure total memory size is greater than all the sub-partition sizes. */
	if (sys_c->c_int.ext_mem_total_size < (uint32_t)(sys_c->c_int.ext_mem_cfgdata_size +
					sys_c->c_int.ext_mem_cpireg_size +
					sys_c->c_int.ext_mem_except_size +
					sys_c->c_int.ext_mem_logging_size +
					sys_c->c_int.ext_mem_pairing_size +
					sys_c->c_int.ext_mem_srm_size +
					(sys_c->c_int.ext_mem_topo_size * sys_c->c_int.topo_slots) +
					sys_c->c_int.ext_mem_topo_seed_size))
		return ESM_CONFIG_VERIFY_FAILED;

	hdcp_c->woocompatability = (uint8_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->woocompatability));
	pc += sizeof(hdcp_c->woocompatability);

	hdcp_c->c_int.ake_timeouts = (uint8_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.ake_timeouts));
	pc += sizeof(hdcp_c->c_int.ake_timeouts);

	hdcp_c->c_int.cee_enable = (uint8_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.cee_enable));
	pc += sizeof(hdcp_c->c_int.cee_enable);

	hdcp_c->c_int.cee_bsod = (uint32_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.cee_bsod));
	pc += sizeof(hdcp_c->c_int.cee_bsod);

	hdcp_c->c_int.cee_frame_cnt_debug = (uint32_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.cee_frame_cnt_debug));
	pc += sizeof(hdcp_c->c_int.cee_frame_cnt_debug);

	/* Calculate the offset in bytes */
	*offset = (pc - config_table);

	return ESM_SUCCESS;
}

void esm_image_read_base_config(uint8_t *ptr, esm_hdmi_hdcp_controls *i_hdcp_c)
{
	uint8_t *pc;
	esm_hdmi_hdcp_controls *hdcp_c;

	/* Parameter checks. */
	ESM_MACRO_ASSERT(ptr != 0);
	ESM_MACRO_ASSERT(i_hdcp_c != 0);

	pc = ptr;
	hdcp_c = i_hdcp_c;

	hdcp_c->c_int.cee_woo_start = (uint16_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.cee_woo_start));
	pc += sizeof(hdcp_c->c_int.cee_woo_start);

	hdcp_c->c_int.cee_woo_end = (uint16_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.cee_woo_end));
	pc += sizeof(hdcp_c->c_int.cee_woo_end);

	hdcp_c->c_int.cee_enable_ctls = (uint8_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.cee_enable_ctls));
	pc += sizeof(hdcp_c->c_int.cee_enable_ctls);
}

ELP_STATUS esm_image_read_tx(uint8_t *config_table, void *app_c, uint32_t *offset)
{
	esm_hdmi_hdcp_controls *hdcp_c;
	uint8_t *pc;

	/* Parameter checks. */
	ESM_MACRO_ASSERT(offset != 0);
	ESM_MACRO_ASSERT(config_table != 0);

	hdcp_c = (esm_hdmi_hdcp_controls *)app_c;
	pc = config_table;

	/* TX */
#if (ESM_HDCP_APP_MODE == ESM_HDCP_RX_TX_APP) || (ESM_HDCP_APP_MODE == ESM_HDCP_TX_APP) || \
	(ESM_HDCP_APP_MODE == ESM_HDCP_HOSTLIB_APP)
	hdcp_c->c_int.pairing_enabled = (uint8_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.pairing_enabled));
	pc += sizeof(hdcp_c->c_int.pairing_enabled);

	hdcp_c->c_int.max_pairing_devices = (uint8_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.max_pairing_devices));
	pc += sizeof(hdcp_c->c_int.max_pairing_devices);

	hdcp_c->c_int.max_attempts_li = (uint8_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.max_attempts_li));
	pc += sizeof(hdcp_c->c_int.max_attempts_li);

	hdcp_c->c_int.tx_poll_rxstatus_li = (uint16_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.tx_poll_rxstatus_li));
	pc += sizeof(hdcp_c->c_int.tx_poll_rxstatus_li);

	hdcp_c->c_int.tx_wait_li_rxstatus_timeout =
		(uint16_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.tx_wait_li_rxstatus_timeout));
	pc += sizeof(hdcp_c->c_int.tx_wait_li_rxstatus_timeout);

	hdcp_c->c_int.tx_wait_hdcp2capable_timeout =
		(uint16_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.tx_wait_hdcp2capable_timeout));
	pc += sizeof(hdcp_c->c_int.tx_wait_hdcp2capable_timeout);

	hdcp_c->c_int.tx_wait_ake_complete_timeout =
		(uint16_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.tx_wait_ake_complete_timeout));
	pc += sizeof(hdcp_c->c_int.tx_wait_ake_complete_timeout);

	hdcp_c->c_int.tx_wait_cs_message_timeout =
		(uint16_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.tx_wait_cs_message_timeout));
	pc += sizeof(hdcp_c->c_int.tx_wait_cs_message_timeout);

	hdcp_c->c_int.tx_srm_version = (uint16_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.tx_srm_version));
	pc += sizeof(hdcp_c->c_int.tx_srm_version);

	hdcp_c->c_int.tx_lvc_timeout = (uint16_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.tx_lvc_timeout));
	pc += sizeof(hdcp_c->c_int.tx_lvc_timeout);

	hdcp_c->c_int.tx_lvc_ignore_timeout =
		(uint16_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.tx_lvc_ignore_timeout));
	pc += sizeof(hdcp_c->c_int.tx_lvc_ignore_timeout);

	hdcp_c->c_int.tx_capable_bypass_time =
		(uint16_t)ELP_BYTE_GET(pc, sizeof(hdcp_c->c_int.tx_capable_bypass_time));
	pc += sizeof(hdcp_c->c_int.tx_capable_bypass_time);

	/* spare gap */
	pc += ESM_HDCP_HDMI_GAP_1_TX;

	esm_image_read_base_config(pc, hdcp_c);

	/* spare gap */
	pc += ESM_HDCP_HDMI_GAP_2;
#endif

	/* Calculate the offset in bytes. */
	*offset = (pc - config_table);

	return ESM_SUCCESS;
}


/* Initializes the code and data memory.   write firmware image to esm? or esm booting */
static ESM_STATUS esm_init_mem(esm_instance_t *esm, unsigned int image,
							uint32_t image_size)
{
	uint32_t data = 0;
	u32 addr = 0;
	ESM_STATUS err = -1;

	if (esm == 0)
		return ESM_HL_NO_INSTANCE;

	/* CODE: Write the physical address of the code memory to the ESM. */
	if (esm->driver->hpi_write(esm->driver->instance,
			ESM_REG_HP_FW_BASE0(HPI_HOST_OFF),
			esm->driver->code_base) != ESM_HL_DRIVER_SUCCESS) {
		hdmi_inf("Failed to set the code address\n");
		return ESM_HL_FAILED_TO_SET_CODE_ADDR;
	}

	/* CODE: Read the physical address of the code memory back from the ESM. */
	if (esm->driver->hpi_read(esm->driver->instance,
		ESM_REG_HP_FW_BASE0(HPI_HOST_OFF), &addr) != ESM_HL_DRIVER_SUCCESS) {
		hdmi_inf("Failed to read code address\n");
		return ESM_HL_FAILED_TO_SET_CODE_ADDR;
	}

	/* CODE: Check for an address mismatch. */
	if (addr != esm->driver->code_base) {
		hdmi_inf("Code address mismatch, code_phy_addr:%x  ESM code addr:%x\n", esm->driver->code_base, addr);
		return ESM_HL_FAILED_TO_SET_CODE_ADDR;
	}

	/* DATA: Write the physical address of the data memory to the ESM. */
	if (esm->driver->hpi_write(esm->driver->instance,
		ESM_REG_HP_DATA_BASE0(HPI_HOST_OFF), esm->driver->data_base) != ESM_HL_DRIVER_SUCCESS) {
		hdmi_inf("Failed to set the data address\n");
		return ESM_HL_FAILED_TO_SET_DATA_ADDR;
	}
	/* DATA: Read the physical address of the data memory back from the ESM. */
	if (esm->driver->hpi_read(esm->driver->instance,
		ESM_REG_HP_DATA_BASE0(HPI_HOST_OFF), &addr) != ESM_HL_DRIVER_SUCCESS) {
		hdmi_inf("Failed to read data address\n");
		return ESM_HL_FAILED_TO_SET_DATA_ADDR;
	}

	/* DATA: Check for an address mismatch. */
	if (addr != esm->driver->data_base) {
		hdmi_inf("Data address mismatch\n");
		hdmi_inf("Code address mismatch, data_phy_addr:%x  ESM code addr:%x\n",
							esm->driver->data_base, addr);
		return ESM_HL_FAILED_TO_SET_DATA_ADDR;
	}
	/* Set FW_VLD bit. */
	if (esm->driver->hpi_write(esm->driver->instance, HP_CTRL, 0x1) != ESM_HL_DRIVER_SUCCESS) {
		hdmi_inf("Failed to set FW_VLD bit\n");
		return ESM_HL_FAILED_TO_SET_VLD_BIT;
	}
	mdelay(200);
	/* Check HP_IRQ_STAT bit MB_MSG. */
	if (esm_hostlib_wait_mb_response(esm, CMD_DEFAULT_TIMEOUT) != ESM_HL_SUCCESS) {
		hdmi_inf("Failed to set FW_VLD bit (no response)\n");
		return ESM_HL_FAILED_TO_SET_VLD_BIT;
	}

	/* Display some information on the ESM product. */
	if (esm->driver->hpi_read(esm->driver->instance,
		ESM_REG_AE_2HP_MB(HPI_HOST_OFF), &data) != ESM_HL_DRIVER_SUCCESS) {
		hdmi_inf("Failed to read FW_VLD [%d]\n", err);
		return ESM_HL_DRIVER_HPI_READ_FAILED;
	}
	hdcp_log("\tESM FW_VLD\t\t= 0x%x\n", data);

	if (esm->driver->hpi_read(esm->driver->instance,
		ESM_REG_AE_MB_P00(HPI_HOST_OFF), &data) != ESM_HL_DRIVER_SUCCESS) {
		hdmi_inf("Failed to read FW_VERSION [%d]\n", err);
		return ESM_HL_DRIVER_HPI_READ_FAILED;
	}
	hdcp_log("\tESM FW_VERSION\t\t= 0x%x\n", data);
	esm->fw_version = data;

	/* Done with mailbox, return it to AE. */
	err = esm->driver->hpi_write(esm->driver->instance,
		ESM_REG_AE_MB_STAT(HPI_HOST_OFF),
		1ul << ESM_REG_AE_MB_STAT_MB_RTN_BIT);
	if (err != ESM_HL_DRIVER_SUCCESS) {
		hdmi_inf("Failed to write AE_MB_STAT [%d]\n", err);
		return ESM_HL_DRIVER_HPI_WRITE_FAILED;
	}

	if (esm->driver->hpi_read(esm->driver->instance,
		ESM_REG_DESIGN_ID0(HPI_HOST_OFF), &data) != ESM_HL_DRIVER_SUCCESS) {
		hdmi_inf("Failed to read DESIGN_ID [%d]\n", err);
		return ESM_HL_DRIVER_HPI_READ_FAILED;
	}

	hdcp_log("\tESM DESIGN_ID\t\t= 0x%x\n", data);

	if (esm->driver->hpi_read(esm->driver->instance,
		ESM_REG_PRODUCT_ID0(HPI_HOST_OFF), &data) != ESM_HL_DRIVER_SUCCESS) {
		hdmi_inf("Failed to read PRODUCT_ID [%d]\n", err);
		return ESM_HL_DRIVER_HPI_READ_FAILED;
	}
	hdcp_log("\tESM PRODUCT_ID\t\t= 0x%x\n", data);

	if (esm->driver->hpi_read(esm->driver->instance,
		ESM_REG_FW_INFO00(HPI_HOST_OFF), &data) != ESM_HL_DRIVER_SUCCESS) {
		hdmi_inf("Failed to read FW_INFO [%d]\n", err);
		return ESM_HL_DRIVER_HPI_READ_FAILED;
	}
	hdcp_log("\tESM FW_INFO\t\t= 0x%x\n", data);

	return ESM_HL_SUCCESS;
}

/* Retrieves the configuration from the data memory. */
ESM_STATUS esm_get_configuration(esm_instance_t *esm, esm_config_t *returned_config)
{
	ESM_STATUS err = -1;
	uint32_t offset = 0;
	uint8_t config[ESM_CONFIG_MAX_SIZE];
	uint32_t fw_version = 0;
	uint8_t fw_major = 0;
	uint8_t fw_minor = 0;
	uint8_t fw_driver = 0;
	uint8_t fw_app = 0;
	uint8_t config_app_version = 0;
	uint8_t ctrl = 0;

	if (esm == 0)
		return ESM_HL_NO_INSTANCE;

	/* Read the configuration table from the data memory. */
	if (esm->driver->data_read(esm->driver->instance, 0,
		config, ESM_CONFIG_MAX_SIZE) != ESM_HL_DRIVER_SUCCESS) {
		hdmi_inf("Failed to read configuration data from memory [%d]", err);
		return ESM_HL_FAILED_TO_RD_CONFIG_DATA;
	}

	/* Parse the configuration parameters. */
	if (esm_image_read(config, 0, 0, 0, &esm->esm_ctrl,
			&esm->hdcp_c, 0, &offset) != ESM_SUCCESS) {
		hdmi_inf("Failed to parse configuration data [%d]", err);
		return ESM_HL_FAILED_TO_PARSE_CONFIG_DATA;
	}

	/* Get the application ID. */
	esm->app_id = ESM_GET_APP_ID(esm->esm_ctrl.config_version);
	ctrl = ESM_GET_CONFIG_CTRL(esm->esm_ctrl.config_version);
	esm->fw_version = esm->esm_ctrl.fw_version;

	switch (esm->app_id) {
	case ESM_HDCP_HDMI_RPTX_ID:
		hdmi_wrn("This is not a Tx repeater\n");
		break;

	case ESM_HDCP_HDMI_RPRX_ID:
		hdmi_wrn("This is not a Rx repeater\n");
		break;

	case ESM_HDCP_HDMI_RX_ID:
		hdmi_wrn("This is not a Rx\n");
		break;

	case ESM_HDCP_HDMI_TX_ID:
		esm->fw_type = ESM_HOST_LIB_TX;
		hdcp_log("\tconfig_version\t\t= %d (TX)", esm->app_id);

		if (esm_image_read_tx(config + offset, &esm->hdcp_c, &offset) != ESM_SUCCESS) {
			hdmi_inf("[ESM_HDCP_HDMI_TX_ID] Failed to parse configuration data [%d]", err);
			return ESM_HL_FAILED_TO_PARSE_CONFIG_DATA;
		}

		hdcp_log("\tpairing_enabled\t\t= %d (TX)", esm->hdcp_c.c_int.pairing_enabled);
		hdcp_log("\tmax_pairing_devices\t= %d (TX)", esm->hdcp_c.c_int.max_pairing_devices);
		hdcp_log("\tlvc_timeout\t\t= %d ms (TX)", esm->hdcp_c.c_int.tx_lvc_timeout);
		hdcp_log("\tlvc_ignore_timeout\t= %d ms (TX)",
			esm->hdcp_c.c_int.tx_lvc_ignore_timeout);
		break;
	}

	/* CPI OUT mirror memory offset and size. */
	esm->esm_cpi_off  = esm->esm_ctrl.c_int.ext_mem_cfgdata_size;
	esm->esm_cpi_size = esm->esm_ctrl.c_int.ext_mem_cpireg_size;

	/* Mailbox memory offset and size. */
	esm->esm_mb_off  = esm->esm_cpi_off + esm->esm_cpi_size;
	esm->esm_mb_size = esm->esm_ctrl.c_int.ext_mem_mb_size;

	/* Exception memory offset and size. */
	esm->esm_exc_off = esm->esm_mb_off + esm->esm_mb_size;
	esm->esm_exceptions_size = esm->esm_ctrl.c_int.ext_mem_except_size;
	esm->esm_exceptions_last_id = 0;

	/* Logs memory offset and size. */
	esm->esm_log_header_off = esm->esm_exc_off + esm->esm_exceptions_size;
	esm->esm_log_off = esm->esm_log_header_off + ESM_LOG_HEADER_SIZE;
	esm->esm_log_size = esm->esm_ctrl.c_int.ext_mem_logging_size - ESM_LOG_HEADER_SIZE;
	esm->esm_log_hdr_last_id = -1;
	esm->esm_log_hdr_last_item = ESM_LOG_HEADER_INVALID_ID;

	/* SRM memory offset and size. */
	esm->esm_srm_off  = esm->esm_log_header_off + esm->esm_ctrl.c_int.ext_mem_logging_size;
	esm->esm_srm_size = esm->esm_ctrl.c_int.ext_mem_srm_size;

	/* Topology memory offset and size. */
	esm->esm_topo_off = esm->esm_srm_off + esm->esm_srm_size;
	esm->esm_topo_size = esm->esm_ctrl.c_int.ext_mem_topo_size;
	esm->esm_topo_slots = esm->esm_ctrl.c_int.topo_slots;
	esm->esm_topo_seed_off = esm->esm_topo_off + (esm->esm_topo_size * esm->esm_topo_slots);
	esm->esm_topo_seed_size = esm->esm_ctrl.c_int.ext_mem_topo_seed_size;

	/* Pairing memory offset and size. */
	esm->esm_pair_off = esm->esm_topo_seed_off + esm->esm_topo_seed_size;
	esm->esm_pair_size = esm->esm_ctrl.c_int.ext_mem_pairing_size;

	hdcp_log("\t------------------------------");
	hdcp_log("\texcept_mode\t\t= %x", esm->esm_ctrl.c_int.except_mode);
	hdcp_log("\text_mem_total_size\t= %d", esm->esm_ctrl.c_int.ext_mem_total_size);
	hdcp_log("\text_mem_cfgdata_size\t= %d", esm->esm_ctrl.c_int.ext_mem_cfgdata_size);
	hdcp_log("\text_mem_cpireg_size\t= %d", esm->esm_ctrl.c_int.ext_mem_cpireg_size);
	hdcp_log("\text_mem_mb_size\t\t= %d", esm->esm_ctrl.c_int.ext_mem_mb_size);
	hdcp_log("\text_mem_except_size\t= %d", esm->esm_ctrl.c_int.ext_mem_except_size);
	hdcp_log("\text_mem_logging_size\t= %d", esm->esm_ctrl.c_int.ext_mem_logging_size);
	hdcp_log("\text_mem_srm_size\t= %d", esm->esm_ctrl.c_int.ext_mem_srm_size);
	hdcp_log("\text_mem_topo_size\t= %d", esm->esm_ctrl.c_int.ext_mem_topo_size);
	hdcp_log("\text_mem_topo_seed_size\t= %d", esm->esm_ctrl.c_int.ext_mem_topo_seed_size);
	hdcp_log("\text_mem_pairing_size\t= %d", esm->esm_ctrl.c_int.ext_mem_pairing_size);
	hdcp_log("\twoocompatability\t= %d", esm->hdcp_c.woocompatability);
	hdcp_log("\take_timeouts\t\t= %d", esm->hdcp_c.c_int.ake_timeouts);
	hdcp_log("\ti2c_freq bitrate\t= %d khz", esm->esm_ctrl.i2c_freq);
	hdcp_log("\tcee_enable\t\t= %d", esm->hdcp_c.c_int.cee_enable);
	hdcp_log("\tcee_state_wait_time\t= %d us", esm->esm_ctrl.cee_state_wait_time);

	hdcp_log("\tesm_cpi_off\t\t= %d", esm->esm_cpi_off);
	hdcp_log("\tesm_mb_off\t\t= %d", esm->esm_mb_off);
	hdcp_log("\tesm_exc_off\t\t= %d", esm->esm_exc_off);
	hdcp_log("\tesm_log_(header)_off\t= %d", esm->esm_log_header_off);
	hdcp_log("\tesm_srm_off\t\t= %d", esm->esm_srm_off);
	hdcp_log("\tesm_topo_off\t\t= %d", esm->esm_topo_off);
	hdcp_log("\tesm_topo_slots\t\t= %d", esm->esm_topo_slots);
	hdcp_log("\tesm_topo_seed_off\t= %d", esm->esm_topo_seed_off);
	hdcp_log("\tesm_pair_off\t\t= %d", esm->esm_pair_off);
	hdcp_log("\tesm_pair_size\t\t= %d", esm->esm_pair_size);

#ifdef ESM_BUILD_EXT_0
	esm_hostlib_load_ext0(esm);
#endif

	if (returned_config) {
		switch (esm->app_id) {
		case ESM_HDCP_HDMI_RX_ID:
			returned_config->esm_type = ESM_HDCP_HDMI_RX_ID;
			config_app_version = HOSTLIB_ESM_APP_RX_VERSION;
			break;

		case ESM_HDCP_HDMI_TX_ID:
			returned_config->esm_type = ESM_HDCP_HDMI_TX_ID;
			config_app_version = HOSTLIB_ESM_APP_TX_VERSION;
			break;

		case ESM_HDCP_HDMI_RPRX_ID:
			returned_config->esm_type = ESM_HDCP_HDMI_RPRX_ID;
			config_app_version = HOSTLIB_ESM_APP_RPRX_VERSION;
			break;

		case ESM_HDCP_HDMI_RPTX_ID:
			returned_config->esm_type = ESM_HDCP_HDMI_RPTX_ID;
			config_app_version = HOSTLIB_ESM_APP_RPTX_VERSION;
			break;

		default:
			returned_config->esm_type = ESM_HDCP_HDMI_UNKNOWN_ID;
			config_app_version = 0xFF;
			break;
		}

		returned_config->topo_buffer_size       = esm->esm_topo_size;
		returned_config->topo_slots             = esm->esm_topo_slots;
		returned_config->topo_seed_buffer_size  = esm->esm_topo_seed_size;
		returned_config->log_buffer_size        = esm->esm_log_size;
		returned_config->mb_buffer_size         = esm->esm_mb_size;
		returned_config->exceptions_buffer_size = esm->esm_exceptions_size;
		returned_config->srm_buffer_size        = esm->esm_srm_size;
		returned_config->pairing_buffer_size    = esm->esm_pair_size;
	}

	/* Check if ESM firmware version is supported by the current Host Lib. */
	fw_version = esm->fw_version;
	fw_major = ((fw_version) >> 24) & 0xFF;
	fw_minor = ((fw_version) >> 16) & 0xFF;
	fw_driver = ((fw_version) >> 8) & 0xFF;
	fw_app = (fw_version) & 0xFF;

	if (!((fw_major >= HOSTLIB_ESM_MAJOR) && (fw_minor >= HOSTLIB_ESM_MINOR) &&
			(fw_app >= config_app_version))) {
		hdcp_log("ESM FW VERSION MAY NOT WORK WITH HOSTLIB VERSION");
		hdcp_log("ESM FW_VERSION = 0x%x.%x.%x.%x",
		fw_major, fw_minor, fw_driver, fw_app);
		/* hdmi_inf("HOSTLIB VERSION = %s", ESM_HostVersion()); */
	}

	return ESM_HL_SUCCESS;
}

ESM_STATUS ESM_Initialize(esm_instance_t *esm,
	unsigned int image, uint32_t image_size, uint32_t flags,
	esm_host_driver_t *driver, esm_config_t *config)
{

	if (esm == 0)
		return ESM_HL_NO_INSTANCE;

	/* Parameter checks. */
	if (!image || !image_size || !driver)
		return ESM_HL_INVALID_PARAMETERS;

	/* For now, interrupts are not supported. */
	esm->irq_supported = 0;

	/* Initialize ESM code and data memory. */
	if (esm_init_mem(esm, image, image_size) != ESM_HL_SUCCESS) {
		hdmi_inf("esm init mem failed!\n");
		return -1;
	}
	/* return ESM_HL_SUCCESS; */
	/* If requested, read firmware configuration. This step is
	* NOT optional for production firmware. */
	if (!config)
		return ESM_HL_SUCCESS;
	return esm_get_configuration(esm, config);
}
