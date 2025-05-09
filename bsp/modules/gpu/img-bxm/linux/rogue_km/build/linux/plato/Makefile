########################################################################### ###
#@Copyright     Copyright (c) Imagination Technologies Ltd. All Rights Reserved
#@License       Dual MIT/GPLv2
#
# The contents of this file are subject to the MIT license as set out below.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# Alternatively, the contents of this file may be used under the terms of
# the GNU General Public License Version 2 ("GPL") in which case the provisions
# of GPL are applicable instead of those above.
#
# If you wish to allow use of your version of this file only under the terms of
# GPL, and not to allow others to use your version of this file under the terms
# of the MIT license, indicate your decision by deleting the provisions above
# and replace them with the notice and other provisions required by GPL as set
# out in the file called "GPL-COPYING" included in this distribution. If you do
# not delete the provisions above, a recipient may use your version of this file
# under the terms of either the MIT license or GPL.
#
# This License is also included in this distribution in the file called
# "MIT-COPYING".
#
# EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
# PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
# BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
### ###########################################################################

RGX_BVNC ?= 6.34.4.35

include ../config/preconfig.mk
include ../config/window_system.mk

NO_HARDWARE := 0
KERNEL_COMPONENTS := srvkm

ifeq ($(PVR_REMVIEW),1)
 SUPPORT_PLATO_DISPLAY := 0
else
 SUPPORT_PLATO_DISPLAY ?= 1
endif

PLATO_DUAL_CHANNEL_DDR ?= 1
PDP_DEBUG ?= 0
HDMI_DEBUG ?= 0
HDMI_EXPERIMENTAL_CLOCKS ?= 0
HDMI_PDUMP ?= 0
EMULATOR ?= 0
HDMI_FORCE_MODE ?= 0
PVRSRV_FORCE_UNLOAD_IF_BAD_STATE := 1
PLATO_PDP_MAX_RELOADS ?= 15
ZEUS_SUPPORT_DEVICE_LOCAL_HEAP ?= 1

PLATO_DDR_BDLR_TRAINING ?= 1

PVR_PMR_TRANSLATE_UMA_ADDRESSES := 1

ifeq ($(VIRTUAL_PLATFORM),1)
 PLATO_MEMORY_SIZE_GB ?= 2
else
 PLATO_MEMORY_SIZE_GB ?= 8
endif
PLATO_DC_MEM_SIZE_MB ?= 36

ifneq ($(shell expr 32 % $(PLATO_MEMORY_SIZE_GB)),0)
 $(error PLATO_MEMORY_SIZE_GB has to be an integer divisor of 32)
endif

ifeq ($(SUPPORT_KMS),1)
 PVR_SYSTEM := rgx_linux_plato
 PVR_LDM_PLATFORM_PRE_REGISTERED := 1

 ifeq ($(SUPPORT_PLATO_DISPLAY),1)
  PLATO_MEMORY_CONFIG ?= PLATO_MEMORY_HYBRID
  DISPLAY_CONTROLLER ?= drm_pdp
  HDMI_CONTROLLER ?= drm_pdp2_hdmi
  PVR_DRM_MODESET_DRIVER_NAME ?= pdp
  ENABLE_PLATO_HDMI ?= 1
 else
  PLATO_MEMORY_CONFIG := PLATO_MEMORY_HYBRID
  DISPLAY_CONTROLLER := "drm_nulldisp"
  PVR_DRM_MODESET_DRIVER_NAME ?= "nulldisp"
 endif
 KERNEL_COMPONENTS += plato
else
 PVR_SYSTEM := plato
 PVR_LOADER := pvr_pci_drv

 PLATO_MEMORY_CONFIG ?= PLATO_MEMORY_HYBRID

 ifeq ($(SUPPORT_DISPLAY_CLASS),1)
  ifeq ($(SUPPORT_PLATO_DISPLAY),1)
   DISPLAY_CONTROLLER ?= dc_pdp2
   HDMI_CONTROLLER ?= plato_hdmi
   ENABLE_PLATO_HDMI ?= 1
   ifeq ($(EMULATOR), 1)
    HDMI_PDUMP := 1
   endif
  else
   DISPLAY_CONTROLLER ?= dc_example
  endif
 else
  SUPPORT_PLATO_DISPLAY ?= 0
 endif
