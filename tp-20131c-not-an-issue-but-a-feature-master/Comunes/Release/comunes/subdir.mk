################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../comunes/config.c \
../comunes/error.c \
../comunes/log.c \
../comunes/process.c \
../comunes/string.c \
../comunes/temporal.c \
../comunes/txt.c 

OBJS += \
./comunes/config.o \
./comunes/error.o \
./comunes/log.o \
./comunes/process.o \
./comunes/string.o \
./comunes/temporal.o \
./comunes/txt.o 

C_DEPS += \
./comunes/config.d \
./comunes/error.d \
./comunes/log.d \
./comunes/process.d \
./comunes/string.d \
./comunes/temporal.d \
./comunes/txt.d 


# Each subdirectory must supply rules for building sources it contributes
comunes/%.o: ../comunes/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


