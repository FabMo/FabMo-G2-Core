# Open and connect to openocd with the ATMEL-ICE
#target remote | /usr/local/bin/openocd -c "set CHIPNAME ${CHIP}" -f ${MOTATE_PATH}/openocd.cfg -f ${MOTATE_PATH}/platform/atmel_sam/atmel_sam.cfg -c "gdb_port pipe; log_output openocd.log"

target extended-remote :2331

source ./board/g2_default.gdb

define boot_from_flash
  monitor at91sam3 gpnvm set 1
end

source -s arm.gdb
