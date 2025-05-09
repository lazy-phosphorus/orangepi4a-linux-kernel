/*****************************************************************************
 * nand_base.c for  SUNXI NAND .
 *
 * Copyright (C) 2016 Allwinner.
 *
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 *
*****************************************************************************/

#include "nand_base.h"
/*#include "../phy-nand/nand-partition2/nand_info_init_v2.h"*/
#include "../phy-nand/nand_physic_interface.h"
#include "../phy-nand/rawnand/controller/ndfc_base.h"
#include "nand_osal_for_linux.h"
#include "nand_panic.h"
#include "nand_type.h"

struct sunxi_ndfc aw_ndfc;
struct nand_controller_info g_nctri_data[2] = {0};
static unsigned int channel0;


extern int nand_info_init(struct _nand_info *nand_info, int state);

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
******************************************************************************/

static irqreturn_t nand_interrupt(int irq, void *channel)
{
	unsigned int no;
	unsigned long iflags;

	spin_lock_irqsave(&aw_ndfc.nand_int_lock, iflags);
	no = *((unsigned int *)channel);
	do_nand_interrupt(no);
	spin_unlock_irqrestore(&aw_ndfc.nand_int_lock, iflags);
	return IRQ_HANDLED;
}


#if 0
/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
******************************************************************************/
static int nand_early_suspend(void)
{
	struct _nftl_blk *nftl_blk;
	struct nand_blk_ops *tr = &mytr;

	nftl_blk = tr->nftl_blk_head.nftl_blk_next;

	nand_dbg_err("nand_early_suspend\n");
	while (nftl_blk != NULL) {
		nand_dbg_err("nand\n");
		mutex_lock(nftl_blk->blk_lock);
		nftl_blk->flush_write_cache(nftl_blk, 0xffff);
		mutex_unlock(nftl_blk->blk_lock);
		nftl_blk = nftl_blk->nftl_blk_next;
	}
	return 0;
}

