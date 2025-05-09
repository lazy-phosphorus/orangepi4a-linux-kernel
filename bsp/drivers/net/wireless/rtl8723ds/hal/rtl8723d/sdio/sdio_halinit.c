/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#define _SDIO_HALINIT_C_

#include <rtl8723d_hal.h>
#include "hal_com_h2c.h"

/*
 * Description:
 *	Call power on sequence to enable card
 *
 * Return:
 *	_SUCCESS	enable success
 *	_FAIL		enable fail
 */
static u8 CardEnable(PADAPTER padapter)
{
	u8 val8 = 0;
	u8 bMacPwrCtrlOn;
	u8 ret = _FAIL;

	bMacPwrCtrlOn = _FALSE;
	rtw_hal_get_hwreg(padapter, HW_VAR_APFM_ON_MAC, &bMacPwrCtrlOn);
	if (bMacPwrCtrlOn == _FALSE) {
		/* RSV_CTRL 0x1C[7:0] = 0x00 */
		/* unlock ISO/CLK/Power control register */
		val8 = rtw_read8(padapter, REG_WLLPS_CTRL + 3);

		if (val8 & BIT(1))
			RTW_INFO("%s: LP-LPS: %02x\n", __func__, val8);

		rtw_write8(padapter, REG_RSV_CTRL, 0x0);

		ret = HalPwrSeqCmdParsing(padapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_SDIO_MSK, rtl8723D_card_enable_flow);
		if (ret == _SUCCESS) {
			u8 bMacPwrCtrlOn = _TRUE;

			rtw_hal_set_hwreg(padapter, HW_VAR_APFM_ON_MAC, &bMacPwrCtrlOn);
		}
	} else
		ret = _SUCCESS;

	return ret;
}

/* static */
u32 _InitPowerOn_8723DS(PADAPTER padapter)
{
	u8 value8;
	u16 value16;
	u32 value32;
	u8 ret;
	u8 pwron_chk_cnt = 0;
	/*	u8 bMacPwrCtrlOn; */

_init_power_on:

#if 1
	/* all of these MUST be configured before power on */
#ifdef CONFIG_XTAL_26M
	/* Config PLL Reference CLK, */
	/* Change crystal to 26M, APLL_FREF_SEL = 4b'0101 */
	/* APLL_FREF_SEL[0]=1b'1 */
	value8 = rtw_read8(padapter, REG_AFE_PLL_CTRL);
	value8 |= BIT(2);
	rtw_write8(padapter, REG_AFE_PLL_CTRL, value8);
	/* APLL_FREF_SEL[2:1]=2b'10 */
	value8 = rtw_read8(padapter, REG_AFE_CTRL_4_8723D + 1);
	value8 &= ~(BIT(1) | BIT(0));
	value8 |= BIT(1);
	rtw_write16(padapter, REG_AFE_CTRL_4_8723D + 1, value8);
	/* APLL_FREF_SEL[3]=1b'0 */
	value8 = rtw_read8(padapter, REG_AFE_CTRL_4_8723D);
	value8 &= ~BIT(7);
	rtw_write16(padapter, REG_AFE_CTRL_4_8723D, value8);
#endif /* CONFIG_XTAL_26M */

#ifdef CONFIG_EXT_CLK
	/* Use external crystal(XTAL) */
	value8 = rtw_read8(padapter, REG_PAD_CTRL1_8723D + 2);
	value8 |=  BIT(7);
	rtw_write8(padapter, REG_PAD_CTRL1_8723D + 2, value8);

	/* CLK_REQ High active or Low Active */
	/* Request GPIO polarity: */
	/* 0: low active */
	/* 1: high active */
	value8 = rtw_read8(padapter, REG_MULTI_FUNC_CTRL + 1);
	value8 |= BIT(5);
	rtw_write8(padapter, REG_MULTI_FUNC_CTRL + 1, value8);
#endif /* CONFIG_EXT_CLK */
#endif /* all of these MUST be configured before power on */

	/* only cmd52 can be used before power on(card enable) */
	ret = CardEnable(padapter);
	if (ret == _FALSE) {
		return _FAIL;
	}

	/* Radio-Off Pin Trigger */
	value8 = rtw_read8(padapter, REG_GPIO_INTM + 1);
	value8 |= BIT(1); /* Enable falling edge triggering interrupt */
	rtw_write8(padapter, REG_GPIO_INTM + 1, value8);
	value8 = rtw_read8(padapter, REG_GPIO_IO_SEL_2 + 1);
	value8 |= BIT(1);
	rtw_write8(padapter, REG_GPIO_IO_SEL_2 + 1, value8);

	/* Enable power down and GPIO interrupt */
	value16 = rtw_read16(padapter, REG_APS_FSMCO);
	value16 |= EnPDN; /* Enable HW power down and RF on */
	rtw_write16(padapter, REG_APS_FSMCO, value16);

	/* Enable CMD53 R/W Operation
	*	bMacPwrCtrlOn = _TRUE;
	*	rtw_hal_set_hwreg(padapter, HW_VAR_APFM_ON_MAC, &bMacPwrCtrlOn); */

	rtw_write8(padapter, REG_CR, 0x00);
	/* Enable MAC DMA/WMAC/SCHEDULE/SEC block */
	value16 = rtw_read16(padapter, REG_CR);
	value16 |= (HCI_TXDMA_EN | HCI_RXDMA_EN | TXDMA_EN | RXDMA_EN
		    | PROTOCOL_EN | SCHEDULE_EN | ENSEC | CALTMR_EN);
	rtw_write16(padapter, REG_CR, value16);


	/* PowerOnCheck() */
	ret = sdio_power_on_check(padapter);
	pwron_chk_cnt++;
	if (_FAIL == ret) {
		if (pwron_chk_cnt > 1) {
			RTW_INFO("Failed to init Power On!\n");
			return _FAIL;
		}
		RTW_INFO("Power on Fail! do it again\n");
		goto _init_power_on;
	} else {
		/* MAC team suggest to keep ana clock at 12MHz  0x04[22] = 1 */
		value8 = rtw_read8(padapter, REG_APS_FSMCO + 2);
		value8 |= BIT(6);
		rtw_write8(padapter, REG_APS_FSMCO + 2, value8);
	}

	value8 = rtw_read8(padapter, REG_SYS_CFG1_8723D + 1);
	RTW_INFO("%s: %s\n", __func__,
		(value8 & BIT3) ? "Test Mode" : "Normal Mode");

	value8 = rtw_read8(padapter, REG_SYS_CFG1_8723D + 3);
	RTW_INFO("%s: %s\n", __func__,
		(value8 & BIT0) ? "LDO Mode" : "SPS Mode");

	if(!(value8 & BIT0)){
		/*MAC team suggest to set 0x38[6] = 1 in SPS Mode*/
		value8 = rtw_read8(padapter, REG_PWR_DATA_8723D);
		value8 |= BIT(6);
		rtw_write8(padapter, REG_PWR_DATA_8723D, value8);
	}

	return _SUCCESS;
}

 

static void _InitQueueReservedPage(PADAPTER padapter)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(padapter);
	struct registry_priv	*pregistrypriv = &padapter->registrypriv;
	u32			outEPNum	= (u32)pHalData->OutEpNumber;
	u32			numHQ		= 0;
	u32			numLQ		= 0;
	u32			numNQ		= 0;
	u32			numPubQ;
	u32			value32;
	u8			value8;
	BOOLEAN			bWiFiConfig	= pregistrypriv->wifi_spec;

	if (pHalData->OutEpQueueSel & TX_SELE_HQ)
		numHQ = bWiFiConfig ? WMM_NORMAL_PAGE_NUM_HPQ_8723D : NORMAL_PAGE_NUM_HPQ_8723D;

	if (pHalData->OutEpQueueSel & TX_SELE_LQ)
		numLQ = bWiFiConfig ? WMM_NORMAL_PAGE_NUM_LPQ_8723D : NORMAL_PAGE_NUM_LPQ_8723D;

	/* NOTE: This step shall be proceed before writing REG_RQPN. */
	if (pHalData->OutEpQueueSel & TX_SELE_NQ)
		numNQ = bWiFiConfig ? WMM_NORMAL_PAGE_NUM_NPQ_8723D : NORMAL_PAGE_NUM_NPQ_8723D;

	numPubQ = TX_TOTAL_PAGE_NUMBER_8723D - numHQ - numLQ - numNQ;

	value8 = (u8)_NPQ(numNQ);
	rtw_write8(padapter, REG_RQPN_NPQ, value8);

	/* TX DMA */
	value32 = _HPQ(numHQ) | _LPQ(numLQ) | _PUBQ(numPubQ) | LD_RQPN;
	rtw_write32(padapter, REG_RQPN, value32);

	rtw_hal_set_sdio_tx_max_length(padapter, numHQ, numNQ, numLQ, numPubQ, SDIO_TX_DIV_NUM);

