################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/cl_ser_pid_kal_rasp.cpp \
../src/kalman-filter.cpp \
../src/pid.cpp 

OBJS += \
./src/cl_ser_pid_kal_rasp.o \
./src/kalman-filter.o \
./src/pid.o 

CPP_DEPS += \
./src/cl_ser_pid_kal_rasp.d \
./src/kalman-filter.d \
./src/pid.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


