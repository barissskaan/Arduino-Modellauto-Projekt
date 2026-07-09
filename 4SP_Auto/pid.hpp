/////////////////////////////////////////////////////////
///// This file does not need any modification, /////////
///// if needed it can be extended //////////////////////
///////////////////////////////////////////////////////// 
#pragma once

struct pid_settings_t{
  float p;            //Proportional part of the PID
  float i;            //Integral part of the PID
  float d;            //Differential part of the PID
  float anti_windup;  //Max of integral
  float integral;     //Integration variable (for i component)
  float last_error;   //Last calucalted error (For d component)
};

float pid(pid_settings_t &s, float soll, float in);
void reset_integrator(pid_settings_t &s);