#ifdef CONFIG_SDIO_TX_ENABLE_AVAL_INT
	rtw_hal_sdio_avail_page_threshold_init(padapter);
#endif
}
static void _InitTxBufferBoundary(PADAPTER padapter)
{
	struct registry_priv *pregistrypriv = &padapter->registrypriv;
#ifdef CONFIG_CONCURRENT_MODE
	u8 val8;
#endif /* CONFIG_CONCURRENT_MODE */

	/* u16	txdmactrl; */
	u8	txpktbuf_bndy;

	if (!pregistrypriv->wifi_spec)
		txpktbuf_bndy = TX_PAGE_BOUNDARY_8723D;
	else {
		/* for WMM */
		txpktbuf_bndy = WMM_NORMAL_TX_PAGE_BOUNDARY_8723D;
	}

	rtw_write8(padapter, REG_TXPKTBUF_BCNQ_BDNY_8723D, txpktbuf_bndy);
	rtw_write8(padapter, REG_TXPKTBUF_MGQ_BDNY_8723D, txpktbuf_bndy);
	rtw_write8(padapter, REG_TXPKTBUF_WMAC_LBK_BF_HD_8723D, txpktbuf_bndy);
	rtw_write8(padapter, REG_TRXFF_BNDY, txpktbuf_bndy);
	rtw_write8(padapter, REG_TDECTRL + 1, txpktbuf_bndy);

#ifdef CONFIG_CONCURRENT_MODE
	val8 = txpktbuf_bndy + BCNQ_PAGE_NUM_8723D + WOWLAN_PAGE_NUM_8723D;
	rtw_write8(padapter, REG_BCNQ1_BDNY, val8);
	rtw_write8(padapter, REG_DWBCN1_CTRL_8723D + 1, val8); /* BCN1_HEAD */

	val8 = rtw_read8(padapter, REG_DWBCN1_CTRL_8723D + 2);
	val8 |= BIT(1); /* BIT1- BIT_SW_BCN_SEL_EN */
	rtw_write8(padapter, REG_DWBCN1_CTRL_8723D + 2, val8);
#endif /* CONFIG_CONCURRENT_MODE */
}

static void
_InitNormalChipRegPriority(
		PADAPTER	Adapter,
		u16		beQ,
		u16		bkQ,
		u16		viQ,
		u16		voQ,
		u16		mgtQ,
		u16		hiQ
)
{
	u16 value16		= (rtw_read16(Adapter, REG_TRXDMA_CTRL) & 0x7);

	value16 |=	_TXDMA_BEQ_MAP(beQ)	| _TXDMA_BKQ_MAP(bkQ) |
			_TXDMA_VIQ_MAP(viQ)	| _TXDMA_VOQ_MAP(voQ) |
			_TXDMA_MGQ_MAP(mgtQ) | _TXDMA_HIQ_MAP(hiQ);

	rtw_write16(Adapter, REG_TRXDMA_CTRL, value16);
}

static void
_InitNormalChipOneOutEpPriority(
		PADAPTER Adapter
)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(Adapter);

	u16	value = 0;

	switch (pHalData->OutEpQueueSel) {
	case TX_SELE_HQ:
		value = QUEUE_HIGH;
		break;
	case TX_SELE_LQ:
		value = QUEUE_LOW;
		break;
	case TX_SELE_NQ:
		value = QUEUE_NORMAL;
		break;
	default:
		/* RT_ASSERT(FALSE,("Shall not reach here!\n")); */
		break;
	}

	_InitNormalChipRegPriority(Adapter,
				   value,
				   value,
				   value,
				   value,
				   value,
				   value
				  );

}

static void
_InitNormalChipTwoOutEpPriority(
		PADAPTER Adapter
)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(Adapter);
	struct registry_priv *pregistrypriv = &Adapter->registrypriv;
	u16			beQ, bkQ, viQ, voQ, mgtQ, hiQ;


	u16	valueHi = 0;
	u16	valueLow = 0;

	switch (pHalData->OutEpQueueSel) {
	case (TX_SELE_HQ | TX_SELE_LQ):
		valueHi = QUEUE_HIGH;
		valueLow = QUEUE_LOW;
		break;
	case (TX_SELE_NQ | TX_SELE_LQ):
		valueHi = QUEUE_NORMAL;
		valueLow = QUEUE_LOW;
		break;
	case (TX_SELE_HQ | TX_SELE_NQ):
		valueHi = QUEUE_HIGH;
		valueLow = QUEUE_NORMAL;
		break;
	default:
		/* RT_ASSERT(FALSE,("Shall not reach here!\n")); */
		break;
	}

	if (!pregistrypriv->wifi_spec) {
		beQ		= valueLow;
		bkQ		= valueLow;
		viQ		= valueHi;
		voQ		= valueHi;
		mgtQ	= valueHi;
		hiQ		= valueHi;
	} else { /* for WMM ,CONFIG_OUT_EP_WIFI_MODE */
		beQ		= valueLow;
		bkQ		= valueHi;
		viQ		= valueHi;
		voQ		= valueLow;
		mgtQ	= valueHi;
		hiQ		= valueHi;
	}

	_InitNormalChipRegPriority(Adapter, beQ, bkQ, viQ, voQ, mgtQ, hiQ);

}

static void
_InitNormalChipThreeOutEpPriority(
		PADAPTER padapter
)
{
	struct registry_priv *pregistrypriv = &padapter->registrypriv;
	u16			beQ, bkQ, viQ, voQ, mgtQ, hiQ;

	if (!pregistrypriv->wifi_spec) { /* typical setting */
		beQ		= QUEUE_LOW;
		bkQ		= QUEUE_LOW;
		viQ		= QUEUE_NORMAL;
		voQ		= QUEUE_HIGH;
		mgtQ	= QUEUE_HIGH;
		hiQ		= QUEUE_HIGH;
	} else { /* for WMM */
		beQ		= QUEUE_LOW;
		bkQ		= QUEUE_NORMAL;
		viQ		= QUEUE_NORMAL;
		voQ		= QUEUE_HIGH;
		mgtQ	= QUEUE_HIGH;
		hiQ		= QUEUE_HIGH;
	}
	_InitNormalChipRegPriority(padapter, beQ, bkQ, viQ, voQ, mgtQ, hiQ);
}

static void
_InitNormalChipQueuePriority(
		PADAPTER Adapter
)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(Adapter);

	switch (pHalData->OutEpNumber) {
	case 1:
		_InitNormalChipOneOutEpPriority(Adapter);
		break;
	case 2:
		_InitNormalChipTwoOutEpPriority(Adapter);
		break;
	case 3:
		_InitNormalChipThreeOutEpPriority(Adapter);
		break;
	default:
		/* RT_ASSERT(FALSE,("Shall not reach here!\n")); */
		break;
	}


}

static void _InitQueuePriority(PADAPTER padapter)
{
	_InitNormalChipQueuePriority(padapter);
}

static void _InitPageBoundary(PADAPTER padapter)
{
	/* RX Page Boundary */
	u16 rxff_bndy = RX_DMA_BOUNDARY_8723D;

	rtw_write16(padapter, (REG_TRXFF_BNDY + 2), rxff_bndy);
}

static void _InitTransferPageSize(PADAPTER padapter)
{
	/* Tx page size is always 128. */

	u8 value8;

	value8 = _PSRX(PBP_128) | _PSTX(PBP_128);
	rtw_write8(padapter, REG_PBP, value8);
}

void _InitDriverInfoSize(PADAPTER padapter, u8 drvInfoSize)
{
	rtw_write8(padapter, REG_RX_DRVINFO_SZ, drvInfoSize);
}

void _InitNetworkType(PADAPTER padapter)
{
	u32 value32;

	value32 = rtw_read32(padapter, REG_CR);

	/* TODO: use the other function to set network type
	*	value32 = (value32 & ~MASK_NETTYPE) | _NETTYPE(NT_LINK_AD_HOC); */
	value32 = (value32 & ~MASK_NETTYPE) | _NETTYPE(NT_LINK_AP);

	rtw_write32(padapter, REG_CR, value32);
}

