################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../library/error.c \
../library/file_helper.c \
../library/wrap_sock.c \
../library/wrap_unix.c 

OBJS += \
./library/error.o \
./library/file_helper.o \
./library/wrap_sock.o \
./library/wrap_unix.o 

C_DEPS += \
./library/error.d \
./library/file_helper.d \
./library/wrap_sock.d \
./library/wrap_unix.d 


# Each subdirectory must supply rules for building sources it contributes
library/%.o: ../library/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


