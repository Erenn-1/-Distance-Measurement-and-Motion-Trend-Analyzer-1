TARGET    = distance_analyzer
BUILD_DIR = build

# Toolchain (arm-none-eabi from ARM GNU Toolchain)
CC = arm-none-eabi-gcc
AS = arm-none-eabi-gcc -x assembler-with-cpp
CP = arm-none-eabi-objcopy
SZ = arm-none-eabi-size

# MCU — Cortex-M4 with FPU, hard float ABI
MCU = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

C_SOURCES = \
    Src/main.c    \
    Src/system.c  \
    Src/hc_sr04.c \
    Src/uart.c    \
    Src/motion.c

ASM_SOURCES = startup/startup_stm32f411xe.s

C_INCLUDES = -IInc

CFLAGS  = $(MCU) $(C_INCLUDES) -Wall -Wextra -O2 \
          -fdata-sections -ffunction-sections -std=c11

ASFLAGS = $(MCU) -Wall -fdata-sections -ffunction-sections

LDSCRIPT = linker/STM32F411RETx_FLASH.ld
LIBS     = -lc -lm -lnosys
LDFLAGS  = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBS) \
           -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref \
           -Wl,--gc-sections

OBJECTS  = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))

vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

.PHONY: all clean flash size

all: $(BUILD_DIR)/$(TARGET).elf \
     $(BUILD_DIR)/$(TARGET).hex \
     $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(CP) -O ihex $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(CP) -O binary -S $< $@

$(BUILD_DIR):
	mkdir -p $@

# Flash via ST-Link (requires stlink-tools)
flash: $(BUILD_DIR)/$(TARGET).bin
	st-flash write $< 0x8000000

# Flash via OpenOCD (alternative)
flash-ocd: $(BUILD_DIR)/$(TARGET).elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
	        -c "program $< verify reset exit"

size: $(BUILD_DIR)/$(TARGET).elf
	$(SZ) $<

clean:
	rm -rf $(BUILD_DIR)
