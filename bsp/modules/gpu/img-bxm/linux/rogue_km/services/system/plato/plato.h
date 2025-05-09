/*************************************************************************/ /*!
@File
@Title          System Description Header
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    This header provides system-specific declarations and macros
@License        Dual MIT/GPLv2

The contents of this file are subject to the MIT license as set out below.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

Alternatively, the contents of this file may be used under the terms of
the GNU General Public License Version 2 ("GPL") in which case the provisions
of GPL are applicable instead of those above.

If you wish to allow use of your version of this file only under the terms of
GPL, and not to allow others to use your version of this file under the terms
of the MIT license, indicate your decision by deleting the provisions above
and replace them with the notice and other provisions required by GPL as set
out in the file called "GPL-COPYING" included in this distribution. If you do
not delete the provisions above, a recipient may use your version of this file
under the terms of either the MIT license or GPL.

This License is also included in this distribution in the file called
"MIT-COPYING".

EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/ /**************************************************************************/

#if !defined(PLATO_H)
#define PLATO_H

#define PLATO_SYSTEM_NAME			"Plato"

/* Valid values for the PLATO_MEMORY_CONFIG configuration option */
#define PLATO_MEMORY_LOCAL			(1)
#define PLATO_MEMORY_HOST			(2)
#define PLATO_MEMORY_HYBRID			(3)

/* Interrupt defines */
typedef enum _PLATO_IRQ_
{
    PLATO_IRQ_GPU = 0,
    PLATO_IRQ_PDP,
    PLATO_IRQ_HDMI,
    PLATO_IRQ_MAX,
}PLATO_IRQ;

#define PLATO_GPU_INT_SHIFT				(0)
#define PLATO_PDP_INT_SHIFT				(8)
#define PLATO_HDMI_INT_SHIFT			(9)
#define PLATO_HDMI_WAKEUP_INT_SHIFT		(11)
#define PLATO_TEMP_A_INT_SHIFT			(12)


#if (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HOST)
#define PHYS_HEAP_ID_CPU_LOCAL 0
#elif (PLATO_MEMORY_CONFIG == PLATO_MEMORY_HYBRID)
#define PHYS_HEAP_ID_GPU_LOCAL 0
#define PHYS_HEAP_ID_CPU_LOCAL 1
#define PHYS_HEAP_ID_PDP_LOCAL 2
#define PHYS_HEAP_ID_NON_MAPPABLE 3
#define PHYS_HEAP_ID_FW_LOCAL 4
#elif (PLATO_MEMORY_CONFIG == PLATO_MEMORY_LOCAL)
#define PHYS_HEAP_ID_GPU_LOCAL 0
#define PHYS_HEAP_ID_PDP_LOCAL 1
#define PHYS_HEAP_ID_NON_MAPPABLE 2
#endif

#define DCPDP_PHYS_HEAP_ID PHYS_HEAP_ID_PDP_LOCAL

#if defined(SUPPORT_DISPLAY_CLASS)
#define RGX_PLATO_RESERVE_DC_MEM_SIZE (PLATO_DC_MEM_SIZE_MB * 1024 * 1024)
#endif

#define SYS_PLATO_REG_DMA_BASENUM	(0)
#define SYS_PLATO_REG_DMA_SIZE		(64 * 1024)

#define SYS_PLATO_REG_PCI_BASENUM	(1)
#define SYS_PLATO_REG_REGION_SIZE	(4 * 1024 * 1024)

#define SYS_PLATO_SRAM_BASENUM		(2)
#define SYS_PLATO_SRAM_SIZE			(2 * 1024 * 1024)
/*
 * Give system region a whole span of the reg space including
 * RGX registers. That's because there are sys register segments
 * both before and after the RGX segment.
 */
#define SYS_PLATO_REG_SYS_OFFSET			(0x0)
#define SYS_PLATO_REG_SYS_SIZE				(4 * 1024 * 1024)

/* Entire Peripheral region */
#define SYS_PLATO_REG_PERIP_OFFSET			(0x20000)
#define SYS_PLATO_REG_PERIP_SIZE			(164 * 1024)

