#include "speed_sens.hpp"
#include "Arduino.h"
#include "config.h"

// The speed sensor is a latching Hall sensor (DRV5011). A magnet on the drive
// shaft passes it once per revolution, so its output toggles once per turn.
// We register an external interrupt on that pin; the interrupt routine
// speed_interrupt() (defined in the main .ino) then measures the time between
// two edges = the duration of one revolution.

void setup_speed_sense(speed_sense_settings_t &s) {
  pinMode(s.pin, INPUT);          // DRV5011 has a push-pull output (no pull-up needed)
  s.time_diff = 0;
  s.last_time = micros();
  attachInterrupt(digitalPinToInterrupt(s.pin), speed_interrupt, SPEED_SENSOR_EDGE);
}
