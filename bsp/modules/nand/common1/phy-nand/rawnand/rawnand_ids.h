/*
 * rawnand_ids.h
 *
 * Copyright (C) 2019 Allwinner.
 *
 * cuizhikui <cuizhikui@allwinnertech.com>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef __RAWNAND_IDS_H__
#define __RAWNAND_IDS_H__

#include <mtd/aw-spinand-nftl.h>
#include "../../aw_nand_type.h"
#include "../nand.h"
#include "rawnand.h"

//#define SECTOR_SIZE (512)
/*
 * NAND Flash Manufacturer ID Codes
 */
#define NAND_MFR_SPANSION 0x01
#define NAND_MFR_ATO 0x9b
#define NAND_MFR_EON 0x92
#define NAND_MFR_ESMT 0xc8
#define NAND_MFR_FUJITSU 0x04
#define NAND_MFR_HYNIX 0xad
#define NAND_MFR_INTEL 0x89
#define NAND_MFR_GIGA 0xc8
#define NAND_MFR_MXIC 0xc2
#define NAND_MFR_MACRONIX 0xc2
#define NAND_MFR_MICRON 0x2c
#define NAND_MFR_NATIONAL 0x8f
#define NAND_MFR_RENESAS 0x07
#define NAND_MFR_SAMSUNG 0xec
#define NAND_MFR_SANDISK 0x45
#define NAND_MFR_STMICRO 0x20
#define NAND_MFR_TOSHIBA 0x98
#define NAND_MFR_WINBOND 0xef
#define NAND_MFR_FORESEE 0xec

/*
 * NAND Flash Manufacture name
 * */
#define SPANSION_NAME "spansion"
#define ATO_NAME "ato"
#define EON_NAME "eon"
#define ESMT_NAME "esmt"
#define FUJITSU_NAME "fujitsu"
#define HYNIX_NAME "hynix"
#define INTEL_NAME "intel"
#define MACRONIX_NAME "macronix"
#define GIGA_NAME "giga"
#define MXIC_NAME "mxic"
#define MICRON_NAME "micron"
#define NATIONAL_NAME "national"
#define RENESAS_NAME "renesas"
#define SAMSUNG_NAME "samsung"
#define SANDISK_NAME "sandisk"
#define STMICRO_NAME "stmicro"
#define TOSHIBA_NAME "toshiba"
#define WINBOND_NAME "winbond"
#define FORESEE_NAME "foresee"

/**
 *  write boot0 special requirement
 *
 */
/*
 *#define NAND_WRITE_BOOT0_GENERIC 0x00
 *#define NAND_WRITE_BOOT0_HYNIX_16NM_4G 0x01
 *#define NAND_WRITE_BOOT0_HYNIX_20NM 0x02
 *#define NAND_WRITE_BOOT0_HYNIX_26NM 0x03
 */

/**
 * operation_opt explain
 * define the mask for the nand flash optional operation
 *
 */
/*nand flash support cache read operation*/
#define NAND_CACHE_READ (1 << 0)
/*nand flash support page cache program operation*/
#define NAND_CACHE_PROGRAM (1 << 1)
/*nand flash support multi-plane page read operation*/
#define NAND_MULTI_READ (1 << 2)
/*nand flash support multi-plane page program operation*/
#define NAND_MULTI_PROGRAM (1 << 3)
/*nand flash support page copy-back command mode operation*/
#define NAND_PAGE_COPYBACK (1 << 4)
/*nand flash support internal inter-leave operation, it based multi-bank*/
#define NAND_INT_INTERLEAVE (1 << 5)
/*nand flash support external inter-leave operation, it based multi-chip*/
#define NAND_EXT_INTERLEAVE (1 << 6)
/*nand flash support RANDOMIZER*/
#define NAND_RANDOM (1 << 7)
/*nand falsh support READ RETRY*/
#define NAND_READ_RETRY (1 << 8)
/* nand falsh support READ UNIQUE_ID*/
#define NAND_READ_UNIQUE_ID (1 << 9)
/*nand falsh page adr no skip is requiered*/
#define NAND_PAGE_ADR_NO_SKIP (1 << 10)
/*nand flash die adr skip*/
#define NAND_DIE_SKIP (1 << 11)
/*nand flash log block type management*/
//#define NAND_LOG_BLOCK_MANAGE   (1<<12)
/*nand flash need check status after write or erase*/
//#define NAND_FORCE_WRITE_SYNC    (1<<13)
/*nand flash log block lsb page type*/
//#define NAND_LOG_BLOCK_LSB_TYPE (0xff<<16)
/* nand flash lsb page type*/
#define NAND_LSB_PAGE_TYPE (0xff << 12)
/*nand flash support the maximum block erase cnt*/
#define NAND_MAX_BLK_ERASE_CNT (1 << 20)
/*nand flash support to read reclaim Operation*/
#define NAND_READ_RECLAIM (1 << 21)
/*
 * micron: different flash have the same id
 */
