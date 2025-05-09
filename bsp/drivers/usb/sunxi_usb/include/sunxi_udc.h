/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * (C) Copyright 2010-2015
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * javen, 2010-3-3, create this file
 *
 * usb udc head file.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef __SUNXI_UDC_H__
#define __SUNXI_UDC_H__

#include <linux/usb.h>
#include <linux/usb/gadget.h>
#include <linux/dma-mapping.h>
#include <linux/regulator/consumer.h>
#include <linux/phy/phy.h>

#if 1
typedef struct sunxi_udc_dma {
	char name[32];
	int is_start;
#ifdef SW_UDC_DMA_INNER
	int *dma_hdle; /* dma channel handle */
#else
	struct dma_chan *chan;
#endif
} sunxi_udc_dma_t;

typedef struct sunxi_udc_ep {
	struct list_head	queue;
	unsigned long		last_io; /* jiffies timestamp */
	struct usb_gadget	*gadget;
	struct sunxi_udc	*dev;
	const struct usb_endpoint_descriptor *desc;
	struct usb_ep		ep;
	u8			num;

	unsigned short		fifo_size;
	u8			bEndpointAddress;
	u8			bmAttributes;

	unsigned		halted : 1;
	unsigned		already_seen : 1;
	unsigned		setup_stage : 1;

#ifdef SW_UDC_DMA_INNER
	int				*dma_hdle;
#else
	sunxi_udc_dma_t			sunxi_udc_dma[6];
#endif
	bool			dma_addr_ext_enable; /* Feature: 16GB DDR Memory Access */
	__u32			dma_working; /* flag. is dma busy? */
	__u32			dma_transfer_len; /* dma want transfer length */
	/**
	 * Feature: dma word addr bypass.
	 * 1 - config by byte addr and burst length alignment,
	 * 0 - config by word addr and controller left shift 2 bit for word addr alignment.
	 */
	__u32			dma_wordaddr_bypass;
} sunxi_udc_ep_t;

/**
 * Warning : ep0 has a fifo of 16 bytes.
 * Don't try to set 32 or 64.
 * Also testusb 14 fails  wit 16 but is fine with 8.
 */
#define EP0_FIFO_SIZE			64

#define SW_UDC_EP_FIFO_SIZE		512
#define SW_UDC_EP_ISO_FIFO_SIZE		1024

#define	SW_UDC_EP_CTRL_INDEX		0x00
#define SW_UDC_EP_BULK_IN_INDEX	0x01
#define SW_UDC_EP_BULK_OUT_INDEX	0x02

#ifdef SW_UDC_DOUBLE_FIFO
#define SW_UDC_FIFO_NUM		1
#else
#define SW_UDC_FIFO_NUM		0
#endif

#define	SUNXI_UDC_TEST_J		0x0100
#define	SUNXI_UDC_TEST_K		0x0200
#define	SUNXI_UDC_TEST_SE0_NAK		0x0300
#define	SUNXI_UDC_TEST_PACKET		0x0400

/*
 * BSP linux5.10 copy from linux5.4 -> net2272.h;
 * Because linux5.10 modified the net2272.h file.
 */
#define 	TEST_J			1
#define 	TEST_K			2
#define 	TEST_SE0_NAK		3
#define		TEST_PACKET		4


static const char ep0name[] = "ep0";
static const char ep1in_bulk_name[]  = "ep1in-bulk";
static const char ep1out_bulk_name[] = "ep1out-bulk";
static const char ep2in_bulk_name[]  = "ep2in-bulk";
static const char ep2out_bulk_name[] = "ep2out-bulk";
static const char ep3_iso_name[]     = "ep3-iso";
static const char ep4_int_name[]     = "ep4-int";
static const char ep5in_bulk_name[]  = "ep5in-bulk";
static const char ep5out_bulk_name[] = "ep5out-bulk";

struct sw_udc_fifo {
	const char *name;
	u32 fifo_addr;
	u32 fifo_size;
	u8  double_fifo;
};

#if IS_ENABLED(CONFIG_ARCH_SUN50IW1) || IS_ENABLED(CONFIG_ARCH_SUN50IW3) \
	|| IS_ENABLED(CONFIG_ARCH_SUN8IW6) || IS_ENABLED(CONFIG_ARCH_SUN8IW15) \
	|| IS_ENABLED(CONFIG_ARCH_SUN50IW10) || IS_ENABLED(CONFIG_ARCH_SUN50IW9) \
	|| IS_ENABLED(CONFIG_ARCH_SUN8IW20) || IS_ENABLED(CONFIG_ARCH_SUN20IW1) \
	|| IS_ENABLED(CONFIG_ARCH_SUN55IW3) || IS_ENABLED(CONFIG_ARCH_SUN60IW2) \
	|| IS_ENABLED(CONFIG_ARCH_SUN8IW21) || IS_ENABLED(CONFIG_ARCH_SUN55IW6) \
	|| IS_ENABLED(CONFIG_ARCH_SUN300IW1) || IS_ENABLED(CONFIG_ARCH_SUN65IW1) \
	|| IS_ENABLED(CONFIG_ARCH_SUN251IW1)
