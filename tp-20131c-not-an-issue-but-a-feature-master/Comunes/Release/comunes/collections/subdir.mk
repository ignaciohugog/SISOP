################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../comunes/collections/dictionary.c \
../comunes/collections/list.c \
../comunes/collections/queue.c 

OBJS += \
./comunes/collections/dictionary.o \
./comunes/collections/list.o \
./comunes/collections/queue.o 

C_DEPS += \
./comunes/collections/dictionary.d \
./comunes/collections/list.d \
./comunes/collections/queue.d 


# Each subdirectory must supply rules for building sources it contributes
comunes/collections/%.o: ../comunes/collections/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