#define NAND_FIND_ID_TAB_BY_NAME (1 << 22)

/*it use in the case: name(read from flash) is not the unique,
 * but id table 1 is conifure to NAND_FIND_ID_TAB_BY_NAME. in this case,
 * you can use NAND_FIND_IN_TAB_BY_NAME_CANCEL to cancel name matching in id table 2.
 * id table 2 is the same with id table1 except the optional*/
#define NAND_FIND_ID_TAB_BY_NAME_CANCEL (1 << 23)
/* nand flash support onfi's sync reset*/
#define NAND_ONFI_SYNC_RESET_OP (1 << 28)
/* nand flash support toggle interface only, and do not support switch between legacy and toggle*/
#define NAND_TOGGLE_ONLY (1 << 29)
/* nand flash has two row address only*/
#define NAND_WITH_TWO_ROW_ADR (1 << 30)
/*nand flash support cache for lsb page data untill shared page data is writed. (only for 3D nand)*/
#define NAND_PAIRED_PAGE_SYNC (1 << 31)



/*ddr_opt*/
/*nand flash support to set ddr2 specific feature according to ONFI 3.0*/
#define NAND_ONFI_DDR2_CFG (1 << 0)
/*nand flash support to set io driver strength according to ONFI 2.x/3.0*/
#define NAND_ONFI_IO_DRIVER_STRENGTH (1 << 1)
/*nand flash support to set io RB strength according to ONFI 2.x/3.0*/
#define NAND_ONFI_RB_STRENGTH (1 << 2)
/* nand flash support to change timing mode according to ONFI 3.0*/
#define NAND_ONFI_TIMING_MODE (1 << 3)
/*nand flash support to set ddr2 specific feature according to Toggle DDR2*/
#define NAND_TOGGLE_SPECIFIC_CFG (1 << 8)
/*nand flash support to set io driver strength according to Toggle DDR1/DDR2*/
#define NAND_TOGGLE_IO_DRIVER_STRENGTH (1 << 9)
/* nand flash support to set vendor's specific configuration*/
#define NAND_TOGGLE_VENDOR_SPECIFIC_CFG (1 << 10)
/*
 * nand flash VccQ need 1.8v, it affect to the DC input high/low voltage
 * reference flash datasheet electrical specifications - DC characteristics
 * and operating characteristrics
 * */
#define NAND_VCCQ_1P8V (1 << 16)

/**
 * valid_blk_ratio
 */
#define VALID_BLK_RATIO_DEFAULT 896

/**
 * NDFC ECC MODE
 * ECC MODE calculate by the way:
 * ((spare_size(Byte) / page_size(KByte) - 4) / 14) * 8
 * eg. one flash one page is 8192 Bytes(B KByte),corresponding spare is 744(A)Bytes
 * ecc mode is (((744(A) / 8(B)) - 4) / 14) * 8
 * so ecc mode chose BCH_48
 * BCH_x choose the closest to the calculated value C and x <= C
 */
