/*
 * nand_dev.c for  SUNXI NAND .
 *
 * Copyright (C) 2016 Allwinner.
 *
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#define _NAND_DEV_C_

#include "nand_blk.h"
#include "nand_dev.h"
#include "nand_base.h"
#include "nand_class.h"
#include <linux/sysfs.h>

#define NAND_SCHEDULE_TIMEOUT  (HZ >> 1)
#define NFTL_SCHEDULE_TIMEOUT  (HZ >> 1)
#define NFTL_FLUSH_DATA_TIME	 1
#define WEAR_LEVELING 1

static int dev_num;

unsigned long nand_active_time;
unsigned long nand_write_active_time;
unsigned long nand_read_active_time;

struct _nand_info *p_nand_info;
/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int nand_thread(void *arg)
{
	unsigned long ret, time_val;
	struct nand_blk_ops *tr = (struct nand_blk_ops *)arg;

	unsigned int start_time = 600;
	unsigned char *temp_buf = kmalloc(64 * 1024, GFP_KERNEL);

	time_val = NAND_SCHEDULE_TIMEOUT;

	while (!kthread_should_stop()) {
		if (start_time != 0) {
			start_time--;
			goto nand_thread_exit;
		}
		ret =
		    nand_read_reclaim(tr->nftl_blk_head.nftl_blk_next,
				      temp_buf);

nand_thread_exit:
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(time_val);
	}

	return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
static int nftl_thread(void *arg)
{
	struct _nftl_blk *nftl_blk = arg;
	unsigned long time;
	unsigned long swl_time = jiffies;
	int first_miss_swl = 0;
	int need_swl = 0;
	int swl_done = 0;

	while (!kthread_should_stop()) {
		mutex_lock(nftl_blk->blk_lock);

		if (NAND_STORAGE_TYPE_RAWNAND == get_storage_type())
			static_wear_leveling(nftl_blk->nftl_zone);

#if WEAR_LEVELING

		/*
		 * add static WL:
		 * we do the static WL when nftl ops is idle over 4s;
		 * if missing the chance over 64s, we will do it mandatorily!!!
		 * if we have done the static WL successfully, the time interval
		 * to next static WL must be over 64s.
		 */
		time = jiffies;
		if (swl_done) {
			if (time_after(time, swl_time + (HZ << 6)))
				need_swl = 1;	/* next static WL is over 64s */
		} else if (time_after(time,
			 (unsigned long)(nftl_blk->ops_time + (HZ << 2)))) {
			/* nftl ops is idle over 4s */
			need_swl = 1;
		} else if (first_miss_swl) {
			if (time_after(time, swl_time + (HZ << 6)))
				need_swl = 1;	/* next static WL is over 64s */
		} else {
			first_miss_swl = 1;
			swl_time = jiffies;
		}

		if (need_swl) {
			first_miss_swl = 0;
			need_swl = 0;
			swl_done = do_static_wear_leveling(nftl_blk->nftl_zone);
			if (!swl_done) {
				swl_done = 1;
				swl_time = jiffies;
			} else
				swl_done = 0;
		}
#endif

		if (garbage_collect(nftl_blk->nftl_zone) != 0)
			nand_dbg_err("nftl_thread garbage_collect error!\n");

		if (do_prio_gc(nftl_blk->nftl_zone) != 0)
			nand_dbg_err("nftl_thread do_prio_gc error!\n");

		/*
		 * deep gc for small spinand
		 * In order to recover small spinand speed with DISCARD NOT OK
		 * YET, we can do deep gc. It means gc more block dynamically
		 * according to how many free blocks. The fewer free blocks,
		 * the more recycled.
		 * As far as we know, the speed problem is only issued on small
		 * nand, for example 128MB. So, it works only when NAND has
		 * samll capacity (128M).
		 * Avoid to reduce speed for userspace, we do it when in idle.
		 */
		/* nand 10 min idle */
		if (time_after(jiffies, nand_active_time + (HZ * 60 * 60))) {
			int ret;
			/*
			 * gc with invalid_page_count = (page_per_block / 2)
			 *
			 * gc_all_enhance may take a few seconds. To avoid
			 * reduce read/write speed, do it only when nand is in
			 * idle for 10min
			 */
			ret = gc_all_enhance(nftl_blk->nftl_zone);
			if (ret)
				nand_dbg_err("nftl_thread: enhance gc all error\n");
		/* read nand 8 sec idle */
		} else if (time_after(jiffies, nand_read_active_time + (HZ << 3))) {
			int ret;
			/*
			 * gc according to free block count
			 * invalid page cnt between 32 to 61
			 *
			 * gc_all_base_on_free_blks just needs to check
			 * read_active_time rather than active_time. Because
			 * if some apps, like syslogd, keep writing, it may
			 * never be idle if active_time.
			 * Write operation allways takes time to do gc, it's no
			 * matter to do more here.
			 */
			ret = gc_all_base_on_free_blks(nftl_blk->nftl_zone);
			if (ret)
				nand_dbg_err("nftl_thread: dynamic gc all error\n");
		}

		mutex_unlock(nftl_blk->blk_lock);

		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(NFTL_SCHEDULE_TIMEOUT);
	}

	nftl_blk->nftl_thread = (void *)NULL;
	return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
