/////////////////////////////////////////////////////////
///// This file does not need any modification, /////////
///// if needed it can be extended //////////////////////
///////////////////////////////////////////////////////// 
#pragma once

struct motor_settings_t{
  unsigned int pin;          //Pin the motor is connected to
  unsigned int max_speed;    //Max speed setting
  unsigned int min_speed;    //Min speed setting
};

void motor_setup(motor_settings_t &s);
void motor_set_speed(motor_settings_t &s, unsigned int speed);



