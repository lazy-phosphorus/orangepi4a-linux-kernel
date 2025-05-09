/* SPDX-License-Identifier: GPL-2.0 */
/*
 ************************************************************************************************************************
 *                                                      eNand
 *                                           Nand flash driver scan module
 *
 *                             Copyright(C), 2008-2009, SoftWinners Microelectronic Co., Ltd.
 *                                                  All Rights Reserved
 *
 * File Name : ndfc_base.h
 *
 * Author :
 *
 * Version : v0.1
 *
 * Date : 2013-11-20
 *
 * Description :
 *
 * Others : None at present.
 *
 *
 *
 ************************************************************************************************************************
 */

#ifndef _NAND_CONTROLLER_H
#define _NAND_CONTROLLER_H

#include "../rawnand_cfg.h"
#include "../rawnand.h"

#define MAX_CHANNEL (nand_get_max_channel_cnt())
#define MAX_CHIP_PER_CHANNEL 4
#define MAX_CMD_PER_LIST 16
#define MAX_UDATA_LEN_FOR_ECCBLOCK 32
#define UDATA_LEN_FOR_4BYTESPER1K 4


enum _cmd_type {
	CMD_TYPE_NORMAL = 0,
	CMD_TYPE_BATCH,
	CMD_TYPE_MAX_CNT,
};

enum _ecc_layout_type {
	ECC_LAYOUT_INTERLEAVE = 0,
	ECC_LAYOUT_SEQUENCE,
	ECC_LAYOUT_TYPE_MAX_CNT,
};

enum _dma_mode {
	DMA_MODE_GENERAL_DMA = 0,
	DMA_MODE_MBUS_DMA,
	DMA_MODE_MAX_CNT
};

enum _ndfc_version {
	NDFC_VERSION_V1 = 1,
	NDFC_VERSION_V2,
	NDFC_VERSION_MAX_CNT
};

#define NDFC_DESC_FIRST_FLAG (0x1 << 3)
#define NDFC_DESC_LAST_FLAG (0x1 << 2)
#define MAX_NFC_DMA_DESC_NUM 32
#define NDFC_DESC_BSIZE(bsize) ((bsize)&0xFFFF) //??? 64KB
struct _ndfc_dma_desc_t {
	__u32 cfg;
	__u32 bcnt;
	__u32 buff;		       /*buffer address*/
	struct _ndfc_dma_desc_t *next; /*pointer to next descriptor*/
};
struct _nctri_cmd {
	u32 cmd_valid; //0: skip current cmd io; 1: valid cmd io

	u32 cmd;	 //cmd value
	u32 cmd_send;    //0: don't send cmd; 1: send cmd. only valid in normal command type.
	u32 cmd_wait_rb; //0: ndfc don't wait rb ready; 1: ndfc wait br ready;

	u8 cmd_addr[MAX_CMD_PER_LIST]; //the buffer of address
	u32 cmd_acnt;		       //the number of address

	u32 cmd_trans_data_nand_bus; //0: don't transfer data on external nand bus;  1: transfer data;
	u32 cmd_swap_data;	   //0: don't swap data with host memory; 1: swap data with host memory;
	u32 cmd_swap_data_dma;       //0: transfer data using cpu; 1: transfer data useing dma
	u32 cmd_direction;	   //0: read data;  1: write data
	u32 cmd_mdata_len;	   //the byte length of main data
	u32 cmd_data_block_mask;     //data block mask of main data
	//    u32   cmd_mdata_addr;      //the address of main data
	u8 *cmd_mdata_addr; //the address of main data
};

struct _nctri_cmd_seq {
	u32 cmd_type;   //0: normal command; 1: page command;
	u32 ecc_layout; //0: interleave mode; 1: sequence mode;
	//u32   row_addr_auto_inc; 	 //0:    ; 1: row address auto increase;
	struct _nctri_cmd nctri_cmd[MAX_CMD_PER_LIST];
	u32 re_start_cmd;			    //index of first repeated cmd [0,7] [0xff]
	u32 re_end_cmd;				    //index of last repeated cmd  [0,7] [0xff]
	u32 re_cmd_times;			    //repeat times
	u8 udata_len_mode[32];			    //user data lenth for every ecc block,udata_len is 4 bytes aligned
	struct _ndfc_dma_desc_t *ndfc_dma_desc;     //physic addr
	struct _ndfc_dma_desc_t *ndfc_dma_desc_cpu; //virtual addr
};