/****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
static int nand_early_resume(void)
{
	nand_dbg_err("nand_early_resume\n");
	return 0;
}
#endif
/****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
static int nand_suspend(struct platform_device *plat_dev, pm_message_t state)
{
#if 0
	if (standby_type == NORMAL_STANDBY) {
		nand_dbg_err("[NAND] nand_suspend normal\n");

		NandHwNormalStandby();
	} else if (standby_type == SUPER_STANDBY) {
		nand_dbg_err("[NAND] nand_suspend super\n");
		nand_hw_super_standby();
	}

	nand_dbg_err("[NAND] nand_suspend ok\n");
#else
	nand_dbg_err("[NAND] nand_suspend\n");
	nand_hw_super_standby();
	nand_dbg_err("[NAND] nand_suspend ok\n");
#endif
	return 0;
}

/****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
static int nand_resume(struct platform_device *plat_dev)
{
#if 0
	if (standby_type == NORMAL_STANDBY) {
		nand_dbg_err("[NAND] nand_resume normal\n");
		NandHwNormalResume();
	} else if (standby_type == SUPER_STANDBY) {
		nand_dbg_err("[NAND] nand_resume super\n");
		nand_hw_super_resume();
	}

	nand_dbg_err("[NAND] nand_resume ok\n");
#else
	nand_dbg_err("[NAND] nand_resume\n");
	nand_hw_super_resume();
	nand_dbg_err("[NAND] nand_resume ok\n");
#endif
	return 0;
}


#if (defined(CONFIG_ARCH_SUN50IW12) ||  defined(CONFIG_ARCH_SUN50IW10) || \
		defined(CONFIG_ARCH_SUN50IW9) || defined(CONFIG_ARCH_SUN55IW3))
int nand_init_real(void)
{
	int nand_cache_level = 0;
	int nand_capacity_level = 0;
	int nand_flush_cache_num = 8;
	int dragonboard_flag = 0;
	int ret;

	ret = of_property_read_u32(aw_ndfc.dev->of_node, "nand0_cache_level",
				   &nand_cache_level);
	if (ret) {
		nand_dbg_err("Failed to get nand0_cache_level\n");
		nand_cache_level = 0;
	} else {
		if (nand_cache_level == 0x55aaaa55) {
			nand_dbg_inf("nand0_cache_level is no used\n");
			nand_cache_level = 0;
		}
	}

	ret = of_property_read_u32(aw_ndfc.dev->of_node, "nand0_flush_cache_num",
				   &nand_flush_cache_num);
	if (ret) {
		nand_dbg_err("Failed to get nand_flush_cache_num\n");
		nand_flush_cache_num = 8;
	} else {
		if (nand_flush_cache_num == 0x55aaaa55) {
			nand_dbg_inf("nand_flush_cache_num is no used\n");
			nand_flush_cache_num = 8;
		}
	}

	ret = of_property_read_u32(aw_ndfc.dev->of_node, "nand0_capacity_level",
				   &nand_capacity_level);
	if (ret) {
		nand_dbg_err("Failed to get nand_capacity_level\n");
		nand_capacity_level = 0;
	} else {
		if (nand_capacity_level == 0x55aaaa55) {
			nand_dbg_inf("nand_capacity_level is no used\n");
			nand_capacity_level = 0;
		}
	}

	dragonboard_flag = nand_get_dragon_board_flag(&aw_ndfc);

	if (dragonboard_flag == 0) {
		nand_dbg_err("nand init start\n");

		p_nand_info = nand_hw_init();
		if (p_nand_info == NULL) {
			return -EAGAIN;
		}

		set_cache_level(p_nand_info, nand_cache_level);
		set_capacity_level(p_nand_info, nand_capacity_level);
		/*ret = nand_info_init(p_nand_info, 0, 8, NULL);*/
		ret = nand_info_init(p_nand_info, 0);
		if (ret != 0) {
			nand_dbg_err("nand_info_init error\n");
			return ret;
		}

		init_blklayer();

	} else {
		nand_dbg_err("dragonboard_flag=%d,run nand test for dragonboard\n",
			     dragonboard_flag);
		init_blklayer_for_dragonboard();
		return 0;
	}
	kthread_run(nand_thread, &mytr, "%sd", "nand_rc");

	nand_panic_init(mytr.nftl_blk_head.nftl_blk_next);

	return 0;
}
#endif
/****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
u64 nand_dma_mask;
static int nand_probe(struct platform_device *pdev)
{
	__u32 irq;
	char *dev_name = "nand_dev";
	struct nand_controller_info *nctri0 = &g_nctri_data[0];
	struct nand_controller_info *nctri1 = &g_nctri_data[1];
	int err = 0;
    nand_dbg_err("nand_probe start\n");


	aw_ndfc.dev = &pdev->dev;

	nand_dma_mask = DMA_BIT_MASK(64);

	pdev->dev.dma_mask = &nand_dma_mask;
	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(64);
	aw_ndfc.p = pinctrl_get(aw_ndfc.dev);

	spin_lock_init(&aw_ndfc.nand_int_lock);

	if (get_storage_type() == NAND_STORAGE_TYPE_RAWNAND) {
		if ((nand_wait_rb_mode() != 0) || (nand_wait_dma_mode() != 0)) {
			nand_dbg_inf("nand interrupt request\n");

			irq = irq_of_parse_and_map(aw_ndfc.dev->of_node, 0);
			if (request_irq(irq, nand_interrupt, 0, dev_name, &channel0)) {
				nand_dbg_err("nand: allocating IRQ %d error\n", irq);
				err = -EAGAIN;
				goto fail;
			}
		}

		if (nand_get_max_channel_cnt() == 1) {
			nctri0->nreg_base = (void *)of_iomap(aw_ndfc.dev->of_node, 0);
			nand_dbg_inf("NDFC0_BASE_ADDR %p\n", nctri0->nreg_base);
			if (!nctri0->nreg_base) {
				nand_dbg_err("Failed to map NDFC0 IO space1\n");
				err = -EAGAIN;
				goto fail;
			}
		} else if (nand_get_max_channel_cnt() == 2) {
			nctri0->nreg_base = (void *)of_iomap(aw_ndfc.dev->of_node, 0);
			if (!nctri0->nreg_base) {
				nand_dbg_err("Failed to map NDFC0 IO space2\n");
				err =  -EAGAIN;
				goto fail;
			}
			nctri1->nreg_base = (void *)of_iomap(aw_ndfc.dev->of_node, 1);
			if (!nctri1->nreg_base) {
				nand_dbg_err("Failed to map NDFC1 IO space3\n");
				err =  -EAGAIN;
				goto fail;
			}
		}

#if (defined(CONFIG_ARCH_SUN50IW12) ||  defined(CONFIG_ARCH_SUN50IW10) || \
		defined(CONFIG_ARCH_SUN50IW9) || defined(CONFIG_ARCH_SUN55IW3))
		nand_init_real();
#endif
	} else if (get_storage_type() == NAND_STORAGE_TYPE_SPINAND) {
#if 0
		nand_dbg_inf("spinand interrupt request\n");
		irq = irq_of_parse_and_map(aw_ndfc.dev->of_node, 0);
		if (request_irq(irq, nand_interrupt, 0, dev_name, &channel0)) {
			nand_dbg_err("nand: allocating IRQ %d error\n", irq);
			return -EAGAIN;
		}

		SPIC0_IO_BASE = (void *)of_iomap(aw_ndfc.dev->of_node, 0);
		nand_dbg_err("SPIC0_BASE_ADDR %p\n", SPIC0_IO_BASE);
		if (!SPIC0_IO_BASE) {
			nand_dbg_err("Failed to map SPIC0 IO space1\n");
			return -EAGAIN;
		}
		init_completion(&spinand_dma_done);
#endif
	}

	/*exit_probe_flag = 1;*/
	nand_dbg_err("nand_probe end\n");
	return 0;

