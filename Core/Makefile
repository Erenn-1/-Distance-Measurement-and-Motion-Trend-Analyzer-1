######################################
# Target Settings
######################################
TARGET    = distance_analyzer
BUILD_DIR = build

######################################
# Toolchain Settings
######################################
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size

######################################
# MCU Settings
######################################
MCU = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

######################################
# Source Files
######################################
C_SOURCES = \
    Src/main.c \
    Src/system.c \
    Src/hc_sr04.c \
    Src/uart.c \
    Src/motion.c

# Nucleo-F411RE için doğru startup dosyası
ASM_SOURCES = \
    startup/startup_stm32f411retx.s

######################################
# Includes & Flags
######################################
C_INCLUDES = -IInc

CFLAGS  = $(MCU) $(C_INCLUDES) -Wall -Wextra -O2 \
          -fdata-sections -ffunction-sections -std=c11 -DSTM32F411xE

ASFLAGS = $(MCU) -Wall -fdata-sections -ffunction-sections

LDSCRIPT = linker/STM32F411RETx_FLASH.ld
LIBS     = -lc -lm -lnosys
LDFLAGS  = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBS) \
           -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref \
           -Wl,--gc-sections

######################################
# Build Rules
######################################
OBJECTS  = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin size

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	$(CP) -O ihex $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(CP) -O binary -S $< $@

$(BUILD_DIR):
	mkdir -p $@

######################################
# Flash & Clean
######################################
flash: all
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit"

size: $(BUILD_DIR)/$(TARGET).elf
	$(SZ) $<

clean:
	rm -rf $(BUILD_DIR)