void _InitWMACSetting(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData;
	u16 value16;
	u32 rcr;

	pHalData = GET_HAL_DATA(padapter);

	rcr = 0
		| RCR_APM | RCR_AM | RCR_AB
		| RCR_CBSSID_DATA | RCR_CBSSID_BCN | RCR_AMF
		| RCR_HTC_LOC_CTRL
		| RCR_APP_PHYST_RXFF | RCR_APP_ICV | RCR_APP_MIC
		| RCR_APPFCS
		#ifdef CONFIG_MAC_LOOPBACK_DRIVER
		| RCR_AAP
		| RCR_ADD3 | RCR_APWRMGT | RCR_ACRC32 | RCR_ADF
		#endif
		;
	rtw_hal_set_hwreg(padapter, HW_VAR_RCR, (u8 *)&rcr);

	/* Accept all multicast address */
	rtw_write32(padapter, REG_MAR, 0xFFFFFFFF);
	rtw_write32(padapter, REG_MAR + 4, 0xFFFFFFFF);

	/* Accept all data frames */
	value16 = 0xFFFF;
	rtw_write16(padapter, REG_RXFLTMAP2, value16);

	/* 2010.09.08 hpfan */
	/* Since ADF is removed from RCR, ps-poll will not be indicate to driver, */
	/* RxFilterMap should mask ps-poll to gurantee AP mode can rx ps-poll. */
	value16 = 0x400;
	rtw_write16(padapter, REG_RXFLTMAP1, value16);

	/* Accept all management frames */
	value16 = 0xFFFF;
	rtw_write16(padapter, REG_RXFLTMAP0, value16);
}

void _InitAdaptiveCtrl(PADAPTER padapter)
{
	u16	value16;
	u32	value32;

	/* Response Rate Set */
	value32 = rtw_read32(padapter, REG_RRSR);
	value32 &= ~RATE_BITMAP_ALL;
	value32 |= RATE_RRSR_CCK_ONLY_1M;

	rtw_phydm_set_rrsr(padapter, value32, TRUE);


	/* CF-END Threshold */
	/* m_spIoBase->rtw_write8(REG_CFEND_TH, 0x1); */

	/* Retry Limit */
	value16 = BIT_LRL(RL_VAL_STA) | BIT_SRL(RL_VAL_STA);
	rtw_write16(padapter, REG_RETRY_LIMIT, value16);
}

void _InitEDCA(PADAPTER padapter)
{
	/* TXOP */
	rtw_write32(padapter, REG_EDCA_BE_PARAM, 0x005EA42B);
	rtw_write32(padapter, REG_EDCA_BK_PARAM, 0x0000A44F);
	rtw_write32(padapter, REG_EDCA_VI_PARAM, 0x005EA324);
	rtw_write32(padapter, REG_EDCA_VO_PARAM, 0x002FA226);
}

void _InitRetryFunction(PADAPTER padapter)
{
	u8	value8;

	value8 = rtw_read8(padapter, REG_FWHW_TXQ_CTRL);
	value8 |= EN_AMPDU_RTY_NEW;
	rtw_write8(padapter, REG_FWHW_TXQ_CTRL, value8);

	/* Set ACK timeout */
	rtw_write8(padapter, REG_ACKTO, 0x40);
}

static void HalRxAggr8723DSdio(PADAPTER padapter)
{
	u8	dma_time_th;
	u8	dma_len_th;

	if (padapter->registrypriv.wifi_spec) {
		dma_time_th = 0x01;
		dma_len_th = 0x0f;
	} else {
		dma_time_th = 0x03;
		dma_len_th = 0x0a;
	}

	if (dma_len_th * 1024 > MAX_RECVBUF_SZ) {
		RTW_PRINT("Reduce RXDMA_AGG_LEN_TH from %u to %u\n"
			  , dma_len_th, MAX_RECVBUF_SZ / 1024);
		dma_len_th = MAX_RECVBUF_SZ / 1024;
	}

	rtw_write16(padapter, REG_RXDMA_AGG_PG_TH, (dma_time_th << 8) | dma_len_th);

}

void sdio_AggSettingRxUpdate(PADAPTER padapter)
{
	HAL_DATA_TYPE *pHalData;
	u8 val8;
	u8 valueDMA;
	u8 valueRxAggCtrl = 0;
	u8 aggBurstNum = 3;  /* 0:1, 1:2, 2:3, 3:4 */
	u8 aggBurstSize = 0;  /* 0:1K, 1:512Byte, 2:256Byte... */

	pHalData = GET_HAL_DATA(padapter);

	val8 = rtw_read8(padapter, REG_RXDMA_AGG_PG_TH + 3);
	val8 = val8 | BIT(7) | BIT(5);
	rtw_write8(padapter, REG_RXDMA_AGG_PG_TH + 3, val8);
	
	valueDMA = rtw_read8(padapter, REG_TRXDMA_CTRL);
	valueDMA |= RXDMA_AGG_EN;
	rtw_write8(padapter, REG_TRXDMA_CTRL, valueDMA);

	valueRxAggCtrl |= RXDMA_AGG_MODE_EN;
	valueRxAggCtrl |= ((aggBurstNum << 2) & 0x0C);
	valueRxAggCtrl |= ((aggBurstSize << 4) & 0x30);
	rtw_write8(padapter, REG_RXDMA_MODE_CTRL_8723D, valueRxAggCtrl);/* RxAggLowThresh = 4*1K */
}

void _initSdioAggregationSetting(PADAPTER padapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);

	/* Tx aggregation setting
	*	sdio_AggSettingTxUpdate(padapter); */

	/* Rx aggregation setting */
	HalRxAggr8723DSdio(padapter);

	sdio_AggSettingRxUpdate(padapter);
}
#if 0
static void _RXAggrSwitch(PADAPTER padapter, u8 enable)
{
	PHAL_DATA_TYPE pHalData;
	u8 valueDMA;
	u8 valueRxAggCtrl;


	pHalData = GET_HAL_DATA(padapter);

	valueDMA = rtw_read8(padapter, REG_TRXDMA_CTRL);
	valueRxAggCtrl = rtw_read8(padapter, REG_RXDMA_MODE_CTRL_8723D);

	if (_TRUE == enable) {
		valueDMA |= RXDMA_AGG_EN;
		valueRxAggCtrl |= RXDMA_AGG_MODE_EN;
	} else {
		valueDMA &= ~RXDMA_AGG_EN;
		valueRxAggCtrl &= ~RXDMA_AGG_MODE_EN;
	}

	rtw_write8(padapter, REG_TRXDMA_CTRL, valueDMA);
	rtw_write8(padapter, REG_RXDMA_MODE_CTRL_8723D, valueRxAggCtrl);
}
#endif

void _InitInterrupt(PADAPTER padapter)
{
	/* HISR - turn all off */
	rtw_write32(padapter, REG_HISR, 0);

	/* HIMR - turn all off */
	rtw_write32(padapter, REG_HIMR, 0);

	/* */
	/* Initialize and enable SDIO Host Interrupt. */
	/* */
	InitInterrupt8723DSdio(padapter);

	/* */
	/* Initialize system Host Interrupt. */
	/* */
	InitSysInterrupt8723DSdio(padapter);
}

void _InitRDGSetting(PADAPTER padapter)
{
	rtw_write8(padapter, REG_RD_CTRL, 0xFF);
	rtw_write16(padapter, REG_RD_NAV_NXT, 0x200);
	rtw_write8(padapter, REG_RD_RESP_PKT_TH, 0x05);
}

static void _InitRFType(PADAPTER padapter)
{
	struct registry_priv *pregpriv = &padapter->registrypriv;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(padapter);

#if	DISABLE_BB_RF
	pHalData->rf_chip	= RF_PSEUDO_11N;
	return;
#endif
	pHalData->rf_chip	= RF_6052;

	RTW_INFO("Set RF Chip ID to RF_6052 and RF type to %d.\n", pHalData->rf_type);
}

/* Set CCK and OFDM Block "ON" */
static void _BBTurnOnBlock(PADAPTER padapter)
{
#if (DISABLE_BB_RF)
	return;
#endif

	phy_set_bb_reg(padapter, rFPGA0_RFMOD, bCCKEn, 0x1);
	phy_set_bb_reg(padapter, rFPGA0_RFMOD, bOFDMEn, 0x1);
}

void _InitBBRegBackup_8723DS(PADAPTER	Adapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u8	i;

	/* For Channel 1~11 (Default Value)*/
	pHalData->RegForRecover[0].offset = rCCK0_TxFilter2;
	pHalData->RegForRecover[0].value =
		phy_query_bb_reg(Adapter,
			       pHalData->RegForRecover[0].offset, bMaskDWord);

	pHalData->RegForRecover[1].offset = rCCK0_DebugPort;
	pHalData->RegForRecover[1].value =
		phy_query_bb_reg(Adapter,
			       pHalData->RegForRecover[1].offset, bMaskDWord);

	pHalData->RegForRecover[2].offset = 0xAAC;
	pHalData->RegForRecover[2].value =
		phy_query_bb_reg(Adapter,
			       pHalData->RegForRecover[2].offset, bMaskDWord);
#if 0
	/* For 20 MHz	(Default Value)*/
	pHalData->RegForRecover[2].offset = rBBrx_DFIR;
	pHalData->RegForRecover[2].value = phy_query_bb_reg(Adapter, pHalData->RegForRecover[2].offset, bMaskDWord);

	pHalData->RegForRecover[3].offset = rOFDM0_XATxAFE;
	pHalData->RegForRecover[3].value = phy_query_bb_reg(Adapter, pHalData->RegForRecover[3].offset, bMaskDWord);

	pHalData->RegForRecover[4].offset = 0x1E;
	pHalData->RegForRecover[4].value = phy_query_rf_reg(Adapter, RF_PATH_A, pHalData->RegForRecover[4].offset, bRFRegOffsetMask);
#endif
}

