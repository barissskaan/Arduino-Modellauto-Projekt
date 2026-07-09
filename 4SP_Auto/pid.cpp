#include "pid.hpp"

// Discrete PID controller, exactly in the form of the script (chapter 6, eq. 9):
//
//    u(k) = Kp*e(k) + Ki*T*SUM(e(i)) + Kd*(e(k) - e(k-1)) / T
//
// with e(k) = soll - in  (Regelabweichung), T = sampling period.
// The period T is folded into the gains Ki and Kd (config.h), which is why the
// main loop must call this at a CONSTANT rate (CONTROL_PERIOD_MS). That keeps
// the controller behaviour reproducible.
//
//   s.integral   holds the running sum SUM(e(i))
//   s.last_error holds e(k-1)
//   s.anti_windup limits the integral so it cannot grow without bound
float pid(pid_settings_t &s, float soll, float in) {
  float error = soll - in;                        // e(k)

  // I-part: accumulate the error, but keep it within +/- anti_windup
  s.integral += error;
  if (s.integral >  s.anti_windup) s.integral =  s.anti_windup;
  if (s.integral < -s.anti_windup) s.integral = -s.anti_windup;

  // D-part: change of the error compared to the previous step
  float derivative = error - s.last_error;
  s.last_error = error;

  // P + I + D
  return s.p * error + s.i * s.integral + s.d * derivative;
}

// Clear the controller state (e.g. before starting to drive), so that an old
// integral or derivative from a previous run does not disturb the start.
void reset_integrator(pid_settings_t &s) {
  s.integral   = 0.0;
  s.last_error = 0.0;
}