fail:
	nand_dbg_err("nand_probe fail");
	pinctrl_put(aw_ndfc.p);
	aw_ndfc.dev = NULL;
	return err;
}

/****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
static int nand_remove(struct platform_device *plat_dev)
{
	nand_dbg_inf("nand_remove\n");
	return 0;
}

/****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
uint32 shutdown_flush_write_cache(void)
{
	struct _nftl_blk *nftl_blk;
	struct nand_blk_ops *tr = &mytr;

	nftl_blk = tr->nftl_blk_head.nftl_blk_next;

	while (nftl_blk != NULL) {
		nand_dbg_err("shutdown_flush_write_cache\n");
		mutex_lock(nftl_blk->blk_lock);
		nftl_blk->flush_write_cache(nftl_blk, 0xffff);

		print_nftl_zone(nftl_blk->nftl_zone);

		nftl_blk = nftl_blk->nftl_blk_next;
	}
	return 0;
}

/****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
void nand_shutdown(struct platform_device *plat_dev)
{
	shutdown_flush_write_cache();
	nand_hw_shutdown();
	nand_dbg_err("[NAND]shutdown end\n");
}

/****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
static const struct of_device_id of_nand_id[] = {
	{ .compatible = "allwinner,sun50iw9-nand"},
	{ .compatible = "allwinner,sun50iw10-nand"},
	{ .compatible = "allwinner,sun50iw11-nand"},
	{ .compatible = "allwinner,sun50iw12-nand"},
	{ .compatible = "allwinner,sun8iw18-nand"},
	{ .compatible = "allwinner,sun55iw3-nand"},
	{/* sentinel */},
};

static struct platform_driver nand_driver = {
    .probe = nand_probe,
    .remove = nand_remove,
    .shutdown = nand_shutdown,
    .suspend = nand_suspend,
    .resume = nand_resume,
    .driver = {
	.name = "sw_nand",
	.owner = THIS_MODULE,
	.of_match_table = of_nand_id,
    },
};

static const struct of_device_id of_spinand_id = {
	.compatible = "allwinner,sunxi-spinand",
};

static struct platform_driver spinand_driver = {
    .probe = nand_probe,
    .remove = nand_remove,
    .shutdown = nand_shutdown,
    .suspend = nand_suspend,
    .resume = nand_resume,
    .driver = {
	.name = "sw_nand",
	.owner = THIS_MODULE,
	.of_match_table = &of_spinand_id,
    },
};