/*
 * 2010/08/09 MH Add for power down check.
 *   */
static BOOLEAN HalDetectPwrDownMode(PADAPTER Adapter)
{
	u8 tmpvalue;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(Adapter);


	EFUSE_ShadowRead(Adapter, 1, EEPROM_FEATURE_OPTION_8723D, (u32 *)&tmpvalue);

	/* 2010/08/25 MH INF priority > PDN Efuse value. */
	if (tmpvalue & BIT(4) && pwrctrlpriv->reg_pdnmode)
		pHalData->pwrdown = _TRUE;
	else
		pHalData->pwrdown = _FALSE;

	RTW_INFO("HalDetectPwrDownMode(): PDN=%d\n", pHalData->pwrdown);

	return pHalData->pwrdown;
}	/* HalDetectPwrDownMode */

static void	rtl8723ds_init_burst_pkt_len(PADAPTER padapter)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(padapter);

	rtw_write8(padapter, 0x4c7, rtw_read8(padapter, 0x4c7) | BIT(7)); /* enable single pkt ampdu */
	rtw_write8(padapter, REG_RX_PKT_LIMIT_8723D, 0x18);		/* for VHT packet length 11K */
	rtw_write8(padapter, REG_MAX_AGGR_NUM_8723D, 0x1F);
	rtw_write8(padapter, REG_PIFS_8723D, 0x00);
	rtw_write8(padapter, REG_FWHW_TXQ_CTRL_8723D, rtw_read8(padapter, REG_FWHW_TXQ_CTRL) & (~BIT(7)));
	if (pHalData->AMPDUBurstMode)
		rtw_write8(padapter, REG_AMPDU_BURST_MODE_8723D,  0x5F);
	rtw_write8(padapter, REG_AMPDU_MAX_TIME_8723D, 0x70);

	/* ARFB table 9 for 11ac 5G 2SS */
	rtw_write32(padapter, REG_ARFR0_8723D, 0x00000010);
	if (IS_NORMAL_CHIP(pHalData->version_id))
		rtw_write32(padapter, REG_ARFR0_8723D + 4, 0xfffff000);
	else
		rtw_write32(padapter, REG_ARFR0_8723D + 4, 0x3e0ff000);

	/* ARFB table 10 for 11ac 5G 1SS */
	rtw_write32(padapter, REG_ARFR1_8723D, 0x00000010);
	rtw_write32(padapter, REG_ARFR1_8723D + 4, 0x003ff000);
}

#if 0
static void rtl8723ds_init_lte_coex(PADAPTER padapter)
{
	/* LTE COEX setting */
	rtw_write16(padapter, REG_LTECOEX_WRITE_DATA, 0x7700);
	rtw_write32(padapter, REG_LTECOEX_CTRL, 0xc0020038);
	rtw_write8(padapter, 0x73, 0x04);
}
#endif

