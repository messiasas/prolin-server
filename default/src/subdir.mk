################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/clientThread.c \
../src/file_manager.c \
../src/main.c \
../src/wifi.c 

OBJS += \
./src/clientThread.o \
./src/file_manager.o \
./src/main.o \
./src/wifi.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Compiler'
	"C:/Users/filipe.silva/prolin_sdk_win-3.5_/prolin_sdk_win-3.5/sdk/toolchains/arm-8.3.0/bin/arm-none-linux-gnueabi-gcc" -O0 -g2 -Wall -funwind-tables -I"C:/Users/filipe.silva/workspaces2/server-prolin1.1/inc" -I"C:/Users/filipe.silva/workspaces2/server-prolin1.1/src" -I"C:/Users/filipe.silva/prolin_sdk_win-3.5_/prolin_sdk_win-3.5/sdk/platforms/prolin-dev-8.3.0/include" -I"C:/Users/filipe.silva/prolin_sdk_win-3.5_/prolin_sdk_win-3.5/sdk/platforms/prolin-dev-8.3.0/include/freetype2" -I"C:/Users/filipe.silva/prolin_sdk_win-3.5_/prolin_sdk_win-3.5/sdk/toolchains/arm-8.3.0/arm-none-linux-gnueabi/sysroot/usr/include" -I"C:/Users/filipe.silva/prolin_sdk_win-3.5_/prolin_sdk_win-3.5/sdk/toolchains/arm-8.3.0/lib/gcc/arm-none-linux-gnueabi/8.3.0/include" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