void add_nand_dev_list(struct _nand_dev *head, struct _nand_dev *nand_dev)
{
	struct _nand_dev *p = head;

	nand_dev->nand_dev_next = NULL;
	while (p->nand_dev_next != NULL)
		p = p->nand_dev_next;

	p->nand_dev_next = nand_dev;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
struct _nand_dev *del_last_nand_dev(struct _nand_dev *head)
{
	struct _nand_dev *nand_dev = NULL;
	struct _nand_dev *p = head;

	while (p->nand_dev_next != NULL) {
		nand_dev = p->nand_dev_next;
		if (nand_dev->nand_dev_next == NULL) {
			p->nand_dev_next = NULL;
			return nand_dev;
		}
		p = p->nand_dev_next;
	}
	return NULL;
}

int nand_gpt_enable;

static int get_gpt_para_from_cmdline(const char *cmdline, const char *name, char *value, int maxsize)
{
	const char *p = cmdline;
	char *value_p = value;
	int size = 0;

	if (!cmdline || !name || !value)
		return -1;

	for (; *p != 0; p++) {
		if (*p == ' ')
			continue;
		if (strncmp(p, name, strlen(name)) == 0) {
			p += strlen(name);
			if (*p++ != '=')
				continue;
			while ((*p != 0) && (*p != ' ') && (++size < maxsize))
				*value_p++ = *p++;
			*value_p = '\0';
			return value_p - value;
		}
	}
	return 0;
}

static int nand_get_gpt_flg(void)
{
	char gpt_flg[16] = {'0', 0};

	get_gpt_para_from_cmdline(saved_command_line, "gpt",  gpt_flg, 16);
	nand_dbg_inf("nand gpt commandline =%c\n", gpt_flg[0]);
#if 0
	nand_dbg_inf("force nand_gpt_enable = 0\n");
	nand_gpt_enable = 0;
	return 0;
#endif
	if (gpt_flg[0] == '1') {
		nand_dbg_inf("nand enable GPT\n");
		nand_gpt_enable = 1;
	} else {
		nand_dbg_inf("nand disable GPT\n");
		nand_gpt_enable = 0;
	}
	return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int add_nand(struct nand_blk_ops *tr, struct _nand_phy_partition *phy_partition)
{
	int i, ret;
	__u32 cur_offset = 0;
	struct _nftl_blk *nftl_blk;
	struct _nand_dev *nand_dev;
	struct _nand_disk *disk;
	struct _nand_disk *head_disk;
	uint16 PartitionNO;
	unsigned int partition_total_sector = 0;
	unsigned char patition_cnt = MAX_PART_COUNT_PER_FTL;

	printk("<%s:%d> tr: %lx, dev: %lx\n", __func__, __LINE__, (unsigned long)tr, (unsigned long)phy_partition);

	PartitionNO = get_partitionNO(phy_partition);

	nftl_blk = kmalloc(sizeof(struct _nftl_blk), GFP_KERNEL);
	if (!nftl_blk)
		return -ENOMEM;

	nftl_blk->nand = build_nand_partition(phy_partition);

	if (nftl_initialize(nftl_blk, PartitionNO)) {
		nand_dbg_err("nftl_initialize failed\n");
		return 1;
	}

	nftl_blk->blk_lock = kmalloc(sizeof(struct mutex), GFP_KERNEL);
	if (!nftl_blk->blk_lock)
		return -ENOMEM;

	mutex_init(nftl_blk->blk_lock);

	nftl_blk->nftl_thread =
	    kthread_run(nftl_thread, nftl_blk, "%sd", "nftl");
	if (IS_ERR(nftl_blk->nftl_thread)) {
		nand_dbg_err("init kthread_run fail!\n");
		return 1;
	}

	add_nftl_blk_list(&tr->nftl_blk_head, nftl_blk);

	ret = nand_debug_init(nftl_blk, PartitionNO);
	if (ret) {
		nand_dbg_err("init debug (sysfs) fail!\n");
		return 1;
	}

	disk = get_disk_from_phy_partition(phy_partition);
	for (i = 0; i < MAX_PART_COUNT_PER_FTL; i++) {
		/*nand_dbg_err("disk->name %s\n",(char *)(disk->name));*/
		/*nand_dbg_err("disk->type %x\n",disk[i].type);*/
		/*nand_dbg_err("disk->size %x\n",disk[i].size);*/
		if (disk[i].name[0] == 0xFF && disk[i].name[1] == 0xFF)
			break;
		partition_total_sector += disk[i].size;
	}

	head_disk = get_disk_from_phy_partition(phy_partition);

	nand_get_gpt_flg();

	if (NAND_SUPPORT_GPT) {
		patition_cnt = 1;
		head_disk->size = partition_total_sector;
		partition_total_sector = 0;
	}

	for (i = 0; i < patition_cnt; i++) {
		disk = head_disk + i;
		if (disk->type == 0xffffffff)
			break;

		nand_dev = kmalloc(sizeof(struct _nand_dev), GFP_KERNEL);
		if (!nand_dev)
			return -ENOMEM;

		add_nand_dev_list(&tr->nand_dev_head, nand_dev);

		nand_dev->nbd.nandr = &mytr;

		if (dev_initialize(nand_dev, nftl_blk, cur_offset, disk->size))
			return 1;

		nand_dev->nbd.size = (unsigned int)nand_dev->size;
		nand_dev->nbd.priv = (void *)nand_dev;

		memcpy(nand_dev->nbd.name, disk->name, strlen(disk->name) + 1);
		memcpy(nand_dev->name, disk->name, strlen(disk->name) + 1);
		nand_dbg_inf("nand_dev add %s\n", nand_dev->name);

		if (NAND_SUPPORT_GPT) {
			dev_num = 0;
			nand_dev->nbd.devnum = dev_num;
			if (add_nand_blktrans_dev(&nand_dev->nbd)) {
				nand_dbg_err("nftl add blk disk dev failed\n");
				return 1;
			}
		} else {
			if ((PartitionNO == 0) && (i == 0)) {
				dev_num = -1;
			} else {
				dev_num++;
				nand_dev->nbd.devnum = dev_num;
				if (add_nand_blktrans_dev(&nand_dev->nbd)) {
					nand_dbg_err("nftl add blk disk dev failed\n");
					return 1;
				}
			}
		}
		cur_offset += disk->size;
	}

	if ((nftl_get_boot_cnt(nftl_blk->nftl_zone) % 1000) == 0)
		if (NAND_CheckBoot() != 0)
			nand_dbg_err("nand CheckBoot error\n");
	return 0;
}

int add_nand_for_dragonboard_test(struct nand_blk_ops *tr)
{
	struct _nand_dev *nand_dev;

	nand_dev = kmalloc(sizeof(struct _nand_dev), GFP_KERNEL);
	if (!nand_dev)
		return 1;

	add_nand_dev_list(&tr->nand_dev_head, nand_dev);

	nand_dev->nbd.nandr = &mytr;

	nand_dev->nbd.size = 1024 * 4096;
	nand_dev->nbd.priv = (void *)nand_dev;

	dev_num = 0;
	nand_dev->nbd.devnum = dev_num;
	nand_dbg_inf("before add nand blktrans dev\n");
	if (add_nand_blktrans_dev_for_dragonboard(&nand_dev->nbd)) {
		nand_dbg_err("nftl add blk disk dev failed\n");
		return 1;
	}
	return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int remove_nand(struct nand_blk_ops *tr)
{
	struct _nftl_blk *nftl_blk;
	struct _nand_dev *nand_dev;

	nand_dbg_err("remove_nand\n");

	nftl_blk = &tr->nftl_blk_head;
	nand_dev = &tr->nand_dev_head;

	nand_dev = del_last_nand_dev(&tr->nand_dev_head);
	while (nand_dev != NULL) {
		nand_flush(&nand_dev->nbd);
		del_nand_blktrans_dev(&nand_dev->nbd);
		kfree(nand_dev);
		nand_dev = del_last_nand_dev(&tr->nand_dev_head);
	}

	nftl_blk = del_last_nftl_blk(&tr->nftl_blk_head);
	while (nftl_blk != NULL) {
		if (nftl_blk->nftl_thread != NULL) {
			kthread_stop(nftl_blk->nftl_thread);
			nftl_blk->nftl_thread = NULL;
		}
		kfree(nftl_blk->blk_lock);
		nftl_exit(nftl_blk);
		kfree(nftl_blk);
		nftl_blk = del_last_nftl_blk(&tr->nftl_blk_head);
	}
	return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int nand_flush(struct nand_blk_dev *dev)
{
	int error = 0;
	struct _nand_dev *nand_dev = (struct _nand_dev *)(dev->priv);

	error = nand_dev->flush_write_cache(nand_dev, 0xffff);

	return error;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int dev_initialize(struct _nand_dev *nand_dev, struct _nftl_blk *nftl_blk,
		   __u32 offset, __u32 size)
{
	__u32 offset_t, size_t;

	offset_t = offset;
	size_t = size;

	nand_dev->nftl_blk = nftl_blk;

	if (offset_t < nftl_blk->nftl_logic_size)
		nand_dev->offset = offset_t;
	else
		return 1;

	if ((nand_dev->offset + size_t) <= nftl_blk->nftl_logic_size)
		nand_dev->size = size_t;
	else
		nand_dev->size = nftl_blk->nftl_logic_size - nand_dev->offset;

	nand_dev->read_data = _dev_nand_read;
	nand_dev->write_data = _dev_nand_write;
	nand_dev->flush_write_cache = _dev_flush_write_cache;
	nand_dev->flush_sector_write_cache = _dev_flush_sector_write_cache;
	nand_dev->discard = _dev_nand_discard;

	return 0;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int _dev_nand_read(struct _nand_dev *nand_dev, __u32 start_sector, __u32 len,
		   unsigned char *buf)
{
	int ret;
	struct _nftl_blk *nftl_blk = nand_dev->nftl_blk;

	nand_active_time = jiffies;
	nand_read_active_time = jiffies;

	mutex_lock(nftl_blk->blk_lock);

	if (start_sector + len > nand_dev->size) {
		ret = 0xfffff;
		nand_dbg_err("_dev_nand_read over size 0x%x 0x%x\n",
			     start_sector, nand_dev->size);
		while (--ret)
			;
		ret = -1;
		goto _dev_nand_read_end;

	}

	ret = nand_dev->nftl_blk->read_data(nand_dev->nftl_blk,
		    start_sector + nand_dev->offset, len, buf);

_dev_nand_read_end:

	nand_active_time = jiffies;
	nand_read_active_time = jiffies;

	mutex_unlock(nftl_blk->blk_lock);

	return ret;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int _dev_nand_write(struct _nand_dev *nand_dev, __u32 start_sector, __u32 len,
		    unsigned char *buf)
{
	__u32 ret;
	struct _nftl_blk *nftl_blk = nand_dev->nftl_blk;

	nand_active_time = jiffies;
	nand_write_active_time = jiffies;

	mutex_lock(nftl_blk->blk_lock);

	if (start_sector + len > nand_dev->size) {
		ret = 0xffffff;
		nand_dbg_err("_dev_nand_write over size 0x%x 0x%x\n",
		    start_sector, nand_dev->size);
		while (--ret)
			;
		ret = -1;
		goto _dev_nand_write_end;
	}

	ret = nand_dev->nftl_blk->write_data(nand_dev->nftl_blk,
		    start_sector + nand_dev->offset, len, buf);

_dev_nand_write_end:

	nand_active_time = jiffies;
	nand_write_active_time = jiffies;

	mutex_unlock(nftl_blk->blk_lock);

	return ret;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int _dev_nand_discard(struct _nand_dev *nand_dev, __u32 start_sector, __u32 len)
{
	__u32 ret = 0;
	struct _nftl_blk *nftl_blk = nand_dev->nftl_blk;

	mutex_lock(nftl_blk->blk_lock);

	/*nand_dbg_err("======nand_discard====== %d,%d\n",start_sector,len);*/

	ret =
		nand_dev->nftl_blk->discard(nand_dev->nftl_blk,
					start_sector + nand_dev->offset, len);

	nand_active_time = jiffies;

	mutex_unlock(nftl_blk->blk_lock);

	return ret;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int _dev_flush_write_cache(struct _nand_dev *nand_dev, __u32 num)
{
	__u32 ret;
	struct _nftl_blk *nftl_blk = nand_dev->nftl_blk;

	nand_active_time = jiffies;

	mutex_lock(nftl_blk->blk_lock);

	ret = nand_dev->nftl_blk->flush_write_cache(nand_dev->nftl_blk, num);

	nand_active_time = jiffies;

	mutex_unlock(nftl_blk->blk_lock);

	return ret;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int _dev_flush_sector_write_cache(struct _nand_dev *nand_dev, __u32 num)
{
	__u32 ret;
	struct _nftl_blk *nftl_blk = nand_dev->nftl_blk;

	mutex_lock(nftl_blk->blk_lock);

	ret =
	    nand_dev->nftl_blk->flush_sector_write_cache(nand_dev->nftl_blk,
							 num);

	mutex_unlock(nftl_blk->blk_lock);

	return ret;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
struct _nand_dev *_get_nand_dev_by_name(char *name)
{
	int len;
	struct _nand_dev *p;

	len = strlen(name) + 1;

	if (len > 16)
		len = 16;

	for (p = &mytr.nand_dev_head; p != NULL; p = p->nand_dev_next) {
		if (memcmp(p->name, name, len) == 0)
			return p;
	}
	return NULL;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int _dev_nand_read2(char *name, unsigned int start_sector, unsigned int len,
		    unsigned char *buf)
{
	int ret;
	struct _nand_dev *nand_dev = _get_nand_dev_by_name(name);

	if (nand_dev == NULL)
		return -1;

	if (start_sector + len > nand_dev->size) {
		nand_dbg_err("_dev_nand_read over size 0x%x 0x%x\n",
			     start_sector, nand_dev->size);
		return -1;
	}

	ret =
	    nand_dev->nftl_blk->read_data(nand_dev->nftl_blk,
					  start_sector + nand_dev->offset, len,
					  buf);
	nand_active_time = jiffies;

	return ret;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
int _dev_nand_write2(char *name, unsigned int start_sector, unsigned int len,
		     unsigned char *buf)
{
	int ret;
	struct _nand_dev *nand_dev = _get_nand_dev_by_name(name);

	if (nand_dev == NULL)
		return -1;

	if (start_sector + len > nand_dev->size) {
		nand_dbg_err("_dev_nand_write over size 0x%x 0x%x\n",
		    start_sector, nand_dev->size);
		return -1;
	}

	ret = nand_dev->nftl_blk->write_data(nand_dev->nftl_blk,
		    start_sector + nand_dev->offset, len, buf);
	nand_active_time = jiffies;

	return ret;
}

/*****************************************************************************
*Name         :
*Description  :
*Parameter    :
*Return       :
*Note         :
*****************************************************************************/
uint32 nand_read_reclaim(struct _nftl_blk *start_blk, unsigned char *buf)
{
	uint32 ret = 0;
	struct _nftl_blk *nftl_blk;

	nftl_blk = get_nftl_need_read_claim(start_blk);
	if (nftl_blk == NULL)
		return 1;

	mutex_lock(nftl_blk->blk_lock);

	ret = read_reclaim(start_blk, nftl_blk, buf);

	mutex_unlock(nftl_blk->blk_lock);

	return ret;
}
