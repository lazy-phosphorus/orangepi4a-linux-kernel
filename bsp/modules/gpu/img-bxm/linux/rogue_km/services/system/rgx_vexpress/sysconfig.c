/*************************************************************************/ /*!
@File
@Title          System Configuration
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Implements the system layer for QEMU vexpress virtual-platform
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

#if defined(__linux__)
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#endif

#include "pvrsrv_device.h"
#include "syscommon.h"
#include "sysinfo.h"
#include "sysconfig.h"
#include "physheap.h"
#include "interrupt_support.h"
#include "vz_vmm_pvz.h"
#if defined(SUPPORT_PDVFS)
#include "rgxpdvfs.h"
#endif

static RGX_TIMING_INFORMATION	gsRGXTimingInfo;
static RGX_DATA					gsRGXData;
static PVRSRV_DEVICE_CONFIG		gsDevices[1];
static PHYS_HEAP_FUNCTIONS		gsPhysHeapFuncs;
static PHYS_HEAP_CONFIG			gsPhysHeapConfig[2];

#if defined(SUPPORT_PDVFS)
static const IMG_OPP asOPPTable[] =
{
	{ 824,  240000000},
	{ 856,  280000000},
	{ 935,  380000000},
	{ 982,  440000000},
	{ 1061, 540000000},
};

#define LEVEL_COUNT (sizeof(asOPPTable) / sizeof(asOPPTable[0]))

static void SetFrequency(IMG_UINT32 ui32Frequency) {}

static void SetVoltage(IMG_UINT32 ui32Volt) {}
#endif

/*
	CPU to Device physical address translation
*/
static
void UMAPhysHeapCpuPAddrToDevPAddr(IMG_HANDLE hPrivData,
								   IMG_UINT32 ui32NumOfAddr,
								   IMG_DEV_PHYADDR *psDevPAddr,
								   IMG_CPU_PHYADDR *psCpuPAddr)
{
	PVR_UNREFERENCED_PARAMETER(hPrivData);

	/* Optimise common case */
	psDevPAddr[0].uiAddr = psCpuPAddr[0].uiAddr;
	if (ui32NumOfAddr > 1)
	{
		IMG_UINT32 ui32Idx;
		for (ui32Idx = 1; ui32Idx < ui32NumOfAddr; ++ui32Idx)
		{
			psDevPAddr[ui32Idx].uiAddr = psCpuPAddr[ui32Idx].uiAddr;
		}
	}
}

/*
	Device to CPU physical address translation
*/
static
void UMAPhysHeapDevPAddrToCpuPAddr(IMG_HANDLE hPrivData,
								   IMG_UINT32 ui32NumOfAddr,
								   IMG_CPU_PHYADDR *psCpuPAddr,
								   IMG_DEV_PHYADDR *psDevPAddr)
{
	PVR_UNREFERENCED_PARAMETER(hPrivData);

	/* Optimise common case */
	psCpuPAddr[0].uiAddr = psDevPAddr[0].uiAddr;
	if (ui32NumOfAddr > 1)
	{
		IMG_UINT32 ui32Idx;
		for (ui32Idx = 1; ui32Idx < ui32NumOfAddr; ++ui32Idx)
		{
			psCpuPAddr[ui32Idx].uiAddr = psDevPAddr[ui32Idx].uiAddr;
		}
	}
}

static void SysDevFeatureDepInit(PVRSRV_DEVICE_CONFIG *psDevConfig, IMG_UINT64 ui64Features)
{
#if defined(SUPPORT_AXI_ACE_TEST)
		if ( ui64Features & RGX_FEATURE_AXI_ACELITE_BIT_MASK)
		{
			gsDevices[0].eCacheSnoopingMode     = PVRSRV_DEVICE_SNOOP_CPU_ONLY;
		}
		else
#endif
		{
			psDevConfig->eCacheSnoopingMode		= PVRSRV_DEVICE_SNOOP_EMULATED;
		}
}