struct _nand_controller_reg {
	volatile u32 *reg_ctl;			/*0x0000 NDFC Control Register*/
	volatile u32 *reg_sta;			/*0x0004 NDFC Status Register*/
	volatile u32 *reg_int;			/*0x0008 NDFC Interrupt and DMA Enable Register*/
	volatile u32 *reg_timing_ctl;		/*0x000C NDFC Timing Control Register*/
	volatile u32 *reg_timing_cfg;		/*0x0010 NDFC Timing Configure Register*/
	volatile u32 *reg_addr_low;		/*0x0014 NDFC Address Low Word Register*/
	volatile u32 *reg_addr_high;		/*0x0018 NDFC Address High Word Register*/
	volatile u32 *reg_data_block_mask;      /*0x001C NDFC Data Block Mask Register*/
	volatile u32 *reg_ndfc_cnt;		/*0x0020 NDFC Data Block Mask Register*/
	volatile u32 *reg_cmd;			/*0x0024 NDFC Command IO Register*/
	volatile u32 *reg_read_cmd_set;		/*0x0028 NDFC Command Set Register 0*/
	volatile u32 *reg_write_cmd_set;	/*0x002C NDFC Command Set Register 1*/
	volatile u32 *reg_ecc_ctl;		/*0x0034 NDFC ECC Control Register*/
	volatile u32 *reg_ecc_sta;		/*0x0038 NDFC ECC Status Register*/
	volatile u32 *reg_data_pattern_sta;     /*0x003C NDFC Data Pattern Status Register*/
	volatile u32 *reg_efr;			/*0x0040 NDFC Enhanced Featur Register*/
	volatile u32 *reg_rdata_sta_ctl;	/*0x0044 NDFC Read Data Status Control Register*/
	volatile u32 *reg_rdata_sta_0;		/*0x0048 NDFC Read Data Status Register 0*/
	volatile u32 *reg_rdata_sta_1;		/*0x004C NDFC Read Data Status Register 1*/
	volatile u32 *reg_err_cnt0;		/*0x0050 NDFC Error Counter Register 0*/
	volatile u32 *reg_err_cnt1;		/*0x0054 NDFC Error Counter Register 1*/
	volatile u32 *reg_err_cnt2;		/*0x0058 NDFC Error Counter Register 2*/
	volatile u32 *reg_err_cnt3;		/*0x005C NDFC Error Counter Register 3*/
	volatile u32 *reg_err_cnt4;		/*0x0060 NDFC Error Counter Register 4*/
	volatile u32 *reg_err_cnt5;		/*0x0064 NDFC Error Counter Register 5*/
	volatile u32 *reg_err_cnt6;		/*0x0068 NDFC Error Counter Register 6*/
	volatile u32 *reg_err_cnt7;		/*0x006C NDFC Error Counter Register 7*/
	volatile u32 *reg_user_data_len_base;   /*0x0070 NDFC User Data Length Register X*/
	volatile u32 *reg_user_data_base;       /*0x0080 NDFC User Data Register X*/
	volatile u32 *reg_flash_sta;		/*0x0100 NDFC Flash Status Register*/
	volatile u32 *reg_cmd_repeat_cnt;       /*0x0104 NDFC Command Repeat Counter Register*/
	volatile u32 *reg_cmd_repeat_interval;  /*0x0108 NDFC Command Repeat Interval Register*/
	volatile u32 *reg_efnand_sta;		/*0x0110 NDFC EFNAND STATUS Register*/
	volatile u32 *reg_spare_area;		/*0x0114 NDFC Spare Aera Register*/
	volatile u32 *reg_pat_id;		/*0x0118 NDFC Pattern ID Register*/
	volatile u32 *reg_ddr2_spec_ctl;	/*0x011C NDFC DDR2 Specific Control Register*/
	volatile u32 *reg_ndma_mode_ctl;	/*0x0120 NDFC Normal DMA Mode Control Register*/
	volatile u32 *reg_valid_data_dma_cnt;   /*0x012C NDFC Valid Data DMA Counter Register*/
	volatile u32 *reg_data_dma_base;	/*0x0130 NDFC Data DMA Address X Register */
	volatile u32 *reg_data_dma_size_2x;     /*0x0170 NDFC Data DMA Size 2X and Data DMA Size 2X+1 Register*/
	volatile u32 *reg_random_seed_x;	/*0x0190 NDFC Random Seed X Register*/
	volatile u32 *reg_mbus_dma_dlba;	/*0x0200 NDFC MBUS DMA Descriptor List Base Address Register in no.1 version*/
	volatile u32 *reg_mbus_dma_sta;		/*0x0204 NDFC MBUS DMA Interrupt Status Register in no.1 version*/
	volatile u32 *reg_mdma_int_mask;	/*0x0208 NDFC MBUS DMA Interrupt Enable Register in no.1 version*/
	volatile u32 *reg_mdma_cur_desc_addr;   /*0x020C NDFC MBUS DMA Current Descriptor Address Register in no.1 version*/
	volatile u32 *reg_mdma_cur_buf_addr;    /*0x0210 NDFC MBUS DMA Current Buffer Address Register in no.1 version*/
	volatile u32 *reg_dma_cnt;		/*0x0214 NDFC Normal DMA Byte Counter Register*/
	volatile u32 *reg_emce_ctl;		/*0x0218 NDFC EMCE Control Register*/
	volatile u32 *reg_emce_iv_fac_cmp_val;  /*0x021C NDFC EMCE IV_FAC Compare Value Register*/
	volatile u32 *reg_emce_iv_cal_fac_base; /*0x0220 NDFC EMCE IV Calculate Factor Register X*/
	volatile u32 *reg_io_data;		/*0x02A0 NDFC IO Data Register,in no.1 version NDFC IO Data Registe is 0x0300 */
	volatile u32 *reg_ndfc_ver;		/*0x02F0 NDFC Version Number Register*/
	volatile u32 *reg_ldpc_ctl;		/*0x02FC NDFC LDPC Control Register*/
	volatile u32 *reg_enc_ldpc_mode_set;    /*0x0300 NDFC Encode LDPC Mode Setting Register*/
	volatile u32 *reg_cor_ldpc_mode_set;    /*0x0304 NDFC Correct LDPC Mode Setting Register*/
	volatile u32 *reg_c0_llr_tbl_addr;      /*0x0308 NDFC C0 LLR Table 11111-11100 Register*/
	volatile u32 *reg_c1_llr_tbl_addr;      /*0x0328 NDFC C1 LLR Table 11111-11100 Register*/
	volatile u32 *reg_ram0_base;		/*0x0400 NDFC Control Register*/
	volatile u32 *reg_ram1_base;		/*0x0800 NDFC Control Register*/
	volatile u32 *reg_glb_cfg;		/*0x0C00 NDFC Global Configure Register*/
	volatile u32 *reg_cmd_descr_base_addr;  /*0x0C04 NDFC Command Descriptor Base Address Register*/
	volatile u32 *reg_cmd_descr_sta;	/*0x0C08 NDFC Command Descriptor Status Register*/
	volatile u32 *reg_cmd_descr_intr;       /*0x0C0C NDFC Command Descriptor Interrupt Control Register*/
	volatile u32 *reg_csic_bist_ctl_reg;    /*0x0C10 NDFC BIST Control Register*/
	volatile u32 *reg_bist_start_addr;      /*0x0C14 NDFC BIST Start Address Register */
	volatile u32 *reg_bist_end_addr;	/*0x0C18 NDFC BIST End Address Register*/
	volatile u32 *reg_bist_data_mask;       /*0x0C1C NDFC BIST Data Mask Register*/
	volatile u32 *reg_dma_sta;
};

