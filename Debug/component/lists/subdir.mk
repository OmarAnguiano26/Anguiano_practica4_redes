################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../component/lists/fsl_component_generic_list.c 

C_DEPS += \
./component/lists/fsl_component_generic_list.d 

OBJS += \
./component/lists/fsl_component_generic_list.o 


# Each subdirectory must supply rules for building sources it contributes
component/lists/%.o: ../component/lists/%.c component/lists/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MK64FN1M0VLL12 -DCPU_MK64FN1M0VLL12_cm4 -DFRDM_K64F -DFREEDOM -DMCUXPRESSO_SDK -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -I"C:\Users\Omar_PC\Documents\MCUXpressoIDE_11.8.0_1165\workspace\frdmk64f_uart_hardware_flow_control\source" -I"C:\Users\Omar_PC\Documents\MCUXpressoIDE_11.8.0_1165\workspace\frdmk64f_uart_hardware_flow_control\utilities" -I"C:\Users\Omar_PC\Documents\MCUXpressoIDE_11.8.0_1165\workspace\frdmk64f_uart_hardware_flow_control\drivers" -I"C:\Users\Omar_PC\Documents\MCUXpressoIDE_11.8.0_1165\workspace\frdmk64f_uart_hardware_flow_control\device" -I"C:\Users\Omar_PC\Documents\MCUXpressoIDE_11.8.0_1165\workspace\frdmk64f_uart_hardware_flow_control\component\uart" -I"C:\Users\Omar_PC\Documents\MCUXpressoIDE_11.8.0_1165\workspace\frdmk64f_uart_hardware_flow_control\component\lists" -I"C:\Users\Omar_PC\Documents\MCUXpressoIDE_11.8.0_1165\workspace\frdmk64f_uart_hardware_flow_control\CMSIS" -I"C:\Users\Omar_PC\Documents\MCUXpressoIDE_11.8.0_1165\workspace\frdmk64f_uart_hardware_flow_control\board" -I"C:\Users\Omar_PC\Documents\MCUXpressoIDE_11.8.0_1165\workspace\frdmk64f_uart_hardware_flow_control\frdmk64f\driver_examples\uart\hardware_flow_control" -O0 -fno-common -g3 -c -ffunction-sections -fdata-sections -ffreestanding -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-component-2f-lists

clean-component-2f-lists:
	-$(RM) ./component/lists/fsl_component_generic_list.d ./component/lists/fsl_component_generic_list.o

.PHONY: clean-component-2f-lists

