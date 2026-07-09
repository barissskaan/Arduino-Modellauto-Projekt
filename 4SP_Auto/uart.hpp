/////////////////////////////////////////////////////////
///// This file does not need any modification, /////////
///// if needed it can be extended //////////////////////
///////////////////////////////////////////////////////// 
#pragma once
#include "Arduino.h"
#include "servo.hpp"
#include "motor.hpp"
#include "system.hpp"



#define BUFFER_SIZE 50
#define CMD_END ';'


void setup_uart(void);
void handle_serial_input(void);
void uart_attach_servo(servo_settings_t &servo);
void uart_attach_motor(motor_settings_t &motor);
void uart_attach_settings(system_settings_t &settings);