struct _nand_controller_reg_bak {
	u32 reg_ctl;		      /*0x0000 NDFC Control Register*/
	u32 reg_sta;		      /*0x0004 NDFC Status Register*/
	u32 reg_int;		      /*0x0008 NDFC Interrupt and DMA Enable Register*/
	u32 reg_timing_ctl;	   /*0x000C NDFC Timing Control Register*/
	u32 reg_timing_cfg;	   /*0x0010 NDFC Timing Configure Register*/
	u32 reg_addr_low;	     /*0x0014 NDFC Address Low Word Register*/
	u32 reg_addr_high;	    /*0x0018 NDFC Address High Word Register*/
	u32 reg_data_block_mask;      /*0x001C NDFC Data Block Mask Register*/
	u32 reg_ndfc_cnt;	     /*0x0020 NDFC Data Block Mask Register*/
	u32 reg_cmd;		      /*0x0024 NDFC Command IO Register*/
	u32 reg_read_cmd_set;	 /*0x0028 NDFC Command Set Register 0*/
	u32 reg_write_cmd_set;	/*0x002C NDFC Command Set Register 1*/
	u32 reg_ecc_ctl;	      /*0x0034 NDFC ECC Control Register*/
	u32 reg_ecc_sta;	      /*0x0038 NDFC ECC Status Register*/
	u32 reg_data_pattern_sta;     /*0x003C NDFC Data Pattern Status Register*/
	u32 reg_efr;		      /*0x0040 NDFC Enhanced Featur Register*/
	u32 reg_rdata_sta_ctl;	/*0x0044 NDFC Read Data Status Control Register*/
	u32 reg_rdata_sta_0;	  /*0x0048 NDFC Read Data Status Register 0*/
	u32 reg_rdata_sta_1;	  /*0x004C NDFC Read Data Status Register 1*/
	u32 reg_err_cnt0;	     /*0x0050 NDFC Error Counter Register 0*/
	u32 reg_err_cnt1;	     /*0x0054 NDFC Error Counter Register 0*/
	u32 reg_err_cnt2;	     /*0x0058 NDFC Error Counter Register 0*/
	u32 reg_err_cnt3;	     /*0x005C NDFC Error Counter Register 0*/
	u32 reg_err_cnt4;	     /*0x0060 NDFC Error Counter Register 0*/
	u32 reg_err_cnt5;	     /*0x0064 NDFC Error Counter Register 0*/
	u32 reg_err_cnt6;	     /*0x0068 NDFC Error Counter Register 0*/
	u32 reg_err_cnt7;	     /*0x006C NDFC Error Counter Register 0*/
	u32 reg_user_data_len_base;   /*0x0070 NDFC User Data Length Register X*/
	u32 reg_user_data_base;       /*0x0080 NDFC User Data Register X*/
	u32 reg_flash_sta;	    /*0x0100 NDFC Flash Status Register*/
	u32 reg_cmd_repeat_cnt;       /*0x0104 NDFC Command Repeat Counter Register*/
	u32 reg_cmd_repeat_interval;  /*0x0108 NDFC Command Repeat Interval Register*/
	u32 reg_efnand_sta;	   /*0x0110 NDFC EFNAND STATUS Register*/
	u32 reg_spare_area;	   /*0x0114 NDFC Spare Aera Register*/
	u32 reg_pat_id;		      /*0x0118 NDFC Pattern ID Register*/
	u32 reg_ddr2_spec_ctl;	/*0x011C NDFC DDR2 Specific Control Register*/
	u32 reg_ndma_mode_ctl;	/*0x0120 NDFC Normal DMA Mode Control Register*/
	u32 reg_valid_data_dma_cnt;   /*0x012C NDFC Valid Data DMA Counter Register*/
	u32 reg_data_dma_base;	/*0x0130 NDFC Data DMA Address X Register */
	u32 reg_data_dma_size_2x;     /*0x0170 NDFC Data DMA Size 2X and Data DMA Size 2X+1 Register*/
	u32 reg_random_seed_x;	/*0x0190 NDFC Random Seed X Register*/
	u32 reg_mbus_dma_dlba;	/*0x0200 NDFC MBUS DMA Descriptor List Base Address Register in no.1 version*/
	u32 reg_mbus_dma_sta;	 /*0x0204 NDFC MBUS DMA Interrupt Status Register in no.1 version*/
	u32 reg_mdma_int_mask;	/*0x0208 NDFC MBUS DMA Interrupt Enable Register in no.1 version*/
	u32 reg_mdma_cur_desc_addr;   /*0x020C NDFC MBUS DMA Current Descriptor Address Register in no.1 version*/
	u32 reg_mdma_cur_buf_addr;    /*0x0210 NDFC MBUS DMA Current Buffer Address Register in no.1 version*/
	u32 reg_dma_cnt;	      /*0x0214 NDFC Normal DMA Byte Counter Register*/
	u32 reg_emce_ctl;	     /*0x0218 NDFC EMCE Control Register*/
	u32 reg_emce_iv_fac_cmp_val;  /*0x021C NDFC EMCE IV_FAC Compare Value Register*/
	u32 reg_emce_iv_cal_fac_base; /*0x0220 NDFC EMCE IV Calculate Factor Register X*/
	u32 reg_io_data;	      /*0x02A0 NDFC IO Data Register,in no.1 version NDFC IO Data Registe is 0x0300*/
	u32 reg_ndfc_ver;	     /*0x02F0 NDFC Version Number Register*/
	u32 reg_ldpc_ctl;	     /*0x02FC NDFC LDPC Control Register*/
	u32 reg_enc_ldpc_mode_set;    /*0x0300 NDFC Encode LDPC Mode Setting Register*/
	u32 reg_cor_ldpc_mode_set;    /*0x0304 NDFC Correct LDPC Mode Setting Register*/
	u32 reg_c0_llr_tbl_addr;      /*0x0308 NDFC C0 LLR Table 11111-11100 Register*/
	u32 reg_c1_llr_tbl_addr;      /*0x0328 NDFC C1 LLR Table 11111-11100 Register*/
	u32 reg_ram0_base;	    /*0x0400 NDFC Control Register*/
	u32 reg_ram1_base;	    /*0x0800 NDFC Control Register*/
	u32 reg_glb_cfg;	      /*0x0C00 NDFC Global Configure Register*/
	u32 reg_cmd_descr_base_addr;  /*0x0C04 NDFC Command Descriptor Base Address Register*/
	u32 reg_cmd_descr_sta;	/*0x0C08 NDFC Command Descriptor Status Register*/
	u32 reg_cmd_descr_intr;       /*0x0C0C NDFC Command Descriptor Interrupt Control Register*/
	u32 reg_csic_bist_ctl_reg;    /*0x0C10 NDFC BIST Control Register*/
	u32 reg_bist_start_addr;      /*0x0C14 NDFC BIST Start Address Register */
	u32 reg_bist_end_addr;	/*0x0C18 NDFC BIST End Address Register*/
	u32 reg_bist_data_mask;       /*0x0C1C NDFC BIST Data Mask Register*/
	u32 reg_dma_sta;
};
#if 1
//-->

