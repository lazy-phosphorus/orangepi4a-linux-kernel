# Makefile for the focaltech touchscreen drivers.



obj-$(CONFIG_TOUCHSCREEN_FTS) += focaltech_tp.o
focaltech_tp-y := focaltech_core.o \
		focaltech_ex_fun.o \
		focaltech_ex_mode.o \
		focaltech_gesture.o \
		focaltech_esdcheck.o \
		focaltech_point_report_check.o


focaltech_tp-y += focaltech_i2c.o





focaltech_tp-y += focaltech_flash.o
focaltech_tp-y += focaltech_flash/focaltech_upgrade_ft8205.o