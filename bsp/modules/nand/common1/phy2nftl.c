/*SPDX-Licensen-Identifier: GPL-2.0*/

#include "phy2nftl.h"
#include "nfd/nand_osal_for_linux.h"
#include "phy-nand/nand.h"
#include "phy-nand/rawnand/rawnand_chip.h"
#include <mtd/aw-spinand-nftl.h>

void nftl_get_spinand_phyinfo(nand_phyinfo_for_nftl_t *info)
{
	info->die_cnt_per_chip = spinand_nftl_get_die_cnt();
	info->blk_cnt_per_die = spinand_nftl_get_die_size(BLOCK);
	info->page_cnt_per_blk = (__u16)spinand_nftl_get_single_block_size(PAGE);
	info->sector_cnt_per_page = (__u8)spinand_nftl_get_single_page_size(SECTOR);
	spinand_nftl_get_chip_id(info->nand_chip_id);

	return;
}

void nftl_get_rawnand_phyinfo(nand_phyinfo_for_nftl_t *info)
{
	info->die_cnt_per_chip = rawnand_get_chip_die_cnt(g_nsi->nci);
	info->blk_cnt_per_die = rawnand_get_chip_die_size(g_nsi->nci, BLOCK);
	info->page_cnt_per_blk = (__u16)rawnand_get_chip_block_size(g_nsi->nci, PAGE);
	info->sector_cnt_per_page = (__u8)rawnand_get_chip_page_size(g_nsi->nci, SECTOR);
	rawnand_get_chip_id(g_nsi->nci, info->nand_chip_id, NAND_MAX_ID_LEN);
}
void nftl_get_nand_phyinfo(nand_phyinfo_for_nftl_t *info)
{
	if (get_storage_type() == 1)
		nftl_get_rawnand_phyinfo(info);
	else if (get_storage_type() == 2)
		nftl_get_spinand_phyinfo(info);
}