/**
 * enum nand_op_instr_type - Definition of all instruction types
 * @NAND_OP_CMD_INSTR: command instruction
 * @NAND_OP_ADDR_INSTR: address instruction
 * @NAND_OP_DATA_IN_INSTR: data in instruction
 * @NAND_OP_DATA_OUT_INSTR: data out instruction
 * @NAND_OP_WAITRDY_INSTR: wait ready instruction
 */
enum nand_op_instr_type {
	NAND_OP_CMD_INSTR,
	NAND_OP_ADDR_INSTR,
	NAND_OP_DATA_IN_INSTR,
	NAND_OP_DATA_OUT_INSTR,
	NAND_OP_WAITRDY_INSTR,
};

/**
 * struct nand_op_cmd_instr - Definition of a command instruction
 * @opcode: the command to issue in one cycle
 */
struct nand_op_cmd_instr {
	u8 opcode;
};

/**
 * struct nand_op_addr_instr - Definition of an address instruction
 * @naddrs: length of the @addrs array
 * @addrs: array containing the address cycles to issue
 */
struct nand_op_addr_instr {
	unsigned int naddrs;
	const u8 *addrs;
};

/**
 * enum nand_op_instr_type - Definition of data layout typei
 * (user data & bch check word position)
 * @ECC_LAYOUT_INTERLEAVE: on page spare arare
 * @ECC_LAYOUT_SEQUENCE: following data block
 * @ECC_LAYOUT_NULL: if to get flash id or get/set flash feature, use it
 */
enum nand_ecc_layout_type {
	NAND_ECC_LAYOUT_INTERLEAVE = 0,
	NAND_ECC_LAYOUT_SEQUENCE,
	NAND_ECC_LAYOUT_NULL,
};
/**
 * struct data_layout Defintion of an data layout
 * @type: data layout type
 */
struct data_layout {
	enum nand_ecc_layout_type type;
};

/**
 * struct main_data - Definition of main data
 * @len: number of data bytes to move
 * @buf: buffer to fill
 * @buf.in: buffer to fill when reading from the NAND chip
 * @buf.out: buffer to read from when writing to the NAND chip
 *
 * Please note that "in" and "out" are inverted from the ONFI specification
 * and are from the controller perspective, so a "in" is a read from the NAND
 * chip while a "out" is a write to the NAND chip.
 */
struct main_data {
	unsigned int len;
	union {
		void *in;
		const void *out;
	} buf;
};

/**
 * struct user_data - Definition of main data
 * @len: number of data bytes to move
 * @buf: buffer to fill
 * @buf.in: buffer to fill when reading from the NAND chip
 * @buf.out: buffer to read from when writing to the NAND chip
 *
 * Please note that "in" and "out" are inverted from the ONFI specification
 * and are from the controller perspective, so a "in" is a read from the NAND
 * chip while a "out" is a write to the NAND chip.
 */
struct user_data {
	unsigned int len;
	union {
		void *in;
		const void *out;
	} buf;
};
/**
 * struct other- Definition of main data
 * @len: number of data bytes to move
 * @buf: buffer to fill
 * @buf.in: buffer to fill when reading from the NAND chip
 * @buf.out: buffer to read from when writing to the NAND chip
 */
struct other {
	unsigned int len;
	union {
		void *in;
		const void *out;
	} buf;
};

/**
 * struct nand_op_data_instr - Definition of a data instruction
 * @layout: data layout
 * @main: main data
 * @user: user data
 */

struct nand_op_data_instr {
	struct data_layout layout;
	struct main_data main;
	struct user_data user;
	struct other other;
};

/**
 * struct nand_op_waitrdy_instr - Definition of a wait ready instruction
 * @timeout_ms: maximum delay while waiting for the ready/busy pin in ms
 */
struct nand_op_waitrdy_instr {
	unsigned int timeout_ms;
};

/**
 * struct nand_op_instr - Instruction object
 * @type: the instruction type
 * @ctx:  extra data associated to the instruction. You'll have to use the
 *        appropriate element depending on @type
 * @ctx.cmd: use it if @type is %NAND_OP_CMD_INSTR
 * @ctx.addr: use it if @type is %NAND_OP_ADDR_INSTR
 * @ctx.data: use it if @type is %NAND_OP_DATA_IN_INSTR
 *	      or %NAND_OP_DATA_OUT_INSTR
 * @ctx.waitrdy: use it if @type is %NAND_OP_WAITRDY_INSTR
 * @delay_ns: delay the controller should apply after the instruction has been
 *	      issued on the bus. Most modern controllers have internal timings
 *	      control logic, and in this case, the controller driver can ignore
 *	      this field.
 */
struct nand_op_instr {
	enum nand_op_instr_type type;
	union {
		struct nand_op_cmd_instr cmd;
		struct nand_op_addr_instr addr;
		struct nand_op_data_instr data;
		struct nand_op_waitrdy_instr waitrdy;
	} ctx;
	unsigned int delay_ns;
};

enum nand_operation_type {
	OP_CMD_NORMAL = 0,
	OP_CMD_BATCH,
};
/**
 * struct nand_subop - a sub operation
 * @type: the type to select controller noraml send instrs or batch send instrs
 * @instrs: array of instructions to execute
 * @ninstrs: length of the @instrs array
 */
struct nand_opset {
	enum nand_operation_type type;
	const struct nand_op_instr *instrs;
	unsigned int ninstrs;
};

