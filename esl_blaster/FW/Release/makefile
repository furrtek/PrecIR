################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Startup/subdir.mk
-include Src/subdir.mk
-include Middlewares/ST/STM32_USB_Device_Library/Core/Src/subdir.mk
-include Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/subdir.mk
-include Drivers/STM32F0xx_HAL_Driver/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
ESLBlaster.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
ESLBlaster.list \

OBJCOPY_BIN += \
ESLBlaster.bin \


# All Target
all: ESLBlaster.elf secondary-outputs

# Tool invocations
ESLBlaster.elf: $(OBJS) $(USER_OBJS) C:\Users\furrtek\Documents\Electro\ESLBlaster\FW\STM32F070F6PX_FLASH.ld
	arm-none-eabi-gcc -o "ESLBlaster.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m0 -T"C:\Users\furrtek\Documents\Electro\ESLBlaster\FW\STM32F070F6PX_FLASH.ld" --specs=nosys.specs -Wl,-Map="ESLBlaster.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

ESLBlaster.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "ESLBlaster.list"
	@echo 'Finished building: $@'
	@echo ' '

ESLBlaster.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "ESLBlaster.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