static u32 rtl8723ds_hal_init(PADAPTER padapter)
{
	s32 ret;
	PHAL_DATA_TYPE pHalData;
	struct pwrctrl_priv *pwrctrlpriv;
	struct registry_priv *pregistrypriv;
	struct sreset_priv *psrtpriv;
	struct dvobj_priv *psdpriv = padapter->dvobj;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;
	rt_rf_power_state eRfPowerStateToSet;
	u32 NavUpper = WiFiNavUpperUs;
	u8 u1bTmp;
	u16 value16;
	u8 typeid;
	u32 u4Tmp;

	pHalData = GET_HAL_DATA(padapter);
	psrtpriv = &pHalData->srestpriv;
	pwrctrlpriv = adapter_to_pwrctl(padapter);
	pregistrypriv = &padapter->registrypriv;

#ifdef CONFIG_SWLPS_IN_IPS
	if (adapter_to_pwrctl(padapter)->bips_processing == _TRUE) {
		u8 val8, bMacPwrCtrlOn = _TRUE;

		RTW_INFO("%s: run LPS flow in IPS\n", __FUNCTION__);

		/* ser rpwm */
		val8 = rtw_read8(padapter, SDIO_LOCAL_BASE | SDIO_REG_HRPWM1);
		val8 &= 0x80;
		val8 += 0x80;
		val8 |= BIT(6);
		rtw_write8(padapter, SDIO_LOCAL_BASE | SDIO_REG_HRPWM1, val8);

		adapter_to_pwrctl(padapter)->tog = (val8 + 0x80) & 0x80;

		rtw_mdelay_os(5); /* wait set rpwm already */

		ret = HalPwrSeqCmdParsing(padapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_SDIO_MSK, rtl8723D_leave_swlps_flow);
		if (ret == _FALSE) {
			RTW_INFO("%s: run LPS flow in IPS fail!\n", __FUNCTION__);
			return _FAIL;
		}

		rtw_hal_set_hwreg(padapter, HW_VAR_APFM_ON_MAC, &bMacPwrCtrlOn);

		pHalData->LastHMEBoxNum = 0;

#ifdef CONFIG_BT_COEXIST
		rtw_btcoex_HAL_Initialize(padapter, _FALSE);
#else
		rtw_btcoex_HAL_Initialize(padapter, _TRUE);
#endif /* CONFIG_BT_COEXIST */

		return _SUCCESS;
	}
#elif defined(CONFIG_FWLPS_IN_IPS)
	if (adapter_to_pwrctl(padapter)->bips_processing == _TRUE && psrtpriv->silent_reset_inprogress == _FALSE
	    && adapter_to_pwrctl(padapter)->pre_ips_type == 0) {
		systime start_time;
		u8 cpwm_orig, cpwm_now;
		u8 val8, bMacPwrCtrlOn = _TRUE;

		RTW_INFO("%s: Leaving IPS in FWLPS state\n", __FUNCTION__);

		/* for polling cpwm */
		cpwm_orig = 0;
		rtw_hal_get_hwreg(padapter, HW_VAR_CPWM, &cpwm_orig);

		/* ser rpwm */
		val8 = rtw_read8(padapter, SDIO_LOCAL_BASE | SDIO_REG_HRPWM1);
		val8 &= 0x80;
		val8 += 0x80;
		val8 |= BIT(6);
		rtw_write8(padapter, SDIO_LOCAL_BASE | SDIO_REG_HRPWM1, val8);
		RTW_INFO("%s: write rpwm=%02x\n", __FUNCTION__, val8);
		adapter_to_pwrctl(padapter)->tog = (val8 + 0x80) & 0x80;

		/* do polling cpwm */
		start_time = rtw_get_current_time();
		do {

			rtw_mdelay_os(1);

			rtw_hal_get_hwreg(padapter, HW_VAR_CPWM, &cpwm_now);
			if ((cpwm_orig ^ cpwm_now) & 0x80) {
#ifdef DBG_CHECK_FW_PS_STATE
				RTW_INFO("%s: polling cpwm ok when leaving IPS in FWLPS state, cpwm_orig=%02x, cpwm_now=%02x, 0x100=0x%x\n"
					, __FUNCTION__, cpwm_orig, cpwm_now, rtw_read8(padapter, REG_CR));
#endif /* DBG_CHECK_FW_PS_STATE */
				break;
			}

			if (rtw_get_passing_time_ms(start_time) > 100) {
				RTW_INFO("%s: polling cpwm timeout when leaving IPS in FWLPS state\n", __FUNCTION__);
				break;
			}
		} while (1);

		rtw_set_fw_in_ips_mode(padapter, 0);

		rtw_hal_set_hwreg(padapter, HW_VAR_APFM_ON_MAC, &bMacPwrCtrlOn);

#ifdef CONFIG_BT_COEXIST
		rtw_btcoex_HAL_Initialize(padapter, _FALSE);
#else
		rtw_btcoex_HAL_Initialize(padapter, _TRUE);
#endif /* CONFIG_BT_COEXIST */

#ifdef DBG_CHECK_FW_PS_STATE
		if (rtw_fw_ps_state(padapter) == _FAIL) {
			RTW_INFO("after hal init, fw ps state in 32k\n");
			pdbgpriv->dbg_ips_drvopen_fail_cnt++;
		}
#endif /* DBG_CHECK_FW_PS_STATE */
		return _SUCCESS;
	}
#endif /* CONFIG_SWLPS_IN_IPS */

	/* Disable Interrupt first.
	*	rtw_hal_disable_interrupt(padapter); */

	if (rtw_read8(padapter, REG_MCUFWDL) == 0xc6)
		RTW_INFO("FW exist before power on!!\n");
	else
		RTW_INFO("FW does not exist before power on!!\n");
#ifdef DBG_CHECK_FW_PS_STATE
	if (rtw_fw_ps_state(padapter) == _FAIL) {
		RTW_INFO("check fw_ps_state fail before PowerOn!\n");
		pdbgpriv->dbg_ips_drvopen_fail_cnt++;
	}
#endif
	ret = rtw_hal_power_on(padapter);
	if (_FAIL == ret) {
		return _FAIL;
	}
	RTW_INFO("Power on ok!\n");
#ifdef DBG_CHECK_FW_PS_STATE
	if (rtw_fw_ps_state(padapter) == _FAIL) {
		RTW_INFO("check fw_ps_state fail after PowerOn!\n");
		pdbgpriv->dbg_ips_drvopen_fail_cnt++;
	}
#endif

	rtw_write8(padapter, REG_EARLY_MODE_CONTROL, 0);

	if (padapter->registrypriv.mp_mode == 0
		#if defined(CONFIG_MP_INCLUDED) && defined(CONFIG_RTW_CUSTOMER_STR)
		|| padapter->registrypriv.mp_customer_str
		#endif
	) {
		ret = rtl8723d_FirmwareDownload(padapter, _FALSE);
		if (ret != _SUCCESS) {
			pHalData->bFWReady = _FALSE;
			pHalData->fw_ractrl = _FALSE;
			return ret;
		} /*else*/
		{
			pHalData->bFWReady = _TRUE;
			pHalData->fw_ractrl = _TRUE;
		}
	}

	/*	SIC_Init(padapter); */

	if (pwrctrlpriv->reg_rfoff == _TRUE)
		pwrctrlpriv->rf_pwrstate = rf_off;

	/* 2010/08/09 MH We need to check if we need to turnon or off RF after detecting */
	/* HW GPIO pin. Before PHY_RFConfig8192C. */
	HalDetectPwrDownMode(padapter);

	/* Set RF type for BB/RF configuration */
	_InitRFType(padapter);

	/* Save target channel */
	/* <Roger_Notes> Current Channel will be updated again later. */
	pHalData->current_channel = 6;

#if (HAL_MAC_ENABLE == 1)
	ret = PHY_MACConfig8723D(padapter);
	if (ret != _SUCCESS) {
		return ret;
	}
#endif
	/* */
	/* d. Initialize BB related configurations. */
	/* */
#if (HAL_BB_ENABLE == 1)
	ret = PHY_BBConfig8723D(padapter);
	if (ret != _SUCCESS) {
		return ret;
	}

#endif

	/* If RF is on, we need to init RF. Otherwise, skip the procedure. */
	/* We need to follow SU method to change the RF cfg.txt. Default disable RF TX/RX mode. */
	/* if(pHalData->eRFPowerState == eRfOn) */
	{
#if (HAL_RF_ENABLE == 1)
		ret = PHY_RFConfig8723D(padapter);
		if (ret != _SUCCESS) {
			return ret;
		}
#endif
	}

	_InitBBRegBackup_8723DS(padapter);

	_InitMacAPLLSetting_8723D(padapter);

	/* */
	/* Joseph Note: Keep RfRegChnlVal for later use. */
	/* */
	pHalData->RfRegChnlVal[0] = phy_query_rf_reg(padapter, RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask);
	pHalData->RfRegChnlVal[1] = phy_query_rf_reg(padapter, RF_PATH_B, RF_CHNLBW, bRFRegOffsetMask);

#ifdef CONFIG_DLFW_TXPKT
	/* Specially add for FWDL by Tx pkt write.
	   Reset Tx/Rx DMA since the Tx boundary setting
	   is changed during FW download */
	rtw_write8(padapter, REG_CR, 0x00);
	rtw_write8(padapter, REG_CR, 0xFF);
#endif

	/* if (!pHalData->bMACFuncEnable) { */
	_InitQueueReservedPage(padapter);
	_InitTxBufferBoundary(padapter);
	/* init LLT after tx buffer boundary is defined */
	ret = rtl8723d_InitLLTTable(padapter);
	if (_SUCCESS != ret) {
		RTW_INFO("%s: Failed to init LLT Table!\n", __func__);
		return _FAIL;
	}
	/* } */
	_InitQueuePriority(padapter);
	_InitPageBoundary(padapter);
	_InitTransferPageSize(padapter);

	/* Get Rx PHY status in order to report RSSI and others. */
	_InitDriverInfoSize(padapter, DRVINFO_SZ);
	_InitNetworkType(padapter);
	_InitWMACSetting(padapter);
	_InitAdaptiveCtrl(padapter);
	_InitEDCA(padapter);
	_InitRetryFunction(padapter);
	_initSdioAggregationSetting(padapter);
	rtl8723d_InitBeaconParameters(padapter);
	rtl8723d_InitBeaconMaxError(padapter, _TRUE);
	_InitInterrupt(padapter);
	rtl8723ds_init_burst_pkt_len(padapter);

#if 0
	/* 8723D new ADD */
	rtl8723ds_init_lte_coex(padapter);
#endif

	/* YJ,TODO */
	rtw_write8(padapter, REG_SECONDARY_CCA_CTRL_8723D, 0x3);	/* CCA */
	rtw_write8(padapter, 0x976, 0);	/* hpfan_todo: 2nd CCA related */

	_BBTurnOnBlock(padapter);

	invalidate_cam_all(padapter);

	rtw_hal_set_chnl_bw(padapter, padapter->registrypriv.channel,
		CHANNEL_WIDTH_20, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HAL_PRIME_CHNL_OFFSET_DONT_CARE);

	rtl8723d_InitAntenna_Selection(padapter);

	/* */
	/* Disable BAR, suggested by Scott */
	/* 2010.04.09 add by hpfan */
	/* */
	rtw_write32(padapter, REG_BAR_MODE_CTRL, 0x0201ffff);

	/* HW SEQ CTRL */
	/* set 0x0 to 0xFF by tynli. Default enable HW SEQ NUM. */
	rtw_write8(padapter, REG_HWSEQ_CTRL, 0xFF);


#ifdef CONFIG_MAC_LOOPBACK_DRIVER
	u1bTmp = rtw_read8(padapter, REG_SYS_FUNC_EN);
	u1bTmp &= ~(FEN_BBRSTB | FEN_BB_GLB_RSTn);
	rtw_write8(padapter, REG_SYS_FUNC_EN, u1bTmp);

	rtw_write8(padapter, REG_RD_CTRL, 0x0F);
	rtw_write8(padapter, REG_RD_CTRL + 1, 0xCF);
	rtw_write8(padapter, REG_TXPKTBUF_WMAC_LBK_BF_HD, 0x80);
	rtw_write32(padapter, REG_CR, 0x0b0202ff);
#endif

	/* */
	/* Configure SDIO TxRx Control to enable Rx DMA timer masking. */
	/* 2010.02.24. */
	/* */
	rtw_write32(padapter, SDIO_LOCAL_BASE | SDIO_REG_TX_CTRL, 0);

	rtl8723d_InitHalDm(padapter);

	/* dbg_print("pHalData->DefaultTxPwrDbm = %d\n", pHalData->DefaultTxPwrDbm); */

#if 0
	if (pHalData->SwBeaconType < HAL92CSDIO_DEFAULT_BEACON_TYPE) /*  The lowest Beacon Type that HW can support */
		*pHalData->SwBeaconType = HAL92CSDIO_DEFAULT_BEACON_TYPE;
#endif

	/* */
	/* Update current Tx FIFO page status. */
	/* */
	HalQueryTxBufferStatus8723DSdio(padapter);
	HalQueryTxOQTBufferStatus8723DSdio(padapter);
	pHalData->SdioTxOQTMaxFreeSpace = pHalData->SdioTxOQTFreeSpace;

	/* Enable MACTXEN/MACRXEN block */
	u1bTmp = rtw_read8(padapter, REG_CR);
	u1bTmp |= (MACTXEN | MACRXEN);
	rtw_write8(padapter, REG_CR, u1bTmp);

	rtw_hal_set_hwreg(padapter, HW_VAR_NAV_UPPER, (u8 *)&NavUpper);

#ifdef CONFIG_XMIT_ACK
	/* ack for xmit mgmt frames. */
	rtw_write32(padapter, REG_FWHW_TXQ_CTRL, rtw_read32(padapter, REG_FWHW_TXQ_CTRL) | BIT(12));
#endif /* CONFIG_XMIT_ACK	 */

	/*	pHalData->PreRpwmVal = SdioLocalCmd52Read1Byte(padapter, SDIO_REG_HRPWM1) & 0x80; */

#if (MP_DRIVER == 1)
	if (padapter->registrypriv.mp_mode == 1) {
		padapter->mppriv.channel = pHalData->current_channel;
		MPT_InitializeAdapter(padapter, padapter->mppriv.channel);
	} else
#endif /* #if (MP_DRIVER == 1) */
	{
		pwrctrlpriv->rf_pwrstate = rf_on;

		halrf_lck_trigger(&pHalData->odmpriv);

		pHalData->neediqk_24g = _TRUE;

		odm_txpowertracking_check(&pHalData->odmpriv);
	}

#ifdef CONFIG_BT_COEXIST
	/* Init BT hw config.*/
	if (padapter->registrypriv.mp_mode == 1)
		rtw_btcoex_HAL_Initialize(padapter, _TRUE);
	else
		rtw_btcoex_HAL_Initialize(padapter, _FALSE);
#endif

	if (padapter->registrypriv.wifi_spec == 1)
		phy_set_bb_reg(padapter, rOFDM0_ECCAThreshold,
			       0x00ff00ff, 0x00250029);

	return _SUCCESS;
}