/**
 * struct nand_operation - NAND operation descriptor
 * @cs: the CS line to select for this NAND operation
 * @type: the type to select controller noraml to handle subop or batch handle  subop
 * @subop: array of subop to execute
 * @nsubops: length of the @subop array
 */
struct nand_operation {
	unsigned int cs;
	enum nand_operation_type type;
	const struct nand_opset *opset;
	unsigned int nopsets;
};
/**
 * AW_ARRAY_SIZE - get the number of elements in array @arr
 * @arr: array to be sized
 */
#define AW_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define NAND_OPSET(_type, _instrs)                 \
	{                                          \
		.type = _type,                     \
		.instrs = _instrs,                 \
		.ninstrs = AW_ARRAY_SIZE(_instrs), \
	}

#define NAND_OPERATION(_cs, _type, _instrs)        \
	{                                          \
		.cs = _cs,                         \
		.type = _type,                     \
		.instrs = _instrs,                 \
		.ninstrs = AW_ARRAY_SIZE(_instrs), \
	}

static inline void aw_writel(unsigned int n, unsigned volatile int *reg)
{
	*((unsigned volatile int *)reg) = n;
}

static inline void aw_readl(unsigned int *n, unsigned volatile int *reg)
{
	*n = *((unsigned volatile int *)reg);
}

#define NAND_OP_CMD(id, ns)                \
	{                                  \
		.type = NAND_OP_CMD_INSTR, \
		.ctx.cmd.opcode = id,      \
		.delay_ns = ns,            \
	}

#define NAND_OP_ADDR(ncycles, cycles, ns)   \
	{                                   \
		.type = NAND_OP_ADDR_INSTR, \
		.ctx.addr = {               \
		    .naddrs = ncycles,      \
		    .addrs = cycles,        \
		},                          \
		.delay_ns = ns,             \
	}

#define NAND_OP_DATA_IN(lt, ml, mb, ul, ub)    \
	{                                      \
		.type = NAND_OP_DATA_IN_INSTR, \
		.ctx.data = {                  \
			.layout.type = lt,     \
			.main.len = ml,        \
			.main.buf.in = mb,     \
			.user.len = ul,        \
			.user.buf.in = ub,     \
		}                              \
	}

#define NAND_OP_DATA_OUT(lt, ml, mb, ul, ub)      \
	{                                         \
		.type = NAND_OP_DATA_OUT_INSTR,   \
		.ctx.data = {                     \
			.layout.type = lt,        \
			.main.len = ml,           \
			.main.buf.out = mb,       \
			.user.len = ul,           \
			.user.buf.out = ub,       \
		}                                 \
	}
#define NAND_OP_WAIT_RDY(tout_ms, ns)              \
	{                                          \
		.type = NAND_OP_WAITRDY_INSTR,     \
		.ctx.waitrdy.timeout_ms = tout_ms, \
		.delay_ns = ns,                    \
	}

/**
 * struct nand_controller_ops - Controller operations
 *
 * @attach_chip: this method is called after the NAND detection phase after
 *		 flash ID and MTD fields such as erase size, page size and OOB
 *		 size have been set up. ECC requirements are available if
 *		 provided by the NAND chip or device tree. Typically used to
 *		 choose the appropriate ECC configuration and allocate
 *		 associated resources.
 *		 This hook is optional.
 * @detach_chip: free all resources allocated/claimed in
 *		 nand_controller_ops->attach_chip().
 *		 This hook is optional.
 * @exec_op:	 controller specific method to execute NAND operations.
 *		 This method replaces chip->legacy.cmdfunc(),
 *		 chip->legacy.{read,write}_{buf,byte,word}(),
 *		 chip->legacy.dev_ready() and chip->legacy.waifunc().
 * @setup_data_interface: setup the data interface and timing. If
 *			  chipnr is set to %NAND_DATA_IFACE_CHECK_ONLY this
 *			  means the configuration should not be applied but
 *			  only checked.
 *			  This hook is optional.
 */
#if 0
struct nand_controller_ops {
	int (*attach_chip)(struct nand_chip_info *chip);
	void (*detach_chip)(struct nand_chip_info *chip);
	int (*exec_op)(struct nand_chip_info *chip,
		       const struct nand_operation *op,
		       bool check_only);
	int (*setup_data_interface)(struct nand_chip_info *chip);
};
#endif

//lyout add<--/
#endif

/**
 * struct nand_controller_ops - Controller operations
 *
 * @attach_chip: this method is called after the NAND detection phase after
 *		 flash ID and MTD fields such as erase size, page size and OOB
 *		 size have been set up. ECC requirements are available if
 *		 provided by the NAND chip or device tree. Typically used to
 *		 choose the appropriate ECC configuration and allocate
 *		 associated resources.
 *		 This hook is optional.
 * @detach_chip: free all resources allocated/claimed in
 *		 nand_controller_ops->attach_chip().
 *		 This hook is optional.
 * @exec_op:	 controller specific method to execute NAND operations.
 *		 This method replaces chip->legacy.cmdfunc(),
 *		 chip->legacy.{read,write}_{buf,byte,word}(),
 *		 chip->legacy.dev_ready() and chip->legacy.waifunc().
 * @setup_data_interface: setup the data interface and timing. If
 *			  chipnr is set to %NAND_DATA_IFACE_CHECK_ONLY this
 *			  means the configuration should not be applied but
 *			  only checked.
 *			  This hook is optional.
 */
struct nand_controller_ops {
	int (*attach_chip)(struct nand_chip_info *chip);
	void (*detach_chip)(struct nand_chip_info *chip);
	int (*exec_op)(struct nand_chip_info *chip,
		       const struct nand_operation *op,
		       int check_only);
	int (*setup_data_interface)(struct nand_chip_info *chip);
};

//define the nand flash storage system information
struct nand_controller_info {
#define MAX_ECC_BLK_CNT 32

	struct nand_controller_info *next;
	u32 type;       //ndfc type
	u32 channel_id; //0: channel 0; 1: channel 1;
	u32 chip_cnt;
	u32 chip_connect_info;
	u32 rb_connect_info;
	u32 max_ecc_level;
	u32 channel_sel; //0:BCH; 1:LDPC

