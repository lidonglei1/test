################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
modbus/mb.obj: ../modbus/mb.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"D:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.5/bin/cl430" -vmspx --abi=eabi --data_model=restricted --use_hw_mpy=F5 --include_path="D:/ti/ccsv6/ccs_base/msp430/include" --include_path="E:/ldl/prj/workspace_v6_1/EXP430FR5969/FreeModbus/modbus/rtu" --include_path="E:/ldl/prj/workspace_v6_1/EXP430FR5969/FreeModbus/port" --include_path="E:/ldl/prj/workspace_v6_1/EXP430FR5969/FreeModbus/modbus/include" --include_path="D:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.5/include" --advice:power=all --advice:hw_config=all -g --define=__MSP430FR5969__ --define=_MPU_ENABLE --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="modbus/mb.pp" --obj_directory="modbus" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