/**
 * fifo 8k
 *
 * ep				fifo_addr	fifo_size
 * "ep0",			0,		0.5k
 * "ep1in-bulk",		0.5k,		1k
 * "ep1out-bulk",		1.5k,		1k
 * "ep2in-bulk",		2.5k,		1k
 * "ep2out-bulk",		3.5k,		1k
 * "ep3-iso",			4.5k,		2k
 * "ep4-int",			6.5k,		0.5k
 * "ep5in-bulk",		7k,		0.5k
 * "ep5out-bulk",		7.5k,		0.5k
 */
#define SW_UDC_EPNUMS	5
static const struct sw_udc_fifo ep_fifo[] = {
	{ep0name,          0,    512,  0},
	{ep1in_bulk_name,  512,  1024, 1},
	{ep1out_bulk_name, 1536, 1024, 1},
	{ep2in_bulk_name,  2560, 1024, 1},
	{ep2out_bulk_name, 3584, 1024, 1},
	{ep3_iso_name,     4608, 1024, 0},
	{ep4_int_name,     5632, 512,  0},
	{ep5in_bulk_name,  6144, 1024, 1},
	{ep5out_bulk_name, 7168, 1024, 1},
};

#else

/**
 * fifo 4k
 *
 * ep				fifo_addr	fifo_size
 * "ep0",			0,		0.5k
 * "ep1in-bulk",		0.5k,		0.5k
 * "ep1out-bulk",		1k,		0.5k
 * "ep2in-bulk",		1.5k,		0.5k
 * "ep2out-bulk",		2k,		0.5k
 * "ep3-iso",			2.5k,		1k
 * "ep4-int",			3.5k,		0.5k
 */
#define SW_UDC_EPNUMS	4

static const struct sw_udc_fifo ep_fifo[] = {
	{ep0name,          0,    512,  0},
	{ep1in_bulk_name,  512,  512,  0},
	{ep1out_bulk_name, 1024, 512,  0},
	{ep2in_bulk_name,  1536, 512,  0},
	{ep2out_bulk_name, 2048, 512,  0},
	{ep3_iso_name,     2560, 1024, 0},
	{ep4_int_name,     3584, 512,  0},
};
#endif

/**
 * ep_fifo_in[i] = {n} i: the physic ep index, n: ep_fifo's index for the ep
 *
 * eg: ep_fifo_in[2] = {3} ===> ep2_in is in ep_fifo[3]
 *
 * ep3_iso_name and ep4_int_name cannot be tx or rx simultaneously.
 *
 */
static const int ep_fifo_in[] = {0, 1, 3, 5, 6, 7};
static const int ep_fifo_out[] = {0, 2, 4, 5, 6, 8};

#define SW_UDC_ENDPOINTS	ARRAY_SIZE(ep_fifo)

#define  is_tx_ep(ep)		((ep->bEndpointAddress) & USB_DIR_IN)

enum sunxi_buffer_map_state {
	UN_MAPPED = 0,
	PRE_MAPPED,
	SW_UDC_USB_MAPPED
};

struct sunxi_udc_request {
	struct list_head	queue;	/* ep's requests */
	struct usb_request	req;

	__u32 is_queue;			/* flag... */
	enum sunxi_buffer_map_state map_state;
	void *saved_req_buf;
};

enum ep0_state {
	EP0_IDLE,
	EP0_IN_DATA_PHASE,
	EP0_OUT_DATA_PHASE,
	EP0_END_XFER,
	EP0_STALL,
};

typedef struct sunxi_udc_dma_parg {
	struct sunxi_udc *dev;
	struct sunxi_udc_ep *ep[6];
	struct sunxi_udc_request *req;
} sunxi_udc_dma_parg_t;
#endif