PVRSRV_ERROR SysDevInit(void *pvOSDevice, PVRSRV_DEVICE_CONFIG **ppsDevConfig)
{
	IMG_UINT32 ui32NextPhysHeapID = 0;
#if defined(__linux__)
	int iIrq;
	struct resource *psDevMemRes = NULL;
	struct platform_device *psDev;

	psDev = to_platform_device((struct device *)pvOSDevice);
#endif

	if (gsDevices[0].pvOSDevice)
	{
		return PVRSRV_ERROR_INVALID_DEVICE;
	}

#if defined(__linux__)
	dma_set_mask(pvOSDevice, DMA_BIT_MASK(32));
#endif

	/*
	 * Setup information about physical memory heap(s) we have
	 */
	gsPhysHeapFuncs.pfnCpuPAddrToDevPAddr = UMAPhysHeapCpuPAddrToDevPAddr;
	gsPhysHeapFuncs.pfnDevPAddrToCpuPAddr = UMAPhysHeapDevPAddrToCpuPAddr;

	gsPhysHeapConfig[ui32NextPhysHeapID].pszPDumpMemspaceName = "SYSMEM";
	gsPhysHeapConfig[ui32NextPhysHeapID].eType = PHYS_HEAP_TYPE_UMA;
	gsPhysHeapConfig[ui32NextPhysHeapID].psMemFuncs = &gsPhysHeapFuncs;
	gsPhysHeapConfig[ui32NextPhysHeapID].hPrivData = NULL;
	gsPhysHeapConfig[ui32NextPhysHeapID].ui32UsageFlags = PHYS_HEAP_USAGE_GPU_LOCAL;
	ui32NextPhysHeapID += 1;

	/*
	 * Setup RGX specific timing data
	 */
	gsRGXTimingInfo.ui32CoreClockSpeed        = RGX_VEXPRESS_CORE_CLOCK_SPEED;
	gsRGXTimingInfo.bEnableActivePM           = IMG_FALSE;
	gsRGXTimingInfo.bEnableRDPowIsland        = IMG_FALSE;
	gsRGXTimingInfo.ui32ActivePMLatencyms     = SYS_RGX_ACTIVE_POWER_LATENCY_MS;

	/*
	 *Setup RGX specific data
	 */
	gsRGXData.psRGXTimingInfo = &gsRGXTimingInfo;

	/*
	 * Setup device
	 */
	gsDevices[0].pvOSDevice				= pvOSDevice;
	gsDevices[0].pszName                = "vexpress";
	gsDevices[0].pszVersion             = NULL;

	/* Device setup information */
#if defined(__linux__)
	psDevMemRes = platform_get_resource(psDev, IORESOURCE_MEM, 0);
	if (psDevMemRes)
	{
		gsDevices[0].sRegsCpuPBase.uiAddr = psDevMemRes->start;
		gsDevices[0].ui32RegsSize         = (unsigned int)(psDevMemRes->end - psDevMemRes->start);
	}
	else
#endif
	{
#if defined(__linux__)
		PVR_LOG(("%s: platform_get_resource() failed, using mmio/sz 0x%x/0x%x",
				__func__,
				VEXPRESS_GPU_PBASE,
				VEXPRESS_GPU_SIZE));
#endif
		gsDevices[0].sRegsCpuPBase.uiAddr   = VEXPRESS_GPU_PBASE;
		gsDevices[0].ui32RegsSize           = VEXPRESS_GPU_SIZE;
	}

#if defined(__linux__)
	iIrq = platform_get_irq(psDev, 0);
	if (iIrq >= 0)
	{
		gsDevices[0].ui32IRQ = (IMG_UINT32) iIrq;
	}
	else
#endif
	{
#if defined(__linux__)
		PVR_LOG(("%s: platform_get_irq() failed, using irq %d",
				__func__,
				VEXPRESS_IRQ_GPU));
#endif
		gsDevices[0].ui32IRQ = VEXPRESS_IRQ_GPU;
	}

	/* Device's physical heaps */
	gsDevices[0].pasPhysHeaps = gsPhysHeapConfig;
	gsDevices[0].ui32PhysHeapCount = ARRAY_SIZE(gsPhysHeapConfig);
	gsDevices[0].eDefaultHeap = PVRSRV_PHYS_HEAP_GPU_LOCAL;

	/* No power management on VIRTUAL_PLATFORM system */
	gsDevices[0].pfnPrePowerState       = NULL;
	gsDevices[0].pfnPostPowerState      = NULL;

	/* No clock frequency either */
	gsDevices[0].pfnClockFreqGet        = NULL;

	gsDevices[0].hDevData               = &gsRGXData;

	gsDevices[0].pfnSysDevFeatureDepInit = &SysDevFeatureDepInit;

	gsDevices[0].bHasFBCDCVersion31	= IMG_FALSE;
	gsDevices[0].bDevicePA0IsValid	= IMG_FALSE;

	/* device error notify callback function */
	gsDevices[0].pfnSysDevErrorNotify = NULL;

	/* Virtualization support services needs to know which heap ID corresponds to FW */
	PVR_ASSERT(ui32NextPhysHeapID < ARRAY_SIZE(gsPhysHeapConfig));
	gsPhysHeapConfig[ui32NextPhysHeapID].pszPDumpMemspaceName = "SYSMEM";
	gsPhysHeapConfig[ui32NextPhysHeapID].eType = PHYS_HEAP_TYPE_UMA;
	gsPhysHeapConfig[ui32NextPhysHeapID].psMemFuncs = &gsPhysHeapFuncs;
	gsPhysHeapConfig[ui32NextPhysHeapID].hPrivData = NULL;
	gsPhysHeapConfig[ui32NextPhysHeapID].ui32UsageFlags = PHYS_HEAP_USAGE_FW_MAIN;

#if defined(SUPPORT_PDVFS)
	gsDevices[0].sDVFS.sDVFSDeviceCfg.pasOPPTable = asOPPTable;
	gsDevices[0].sDVFS.sDVFSDeviceCfg.ui32OPPTableSize = LEVEL_COUNT;
	gsDevices[0].sDVFS.sDVFSDeviceCfg.pfnSetFrequency = SetFrequency;
	gsDevices[0].sDVFS.sDVFSDeviceCfg.pfnSetVoltage = SetVoltage;
#endif

	*ppsDevConfig = &gsDevices[0];

	return PVRSRV_OK;
}