	struct _nctri_cmd_seq nctri_cmd_seq;

	u32 ce[8];
	u32 rb[8];
	u32 dma_type; //0: general dma; 1: mbus dma;
	u64 dma_addr;

	u32 current_op_type;		 //1: write operation; 0: others
	u32 write_wait_rb_before_cmd_io; //1: before send cmd io; 0: after send cmd io;
	u32 write_wait_rb_mode;		 //0: query mode; 1: interrupt mode
	u32 write_wait_dma_mode;
	u32 rb_ready_flag;
	u32 dma_ready_flag;
	u32 dma_channel;
	u32 nctri_flag;
	u32 ddr_timing_ctl[MAX_CHIP_PER_CHANNEL];
	u32 ddr_scan_blk_no[MAX_CHIP_PER_CHANNEL];

	u32 random_cmd2_send_flag; //special nand cmd for some nand in batch cmd
	u32 random_cmd2;	   //special nand cmd for some nand in batch cmd
	u32 random_addr_num;       //random col addr num in batch cmd
#define SMALL_CAPACITY_NAND (1 << 16)
#define RANDOM_VALID_BITS (0xFFFF)
	int random_factor; /*use in write boot0*/

	void *nreg_base;
	struct _nand_controller_reg nreg;
	struct _nand_controller_reg_bak nreg_bak;

	struct nand_chip_info *nci;
	struct _ndfc_dma_desc_t *ndfc_dma_desc;     //physic addr
	struct _ndfc_dma_desc_t *ndfc_dma_desc_cpu; //virtual addr
	struct nand_controller_ops ops;
};

#define NDFC_READ_REG(n) (*((volatile u32 *)(n)))
#define NDFC_WRITE_REG(n, c) (*((volatile u32 *)(n)) = (c))

/*0x0000 : define bit use in NFC_CTL*/
#define NDFC_RB_SEL (0x3 << 3)  //1 --> 0x3
#define NDFC_SEL_RB0 (0U << 3)
#define NDFC_SEL_RB1 (1U << 3)
#define NDFC_SEL_RB2 (2U << 3)
#define NDFC_SEL_RB3 (3U << 3)
#define NDFC_CE_SEL (0xf << 24) //7 --> 0xf
#define NDFC_SEL_CE_0 (0U << 24)
#define NDFC_SEL_CE_1 (1U << 24)
#define NDFC_SEL_CE_2 (2U << 24)
#define NDFC_SEL_CE_3 (3U << 24)
#define NDFC_SEL_CE_4 (4U << 24)
#define NDFC_SEL_CE_5 (5U << 24)
#define NDFC_SEL_CE_6 (6U << 24)
#define NDFC_SEL_CE_7 (7U << 24)
#define NDFC_SEL_CE_8 (8U << 24)
#define NDFC_SEL_CE_9 (9U << 24)
#define NDFC_SEL_CE_10 (10U << 24)
#define NDFC_SEL_CE_11 (11U << 24)
#define NDFC_SEL_CE_12 (12U << 24)
#define NDFC_SEL_CE_13 (13U << 24)
#define NDFC_SEL_CE_14 (14U << 24)
#define NDFC_SEL_CE_15 (15U << 24)
#define NDFC_CE_CTL (1U << 6)
#define NDFC_CE_CTL1 (1U << 7)
#define NDFC_PAGE_SIZE (0xf << 8)
#define NDFC_DMA_METHOD (1U << 14)
#define NDFC_DEDICATED_DMA (0U << 15)
#define NDFC_NORMAL_DMA (1U << 15)
#define NDFC_NF_TYPE (0x3 << 18)
#define NDFC_ONFI_DDR_TYPE (2 << 18)
#define NDFC_TOG_DDR_TYPE (3 << 18)
#define NDFC_DDR_TYPE (1 << 19)
#define NDFC_DDR_REPEAT_ENABLE (1 << 20)
#define NDFC_DDR_REPEAT_MODE (1 << 21)
#define NDFC_DDR2_TYPE (1 << 28)
/*0x0004 : define bit use in NFC_ST*/
#define NDFC_RB_B2R (1 << 0)
#define NDFC_CMD_INT_FLAG (1 << 1)
#define NDFC_DMA_INT_FLAG (1 << 2)
#define NDFC_CMD_FIFO_STATUS (1 << 3)
#define NDFC_STA (1 << 4)
#define NDFC_RB_STATE0 (1 << 8) /*define NDFC_NATCH_INT_FLAG	(1 << 5)*/
#define NDFC_RB_STATE1 (1 << 9)
#define NDFC_RB_STATE2 (1 << 10)
#define NDFC_RB_STATE3 (1 << 11)
#define NDFC_RDATA_STA_1 (1 << 12)
#define NDFC_RDATA_STA_0 (1 << 13)

/*0x0008 : define bit use in NFC_INT*/
#define NDFC_B2R_INT_ENABLE (1 << 0)
#define NDFC_CMD_INT_ENABLE (1 << 1)
#define NDFC_DMA_INT_ENABLE (1 << 2)
#define NDFC_CMD_REPT_INTR_EN (1 << 5)
/*0x000c : define bit use in NDFC_TIMING_CTL*/
#define NDFC_READ_PIPE (0xf << 8)
#define NDFC_DC_CTL (0x3f << 0)

/*0x0024 : define bit use in NFC_CMD*/
#define NDFC_CMD_LOW_BYTE (0xff << 0)
#define NDFC_ADR_NUM_IN_PAGECMD (0x3 << 8)
#define NDFC_ADR_NUM (0x7 << 16)
#define NDFC_SEND_ADR (1 << 19)
#define NDFC_ACCESS_DIR (1 << 20)
#define NDFC_DATA_TRANS (1 << 21)
#define NDFC_SEND_CMD1 (1 << 22)
#define NDFC_WAIT_FLAG (1 << 23)
#define NDFC_SEND_CMD2 (1 << 24)
#define NDFC_SEQ (1 << 25)
#define NDFC_DATA_SWAP_METHOD (1 << 26)
#define NDFC_SEND_RAN_CMD2 (1 << 27)
#define NDFC_SEND_CMD3 (1 << 28)
#define NDFC_SEND_CMD4 (1 << 29)
#define NDFC_CMD_TYPE (3 << 30)
#define NDFC_CMD_NORMAL (0 << 30)
#define NDFC_CMD_SPECAIL (1 << 30)
#define NDFC_CMD_BATCH (2 << 30)
/*0x0028 : define bit use in NFC_RCMD_SET*/
#define NDFC_READ_CMD (0xff << 0)
#define NDFC_RANDOM_READ_CMD0 (0xff << 8)
#define NDFC_RANDOM_READ_CMD1 (0xff << 16)
#define NDFC_RANDOM_CMD2 (0xff << 24)