/* Chip level registers */
#define SYS_PLATO_REG_CHIP_LEVEL_OFFSET		(SYS_PLATO_REG_PERIP_OFFSET)
#define SYS_PLATO_REG_CHIP_LEVEL_SIZE		(64 * 1024)

#define SYS_PLATO_REG_TEMPA_OFFSET			(0x80000)
#define SYS_PLATO_REG_TEMPA_SIZE			(64 * 1024)

/* USB not included */

#define SYS_PLATO_REG_DDR_A_CTRL_OFFSET		(0x120000)
#define SYS_PLATO_REG_DDR_A_CTRL_SIZE		(64 * 1024)

#define SYS_PLATO_REG_DDR_B_CTRL_OFFSET		(0x130000)
#define SYS_PLATO_REG_DDR_B_CTRL_SIZE		(64 * 1024)

#define SYS_PLATO_REG_DDR_A_PUBL_OFFSET		(0x140000)
#define SYS_PLATO_REG_DDR_A_PUBL_SIZE		(64 * 1024)

#define SYS_PLATO_REG_DDR_B_PUBL_OFFSET		(0x150000)
#define SYS_PLATO_REG_DDR_B_PUBL_SIZE		(64 * 1024)

#define SYS_PLATO_REG_NOC_OFFSET			(0x160000)
#define SYS_PLATO_REG_NOC_SIZE				(64 * 1024)

/* Debug NOC registers */
#define SYS_PLATO_REG_NOC_DBG_DDR_A_CTRL_OFFSET (0x1500)
#define SYS_PLATO_REG_NOC_DBG_DDR_A_DATA_OFFSET (0x1580)
#define SYS_PLATO_REG_NOC_DBG_DDR_A_PUBL_OFFSET (0x1600)
#define SYS_PLATO_REG_NOC_DBG_DDR_B_CTRL_OFFSET (0x1680)
#define SYS_PLATO_REG_NOC_DBG_DDR_B_DATA_OFFSET (0x1700)
#define SYS_PLATO_REG_NOC_DBG_DDR_B_PUBL_OFFSET (0x1780)
#define SYS_PLATO_REG_NOC_DBG_DISPLAY_S_OFFSET  (0x1800)
#define SYS_PLATO_REG_NOC_DBG_GPIO_0_S_OFFSET   (0x1900)
#define SYS_PLATO_REG_NOC_DBG_GPIO_1_S_OFFSET   (0x1980)
#define SYS_PLATO_REG_NOC_DBG_GPU_S_OFFSET      (0x1A00)
#define SYS_PLATO_REG_NOC_DBG_PCI_PHY_OFFSET    (0x1A80)
#define SYS_PLATO_REG_NOC_DBG_PCI_REG_OFFSET    (0x1B00)
#define SYS_PLATO_REG_NOC_DBG_PCI_S_OFFSET      (0x1B80)
#define SYS_PLATO_REG_NOC_DBG_PERIPH_S_OFFSET   (0x1c00)
#define SYS_PLATO_REG_NOC_DBG_RET_REG_OFFSET    (0x1D00)
#define SYS_PLATO_REG_NOC_DBG_SERVICE_OFFSET    (0x1E00)

#define SYS_PLATO_REG_RGX_OFFSET			(0x170000)
#define SYS_PLATO_REG_RGX_SIZE				(64 * 1024)

#define SYS_PLATO_REG_AON_OFFSET			(0x180000)
#define SYS_PLATO_REG_AON_SIZE				(64 * 1024)

#define SYS_PLATO_REG_PDP_OFFSET			(0x200000)
#define SYS_PLATO_REG_PDP_SIZE				(128 * 1024)

#define SYS_PLATO_REG_PDP_BIF_OFFSET        (SYS_PLATO_REG_PDP_OFFSET + SYS_PLATO_REG_PDP_SIZE)
#define SYS_PLATO_REG_PDP_BIF_SIZE          (0x200)

#define SYS_PLATO_REG_HDMI_OFFSET           (SYS_PLATO_REG_PDP_OFFSET + 0x20000)
#define SYS_PLATO_REG_HDMI_SIZE             (128 * 1024)

