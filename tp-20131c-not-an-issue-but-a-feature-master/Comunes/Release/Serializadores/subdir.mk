################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Serializadores/Stream.c \
../Serializadores/serializadores.c 

OBJS += \
./Serializadores/Stream.o \
./Serializadores/serializadores.o 

C_DEPS += \
./Serializadores/Stream.d \
./Serializadores/serializadores.d 


# Each subdirectory must supply rules for building sources it contributes
Serializadores/%.o: ../Serializadores/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