/*0x002C : define bit use in NFC_WCMD_SET*/
#define NDFC_PROGRAM_CMD (0xff << 0)
#define NDFC_RANDOM_WRITE_CMD (0xff << 8)
#define NDFC_READ_CMD0 (0xff << 16)
#define NDFC_READ_CMD1 (0xff << 24)

/*0x0034 : define bit use in NFC_ECC_CTL*/
#define NDFC_ECC_EN (1 << 0)
#define NDFC_ECC_PIPELINE (1 << 3)
#define NDFC_ECC_EXCEPTION (1 << 4)
#define NDFC_ECC_BLOCK_SIZE (1 << 7)
#define NDFC_RANDOM_EN (1 << 5)
#define NDFC_RANDOM_DIRECTION (1 << 6)
#define NDFC_ECC_MODE (0xff << 8)
#define NDFC_RANDOM_SEED (0x7fff << 16)

/*0x0040 : define bit use in NDFC_EFR*/
#define NDFC_ECC_DEBUG (0x3f << 0)
#define NDFC_WP_CTRL (1 << 8)
#define NDFC_DUMMY_BYTE_CNT (0xff << 16)
#define NDFC_DUMMY_BYTE_EN (1 << 24)

/*0x0044 : define bit use in NDFC_RDATA_STA_CTL*/
#define NDFC_RDATA_STA_EN (0x1 << 24)
#define NDFC_RDATA_STA_TH (0x7FF << 0)

/*0x0070 : define bit use in NDFC_USER_DATA_LEN_X*/
#define NDFC_DATA_LEN_DATA_BLOCK (0x4)

/*0x0100 : define bit use in reg_flash_sta*/
#define CMD_REPT_EN (1 << 24)
#define BITMAP_B (0xff << 16)
#define BITMAP_A (0xff << 8)
#define SYNC_CMD_REPT_READ_STA (0xff << 0)

/*0x0104 : define bit use in reg_cmd_repeat_cnt*/
#define CMD_REPT_CNT (0xffff << 0)

/*0x0108 : define bit use in reg_cmd_repeat_interval*/
#define CMD_REPT_INVL (0xffff << 0)

/*0x02FC : define bit use in reg_ldpc_ctl*/
#define CH1_HB_LLR_VAL (0xf << 28)
#define CH1_SOFT_BIT_DEC_EN (1 << 5)
#define CH1_SRAM_MAIN_OR_SPARE_AREA_IND (1 << 4)
#define CH1_SRAM_IND (0x7 << 1)
#define CH1_SOFT_BIT_IND (1 << 0)

/*0x0300 : define bit use in reg_enc_ldpc_mode_set*/
#define ENCODE (1 << 31)
#define DECODE (0 << 31)
#define LDPC_MODE (0x7 << 28)
#define BOOT0_LDPC_MODE ((0 & 0x7) << 28)
#define FW_EXTEND_ENCODE (1 << 27)
#define BOOT0_FW_EXTEND_ENCODE ((1 & 0x1) << 27)

/*0x0304 : define bit use in reg_cor_ldpc_mode_set*/
#define C0_LDPC_MODE (0x7 << 29)
#define BOOT0_C0_LDPC_MODE ((0 & 0x7) << 29)
#define C0_DECODE_MODE (0x7 << 26)
#define C0_SCALE_MODE (0x3 << 24)
#define C1_LDPC_MODE (0x7 << 21)
#define BOOT0_C1_LDPC_MODE ((0 & 0x7) << 21)
#define C1_DECODE_MODE (0x7 << 18)
#define C1_SCALE_MODE (0x3 << 16)
#define FW_EXTEND_DECODE (1 << 15)
#define BOOT0_FW_EXTEND_DECODE ((1 & 0x1) << 15)

/*0x0C00 : define bit use in reg_glb_cfg*/
#define CMD_DESCR_FINISH_NUM (0xffff << 16)
#define LDPC_OUTPUT_DISORDER_CTL (1 << 8)
#define CMD_DESCR_ECC_ERROR_HANDLE_EN (1 << 7)
#define CMD_DESCR_CMD_REPT_TIMEOUT_HANDLE_EN (1 << 6)
#define CMD_DESCR_FLASH_STA_HANDLE_EN (1 << 5)
#define REG_ACCESS_METHOD (1 << 4)
#define CMD_DESCR_CTL_BIT (1 << 3)
#define NDFC_CHANNEL_SEL (1 << 2)
#define NDFC_RESET (1 << 1)
#define NDFC_EN (1 << 0)

/*0x0C08 : define bit use in reg_cmd_descr_sta*/
#define CMD_DESCR_ECC_ERR_STA (1 << 3)
#define CMD_DESCR_REPT_MODE_TIMEOUT_STA (1 << 2)
#define CMD_DESCR_FLASH_STA_HANDLE_STA (1 << 1)
#define CMD_DESCR_STA (1 << 0)

/*0x0C0C : define bit use in reg_cmd_descr_intr*/
#define CMD_DESCR_INTR_EN (1 << 0)

/*0x0C10  : define bit use in reg_csic_bist_ctl_reg*/
#define BIST_ERR_STA (1 << 15)
#define BIST_ERR_PAT (0x7 << 12)
#define BIST_ERR_CYC (3 << 10)
#define BIST_STOP (1 << 9)
#define BIST_BUSY (1 << 8)
#define BIST_REG_SEL (0x7 << 5)
#define BIST_ADDR_MODE_SEL (0x1 << 4)
#define BIST_WDATA_PAT (0x7 << 1)
#define BIST_EN (1 << 0)