/* Device memory (including HP mapping) on base register 4 */
#define SYS_DEV_MEM_PCI_BASENUM		(4)

/* Device memory size */
#define ONE_GB_IN_BYTES				(0x40000000ULL)
#define SYS_DEV_MEM_REGION_SIZE		(PLATO_MEMORY_SIZE_GIGABYTES * ONE_GB_IN_BYTES)

/* Plato DDR offset in device memory map at 32GB */
#define PLATO_DDR_DEV_PHYSICAL_BASE	(0x800000000)

/* DRAM is split at 48GB */
#define PLATO_DRAM_SPLIT_ADDR (0xc00000000)

/* Plato DDR region is aliased if less than 32GB memory is present.
 * This defines memory base closest to the DRAM split point.
 * If 32GB is present this is equal to PLATO_DDR_DEV_PHYSICAL_BASE */
#define PLATO_DDR_ALIASED_DEV_PHYSICAL_BASE	(PLATO_DRAM_SPLIT_ADDR - (SYS_DEV_MEM_REGION_SIZE >> 1))

#define PLATO_DDR_ALIASED_DEV_PHYSICAL_END	(PLATO_DRAM_SPLIT_ADDR + (SYS_DEV_MEM_REGION_SIZE >> 1))

#define PLATO_DDR_ALIASED_DEV_SEGMENT_SIZE	((32ULL / PLATO_MEMORY_SIZE_GIGABYTES) * ONE_GB_IN_BYTES)

/* Plato Host memory offset in device memory map at 512GB */
#define PLATO_HOSTRAM_DEV_PHYSICAL_BASE (0x8000000000)

/* Plato PLL, DDR/GPU and PDP clocks */
#define PLATO_PLL_REF_CLOCK_SPEED		(19200000)

/* 600 MHz */
#define PLATO_MEM_CLOCK_SPEED			(600000000)
#define PLATO_MIN_MEM_CLOCK_SPEED		(600000000)
#define PLATO_MAX_MEM_CLOCK_SPEED		(800000000)

/* 396 MHz (~400 MHz) on HW, around 1MHz on the emulator */
#if defined(EMULATOR) || defined(VIRTUAL_PLATFORM)
#define	PLATO_RGX_CORE_CLOCK_SPEED		(1000000)
#define	PLATO_RGX_MIN_CORE_CLOCK_SPEED		(1000000)
#define	PLATO_RGX_MAX_CORE_CLOCK_SPEED		(1000000)
#else

#define	PLATO_RGX_CORE_CLOCK_SPEED		(396000000)
#define	PLATO_RGX_MIN_CORE_CLOCK_SPEED	(396000000)
#define	PLATO_RGX_MAX_CORE_CLOCK_SPEED	(742500000)
#endif

#define PLATO_MIN_PDP_CLOCK_SPEED				(165000000)
#define PLATO_TARGET_HDMI_SFR_CLOCK_SPEED		(27000000)
#define PLATO_TARGET_HDMI_CEC_CLOCK_SPEED		(32768)

#define REG_TO_CELSIUS(reg)			(((reg) * 352/4096) - 109)
#define CELSIUS_TO_REG(temp)		((((temp) + 109) * 4096) / 352)
#define PLATO_MAX_TEMP_CELSIUS		(100)

#define PLATO_LMA_HEAP_REGION_MAPPABLE			0
#define PLATO_LMA_HEAP_REGION_NONMAPPABLE		1

#if defined(ENABLE_PLATO_HDMI)

#if defined(HDMI_PDUMP)
/* Hard coded video formats for pdump type run only */
#define VIDEO_FORMAT_1280_720p          0
#define VIDEO_FORMAT_1920_1080p         1
#define DC_DEFAULT_VIDEO_FORMAT     (VIDEO_FORMAT_1920_1080p)
#endif

#define DEFAULT_BORDER_WIDTH_PIXELS     (0)
#define HDMI_DTD_MAX                    (20) // Reference driver has 37...seems like a lot

typedef enum
{
	ENCODING_RGB = 0,
	ENCODING_YCC444,
	ENCODING_YCC422,
	ENCODING_YCC420
} ENCODING_T;