static void CardDisableRTL8723DSdio(PADAPTER padapter)
{
	u8		u1bTmp;
	u16		u2bTmp;
	u32		u4bTmp;
	u8		bMacPwrCtrlOn;
	u8		ret = _FAIL;

	/* Run LPS WL RFOFF flow */
	ret = HalPwrSeqCmdParsing(padapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_SDIO_MSK, rtl8723D_enter_lps_flow);
	if (ret == _FAIL)
		RTW_INFO(KERN_ERR "%s: run RF OFF flow fail!\n", __func__);

	/*	==== Reset digital sequence   ====== */

	u1bTmp = rtw_read8(padapter, REG_MCUFWDL);
	if ((u1bTmp & RAM_DL_SEL) && GET_HAL_DATA(padapter)->bFWReady) /* 8051 RAM code */
		rtl8723d_FirmwareSelfReset(padapter);

	/* Reset MCU 0x2[10]=0. Suggested by Filen. 2011.01.26. by tynli. */
	u1bTmp = rtw_read8(padapter, REG_SYS_FUNC_EN + 1);
	u1bTmp &= ~BIT(2);	/* 0x2[10], FEN_CPUEN */
	rtw_write8(padapter, REG_SYS_FUNC_EN + 1, u1bTmp);

	/* MCUFWDL 0x80[1:0]=0 */
	/* reset MCU ready status */
	rtw_write8(padapter, REG_MCUFWDL, 0);

	/* Reset MCU IO Wrapper, added by Roger, 2011.08.30 */
	u1bTmp = rtw_read8(padapter, REG_RSV_CTRL + 1);
	u1bTmp &= ~BIT(0);
	rtw_write8(padapter, REG_RSV_CTRL + 1, u1bTmp);
	u1bTmp = rtw_read8(padapter, REG_RSV_CTRL + 1);
	u1bTmp |= BIT(0);
	rtw_write8(padapter, REG_RSV_CTRL + 1, u1bTmp);

	/*	==== Reset digital sequence end ====== */

	bMacPwrCtrlOn = _FALSE;	/* Disable CMD53 R/W */
	ret = _FALSE;
	rtw_hal_set_hwreg(padapter, HW_VAR_APFM_ON_MAC, &bMacPwrCtrlOn);
	ret = HalPwrSeqCmdParsing(padapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_SDIO_MSK, rtl8723D_card_disable_flow);
	if (ret == _FALSE)
		RTW_INFO(KERN_ERR "%s: run CARD DISABLE flow fail!\n", __func__);

	GET_HAL_DATA(padapter)->bFWReady = _FALSE;
}

static u32 rtl8723ds_hal_deinit(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;
	struct dvobj_priv *psdpriv = padapter->dvobj;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;

#ifdef CONFIG_MP_INCLUDED
	if (padapter->registrypriv.mp_mode == 1)
		MPT_DeInitAdapter(padapter);
#endif

	if (rtw_is_hw_init_completed(padapter)) {
#ifdef CONFIG_SWLPS_IN_IPS
		if (adapter_to_pwrctl(padapter)->bips_processing == _TRUE) {
			u8	bMacPwrCtrlOn;
			u8 ret =  _TRUE;

			RTW_INFO("%s: run LPS flow in IPS\n", __FUNCTION__);

			rtw_write32(padapter, 0x130, 0x0);
			rtw_write32(padapter, 0x138, 0x100);
			rtw_write8(padapter, 0x13d, 0x1);


			bMacPwrCtrlOn = _FALSE;	/* Disable CMD53 R/W	 */
			rtw_hal_set_hwreg(padapter, HW_VAR_APFM_ON_MAC, &bMacPwrCtrlOn);

			ret = HalPwrSeqCmdParsing(padapter, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_SDIO_MSK, rtl8723D_enter_swlps_flow);
			if (ret == _FALSE) {
				RTW_INFO("%s: run LPS flow in IPS fail!\n", __FUNCTION__);
				return _FAIL;
			}
		} else
#elif defined(CONFIG_FWLPS_IN_IPS)
		if (adapter_to_pwrctl(padapter)->bips_processing == _TRUE && psrtpriv->silent_reset_inprogress == _FALSE) {
			if (padapter->netif_up == _TRUE) {
				int cnt = 0;
				u8 val8 = 0;

				RTW_INFO("%s: issue H2C to FW when entering IPS\n", __FUNCTION__);

				rtw_set_fw_in_ips_mode(padapter, 0x1);
				/* poll 0x1cc to make sure H2C command already finished by FW; MAC_0x1cc=0 means H2C done by FW. */
				do {
					val8 = rtw_read8(padapter, REG_HMETFR);
					cnt++;
					RTW_INFO("%s  polling REG_HMETFR=0x%x, cnt=%d\n", __FUNCTION__, val8, cnt);
					rtw_mdelay_os(10);
				} while (cnt < 100 && (val8 != 0));
				/* H2C done, enter 32k */
				if (val8 == 0) {
					/* ser rpwm to enter 32k */
					val8 = rtw_read8(padapter, SDIO_LOCAL_BASE | SDIO_REG_HRPWM1);
					val8 += 0x80;
					val8 |= BIT(0);
					rtw_write8(padapter, SDIO_LOCAL_BASE | SDIO_REG_HRPWM1, val8);
					RTW_INFO("%s: write rpwm=%02x\n", __FUNCTION__, val8);
					adapter_to_pwrctl(padapter)->tog = (val8 + 0x80) & 0x80;
					cnt = val8 = 0;
					do {
						val8 = rtw_read8(padapter, REG_CR);
						cnt++;
						RTW_INFO("%s  polling 0x100=0x%x, cnt=%d\n", __FUNCTION__, val8, cnt);
						rtw_mdelay_os(10);
					} while (cnt < 100 && (val8 != 0xEA));
#ifdef DBG_CHECK_FW_PS_STATE
					if (val8 != 0xEA)
						RTW_INFO("MAC_1C0=%08x, MAC_1C4=%08x, MAC_1C8=%08x, MAC_1CC=%08x\n", rtw_read32(padapter, 0x1c0), rtw_read32(padapter, 0x1c4)
							, rtw_read32(padapter, 0x1c8), rtw_read32(padapter, 0x1cc));
#endif /* DBG_CHECK_FW_PS_STATE */
				} else {
					RTW_INFO("MAC_1C0=%08x, MAC_1C4=%08x, MAC_1C8=%08x, MAC_1CC=%08x\n", rtw_read32(padapter, 0x1c0), rtw_read32(padapter, 0x1c4)
						, rtw_read32(padapter, 0x1c8), rtw_read32(padapter, 0x1cc));
				}

				RTW_INFO("polling done when entering IPS, check result : 0x100=0x%x, cnt=%d, MAC_1cc=0x%02x\n"
					, rtw_read8(padapter, REG_CR), cnt, rtw_read8(padapter, REG_HMETFR));

				adapter_to_pwrctl(padapter)->pre_ips_type = 0;

			} else {
				pdbgpriv->dbg_carddisable_cnt++;
#ifdef DBG_CHECK_FW_PS_STATE
				if (rtw_fw_ps_state(padapter) == _FAIL) {
					RTW_INFO("card disable should leave 32k\n");
					pdbgpriv->dbg_carddisable_error_cnt++;
				}
#endif /* DBG_CHECK_FW_PS_STATE */
				rtw_hal_power_off(padapter);

				adapter_to_pwrctl(padapter)->pre_ips_type = 1;
			}

		} else
#endif /* CONFIG_SWLPS_IN_IPS */
		{
			pdbgpriv->dbg_carddisable_cnt++;
#ifdef DBG_CHECK_FW_PS_STATE
			if (rtw_fw_ps_state(padapter) == _FAIL) {
				RTW_INFO("card disable should leave 32k\n");
				pdbgpriv->dbg_carddisable_error_cnt++;
			}
#endif /* DBG_CHECK_FW_PS_STATE */
			rtw_hal_power_off(padapter);
		}
	} else
		pdbgpriv->dbg_deinit_fail_cnt++;

	return _SUCCESS;
}
static void rtl8723ds_init_default_value(PADAPTER padapter)
{
	PHAL_DATA_TYPE pHalData;


	pHalData = GET_HAL_DATA(padapter);

	rtl8723d_init_default_value(padapter);

	/* interface related variable */
	pHalData->SdioRxFIFOCnt = 0;
}