/*0x0C1C  : define bit use in reg_bist_data_mask*/
#define BIST_DATA_MASK (0xffffffff << 0)

/*cmd flag bit*/
#define NDFC_PAGE_MODE 0x1
#define NDFC_NORMAL_MODE 0x0

#define NDFC_DATA_FETCH 0x1
#define NDFC_NO_DATA_FETCH 0x0
#define NDFC_MAIN_DATA_FETCH 0x1
#define NDFC_SPARE_DATA_FETCH 0x0
#define NDFC_WAIT_RB 0x1
#define NDFC_NO_WAIT_RB 0x0
#define NDFC_IGNORE 0x0

#define NDFC_INT_RB 0
#define NDFC_INT_CMD 1
#define NDFC_INT_DMA 2
#define NDFC_INT_BATCh 5

//=============================================================================
//define the mask for the nand flash operation status
#define NAND_OPERATE_FAIL (1 << 0)  //nand flash program/erase failed mask
#define NAND_CACHE_READY (1 << 5)   //nand flash cache program true ready mask
#define NAND_STATUS_READY (1 << 6)  //nand flash ready/busy status mask
#define NAND_WRITE_PROTECT (1 << 7) //nand flash write protected mask

//=============================================================================
#define CMD_RESET 0xFF
#define CMD_SYNC_RESET 0xFC
#define CMD_READ_STA 0x70
#define CMD_READ_ID 0x90
#define CMD_READ_PARAMETER 0xEC
#define CMD_SET_FEATURE 0xEF
#define CMD_GET_FEATURE 0xEE
#define CMD_READ_PAGE_CMD1 0x00
#define CMD_READ_PAGE_CMD2 0x30
#define CMD_CHANGE_READ_ADDR_CMD1 0x05
#define CMD_CHANGE_READ_ADDR_CMD2 0xE0
#define CMD_WRITE_PAGE_CMD1 0x80
#define CMD_WRITE_PAGE_CMD2 0x10
#define CMD_CHANGE_WRITE_ADDR_CMD 0x85
#define CMD_ERASE_CMD1 0x60
#define CMD_ERASE_CMD2 0xD0

#define NDFC_CMD(x) (x)


#define MICRON_ONFI_PARAMETER_ADDR (0x0)
#define MICRON_JEDEC_PARAMETER_ADDR (0x40)

#define REVISION_FEATURES_BLOCK_INFO_PARAMETER_LEN (256)
#define MCIRON_DEVICE_MODEL_OFFSET (44)
//==============================================================================
//  define some constant variable for the nand flash driver used
//==============================================================================

//define the mask for the nand flash optional operation
//#define NAND_CACHE_READ          (1<<0)              //nand flash support cache read operation
//#define NAND_CACHE_PROGRAM       (1<<1)              //nand flash support page cache program operation
//#define NAND_MULTI_READ          (1<<2)              //nand flash support multi-plane page read operation
//#define NAND_MULTI_PROGRAM       (1<<3)              //nand flash support multi-plane page program operation
//#define NAND_PAGE_COPYBACK       (1<<4)              //nand flash support page copy-back command mode operation
//#define NAND_INT_INTERLEAVE      (1<<5)              //nand flash support internal inter-leave operation, it based multi-bank
//#define NAND_EXT_INTERLEAVE      (1<<6)              //nand flash support external inter-leave operation, it based multi-chip
//#define NAND_RANDOM		         (1<<7)			     //nand flash support RANDOMIZER
//#define NAND_READ_RETRY	         (1<<8)			     //nand falsh support READ RETRY
//#define NAND_READ_UNIQUE_ID	     (1<<9)			     //nand falsh support READ UNIQUE_ID
//#define NAND_PAGE_ADR_NO_SKIP	 (1<<10)			    //nand falsh page adr no skip is requiered
//#define NAND_DIE_SKIP            (1<<11)             //nand flash die adr skip
////#define NAND_LOG_BLOCK_MANAGE   (1<<12)             //nand flash log block type management
//#define NAND_FORCE_WRITE_SYNC    (1<<13)             //nand flash need check status after write or erase
//#define NAND_LOG_BLOCK_LSB_TYPE (0xff<<16)          //nand flash log block lsb page type
//#define NAND_LSB_PAGE_TYPE		 (0xf<<16)			//nand flash lsb page type....

//#define NAND_MAX_BLK_ERASE_CNT	 (1<<20)			//nand flash support the maximum block erase cnt
//#define NAND_READ_RECLAIM		 (1<<21)			//nand flash support to read reclaim Operation
//
//#define NAND_TIMING_MODE    	 (1<<24)            //nand flash support to change timing mode according to ONFI 3.0
//#define NAND_DDR2_CFG			 (1<<25)            //nand flash support to set ddr2 specific feature according to ONFI 3.0 or Toggle DDR2
//#define NAND_IO_DRIVER_STRENGTH  (1<<26)            //nand flash support to set io driver strength according to ONFI 2.x/3.0 or Toggle DDR1/DDR2
//#define NAND_VENDOR_SPECIFIC_CFG (1<<27)            //nand flash support to set vendor's specific configuration
//#define NAND_ONFI_SYNC_RESET_OP	 (1<<28)            //nand flash support onfi's sync reset
//#define NAND_TOGGLE_ONLY 		 (1<<29)            //nand flash support toggle interface only, and do not support switch between legacy and toggle
extern struct nand_controller_info *g_nctri;
extern struct nand_controller_info g_nctri_data[2];

s32 ndfc_wait_dma_end(struct nand_controller_info *nctri, u8 rw, void *buff_addr, u32 len);
s32 ndfc_check_read_data_sta(struct nand_controller_info *nctri, u32 eblock_cnt);
s32 ndfc_get_bad_blk_flag(u32 page_no, u32 dis_random, u32 slen, u8 *sbuf);
s32 ndfc_check_ecc(struct nand_controller_info *nctri, u32 eblock_cnt);
s32 ndfc_update_ecc_sta_and_spare_data(struct _nand_physic_op_par *npo, s32 ecc_sta, unsigned char *sbuf);
#endif //_NAND_CONTROLLER_H
