#include "motor.hpp"
#include "Arduino.h"

// The motor speed is set through the DC/DC converter (buck / Tiefsetzsteller).
// The Arduino outputs a PWM signal; its duty cycle "a" sets the motor voltage
// U_M = a * U_batt and therefore the speed (see script section 2.5).
// -> analogWrite() with a value of 0..255 is exactly this duty cycle.

void motor_setup(motor_settings_t &s) {
  pinMode(s.pin, OUTPUT);
  analogWrite(s.pin, 0);   // start with the motor switched off
}

// speed: PWM duty 0..255. 0 = off. Any other value is clamped into the
// allowed window [min_speed, max_speed] (min_speed = motor starts to turn,
// max_speed = safety limit).
void motor_set_speed(motor_settings_t &s, unsigned int speed) {
  if (speed == 0) {
    analogWrite(s.pin, 0);
    return;
  }
  if (speed < s.min_speed) speed = s.min_speed;
  if (speed > s.max_speed) speed = s.max_speed;
  analogWrite(s.pin, speed);
}