#define BCH_16 (0x00)
#define BCH_24 (0x01)
#define BCH_28 (0x02)
#define BCH_32 (0x03)
#define BCH_40 (0x04)
#define BCH_44 (0x05)
#define BCH_48 (0x06)
#define BCH_52 (0x07)
#define BCH_56 (0x08)
#define BCH_60 (0x09)
#define BCH_64 (0x0A)
#define BCH_68 (0x0B)
#define BCH_72 (0x0C)
#define BCH_76 (0x0D)
#define BCH_80 (0x0E)

/**
 * read_retry_type
 * slc chose READ_RETRY_TYPE_NULL
 * mlc please contact us(allwinner's memory team)
 **/
#define READ_RETRY_TYPE_NULL (0x000000)
/**
 * ddr_type
 * slc is usually SDR
 * mlc chooses according to datasheet
 * (SDR/ONFI_DDR/ONFI_DDR2/TOG_DDR/TOG_DDR2) -- define in ndfc_base.h
 */

/**
 * cmd_set_no
 * flash operation command set number
 * if rawnand_ids.c don't have your flash operation set in phy_op_para table,
 * you should add by yourself,detail see the define of nand_phy_op_par struction
 */
#define CMD_SET_0 (0)
#define CMD_SET_1 (1)
#define CMD_SET_2 (2)
#define CMD_SET_3 (3)


/*
* chose lsb function
* the following parameter should be consistent with
* boot0(nand_scan_for_boot.c's nand_is_lsb_page),
* slc nand flash chose GENERIC_LSB_PAGE |= operation_opt
* (operation_opt define in sunxi_nand_flash_device)
* mlc nand flash,chose thes parameter in below |= operation_opt,
* according to nand flash datasheet.
* if you want to add other parameter, you should do things as below:
* 1. add lsb function in rawnand_readretry.c
* 2. add the case in chose_lsb_func in rawnand_readretry.c
* 3. add lsb case in boot0 nand_scan_for_boot.c's nand_is_lsb_page.
* 4. add parameter in below,it is consistent with nand_is_lsb_page case number.
*/

#define LSB_PAGE_POS (12)
#define GENERIC_LSB_PAGE (0x00 << LSB_PAGE_POS)
#define HYNIX20_26NM_LSB_PAGE (0X01 << LSB_PAGE_POS)
#define HYNIX16NM_LSB_PAGE (0X02 << LSB_PAGE_POS)
#define TOSHIBA_LSB_PAGE (0x10 << LSB_PAGE_POS)
#define SAMSUNG_LSB_PAGE (0x20 << LSB_PAGE_POS)
#define SANDISK_LSB_PAGE (0x30 << LSB_PAGE_POS)
#define MICRON_0x40_LSB_PAGE (0x40 << LSB_PAGE_POS)
#define MICRON_0x41_LSB_PAGE (0x41 << LSB_PAGE_POS)
#define MICRON_0x42_LSB_PAGE (0x42 << LSB_PAGE_POS)
#define MICRON_0x43_LSB_PAGE (0x43 << LSB_PAGE_POS)
#define MICRON_0x44_LSB_PAGE (0x44 << LSB_PAGE_POS)




/**
 * nand_id_tbl_info: nand id table information interface set
 * @
 * @
 */


struct nand_manufacture {
	u8 id;
	char *name;
	struct sunxi_nand_flash_device *dev;
	int ndev;
};

extern struct nand_manufacture mfr_tbl[];
extern struct sunxi_nand_flash_device selected_id_tbl;
extern struct nfc_init_ddr_info def_ddr_info[];
extern struct nand_phy_op_par phy_op_para[];

#define NAND_MANUFACTURE(_id, _name, _mfr)                \
	{                                                 \
		.id = _id,                                \
		.name = _name,                            \
		.dev = _mfr,                              \
		.ndev = (sizeof(_mfr) / sizeof(_mfr[0])), \
	}

typedef int (*lsb_page_t)(__u32 no);
extern lsb_page_t chose_lsb_func(__u32 no);


struct sunxi_nand_flash_device *sunxi_search_id(struct nand_chip_info *nci,
		unsigned char *id);

#if 0
static inline char *dev_info_name(struct nand_chip_info *chip)
{
	struct sunxi_nand_flash_device *info = chip->info;

	return info->name;
}

