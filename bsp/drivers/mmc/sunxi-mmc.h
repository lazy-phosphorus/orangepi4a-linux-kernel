/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
* Sunxi SD/MMC host driver
*
* Copyright (C) 2015 AllWinnertech Ltd.
* Author: lixiang <lixiang@allwinnertech>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed "as is" WITHOUT ANY WARRANTY of any
* kind, whether express or implied; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*/


#include <linux/clk.h>
#include <linux/reset/sunxi.h>

#include <pinctrl-sunxi.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/reset.h>
#include <linux/interrupt.h>

#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>

#include <linux/mmc/host.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/core.h>
#include <linux/mmc/card.h>
#include <linux/mmc/slot-gpio.h>

#include "../core/core.h"

#ifndef __SUNXI_MMC_H__
#define __SUNXI_MMC_H__

#define DRIVER_NAME "sunxi-mmc"
#define DRIVER_RIVISION "v5.39 2024-02-22 13:48 -- v5.48 2024-07-17 16:19"
#define DRIVER_VERSION "SD/MMC/SDIO Host Controller Driver(" DRIVER_RIVISION ")"

#if defined CONFIG_AW_FPGA_S4 || defined CONFIG_AW_FPGA_V7
#define MMC_FPGA
#endif


/* register offset definitions */
#define SDXC_REG_GCTRL	(0x00)	/* SMC Global Control Register */
#define SDXC_REG_CLKCR	(0x04)	/* SMC Clock Control Register */
#define SDXC_REG_TMOUT	(0x08)	/* SMC Time Out Register */
#define SDXC_REG_WIDTH	(0x0C)	/* SMC Bus Width Register */
#define SDXC_REG_BLKSZ	(0x10)	/* SMC Block Size Register */
#define SDXC_REG_BCNTR	(0x14)	/* SMC Byte Count Register */
#define SDXC_REG_CMDR	(0x18)	/* SMC Command Register */
#define SDXC_REG_CARG	(0x1C)	/* SMC Argument Register */
#define SDXC_REG_RESP0	(0x20)	/* SMC Response Register 0 */
#define SDXC_REG_RESP1	(0x24)	/* SMC Response Register 1 */
#define SDXC_REG_RESP2	(0x28)	/* SMC Response Register 2 */
#define SDXC_REG_RESP3	(0x2C)	/* SMC Response Register 3 */
#define SDXC_REG_IMASK	(0x30)	/* SMC Interrupt Mask Register */
#define SDXC_REG_MISTA	(0x34)	/* SMC Masked Interrupt Status Register */
#define SDXC_REG_RINTR	(0x38)	/* SMC Raw Interrupt Status Register */
#define SDXC_REG_STAS	(0x3C)	/* SMC Status Register */
#define SDXC_REG_FTRGL	(0x40)	/* SMC FIFO Threshold Watermark Registe */
#define SDXC_REG_FUNS	(0x44)	/* SMC Function Select Register */
#define SDXC_REG_CBCR	(0x48)	/* SMC CIU Byte Count Register */
#define SDXC_REG_BBCR	(0x4C)	/* SMC BIU Byte Count Register */
#define SDXC_REG_DBGC	(0x50)	/* SMC Debug Enable Register */
#define SDXC_REG_A12A	(0x58)	/* auto cmd12 arg */
#define SDXC_REG_SD_NTSR	(0x005C)	/* new timing set reg */
#define SDXC_REG_HWRST	(0x78)	/* SMC Card Hardware Reset for Register */
#define SDXC_REG_DMAC	(0x80)	/* SMC IDMAC Control Register */
#define SDXC_REG_DLBA	(0x84)	/* SMC IDMAC Descriptor List Base Addre */
#define SDXC_REG_IDST	(0x88)	/* SMC IDMAC Status Register */
#define SDXC_REG_IDIE	(0x8C)	/* SMC IDMAC Interrupt Enable Register */
#define SDXC_REG_CHDA	(0x90)
#define SDXC_REG_CBDA	(0x94)


