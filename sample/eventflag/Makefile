# eventflag

# Target specific macros
TARGET = eventflag
TARGET_SOURCES = eventflag.c
TOPPERS_JSP_CFG_SOURCE = ./itron.cfg
TERGET_HEADERS = defs.h

# Don't modify below part unless you understand them
TOPPERS_KERNEL = NXT_JSP
NXTOSEK_ROOT = /nxtOSEK
BUILD_MODE = RXE_ONLY

download	: all
	sh ./rxeflash.sh

O_PATH ?= build
include $(NXTOSEK_ROOT)/ecrobot/ecrobot.mak
### Dependencies
