/////////////////////////////////////////////////////////
///// This file does not need any modification, /////////
///// if needed it can be extended //////////////////////
/////////////////////////////////////////////////////////
#pragma once

struct system_settings_t{
  unsigned int off_track_detection;  // threshold: below this on BOTH sensors = off track
  unsigned int idle_speed;           // target speed for the speed controller
  bool plot_analog_readings;         // stream analog values to the Serial Plotter
  bool drive_enabled;                // false = manual/test mode, true = autonomous driving
};
