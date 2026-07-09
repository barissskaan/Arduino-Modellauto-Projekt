/////////////////////////////////////////////////////////
///// This file does not need any modification, /////////
///// if needed it can be extended //////////////////////
///////////////////////////////////////////////////////// 
#pragma once

struct servo_settings_t{
  unsigned int pin;                  //Pin the servo is connected to
  unsigned int full_left;   //value for full left
  unsigned int full_right;  //value for full right
  unsigned int center;     //value for center position
};

void servo_setup(servo_settings_t &s);
void servo_set_position(servo_settings_t &s, int pos);
void servo_test(servo_settings_t &s);