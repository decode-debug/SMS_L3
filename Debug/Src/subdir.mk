################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/delay.c \
../Src/main.c \
../Src/stm32f10x_it.c \
../Src/syscalls.c \
../Src/sysmem.c 

OBJS += \
./Src/delay.o \
./Src/main.o \
./Src/stm32f10x_it.o \
./Src/syscalls.o \
./Src/sysmem.o 

C_DEPS += \
./Src/delay.d \
./Src/main.d \
./Src/stm32f10x_it.d \
./Src/syscalls.d \
./Src/sysmem.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103VBTx -DUSE_STDPERIPH_DRIVER -DSTM32F10X_MD -c -I../Inc -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x" -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Libraries/CMSIS/CM3/CoreSupport" -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Libraries/STM32F10x_StdPeriph_Driver/inc" -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Drivers/LCD1602" -I"C:/Users/juzio/STM32CubeIDE/workspace_1.16.1/SMS_L3/Libraries/modbus" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/delay.cyclo ./Src/delay.d ./Src/delay.o ./Src/delay.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/stm32f10x_it.cyclo ./Src/stm32f10x_it.d ./Src/stm32f10x_it.o ./Src/stm32f10x_it.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su

.PHONY: clean-Src

