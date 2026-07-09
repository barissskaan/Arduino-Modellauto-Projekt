#include "servo.hpp"
#include <Servo.h>
#include "Arduino.h"

Servo servo_object;

void servo_setup(servo_settings_t &s) {
  servo_object.attach(s.pin);
  servo_object.writeMicroseconds(s.center);   // start straight ahead
}

// pos: steering command from -100 (full left) over 0 (center) to +100 (full right).
// It is mapped linearly onto the calibrated pulse widths. Using center as the
// pivot means left and right can have different ranges without any problem.
void servo_set_position(servo_settings_t &s, int pos) {
  pos = constrain(pos, -100, 100);
  int us;
  if (pos >= 0) {
    us = map(pos, 0, 100, s.center, s.full_right);
  } else {
    us = map(pos, 0, -100, s.center, s.full_left);
  }
  servo_object.writeMicroseconds(us);
}

// Small mechanical self-test: center -> left -> center -> right -> center.
// Handy to check the steering once after wiring (call it from setup() if needed).
void servo_test(servo_settings_t &s) {
  servo_object.writeMicroseconds(s.center);      delay(500);
  servo_object.writeMicroseconds(s.full_left);   delay(500);
  servo_object.writeMicroseconds(s.center);      delay(500);
  servo_object.writeMicroseconds(s.full_right);  delay(500);
  servo_object.writeMicroseconds(s.center);      delay(500);
}