static void rtl8723ds_interface_configure(PADAPTER padapter)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(padapter);
	struct dvobj_priv		*pdvobjpriv = adapter_to_dvobj(padapter);
	struct registry_priv	*pregistrypriv = &padapter->registrypriv;
	BOOLEAN		bWiFiConfig	= pregistrypriv->wifi_spec;


	pdvobjpriv->RtOutPipe[0] = WLAN_TX_HIQ_DEVICE_ID;
	pdvobjpriv->RtOutPipe[1] = WLAN_TX_MIQ_DEVICE_ID;
	pdvobjpriv->RtOutPipe[2] = WLAN_TX_LOQ_DEVICE_ID;

	if (bWiFiConfig)
		pHalData->OutEpNumber = 2;
	else
		pHalData->OutEpNumber = SDIO_MAX_TX_QUEUE;

	switch (pHalData->OutEpNumber) {
	case 3:
		pHalData->OutEpQueueSel = TX_SELE_HQ | TX_SELE_LQ | TX_SELE_NQ;
		break;
	case 2:
		pHalData->OutEpQueueSel = TX_SELE_HQ | TX_SELE_NQ;
		break;
	case 1:
		pHalData->OutEpQueueSel = TX_SELE_HQ;
		break;
	default:
		break;
	}

	Hal_MappingOutPipe(padapter, pHalData->OutEpNumber);
}

/*
 *	Description:
 *		We should set Efuse cell selection to WiFi cell in default.
 *
 *	Assumption:
 *		PASSIVE_LEVEL
 *
 *	Added by Roger, 2010.11.23.
 *   */
static void
_EfuseCellSel(
		PADAPTER	padapter
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);

	u32			value32;

	/* if(INCLUDE_MULTI_FUNC_BT(padapter)) */
	{
		value32 = rtw_read32(padapter, EFUSE_TEST);
		value32 = (value32 & ~EFUSE_SEL_MASK) | EFUSE_SEL(EFUSE_WIFI_SEL_0);
		rtw_write32(padapter, EFUSE_TEST, value32);
	}
}

static void
_ReadRFType(
		PADAPTER	Adapter
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

#if DISABLE_BB_RF
	pHalData->rf_chip = RF_PSEUDO_11N;
#else
	pHalData->rf_chip = RF_6052;
#endif
}

static u8
_ReadEfuseInfo8723DS(
		PADAPTER			padapter
)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);
	u8 *hwinfo = NULL;
	u8 ret = _FAIL;

	/* */
	/* This part read and parse the eeprom/efuse content */
	/* */

	if (sizeof(pHalData->efuse_eeprom_data) < HWSET_MAX_SIZE_8723D)
		RTW_INFO("[WARNING] size of efuse_eeprom_data is less than HWSET_MAX_SIZE_8723D!\n");

	hwinfo = pHalData->efuse_eeprom_data;

	Hal_InitPGData(padapter, hwinfo);

	Hal_EfuseParseIDCode(padapter, hwinfo);
	Hal_EfuseParseEEPROMVer_8723D(padapter, hwinfo, pHalData->bautoload_fail_flag);
	hal_config_macaddr(padapter, pHalData->bautoload_fail_flag);
	Hal_EfuseParseTxPowerInfo_8723D(padapter, hwinfo, pHalData->bautoload_fail_flag);
	Hal_EfuseParseBoardType_8723D(padapter, hwinfo, pHalData->bautoload_fail_flag);

	/* */
	/* Read Bluetooth co-exist and initialize */
	/* */
	Hal_EfuseParseBTCoexistInfo_8723D(padapter, hwinfo, pHalData->bautoload_fail_flag);
	Hal_EfuseParseChnlPlan_8723D(padapter, hwinfo, pHalData->bautoload_fail_flag);
	Hal_EfuseParseXtal_8723D(padapter, hwinfo, pHalData->bautoload_fail_flag);
	Hal_EfuseParseThermalMeter_8723D(padapter, hwinfo, pHalData->bautoload_fail_flag);
	Hal_EfuseParseAntennaDiversity_8723D(padapter, hwinfo, pHalData->bautoload_fail_flag);
	Hal_EfuseParseCustomerID_8723D(padapter, hwinfo, pHalData->bautoload_fail_flag);

	Hal_EfuseParseVoltage_8723D(padapter, hwinfo, pHalData->bautoload_fail_flag);

#ifdef CONFIG_WOWLAN
	Hal_DetectWoWMode(padapter);
#endif

#ifdef CONFIG_RF_POWER_TRIM
	Hal_ReadRFGainOffset(padapter, hwinfo, pHalData->bautoload_fail_flag);
#endif /* CONFIG_RF_GAIN_OFFSET */

	/* set coex. ant info once efuse parsing is done */
	rtw_btcoex_set_ant_info(padapter);

	if (hal_read_mac_hidden_rpt(padapter) != _SUCCESS)
		goto exit;

	ret = _SUCCESS;

exit:
	return ret;
}

static u8 _ReadPROMContent(
		PADAPTER		padapter
)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);
	u8			eeValue;
	u8 ret = _FAIL;

	eeValue = rtw_read8(padapter, REG_9346CR);
	/* To check system boot selection. */
	pHalData->EepromOrEfuse = (eeValue & BOOT_FROM_EEPROM) ? _TRUE : _FALSE;
	pHalData->bautoload_fail_flag = (eeValue & EEPROM_EN) ? _FALSE : _TRUE;


	/*	pHalData->EEType = IS_BOOT_FROM_EEPROM(Adapter) ? EEPROM_93C46 : EEPROM_BOOT_EFUSE; */

	if (_ReadEfuseInfo8723DS(padapter) != _SUCCESS)
		goto exit;

	ret = _SUCCESS;

exit:
	return ret;
}

static void
_InitOtherVariable(
		PADAPTER		Adapter
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);


	/* if(Adapter->bInHctTest){ */
	/*	pMgntInfo->PowerSaveControl.bInactivePs = FALSE; */
	/*	pMgntInfo->PowerSaveControl.bIPSModeBackup = FALSE; */
	/*	pMgntInfo->PowerSaveControl.bLeisurePs = FALSE; */
	/*	pMgntInfo->keepAliveLevel = 0; */
	/* } */



}

/*
 *	Description:
 *		Read HW adapter information by E-Fuse or EEPROM according CR9346 reported.
 *
 *	Assumption:
 *		PASSIVE_LEVEL (SDIO interface)
 *
 *   */
static u8 ReadAdapterInfo8723DS(PADAPTER padapter)
{
	u8 val8;
	u8 ret = _FAIL;

	val8 = rtw_read8(padapter, 0x4e);
	RTW_INFO("%s, 0x4e=0x%x\n", __func__, val8);
	val8 |= BIT(6);
	rtw_write8(padapter, 0x4e, val8);

	/* Read EEPROM size before call any EEPROM function */
	padapter->EepromAddressSize = GetEEPROMSize8723D(padapter);

	_EfuseCellSel(padapter);
	_ReadRFType(padapter);
	if (_ReadPROMContent(padapter) != _SUCCESS)
		goto exit;

	_InitOtherVariable(padapter);

#ifdef CONFIG_PLATFORM_INTEL_BYT
	{ /* for BT, let BT can control ANT when wifi disable */
		u32 val32;

		RTW_INFO("%s, 0x4c=0x%x\n", __func__, rtw_read32(padapter, 0x4c));
		val32 = rtw_read32(padapter, 0x64);
		RTW_INFO("%s, 0x64=0x%x\n", __func__, val32);
		val32 |= BIT(13);
		rtw_write32(padapter, 0x64, val32);
		RTW_INFO("%s, 0x64=0x%x\n", __func__, rtw_read32(padapter, 0x64));
	}
#endif /* CONFIG_PLATFORM_INTEL_BYT */

	ret = _SUCCESS;

exit:
	return ret;
}

/*
 * If variable not handled here,
 * some variables will be processed in SetHwReg8723D()
 */
