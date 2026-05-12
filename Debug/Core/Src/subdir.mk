################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/hc_sr04.c \
../Core/Src/main.c \
../Core/Src/motion.c \
../Core/Src/system.c \
../Core/Src/uart.c 

OBJS += \
./Core/Src/hc_sr04.o \
./Core/Src/main.o \
./Core/Src/motion.o \
./Core/Src/system.o \
./Core/Src/uart.o 

C_DEPS += \
./Core/Src/hc_sr04.d \
./Core/Src/main.d \
./Core/Src/motion.d \
./Core/Src/system.d \
./Core/Src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/user/Documents/GitHub/-Distance-Measurement-and-Motion-Trend-Analyzer-1/Core/startup" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/hc_sr04.cyclo ./Core/Src/hc_sr04.d ./Core/Src/hc_sr04.o ./Core/Src/hc_sr04.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/motion.cyclo ./Core/Src/motion.d ./Core/Src/motion.o ./Core/Src/motion.su ./Core/Src/system.cyclo ./Core/Src/system.d ./Core/Src/system.o ./Core/Src/system.su ./Core/Src/uart.cyclo ./Core/Src/uart.d ./Core/Src/uart.o ./Core/Src/uart.su

.PHONY: clean-Core-2f-Src

