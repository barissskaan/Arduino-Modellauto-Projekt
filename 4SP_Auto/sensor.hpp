//This file needs value modifications
#pragma once
// AVG_SHIFT sets the size of the moving-average window as a power of two:
//   AVG_SIZE = 2^AVG_SHIFT.  Bigger window = smoother but slower reaction.
//   3 -> window of 8 samples (good starting point). Keep it a power of two so
//   the division in sensor_read() can be a fast bit shift.
#define AVG_SHIFT 3
#define AVG_SIZE (1<<(AVG_SHIFT))

struct moving_average{
  unsigned int pos;           //Position within array
  unsigned int sum;           //Sum of values
  unsigned int measurements[AVG_SIZE]; //Array of measurements
};

struct sensor_settings_t{
  unsigned int pin;        //Pin the sensor is connected to
  unsigned int value;      //The averaged value of the sensor reading
  moving_average avg;      //Pointer to the moving average
};

void sensor_setup(sensor_settings_t &s);
void sensor_read(sensor_settings_t &s);