static inline void dev_info_id(struct nand_chip_info *chip, unsigned char *id,
		int cnt)
{
	struct sunxi_nand_flash_device *info = chip->info;

	memcpy(id, info->id, cnt);
}

static inline unsigned int dev_info_die_cnt(struct nand_chip_info *chip)
{
	struct sunxi_nand_flash_device *info = chip->info;

	return info->die_cnt_per_chip;
}
static inline unsigned int dev_info_page_size(struct nand_chip_info *chip,
		enum size_type type)
{
	struct sunxi_nand_flash_device *info = chip->info;

	if (likely(type == SECTOR)) {
		return info->sect_cnt_per_page;
	} else if (unlikely(type == BYTE)) {
		return info->sect_cnt_per_page * SECTOR_SIZE;
	} else {
		pr_err("no this type:%d single page size in BYTE(0)@byte "
				"SECTOR(1)@sector", type);
		return -EINVAL;
	}

}

static inline unsigned int dev_info_block_size(struct nand_chip_info *chip,
		enum size_type type)
{
	struct sunxi_nand_flash_device *info = chip->info;

	if (likely(type == PAGE)) {
		return info->page_cnt_per_blk;
	} else if (unlikely(type == SECTOR)) {
		return info->page_cnt_per_blk * info->sect_cnt_per_page;
	} else if (unlikely(type == BYTE)) {
		return info->page_cnt_per_blk * info->sect_cnt_per_page
						* SECTOR_SIZE;
	} else {
		pr_err("no this type:%d single block size in BYTE(0)@byte "
				"SECTOR(1)@sector PAGE(2)@page", type);
		return -EINVAL;
	}

}

static inline unsigned int dev_info_die_size(struct nand_chip_info *chip,
		enum size_type type)
{
	struct sunxi_nand_flash_device *info = chip->info;

	if (likely(type == BLOCK))
		return info->blk_cnt_per_die;
	else if (unlikely(type == BYTE))
		return info->blk_cnt_per_die * info->page_cnt_per_blk
			* info->sect_cnt_per_page * SECTOR_SIZE;
	else if (unlikely(type == SECTOR))
		return info->blk_cnt_per_die * info->page_cnt_per_blk
			* info->sect_cnt_per_page;
	else if (unlikely(type == PAGE))
		return info->blk_cnt_per_die * info->page_cnt_per_blk;
	else {
		pr_err("no this type:%d die size in BYTE(0)@byte "
			"SECTOR(1)@sector PAGE(2)@page BLOCK(3)@block", type);
		return -EINVAL;
	}
}

static inline unsigned long long dev_info_opt(struct nand_chip_info *chip)
{
	struct sunxi_nand_flash_device *info = chip->info;

	return info->operation_opt;
}

static inline unsigned int dev_info_ecc_mode(struct nand_chip_info *chip)
{
	struct sunxi_nand_flash_device *info = chip->info;

	return info->ecc_mode;
}
static inline unsigned int dev_info_freq(struct nand_chip_info *chip)
{
	struct sunxi_nand_flash_device *info = chip->info;

	return info->access_freq;
}

static inline unsigned int dev_info_readretry_type(struct nand_chip_info *chip)
{
	struct sunxi_nand_flash_device *info = chip->info;

	return info->read_retry_type;
}

static inline unsigned int dev_info_ddr_type(struct nand_chip_info *chip)
{
	struct sunxi_nand_flash_device *info = chip->info;

	return info->ddr_type;
}

static inline unsigned int dev_info_id_number(struct nand_chip_info *chip)
{
	struct sunxi_nand_flash_device *info = chip->info;

	return info->id_number;
}

static inline unsigned int dev_info_max_erase_times(struct nand_chip_info *chip)
{
	struct sunxi_nand_flash_device *info = chip->info;

	return info->max_blk_erase_times;
}

static inline unsigned int dev_info_access_high_freq(struct nand_chip_info
		*chip)
{
	struct sunxi_nand_flash_device *info = chip->info;

	return info->access_high_freq;
}
#endif
#endif /*RAWNAND_IDS_H*/