/****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int __init nand_init(void)
{
	int ret;
#if (!defined(CONFIG_ARCH_SUN50IW12) && !defined(CONFIG_ARCH_SUN50IW10) && \
		!defined(CONFIG_ARCH_SUN50IW9) && !defined(CONFIG_ARCH_SUN55IW3))
	int nand_cache_level = 0;
	int nand_capacity_level = 0;
	int nand_flush_cache_num = 8;
	int dragonboard_flag = 0;
#endif
	struct device_node *np = NULL;
	const char *sta;

	/*exit_probe_flag = 0;*/
	nand_type = NAND_STORAGE_TYPE_NULL;

	/*raw-nand*/
	np = of_find_node_by_type(NULL, "nand0");
	if (!np) {
		nand_dbg_err("get raw-nand node failed\n");
		/*spi-nand*/
		np = of_find_node_by_type(NULL, "spinand");
		if (!np) {
			nand_dbg_err("get spi-nand node failed\n");
			return 0;
		} else {
			ret = of_property_read_string(np, "status", &sta);
			if (ret)
				nand_dbg_err("get spiN stat fail,%d\n", -ret);

			if (!strcasecmp(sta, "okay"))
				nand_type = NAND_STORAGE_TYPE_SPINAND;
		}
	} else {
		ret = of_property_read_string(np, "status", &sta);
		if (ret)
			nand_dbg_err("get rawN stat fail,%d\n", -ret);

		if (!strcasecmp(sta, "okay"))
			nand_type = NAND_STORAGE_TYPE_RAWNAND;
		else if (!strcasecmp(sta, "disabled"))
			nand_type = NAND_STORAGE_TYPE_NULL;
	}

	if (get_storage_type() == NAND_STORAGE_TYPE_NULL) {
		nand_dbg_err("[NE] Not found valid nand node on dts\n");
		return -EINVAL;
	}

	ret = nand_support_panic();
	if (ret)
		nand_dbg_err("NOT support panic nand\n");
	else
		nand_dbg_err("support panic nand\n");


	if (get_storage_type() == NAND_STORAGE_TYPE_RAWNAND)
		ret = platform_driver_register(&nand_driver);
	else if (get_storage_type() == NAND_STORAGE_TYPE_SPINAND)
		ret = platform_driver_register(&spinand_driver);

	if (ret != 0) {
		nand_dbg_err("Failed to insmod nand!!!\n");
		return 0;
	}


#if (!defined(CONFIG_ARCH_SUN50IW12) &&  !defined(CONFIG_ARCH_SUN50IW10) && \
		!defined(CONFIG_ARCH_SUN50IW9) && !defined(CONFIG_ARCH_SUN55IW3))
	ret = of_property_read_u32(aw_ndfc.dev->of_node, "nand0_cache_level",
				   &nand_cache_level);
	if (ret) {
		nand_dbg_err("Failed to get nand0_cache_level\n");
		nand_cache_level = 0;
	} else {
		if (nand_cache_level == 0x55aaaa55) {
			nand_dbg_inf("nand0_cache_level is no used\n");
			nand_cache_level = 0;
		}
	}
    nand_dbg_err("init no in probe %s,%d\n", __FUNCTION__, __LINE__);

	ret = of_property_read_u32(aw_ndfc.dev->of_node, "nand0_flush_cache_num",
				   &nand_flush_cache_num);
	if (ret) {
		nand_dbg_err("Failed to get nand_flush_cache_num\n");
		nand_flush_cache_num = 8;
	} else {
		if (nand_flush_cache_num == 0x55aaaa55) {
			nand_dbg_inf("nand_flush_cache_num is no used\n");
			nand_flush_cache_num = 8;
		}
	}

	ret = of_property_read_u32(aw_ndfc.dev->of_node, "nand0_capacity_level",
				   &nand_capacity_level);
	if (ret) {
		nand_dbg_err("Failed to get nand_capacity_level\n");
		nand_capacity_level = 0;
	} else {
		if (nand_capacity_level == 0x55aaaa55) {
			nand_dbg_inf("nand_capacity_level is no used\n");
			nand_capacity_level = 0;
		}
	}

	dragonboard_flag = nand_get_dragon_board_flag(&aw_ndfc);

	if (dragonboard_flag == 0) {
		nand_dbg_err("nand init start\n");

		p_nand_info = nand_hw_init();
		if (p_nand_info == NULL) {
			return -EAGAIN;
		}

		set_cache_level(p_nand_info, nand_cache_level);
		set_capacity_level(p_nand_info, nand_capacity_level);
		/*ret = nand_info_init(p_nand_info, 0, 8, NULL);*/
		ret = nand_info_init(p_nand_info, 0);
		if (ret != 0) {
			nand_dbg_err("nand_info_init error\n");
			return ret;
		}

		init_blklayer();

	} else {
		nand_dbg_err("dragonboard_flag=%d,run nand test for dragonboard\n",
			     dragonboard_flag);
		init_blklayer_for_dragonboard();
		return 0;
	}
	kthread_run(nand_thread, &mytr, "%sd", "nand_rc");

	nand_panic_init(mytr.nftl_blk_head.nftl_blk_next);

	nand_dbg_err("nand init end\n");
#endif
	return 0;
}

/****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
void __exit nand_exit(void)
{
	exit_blklayer();

	if (get_storage_type() == NAND_STORAGE_TYPE_RAWNAND)
		platform_driver_unregister(&nand_driver);
	else if (get_storage_type() == NAND_STORAGE_TYPE_SPINAND)
		platform_driver_unregister(&spinand_driver);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("nand flash groups");
MODULE_DESCRIPTION("Generic NAND flash driver code");
