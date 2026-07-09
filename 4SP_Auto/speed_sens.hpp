/////////////////////////////////////////////////////////
///// This file does not need any modification, /////////
///// if needed it can be extended //////////////////////
///////////////////////////////////////////////////////// 
#pragma once

struct speed_sense_settings_t{
  unsigned int pin;        //Pin the sensor is connected to
  unsigned int time_diff;  //Difference since last interrupt
  unsigned long last_time; //Last timestamp
};

//Interrupt function to enable the speed measurement
extern void speed_interrupt();

void setup_speed_sense(speed_sense_settings_t &s);