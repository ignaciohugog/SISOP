################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Orquestador.c \
../_inotify.c \
../comunes_plataforma.c \
../funciones.c \
../planificador.c \
../plataforma.c 

OBJS += \
./Orquestador.o \
./_inotify.o \
./comunes_plataforma.o \
./funciones.o \
./planificador.o \
./plataforma.o 

C_DEPS += \
./Orquestador.d \
./_inotify.d \
./comunes_plataforma.d \
./funciones.d \
./planificador.d \
./plataforma.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/tp-20131c-not-an-issue-but-a-feature/Comunes" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