typedef struct sunxi_udc_io {
	struct resource	*usb_base_res;		/* USB  resources */
	struct resource	*usb_base_req;		/* USB  resources */
	void __iomem	*usb_vbase;		/* USB  base address */

	struct resource	*sram_base_res;		/* SRAM resources */
	struct resource	*sram_base_req;		/* SRAM resources */
	void __iomem	*sram_vbase;		/* SRAM base address */

	struct resource	*clock_base_res;	/* clock resources */
	struct resource	*clock_base_req;	/* clock resources */
	void __iomem	*clock_vbase;		/* clock base address */

	bsp_usbc_t usbc;			/* usb bsp config */
	__hdle usb_bsp_hdle;			/* usb bsp handle */

	__u32 clk_is_open;			/* is usb clock open? */
	struct clk 	*clk_msi_lite;		/* msi-lite */
	struct clk 	*clk_usb_sys_ahb;	/* usb-sys-ahb */
	struct clk 	*clk_res;		/* res_dcap-24m */
	struct clk	*clk_hosc;		/* usb-24m */
	struct clk	*clk_bus_otg;
	struct clk	*clk_phy;
	struct clk      *clk_usb;
	struct clk      *clk_mbus;
	struct clk      *clk_rate;
	struct clk	*ahb_otg;		/* ahb clock handle */
	struct clk	*mod_usbphy;		/* PHY0 clock handle */

	struct reset_control	*reset_otg;
	struct reset_control	*reset_phy;
	struct reset_control	*reset_usb;

	struct phy *usb2_generic_phy;		/* pointer to USB2 PHY */
	int phy_range;
	int rate_clk;
	bool rext_cal_bypass; /* Hardware: the USB0/1-REXT is floating ? */

	bool		dma_addr_ext_enable; /* Feature: 16GB DDR Memory Access */
	__u32		dma_wordaddr_bypass;
#if IS_ENABLED(CONFIG_ARCH_SUN50IW10)
/* for keep common circuit configuration */
	void __iomem	*usb_common_phy_config;
	void __iomem	*usb_ccmu_config;

#define SUNXI_CCMU_SCLK_GATING_USBPHY1_OFFSET	(1UL << 29)
#define SUNXI_CCMU_USBPHY1_RST_OFFSET		(1UL << 30)
#define SUNXI_CCMU_SCLK_GATING_OHCI1_OFFSET	(1UL << 31)

#define SUNXI_CCMU_USBEHCI1_GATING_OFFSET	(1UL << 5)
#define SUNXI_CCMU_USBEHCI1_RST_OFFSET		(1UL << 21)

#define SUNXI_HCI_PHY_CTRL		0x810
#define SUNXI_HCI_PHY_CTRL_SIDDQ	3
#endif
} sunxi_udc_io_t;

typedef struct sunxi_udc {
	spinlock_t			lock;
	struct platform_device		*pdev;
	struct device		        *controller;

	struct sunxi_udc_ep		ep[SW_UDC_ENDPOINTS];
	int				address;
	struct usb_gadget		gadget;
	struct usb_gadget_driver	*driver;
	struct sunxi_udc_request	fifo_req;
	u8				fifo_buf[SW_UDC_EP_FIFO_SIZE];
	u16				devstatus;

	u32				port_status;
	int				ep0state;

	unsigned			got_irq : 1;

	unsigned			req_std : 1;
	unsigned			req_config : 1;
	unsigned			req_pending : 1;
	u8				vbus;
	struct dentry			*regs_info;

	sunxi_udc_io_t			*sunxi_udc_io;
	char				driver_name[32];
	__u32				usbc_no; /* controller port index */

	u32				stopped; /* controller stop work */
	int				irq_no;	/* usb irq no */

	struct work_struct		vbus_det_work;
	struct work_struct		set_cur_vol_work;

	struct wakeup_source		*ws;
	struct regulator		*udc_regulator; /* usbc regulator:vcc-USB */
} sunxi_udc_t;

enum sunxi_udc_cmd_e {
	SW_UDC_P_ENABLE	= 1,		/* Pull-up enable        */
	SW_UDC_P_DISABLE = 2,		/* Pull-up disable       */
	SW_UDC_P_RESET	= 3,		/* UDC reset, in case of */
};

enum sunxi_udc_data_e {
	SW_UDC_D_DISABLE = 0,		/* Usb Data disable      */
	SW_UDC_D_ENABLE	= 1,		/* Usb Data enable       */
};

typedef struct sunxi_udc_mach_info {
	struct usb_port_info *port_info;
	unsigned int usbc_base;
} sunxi_udc_mach_info_t;

extern atomic_t thread_suspend_flag;
extern atomic_t notify_suspend_flag;
extern atomic_t rolesw_suspend_flag;
extern int device_insmod_delay;

extern atomic_t vfs_read_flag;
extern atomic_t vfs_write_flag;
extern unsigned int vfs_amount;
extern loff_t vfs_file_offset;

int sunxi_usb_device_enable(void);
int sunxi_usb_device_disable(void);
int get_dp_dm_status_normal(void);

#endif   /* __SUNXI_UDC_H__ */
