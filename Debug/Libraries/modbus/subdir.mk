################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Libraries/modbus/crc16.c \
../Libraries/modbus/modbus.c 

OBJS += \
./Libraries/modbus/crc16.o \
./Libraries/modbus/modbus.o 

C_DEPS += \
./Libraries/modbus/crc16.d \
./Libraries/modbus/modbus.d 


# Each subdirectory must supply rules for building sources it contributes
Libraries/modbus/%.o Libraries/modbus/%.su Libraries/modbus/%.cyclo: ../Libraries/modbus/%.c Libraries/modbus/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103VBTx -DUSE_STDPERIPH_DRIVER -DSTM32F10X_MD -c -I../Inc -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x" -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Libraries/CMSIS/CM3/CoreSupport" -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Libraries/STM32F10x_StdPeriph_Driver/inc" -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Drivers/LCD1602" -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Libraries/modbus" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Libraries-2f-modbus

clean-Libraries-2f-modbus:
	-$(RM) ./Libraries/modbus/crc16.cyclo ./Libraries/modbus/crc16.d ./Libraries/modbus/crc16.o ./Libraries/modbus/crc16.su ./Libraries/modbus/modbus.cyclo ./Libraries/modbus/modbus.d ./Libraries/modbus/modbus.o ./Libraries/modbus/modbus.su

.PHONY: clean-Libraries-2f-modbus

