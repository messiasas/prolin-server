################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
NOSTRIP += \
app.nostrip \


# All Target
all: server-prolin1.1

# Tool invocations
server-prolin1.1: $(NOSTRIP)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC Strip'
	"C:/Users/filipe.silva/prolin_sdk_win-3.5_/prolin_sdk_win-3.5/sdk/toolchains/arm-8.3.0/bin/arm-none-linux-gnueabi-strip" -g $(NOSTRIP) -o"server-prolin1.1"
	@echo 'Finished building target: $@'
	@echo ' '

app.nostrip: $(OBJS) $(USER_OBJS)
	@echo 'Invoking: GCC Linker'
	"C:/Users/filipe.silva/prolin_sdk_win-3.5_/prolin_sdk_win-3.5/sdk/toolchains/arm-8.3.0/bin/arm-none-linux-gnueabi-gcc" -o"app.nostrip" $(OBJS) $(USER_OBJS) $(LIBS) -L"C:/Users/filipe.silva/prolin_sdk_win-3.5_/prolin_sdk_win-3.5/sdk/platforms/prolin-dev-8.3.0/lib" -Wl,-rpath=//opt/lib,-rpath=./lib,-rpath=/system/lib -Wl,-rpath-link,"C:/Users/filipe.silva/prolin_sdk_win-3.5_/prolin_sdk_win-3.5/sdk/platforms/prolin-dev-8.3.0/lib" -losal -lcrypto -lssh2 -lcurl -lfreetype -lpng -lpthread -lxui -lts -L"C:/Users/filipe.silva/workspaces2/server-prolin1.1/lib"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(NOSTRIP) server-prolin1.1
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