typedef enum
{
	ITU601 = 1,
	ITU709,
	EXTENDED_COLORIMETRY
} COLORIMETRY_T;

typedef struct
{
	/** VIC code */
	IMG_UINT32 mCode;
	/** Identifies modes that ONLY can be displayed in YCC 4:2:0 */
	IMG_UINT8 mLimitedToYcc420;
	/** Identifies modes that can also be displayed in YCC 4:2:0 */
	IMG_UINT8 mYcc420;
	IMG_UINT16 mPixelRepetitionInput;
	/** in units of 10KHz */
	IMG_UINT16 mPixelClock;
	/** 1 for interlaced, 0 progressive */
	IMG_UINT8 mInterlaced;
	IMG_UINT16 mHActive;
	IMG_UINT16 mHBlanking;
	IMG_UINT16 mHBorder;
	IMG_UINT16 mHImageSize;
	IMG_UINT16 mHFrontPorchWidth;
	IMG_UINT16 mHBackPorchWidth;
	IMG_UINT16 mHSyncPulseWidth;
	/** 0 for Active low, 1 active high */
	IMG_UINT8 mHSyncPolarity;
	IMG_UINT16 mVActive;
	IMG_UINT16 mVBlanking;
	IMG_UINT16 mVBorder;
	IMG_UINT16 mVImageSize;
	IMG_UINT16 mVFrontPorchWidth;
	IMG_UINT16 mVBackPorchWidth;
	IMG_UINT16 mVSyncPulseWidth;
	/** 0 for Active low, 1 active high */
	IMG_UINT8 mVSyncPolarity;

    IMG_UINT8 mValid;
} DTD;

/* Monitor range limits comes from Display Descriptor (EDID) */
typedef struct _MONITOR_RANGE_LIMITS
{
	IMG_UINT8 vMinRate;
	IMG_UINT8 vMaxRate;
	IMG_UINT8 hMinRate;
	IMG_UINT8 hMaxRate;
	IMG_UINT8 maxPixelClock;
	IMG_UINT8 valid;
} MONITOR_RANGE_LIMITS;

typedef struct
{
    IMG_UINT8               mHdmi;
    ENCODING_T              mEncodingOut;
    ENCODING_T              mEncodingIn;
    IMG_UINT8               mColorResolution;
    IMG_UINT8               mPixelRepetitionFactor;
    DTD                     mDtdList[HDMI_DTD_MAX];
    IMG_UINT8               mDtdIndex;
    IMG_UINT8               mDtdActiveIndex;
    IMG_UINT8               mRgbQuantizationRange;
    IMG_UINT8               mPixelPackingDefaultPhase;
    IMG_UINT8               mColorimetry;
    IMG_UINT8               mScanInfo;
    IMG_UINT8               mActiveFormatAspectRatio;
    IMG_UINT8               mNonUniformScaling;
    IMG_UINT8               mExtColorimetry;
    IMG_UINT8               mItContent;
    IMG_UINT16              mEndTopBar;
    IMG_UINT16              mStartBottomBar;
    IMG_UINT16              mEndLeftBar;
    IMG_UINT16              mStartRightBar;
    IMG_UINT16              mCscFilter;
    IMG_UINT16              mCscA[4];
    IMG_UINT16              mCscC[4];
    IMG_UINT16              mCscB[4];
    IMG_UINT16              mCscScale;
    IMG_UINT8               mHdmiVideoFormat;
    IMG_UINT8               m3dStructure;
    IMG_UINT8               m3dExtData;
    IMG_UINT8               mHdmiVic;
    IMG_UINT8               mDataEnablePolarity;
    MONITOR_RANGE_LIMITS    mRangeLimits;
    IMG_UINT8               mPreferredTimingIncluded;
} VIDEO_PARAMS;

/* Callback for HDMI->PDP */
PVRSRV_ERROR PDPInitialize(VIDEO_PARAMS * pVideoParams, IMG_UINT32 uiInstance);

#endif /* ENABLE_PLATO_HDMI */

#endif /* if !defined(PLATO_H) */
