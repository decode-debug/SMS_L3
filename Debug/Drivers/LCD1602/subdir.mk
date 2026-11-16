################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/LCD1602/lcd_hd44780.c 

OBJS += \
./Drivers/LCD1602/lcd_hd44780.o 

C_DEPS += \
./Drivers/LCD1602/lcd_hd44780.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/LCD1602/%.o Drivers/LCD1602/%.su Drivers/LCD1602/%.cyclo: ../Drivers/LCD1602/%.c Drivers/LCD1602/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103VBTx -DUSE_STDPERIPH_DRIVER -DSTM32F10X_MD -c -I../Inc -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x" -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Libraries/CMSIS/CM3/CoreSupport" -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Libraries/STM32F10x_StdPeriph_Driver/inc" -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Drivers/LCD1602" -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Libraries/modbus" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-LCD1602

clean-Drivers-2f-LCD1602:
	-$(RM) ./Drivers/LCD1602/lcd_hd44780.cyclo ./Drivers/LCD1602/lcd_hd44780.d ./Drivers/LCD1602/lcd_hd44780.o ./Drivers/LCD1602/lcd_hd44780.su

.PHONY: clean-Drivers-2f-LCD1602