u8 SetHwReg8723DS(PADAPTER padapter, u8 variable, u8 *val)
{
	PHAL_DATA_TYPE pHalData;
	u8 ret = _SUCCESS;
	u8 val8;


	pHalData = GET_HAL_DATA(padapter);

	switch (variable) {
	case HW_VAR_SET_RPWM:
		/* rpwm value only use BIT0(clock bit) ,BIT6(Ack bit), and BIT7(Toggle bit) */
		/* BIT0 value - 1: 32k, 0:40MHz. */
		/* BIT6 value - 1: report cpwm value after success set, 0:do not report. */
		/* BIT7 value - Toggle bit change. */
	{
		val8 = *val;
		val8 &= 0xC1;
		rtw_write8(padapter, SDIO_LOCAL_BASE | SDIO_REG_HRPWM1, val8);
	}
	break;
	case HW_VAR_SET_REQ_FW_PS:
		/* 1. driver write 0x8f[4]=1  //request fw ps state (only can write bit4) */
	{
		u8 req_fw_ps = 0;

		req_fw_ps = rtw_read8(padapter, 0x8f);
		req_fw_ps |= 0x10;
		rtw_write8(padapter, 0x8f, req_fw_ps);
	}
	break;
	case HW_VAR_RXDMA_AGG_PG_TH:
		#if 0
		val8 = *val;

		/* TH=1 => invalidate RX DMA aggregation */
		/* TH=0 => validate RX DMA aggregation, use init value. */
		if (val8 == 0) {
			/* enable RXDMA aggregation */
			/* _RXAggrSwitch(padapter, _TRUE); */
		} else {
			/* disable RXDMA aggregation */
			/* _RXAggrSwitch(padapter, _FALSE); */
		}
		#endif
		break;
#ifdef CONFIG_GPIO_WAKEUP
	case HW_SET_GPIO_WL_CTRL: {
		struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
		u8 enable = *val;
		u8 value = 0;

		if (pwrpriv->wowlan_gpio_index != 6)
			break;

		value = rtw_read8(padapter, REG_GPIO_MUXCFG);

		if (enable && (value & BIT(3))) {
			value &= ~BIT(3);
			rtw_write8(padapter, REG_GPIO_MUXCFG, value);
		} else if (enable == _FALSE) {
			RTW_INFO("%s: keep WLAN ctrl\n", __func__);
		}
		/*0x66 bit4*/
		value = rtw_read8(padapter, REG_PAD_CTRL_1 + 2);
		if (enable && (value & BIT(4))) {
			value &= ~BIT(4);
			rtw_write8(padapter, REG_PAD_CTRL_1 + 2, value);
		} else if (enable == _FALSE) {
			value |= BIT(4);
			rtw_write8(padapter, REG_PAD_CTRL_1 + 2, value);
		}

		/*0x66 bit8*/
		value = rtw_read8(padapter, REG_PAD_CTRL_1 + 3);
		if (enable && (value & BIT(0))) {
			value &= ~BIT(0);
			rtw_write8(padapter, REG_PAD_CTRL_1 + 3, value);
		} else if (enable == _FALSE) {
			value |= BIT(0);
			rtw_write8(padapter, REG_PAD_CTRL_1 + 3, value);
		}
		RTW_INFO("%s: HW_SET_GPIO_WL_CTRL\n", __func__);
	}
		break;
#endif
	default:
		ret = SetHwReg8723D(padapter, variable, val);
		break;
	}

	return ret;
}

/*
 * If variable not handled here,
 * some variables will be processed in GetHwReg8723D()
 */
void GetHwReg8723DS(PADAPTER padapter, u8 variable, u8 *val)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);


	switch (variable) {
	case HW_VAR_CPWM:
		*val = rtw_read8(padapter, SDIO_LOCAL_BASE | SDIO_REG_HCPWM1_8723D);
		break;

	case HW_VAR_FW_PS_STATE: {
		/* 3. read dword 0x88               //driver read fw ps state */
		*((u16 *)val) = rtw_read16(padapter, 0x88);
	}
	break;
	default:
		GetHwReg8723D(padapter, variable, val);
		break;
	}

}

/*
 *	Description:
 *		Query setting of specified variable.
 *   */
u8
GetHalDefVar8723DSDIO(
		PADAPTER				Adapter,
		HAL_DEF_VARIABLE		eVariable,
		void						*pValue
)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(Adapter);
	PSDIO_DATA psdio_data = NULL;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u8			bResult = _SUCCESS;

	switch (eVariable) {
	case HAL_DEF_IS_SUPPORT_ANT_DIV:
#ifdef CONFIG_ANTENNA_DIVERSITY
		*((u8 *)pValue) = _FALSE;
#endif
		break;

	case HW_VAR_MAX_RX_AMPDU_FACTOR:
		/* Stanley@BB.SD3 suggests 16K can get stable performance */
		/* coding by Lucas@20130730 */
		psdio_data = &dvobj->intf_data;
		if (psdio_data->clock > RTW_SDIO_CLK_40M)
			*(HT_CAP_AMPDU_FACTOR *)pValue = MAX_AMPDU_FACTOR_32K;
		else
			*(HT_CAP_AMPDU_FACTOR *)pValue = MAX_AMPDU_FACTOR_16K;
		break;
	default:
		bResult = GetHalDefVar8723D(Adapter, eVariable, pValue);
		break;
	}

	return bResult;
}

/*
 *	Description:
 *		Change default setting of specified variable.
 *   */
u8
SetHalDefVar8723DSDIO(
		PADAPTER				Adapter,
		HAL_DEF_VARIABLE		eVariable,
		void						*pValue
)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	u8			bResult = _SUCCESS;

	switch (eVariable) {
	default:
		bResult = SetHalDefVar8723D(Adapter, eVariable, pValue);
		break;
	}

	return bResult;
}

void rtl8723ds_set_hal_ops(PADAPTER padapter)
{
	struct hal_ops *pHalFunc = &padapter->hal_func;


	rtl8723d_set_hal_ops(pHalFunc);

	pHalFunc->hal_power_on = &_InitPowerOn_8723DS;
	pHalFunc->hal_power_off = &CardDisableRTL8723DSdio;


	pHalFunc->hal_init = &rtl8723ds_hal_init;
	pHalFunc->hal_deinit = &rtl8723ds_hal_deinit;

	pHalFunc->init_xmit_priv = &rtl8723ds_init_xmit_priv;
	pHalFunc->free_xmit_priv = &rtl8723ds_free_xmit_priv;

	pHalFunc->init_recv_priv = &rtl8723ds_init_recv_priv;
	pHalFunc->free_recv_priv = &rtl8723ds_free_recv_priv;
#ifdef CONFIG_RECV_THREAD_MODE
	pHalFunc->recv_hdl = rtl8723ds_recv_hdl;
#endif /* CONFIG_RECV_THREAD_MODE */
#ifdef CONFIG_RTW_SW_LED
	pHalFunc->InitSwLeds = &rtl8723ds_InitSwLeds;
	pHalFunc->DeInitSwLeds = &rtl8723ds_DeInitSwLeds;
#endif
	pHalFunc->init_default_value = &rtl8723ds_init_default_value;
	pHalFunc->intf_chip_configure = &rtl8723ds_interface_configure;
	pHalFunc->read_adapter_info = &ReadAdapterInfo8723DS;

	pHalFunc->enable_interrupt = &EnableInterrupt8723DSdio;
	pHalFunc->disable_interrupt = &DisableInterrupt8723DSdio;
	pHalFunc->check_ips_status = &CheckIPSStatus;
#if defined(CONFIG_WOWLAN) || defined(CONFIG_AP_WOWLAN)
	pHalFunc->clear_interrupt = &ClearInterrupt8723DSdio;
#endif
	pHalFunc->set_hw_reg_handler = &SetHwReg8723DS;
	pHalFunc->GetHwRegHandler = &GetHwReg8723DS;
	pHalFunc->get_hal_def_var_handler = &GetHalDefVar8723DSDIO;
	pHalFunc->SetHalDefVarHandler = &SetHalDefVar8723DSDIO;

	pHalFunc->hal_xmit = &rtl8723ds_hal_xmit;
	pHalFunc->mgnt_xmit = &rtl8723ds_mgnt_xmit;
#ifdef CONFIG_RTW_MGMT_QUEUE
	pHalFunc->hal_mgmt_xmitframe_enqueue = &rtl8723ds_hal_mgmt_xmitframe_enqueue;
#endif
	pHalFunc->hal_xmitframe_enqueue = &rtl8723ds_hal_xmitframe_enqueue;

#ifdef CONFIG_HOSTAPD_MLME
	pHalFunc->hostap_mgnt_xmit_entry = NULL;
#endif

#if defined(CONFIG_CHECK_BT_HANG) && defined(CONFIG_BT_COEXIST)
	pHalFunc->hal_init_checkbthang_workqueue =
		&rtl8723ds_init_checkbthang_workqueue;
	pHalFunc->hal_free_checkbthang_workqueue =
		&rtl8723ds_free_checkbthang_workqueue;
	pHalFunc->hal_cancle_checkbthang_workqueue =
		&rtl8723ds_cancle_checkbthang_workqueue;
	pHalFunc->hal_checke_bt_hang = &rtl8723ds_hal_check_bt_hang;
#endif

}
