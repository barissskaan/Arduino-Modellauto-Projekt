#include "sensor.hpp"
#include <stdlib.h>
#include "Arduino.h"

// The two induction sensors (Schwingkreis + amplifier + low-pass) deliver an
// analog voltage that the Arduino reads with its 10-bit ADC (0..1023).
// To suppress noise, every reading is fed into a moving average of AVG_SIZE
// samples; the averaged value is stored in s.value.

void sensor_setup(sensor_settings_t &s) {
  pinMode(s.pin, INPUT);
  s.value = 0;
  s.avg.pos = 0;
  s.avg.sum = 0;
  for (unsigned int i = 0; i < AVG_SIZE; i++) {
    s.avg.measurements[i] = 0;
  }
}

void sensor_read(sensor_settings_t &s) {
  unsigned int raw = analogRead(s.pin);

  // ring buffer: drop the oldest sample, add the newest one
  s.avg.sum -= s.avg.measurements[s.avg.pos];
  s.avg.measurements[s.avg.pos] = raw;
  s.avg.sum += raw;
  s.avg.pos = (s.avg.pos + 1) & (AVG_SIZE - 1);   // wrap around (AVG_SIZE is 2^n)

  s.value = s.avg.sum >> AVG_SHIFT;               // average = sum / AVG_SIZE
}