endif

ifeq ($(PDUMP),1)
 PLATO_DISPLAY_PDUMP ?= 0
endif

ifeq ($(SUPPORT_GIGACLUSTER),1)
	GC_SINGLE_DEVICE_MODE ?= 0
	GC_PERF_OVERLAY ?= 0
	GC_ENABLE_LIBAV ?= 0
	GC_STREAM_X11 ?= 0
	GC_STREAM_GL_PLATO_DEDICATED ?= 1
endif

KERNEL_COMPONENTS += $(DISPLAY_CONTROLLER) $(HDMI_CONTROLLER)

# Should be last
include ../config/core.mk
-include ../common/lws.mk
include ../common/3rdparty.mk

ifeq ($(PLATO_MEMORY_CONFIG),PLATO_MEMORY_LOCAL)
 $(eval $(call BothConfigC,LMA,))
 $(eval $(call KernelConfigMake,LMA,1))
endif

ifeq ($(SUPPORT_GIGACLUSTER), 1)
$(eval $(call TunableUserConfigC,GC_STREAM_GL_PLATO_DEDICATED,1))
$(eval $(call TunableUserConfigC,GC_STREAM_X11,1))
$(eval $(call TunableUserConfigMake,GC_STREAM_X11,1))
$(eval $(call TunableUserConfigC,GC_ENABLE_LIBAV,1))
$(eval $(call TunableUserConfigC,GC_PERF_OVERLAY,1))
endif

$(eval $(call TunableKernelConfigC,PLATO_MEMORY_SIZE_GIGABYTES, $(PLATO_MEMORY_SIZE_GB)ULL))
$(eval $(call TunableKernelConfigC,PLATO_MEMORY_CONFIG,$(PLATO_MEMORY_CONFIG)))
$(eval $(call TunableKernelConfigC,PLATO_DDR_BDLR_TRAINING,$(PLATO_DDR_BDLR_TRAINING)))
$(eval $(call TunableKernelConfigC,SUPPORT_PLATO_DISPLAY,$(SUPPORT_PLATO_DISPLAY)))

$(eval $(call TunableKernelConfigC,PLATO_PDP_RELOADS_MAX,$(PLATO_PDP_MAX_RELOADS)))

$(eval $(call TunableKernelConfigC,VIRTUAL_PLATFORM,))
$(eval $(call TunableKernelConfigC,EMULATOR,))
$(eval $(call TunableKernelConfigC,PLATO_DUAL_CHANNEL_DDR,))
$(eval $(call TunableKernelConfigC,HDMI_FORCE_MODE,$(HDMI_FORCE_MODE)))

ifneq ($(HDMI_CONTROLLER),)
$(eval $(call BothConfigC,HDMI_CONTROLLER,$(HDMI_CONTROLLER)))
$(eval $(call BothConfigMake,HDMI_CONTROLLER,$(HDMI_CONTROLLER)))
endif
$(eval $(call TunableKernelConfigC,ENABLE_PLATO_HDMI,))

$(eval $(call TunableKernelConfigC,PDP_DEBUG,))
$(eval $(call TunableBothConfigC,HDMI_DEBUG,))
$(eval $(call TunableBothConfigC,HDMI_EXPERIMENTAL_CLOCKS,))
$(eval $(call TunableBothConfigC,HDMI_PDUMP,))
ifeq ($(PDUMP), 1)
$(eval $(call TunableKernelConfigC,PLATO_DISPLAY_PDUMP,))
endif

$(eval $(call TunableKernelConfigC,PLATO_DC_MEM_SIZE_MB,))
$(eval $(call TunableUserConfigC,SUPPORT_VK_PLATO_DUAL_HEAP,1))
