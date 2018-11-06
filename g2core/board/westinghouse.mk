# ----------------------------------------------------------------------------
# This file is part of the Synthetos g2core project


# To compile:
#   make BOARD=westinghouse-r01

# You can also choose a CONFIG from boards.mk:
#   make CONFIG=westinghouse-r01



# Backward compatibility with old projects that use PLATFORM instead, but with a warning:
ifneq ("$(PLATFORM:-)","")
    $(warning Using PLATFORM value of $(PLATFORM) as BOARD.)
    $(warning Please swich to using BOARD on the command line.)
    BOARD = $(PLATFORM)
endif


##########
# BOARDs for use directly from the make command line (with default settings) or by CONFIGs.

ifeq ("$(BOARD)","westinghouse-r01")
    BASE_BOARD = westinghouse
    DEVICE_DEFINES += MOTATE_BOARD="westinghouse-r01"
    DEVICE_DEFINES += SETTINGS_FILE=${SETTINGS_FILE}
endif



##########
# The general g2core-due BASE_BOARD.

ifeq ("$(BASE_BOARD)","westinghouse")
 _BOARD_FOUND = 1

    DEVICE_DEFINES += MOTATE_CONFIG_HAS_USBSERIAL=1 ENABLE_TCM=1

    FIRST_LINK_SOURCES += $(sort $(wildcard ${MOTATE_PATH}/Atmel_sam_common/*.cpp)) $(sort $(wildcard ${MOTATE_PATH}/Atmel_sams70/*.cpp) $(wildcard ${BOARD_PATH}/*.cpp))

    CHIP = SAMS70N19
    export CHIP
    CHIP_LOWERCASE = sams70n19

    BOARD_PATH = ./board/westinghouse
    SOURCE_DIRS += ${BOARD_PATH} device/step_dir_driver

    PLATFORM_BASE = ${MOTATE_PATH}/platform/atmel_sam
    include $(PLATFORM_BASE).mk
endif