void SysDevDeInit(PVRSRV_DEVICE_CONFIG *psDevConfig)
{
	psDevConfig->pvOSDevice = NULL;
}

PVRSRV_ERROR SysInstallDeviceLISR(IMG_HANDLE hSysData,
								  IMG_UINT32 ui32IRQ,
								  const IMG_CHAR *pszName,
								  PFN_LISR pfnLISR,
								  void *pvData,
								  IMG_HANDLE *phLISRData)
{
	PVR_UNREFERENCED_PARAMETER(hSysData);
	return OSInstallSystemLISR(phLISRData, ui32IRQ, pszName, pfnLISR, pvData,
								SYS_IRQ_FLAG_TRIGGER_DEFAULT);
}

PVRSRV_ERROR SysUninstallDeviceLISR(IMG_HANDLE hLISRData)
{
	return OSUninstallSystemLISR(hLISRData);
}

PVRSRV_ERROR SysDebugInfo(PVRSRV_DEVICE_CONFIG *psDevConfig,
				DUMPDEBUG_PRINTF_FUNC *pfnDumpDebugPrintf,
				void *pvDumpDebugFile)
{
	PVR_UNREFERENCED_PARAMETER(psDevConfig);
	PVR_UNREFERENCED_PARAMETER(pfnDumpDebugPrintf);
	PVR_UNREFERENCED_PARAMETER(pvDumpDebugFile);
	return PVRSRV_OK;
}

/******************************************************************************
 End of file (sysconfig.c)
******************************************************************************/