#define SDXC_REG_CQE_CTRL (0x1A0) /* SMH CQE CONTROL Register */
#define SDXC_REG_FIFO	(0x200)

#define SDXC_CQHCI_BASE (0x400)

#if defined(CONFIG_ARCH_SUN55IW3)
/* Parallel access to registers must be avoided */
#define mmc_readl(host, reg) ({						\
	u32 __val;							\
	unsigned long __iflags;						\
	do {								\
		if ((host->sunxi_caps3 & MMC_SUNXI_CQE_ON) && (host->mmc_get_soc_ver < 2)) {					\
			spin_lock_irqsave(&host->read_lock, __iflags);		\
			__val = readl((host)->reg_base + SDXC_##reg);		\
			spin_unlock_irqrestore(&host->read_lock, __iflags);	\
		} else							\
			__val = readl((host)->reg_base + SDXC_##reg);		\
	} while (0);							\
	__val;								\
})
#else
#define mmc_readl(host, reg) \
	readl((host)->reg_base + SDXC_##reg)
#endif

#define mmc_writel(host, reg, value) \
	writel((value), (host)->reg_base + SDXC_##reg)

/* global control register bits */
#define SDXC_SOFT_RESET			BIT(0)
#define SDXC_FIFO_RESET			BIT(1)
#define SDXC_DMA_RESET			BIT(2)
#define SDXC_INTERRUPT_ENABLE_BIT	BIT(4)
#define SDXC_DMA_ENABLE_BIT		BIT(5)
#define SDXC_DEBOUNCE_ENABLE_BIT	BIT(8)
#define SDXC_POSEDGE_LATCH_DATA		BIT(9)
#define SDXC_DDR_MODE			BIT(10)
#define SDXC_DTIME_UNIT			BIT(11)
#define SDXC_MEMORY_ACCESS_DONE		BIT(29)
#define SDXC_ACCESS_DONE_DIRECT		BIT(30)
#define SDXC_ACCESS_BY_AHB		BIT(31)
#define SDXC_ACCESS_BY_DMA		(0 << 31)
#define SDXC_HARDWARE_RESET \
	(SDXC_SOFT_RESET | SDXC_FIFO_RESET | SDXC_DMA_RESET)

/* clock control bits */
#define SDXC_CARD_CLOCK_ON		BIT(16)
#define SDXC_LOW_POWER_ON		BIT(17)
#define SDXC_MASK_DATA0			BIT(31)

/* bus width */
#define SDXC_WIDTH1			0
#define SDXC_WIDTH4			1
#define SDXC_WIDTH8			2

/* smc command bits */
#define SDXC_RESP_EXPECT		BIT(6)
#define SDXC_LONG_RESPONSE		BIT(7)
#define SDXC_CHECK_RESPONSE_CRC		BIT(8)
#define SDXC_DATA_EXPECT		BIT(9)
#define SDXC_WRITE			BIT(10)
#define SDXC_SEQUENCE_MODE		BIT(11)
#define SDXC_SEND_AUTO_STOP		BIT(12)
#define SDXC_WAIT_PRE_OVER		BIT(13)
#define SDXC_STOP_ABORT_CMD		BIT(14)
#define SDXC_SEND_INIT_SEQUENCE		BIT(15)
#define SDXC_R1B_RESP			BIT(16)
#define SDXC_R1B_BUSY_HW_DETECT		BIT(17)
#define SDXC_UPCLK_ONLY			BIT(21)
#define SDXC_READ_CEATA_DEV		BIT(22)
#define SDXC_CCS_EXPECT			BIT(23)
#define SDXC_ENABLE_BIT_BOOT		BIT(24)
#define SDXC_ALT_BOOT_OPTIONS		BIT(25)
#define SDXC_BOOT_ACK_EXPECT		BIT(26)
#define SDXC_BOOT_ABORT			BIT(27)
#define SDXC_VOLTAGE_SWITCH	        BIT(28)
#define SDXC_USE_HOLD_REGISTER	        BIT(29)
#define SDXC_START			BIT(31)

/* interrupt bits */
#define SDXC_RESP_ERROR			BIT(1)
#define SDXC_COMMAND_DONE		BIT(2)
#define SDXC_DATA_OVER			BIT(3)
#define SDXC_TX_DATA_REQUEST		BIT(4)
#define SDXC_RX_DATA_REQUEST		BIT(5)
#define SDXC_RESP_CRC_ERROR		BIT(6)
#define SDXC_DATA_CRC_ERROR		BIT(7)
#define SDXC_RESP_TIMEOUT		BIT(8)
#define SDXC_DATA_TIMEOUT		BIT(9)
#define SDXC_VOLTAGE_CHANGE_DONE	BIT(10)
#define SDXC_FIFO_RUN_ERROR		BIT(11)
#define SDXC_HARD_WARE_LOCKED		BIT(12)
#define SDXC_START_BIT_ERROR		BIT(13)
#define SDXC_BUSY_CLEAR			BIT(13) /* the same bit with SDXC_START_BIT_ERROR */
#define SDXC_AUTO_COMMAND_DONE		BIT(14)
#define SDXC_END_BIT_ERROR		BIT(15)
#define SDXC_SDIO_INTERRUPT		BIT(16)
#define SDXC_CARD_INSERT		BIT(30)
#define SDXC_CARD_REMOVE		BIT(31)
#define SDXC_INTERRUPT_ERROR_BIT \
	(SDXC_RESP_ERROR | SDXC_RESP_CRC_ERROR | SDXC_DATA_CRC_ERROR | \
	 SDXC_RESP_TIMEOUT | SDXC_DATA_TIMEOUT | SDXC_FIFO_RUN_ERROR | \
	 SDXC_HARD_WARE_LOCKED | SDXC_START_BIT_ERROR | SDXC_END_BIT_ERROR)
#define SDXC_INTERRUPT_CMD_ERROR_BIT \
	(SDXC_RESP_ERROR | SDXC_RESP_CRC_ERROR | SDXC_RESP_TIMEOUT)
#define SDXC_INTERRUPT_DONE_BIT \
	(SDXC_AUTO_COMMAND_DONE | SDXC_DATA_OVER | \
	 SDXC_COMMAND_DONE | SDXC_VOLTAGE_CHANGE_DONE)
#define SDXC_INTERRUPT_DDONE_BIT \
	(SDXC_AUTO_COMMAND_DONE | SDXC_DATA_OVER)
#define SDXC_SWITCH_DDONE_BIT \
	(SDXC_VOLTAGE_CHANGE_DONE | SDXC_COMMAND_DONE)


/* status */
#define SDXC_RXWL_FLAG			BIT(0)
#define SDXC_TXWL_FLAG			BIT(1)
#define SDXC_FIFO_EMPTY			BIT(2)
#define SDXC_FIFO_FULL			BIT(3)
#define SDXC_CARD_PRESENT		BIT(8)
#define SDXC_CARD_DATA_BUSY		BIT(9)
#define SDXC_DATA_FSM_BUSY		BIT(10)
#define SDXC_DMA_REQUEST		BIT(31)
#define SDXC_FIFO_SIZE			16

/* Function select */
#define SDXC_CEATA_ON			(0xceaa << 16)
#define SDXC_SEND_IRQ_RESPONSE		BIT(0)
#define SDXC_SDIO_READ_WAIT		BIT(1)
#define SDXC_ABORT_READ_DATA		BIT(2)
#define SDXC_SEND_CCSD			BIT(8)
#define SDXC_SEND_AUTO_STOPCCSD		BIT(9)
#define SDXC_CEATA_DEV_IRQ_ENABLE	BIT(10)

/* new timing set reg */
#define	SDXC_2X_TIMING_MODE			(1U<<31)

/* IDMA controller bus mod bit field */
#define SDXC_IDMAC_SOFT_RESET		BIT(0)
#define SDXC_IDMAC_FIX_BURST		BIT(1)
#define SDXC_IDMAC_IDMA_ON		BIT(7)
#define SDXC_IDMAC_REFETCH_DES		BIT(31)

/* IDMA status bit field */
#define SDXC_IDMAC_TRANSMIT_INTERRUPT		BIT(0)
#define SDXC_IDMAC_RECEIVE_INTERRUPT		BIT(1)
#define SDXC_IDMAC_FATAL_BUS_ERROR		BIT(2)
#define SDXC_IDMAC_DESTINATION_INVALID		BIT(4)
#define SDXC_IDMAC_CARD_ERROR_SUM		BIT(5)
#define SDXC_IDMAC_NORMAL_INTERRUPT_SUM		BIT(8)
#define SDXC_IDMAC_ABNORMAL_INTERRUPT_SUM	BIT(9)
#define SDXC_IDMAC_HOST_ABORT_INTERRUPT		BIT(10)
#define SDXC_IDMAC_IDLE				(0 << 13)
#define SDXC_IDMAC_SUSPEND			(1 << 13)
#define SDXC_IDMAC_DESC_READ			(2 << 13)
#define SDXC_IDMAC_DESC_CHECK			(3 << 13)
#define SDXC_IDMAC_READ_REQUEST_WAIT		(4 << 13)
#define SDXC_IDMAC_WRITE_REQUEST_WAIT		(5 << 13)
#define SDXC_IDMAC_READ				(6 << 13)
#define SDXC_IDMAC_WRITE			(7 << 13)
#define SDXC_IDMAC_DESC_CLOSE			(8 << 13)

/*
* If the idma-des-size-bits of property is ie 13, bufsize bits are:
*  Bits  0-12: buf1 size
*  Bits 13-25: buf2 size
*  Bits 26-31: not used
* Since we only ever set buf1 size, we can simply store it directly.
*/
#define SDXC_IDMAC_DES0_DIC	BIT(1)	/* disable interrupt on completion */
#define SDXC_IDMAC_DES0_LD	BIT(2)	/* last descriptor */
#define SDXC_IDMAC_DES0_FD	BIT(3)	/* first descriptor */
#define SDXC_IDMAC_DES0_CH	BIT(4)	/* chain mode */
#define SDXC_IDMAC_DES0_ER	BIT(5)	/* end of ring */
#define SDXC_IDMAC_DES0_CES	BIT(30)	/* card error summary */
#define SDXC_IDMAC_DES0_OWN	BIT(31)	/* 1-idma owns it, 0-host owns it */

/* SMH CQE CONTROL Register */
#define CQE_CLR_CC_EN		BIT(0)
#define CQE_CLR_DTC_EN		BIT(1)

#define CARD_PWR_GPIO_AUTO	0	/* smhc driver will set card_pwr_gpio */
#define CARD_PWR_GPIO_MANUAL	1	/* debug cmd will set card_pwr_gpio   */

void sunxi_dump_reg(struct mmc_host *mmc);

#define sunxi_r_op(__h, __op) (\
{\
	int __ret_val = 0;\
	struct mmc_host	*mmc = (__h)->mmc;\
	clk_disable_unprepare((__h)->clk_mmc);\
	dev_dbg(mmc_dev(mmc), "%s, %d\n", __FUNCTION__, __LINE__);\
	__op;\
	__ret_val  = clk_prepare_enable((__h)->clk_mmc);\
	if (__ret_val) {\
		dev_err(mmc_dev(mmc), "Enable mmc clk err %d\n", __ret_val);\
	} \
	__ret_val;\
} \
)

enum sunxi_cookie {
	COOKIE_UNMAPPED,
	COOKIE_PRE_MAPPED,	/* mapped by sunxi_mmc_pre_req() */
	COOKIE_MAPPED,		/* mapped by sunxi_mmc_request() or retry */
	COOKIE_MAPPED_ATOMIC,		/* mapped by sunxi_mmc_request() or retry */
};

enum mrq_slot {
	MRQ_SYNC,
	MRQ_ASYNC,
	MRQ_MAX,
	MRQ_UNUSED,
};

struct sunxi_idma_des {
	volatile u32 config;
	volatile u32 buf_size;
	volatile u32 buf_addr_ptr1;
	volatile u32 buf_addr_ptr2;
};

struct sunxi_mmc_ctrl_regs {
	u32 gctrl;
	u32 clkc;
	u32 timeout;
	u32 buswid;
	u32 waterlvl;
	u32 funcsel;
	u32 debugc;
	u32 idmacc;
	u32 dlba;
	u32 imask;

	u32 cqcfg;
	u32 cqtdlba;
	u32 cqtdlbau;
	u32 cqssc1;
	u32 cqssc2;
	u32 cqectrl;
	u32 cqic;
};

struct sunxi_mmc_host_perf{
	ktime_t start;
	int64_t rbytes;
	int64_t wbytes;
	ktime_t rtime;
	ktime_t wtime;

	/***use to compute the time no include busy***/
	int64_t wbytestran;
	ktime_t wtimetran;
};

struct sunxi_mmc_supply {
	struct regulator *vmmc;		/* Card power supply */
	struct regulator *vqmmc;	/* Optional Vccq supply */
	struct regulator *vdmmc;	/* Optional card detect pin supply */
	struct regulator *vdmmc33sw;	/* SD card PMU control */
	struct regulator *vdmmc18sw;
	struct regulator *vqmmc33sw;	/* SD card PMU control */
	struct regulator *vqmmc18sw;
};

struct sunxi_mmc_async_request {
	struct mmc_command cmd;
	struct mmc_command stop;
	struct mmc_data data;
	struct mmc_request mrq;
};

struct sunxi_mmc_host {
	struct mmc_host *mmc;
	struct reset_control *reset;

	/* IO mapping base */
	void __iomem *reg_base;

	/* clock management */
	struct clk *clk_ahb;
	/* mbus gate(for mmc) is enabled by default. But v821 disabled it in boot0. So it need enable in kernel */
	struct clk *clk_mbus;
	struct clk *clk_mmc;
	struct reset_control *clk_rst;

	int (*sunxi_mmc_clk_set_rate)(struct sunxi_mmc_host *host,
				struct mmc_ios *ios);
	int crypt_flag;

	/* irq */
	spinlock_t lock;
	spinlock_t async_lock;
	/* only used for read register serial */
	spinlock_t read_lock;
	int irq;
	u32 int_sum;
	u32 sdio_imask;

	/* dma */
	u32 req_page_count;
	u32 idma_des_size_bits;
	u32 des_cnt;
	dma_addr_t sg_dma;
	void *sg_cpu;
	bool wait_dma;
	u32 dma_tl;
	u64 dma_mask;
	u64 cqe_dma_mask;

	void (*sunxi_mmc_thld_ctl)(struct sunxi_mmc_host *host,
				 struct mmc_ios *ios,
				    struct mmc_data *data);

	struct mmc_request *mrq;
	struct mmc_request *mrq_busy;
	struct mmc_request *mrq_retry;
	struct mmc_request *manual_stop_mrq;
	int ferror;

	u32 power_on;
	u32 time_pwroff_ms;

	/* pinctrl handles */
	struct pinctrl *pinctrl;
	struct pinctrl_state *pins_default;
	struct pinctrl_state *pins_bias_1v8;
	struct pinctrl_state *pins_sleep;
	struct pinctrl_state *pins_uart_jtag;

	/* sys node */
	struct device_attribute maual_insert;
	struct device_attribute *dump_register;
	struct device_attribute dump_clk_dly;
	struct device_attribute host_sample_dly;
	struct device_attribute host_ds_dly;
	struct device_attribute host_send_status;
	struct device_attribute host_debuglevel;
	struct device_attribute host_timing;
	struct device_attribute sd_detect_pin_status;
	struct device_attribute card_pwr_gpio_cfg;
	struct device_attribute card_pwr_gpio_mode;

	void (*sunxi_mmc_dump_dly_table)(struct sunxi_mmc_host *host);

	/* backup register structrue */
	struct sunxi_mmc_ctrl_regs bak_regs;
	void (*sunxi_mmc_save_spec_reg)(struct sunxi_mmc_host *host);
	void (*sunxi_mmc_set_rdtmout_reg)(struct sunxi_mmc_host *host, u32 tmout_clk,  bool set_time);
	void (*sunxi_mmc_restore_spec_reg)(struct sunxi_mmc_host *host);
	void (*sunxi_mmc_set_acmda)(struct sunxi_mmc_host *host);
	void (*sunxi_mmc_shutdown)(struct platform_device *pdev);
	int (*sunxi_mmc_oclk_en)(struct sunxi_mmc_host *host, u32 oclk_en);
	int (*sunxi_mmc_updata_pha)(struct sunxi_mmc_host *host,
			struct mmc_command *cmd, struct mmc_data *data);
	void (*sunxi_mmc_on_off_emce)(struct sunxi_mmc_host *host,
			u32 en_crypt, u32 ac_mode, u32 en_emce, int data_len,
			int bypass, int task_load);
	bool (*sunxi_mmc_hw_busy)(struct sunxi_mmc_host *host);
	int (*sunxi_mmc_dat0_busy)(struct sunxi_mmc_host *host);

	/* really controller id,no logic id */
	int phy_index;

	u32 dat3_imask;

	/* no wait busy if wrtie end, only for customer need */
#define NO_WBUSY_WR_END  0x1
#define NO_REINIT_SHUTDOWN			   0x2
#define CARD_PWR_GPIO_HIGH_ACTIVE	   0x4
#define SUNXI_SC_EN_RETRY					0x8
	/**If set this bit,when use sunxi_check_r1_ready_may_sleep,
	*we will wait 0xFFFFFFFF ms, for debug use
	*it is no meant on linux4.4
	*/
#define SUNXI_R1B_WAIT_MAX					0x10

#define SUNXI_MANUAL_READ_DATA_TIMEOUT      0x20
/*
*Disable linux kernel native broken cd function,use host  defined.
*Host defined cd function only report true when it detect cd pin change.
*If not detect cd pin change,it return false.
*We use it to deal with some bad card which cannot init at all
*But host defined cd function has a disadvantage that it may not detect card
*If card is inserted too often.
*/
#define SUNXI_DIS_KER_NAT_CD			0x40

/* #define SUNXI_NO_ERASE				0x80 */
#define SUNXI_SC_EN_RETRY_CMD			0x100
#define SUNXI_SC_EN_TIMEOUT_DETECT		0x200
#define SUNXI_CMD11_TIMEOUT_DETECT 0x400
/* control specal function control,for customer need */
	u32 ctl_spec_cap;

#define MODE_CMDP		0x0
#define MODE_CMDQ_HALF		(0x1 << 4)
#define MODE_CMDQ_ALMOST	(0x2 << 4)
#define MODE_CMDQ		(0x3 << 4)
#define SET_CMDQ_MD(x)		((x & 0x3) << 4)
/* control special cmdq mode */
	u32 ctl_cmdq_md;
/* interrupt coalescing count */
	u32 intr_clsc_cnt;

#define MMC_SUNXI_CAP3_DAT3_DET	(1 << 0)
#define MMC_SUNXI_CAP3_CD_USED_24M	(1 << 1)
#define MMC_SUNXI_CAP3_HSQ	(1 << 2)
#define MMC_SUNXI_CAP3_RDPST	(1 << 3)
#define MMC_SUNXI_CQE_ON		(1 << 4)
#define MMC_SUNXI_INT_CLSC_ON		(1 << 5)
	u32 sunxi_caps3;

	struct sunxi_mmc_supply supply;
	int card_pwr_gpio;
	int card_pwr_mode;

	u32 retry_cnt;
	u32 errno_retry;
	int (*sunxi_mmc_judge_retry)(struct sunxi_mmc_host *host,
				      struct mmc_command *cmd, u32 rcnt,
				      u32 errno, void *other);
	int sunxi_samp_dl;
	int sunxi_ds_dl;

	u32 sunxi_ds_dl_cnt;
	u32 sunxi_samp_dl_cnt;
	/* only use for MMC_CAP_NEEDS_POLL and SUNXI_DIS_KER_NAT_CD is on */
	u32 present;
	u32 voltage_switching;

	bool perf_enable;
	bool cqe_on;
	bool has_recovery;
	struct device_attribute host_perf;
	struct sunxi_mmc_host_perf perf;
	struct device_attribute filter_sector_perf;
	struct device_attribute filter_speed_perf;
	unsigned int filter_sector;
	unsigned int filter_speed;
	unsigned int debounce_value;
	struct device_attribute host_mwr;

	void *version_priv_dat;

	/* auto command 23 operate */
	/* set auto cmd 23 val and enable bit,or get respose */
	bool (*sunxi_mmc_opacmd23)(struct sunxi_mmc_host *host, bool set, u32 arg, u32 *rep);
	void (*sunxi_mmc_hw_wbusy_wait)(struct sunxi_mmc_host *host, struct mmc_data *data, bool set);

	/* sample fifo contral */
	bool sfc_dis;

	/* Used to prevent sending cmd8 in halt */
	/* restore normal mmc host */
	bool rstr_nrml;

	/* des phy address shift,use for over 4G phy ddrest */
	size_t des_addr_shift;
	char name[32];
	struct delayed_work sunxi_timerout_work;
	int debuglevel;
	u32 async;
	ktime_t rcd_mtx;

	struct mmc_request *mrq_slot[MRQ_MAX];
	enum mrq_slot mrq_type;
	bool mrq_doing;
	struct tasklet_struct  parallel_tasklet;
	void (*async_done)(void *info, s32 error);
	struct sunxi_mmc_async_request async_req;
	ktime_t sync_wait;
#if defined(CONFIG_ARCH_SUN55IW3)
	unsigned int mmc_get_soc_ver;
#endif

#define MMC_TUNING_RETRY_TIMES		20
	u32 kernel_tuning_sample_dly;
	u8 tuning_in_kernel;
	u32 execute_tuning_runing;
	void (*sunxi_mmc_set_samp_dl)(struct sunxi_mmc_host *host,
					int sunxi_samp_dl);

};

/* the struct as the the kernel version changes,which copy form core/slot-gpio.c */
struct mmc_gpio {
	struct gpio_desc *ro_gpio;
	struct gpio_desc *cd_gpio;
	bool override_ro_active_level;
	bool override_cd_active_level;
	irqreturn_t (*cd_gpio_isr)(int irq, void *dev_id);
	char *ro_label;
	char cd_label[0];
};

/* use to check ddr mode,not include hs400 */
#define sunxi_mmc_ddr_timing(it)	\
	(((it) == MMC_TIMING_UHS_DDR50) || ((it) == MMC_TIMING_MMC_DDR52))


/* Transfer core definition here */
/* core.h */
#define MMC_DATA_STREAM			BIT(10)
/* host.h cap2 */
#define MMC_CAP2_CACHE_CTRL	(1 << 1)	/* Allow cache control */
#define MMC_CAP2_PACKED_RD	(1 << 12)	/* Allow packed read */
#define MMC_CAP2_PACKED_WR	(1 << 13)	/* Allow packed write */
#define MMC_CAP2_PACKED_CMD	(MMC_CAP2_PACKED_RD | \
						 MMC_CAP2_PACKED_WR)
#define MMC_CAP2_HC_ERASE_SZ	(1 << 9)	/* High-capacity erase size */

void sunxi_mmc_set_a12a(struct sunxi_mmc_host *host);
void sunxi_mmc_do_shutdown_com(struct platform_device *pdev);
int mmc_wbclr(struct sunxi_mmc_host *host, void __iomem *addr, u32 bmap, u32 tms);
extern int mmc_send_status(struct mmc_card *card, u32 *status);
extern void mmc_set_bus_width(struct mmc_host *host, unsigned int width);
extern int sunxi_sel_pio_mode(struct pinctrl *pinctrl, u32 pm_sel);
extern void sunxi_mmc_dumphex32(struct sunxi_mmc_host *host, char *name, void __iomem *base, int len);
extern int sunxi_mmc_panic_init_ps(void *data);
extern unsigned int sunxi_get_soc_ver(void);

#endif
