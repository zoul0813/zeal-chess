BIN=chess.bin

ifndef ZOS_PATH
    $(error "Failure: ZOS_PATH variable not found. It must point to Zeal 8-bit OS path.")
endif

ifndef ZVB_SDK_PATH
    $(error "Failure: ZVB_SDK_PATH variable not found. It must point to Zeal Video Board SDK path.")
endif

include $(ZVB_SDK_PATH)/sdcc/base_sdcc.mk

run:
	$(ZEAL_NATIVE_BIN) -H bin -r $(ZEAL_NATIVE_ROM) #-t tf.img -e eeprom.img

native: all run

