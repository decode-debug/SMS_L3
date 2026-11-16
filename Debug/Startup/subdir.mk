################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Startup/startup_stm32f103vbtx.s 

OBJS += \
./Startup/startup_stm32f103vbtx.o 

S_DEPS += \
./Startup/startup_stm32f103vbtx.d 


# Each subdirectory must supply rules for building sources it contributes
Startup/%.o: ../Startup/%.s Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m3 -g3 -DDEBUG -c -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x" -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Libraries/CMSIS/CM3/CoreSupport" -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Libraries/STM32F10x_StdPeriph_Driver/inc" -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Drivers/LCD1602" -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Libraries/modbus" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@" "$<"

clean: clean-Startup

clean-Startup:
	-$(RM) ./Startup/startup_stm32f103vbtx.d ./Startup/startup_stm32f103vbtx.o

.PHONY: clean-Startup

