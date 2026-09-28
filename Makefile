.SILENT:

MAKEFLAGS += --no-print-directory

QMK_USERSPACE := $(patsubst %/,%,$(dir $(shell realpath "$(lastword $(MAKEFILE_LIST))")))
ifeq ($(QMK_USERSPACE),)
    QMK_USERSPACE := $(shell pwd)
endif

# Prefer the QMK submodule pinned in this repository, fall back to `qmk config user.qmk_home`.
QMK_FIRMWARE_ROOT ?= $(QMK_USERSPACE)/qmk_firmware
ifeq ($(wildcard $(QMK_FIRMWARE_ROOT)/builddefs/build_keyboard.mk),)
    QMK_FIRMWARE_ROOT := $(shell qmk config -ro user.qmk_home | cut -d= -f2 | sed -e 's@^None$$@@g')
endif
ifeq ($(QMK_FIRMWARE_ROOT),)
    $(error Cannot determine qmk_firmware location: no ./qmk_firmware submodule checked out and `qmk config user.qmk_home` is not set)
endif

%:
	+$(MAKE) -C $(QMK_FIRMWARE_ROOT) $(MAKECMDGOALS) QMK_USERSPACE=$(QMK_USERSPACE)
