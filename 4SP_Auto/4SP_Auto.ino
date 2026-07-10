///////////////////////////////////////////////////////////////////////////////
//  4SP_Auto  -  Institutsprojekt 4. Semester: elektrisches Modellauto (RWTH)
//  ------------------------------------------------------------------------
//  Das Auto folgt einem stromdurchflossenen Leiter. Zwei Induktionssensoren
//  (links/rechts) messen das Feld; daraus regelt ein PID-Regler die LENKUNG.
//  Ein zweiter PID-Regler haelt ueber den Hall-Geschwindigkeitssensor die
//  GESCHWINDIGKEIT konstant (der DC/DC-Wandler stellt die Motorspannung).
//
//  ==> Alle einstellbaren Werte (Pins, Kalibrierung, Reglerparameter) stehen
//      in  config.h .  Diese Datei hier muss man normalerweise nicht aendern.
//
//  Bedienung ueber den Serial Monitor (115200 Baud, "No Line Ending"):
//     help;              Hilfe anzeigen
//     setServoPos<n>;    Servo testen, n = -100..100  (manueller Modus)
//     setMotorSpeed<n>;  Motor  testen, n = 0..255    (manueller Modus)
//     toggleAnalogPlot;  Sensor-Rohwerte fuer den Serial Plotter an/aus
//     startDrive;        autonomes Fahren STARTEN
//     stopDrive;         autonomes Fahren STOPPEN (zurueck in den Testmodus)
///////////////////////////////////////////////////////////////////////////////

#include "Arduino.h"
#include "config.h"
#include "motor.hpp"
#include "pid.hpp"
#include "sensor.hpp"
#include "servo.hpp"
#include "uart.hpp"
#include "system.hpp"
#include "speed_sens.hpp"

//the entries should not rearranged (compiler error)
servo_settings_t servo = {
  .pin = PIN_SERVO,
  .full_left = SERVO_FULL_LEFT,
  .full_right = SERVO_FULL_RIGHT,
  .center = SERVO_CENTER,
};

//the entries should not rearranged (compiler error)
motor_settings_t motor = {
  .pin = PIN_MOTOR,
  .max_speed = MOTOR_MAX_SPEED,
  .min_speed = MOTOR_MIN_SPEED
};

//the entries should not rearranged (compiler error)
sensor_settings_t left_sensor = {
  .pin = PIN_SENSOR_LEFT,
  .value = 0,
  .avg = {}
};

//the entries should not rearranged (compiler error)
sensor_settings_t right_sensor = {
  .pin = PIN_SENSOR_RIGHT,
  .value = 0,
  .avg = {}
};

//the entries should not rearranged (compiler error)
pid_settings_t direction_control = {
  .p = STEER_KP,
  .i = STEER_KI,
  .d = STEER_KD,
  .anti_windup = STEER_ANTIWINDUP,
  .integral = 0.0,
  .last_error = 0.0
};

//the entries should not rearranged (compiler error)
pid_settings_t speed_control = {
  .p = SPEED_KP,
  .i = SPEED_KI,
  .d = SPEED_KD,
  .anti_windup = SPEED_ANTIWINDUP,
  .integral = 0.0,
  .last_error = 0.0
};

//the entries should not rearranged (compiler error)
system_settings_t settings = {
  .off_track_detection = OFF_TRACK_THRESHOLD,
  .idle_speed = TARGET_SPEED,
  .plot_analog_readings = PLOT_ANALOG,
  .drive_enabled = true          // start in manual/test mode for safety
};

speed_sense_settings_t speed_sense = {
  .pin = PIN_SPEED_SENSOR,
  .time_diff = 0,
  .last_time = 0
};

// last steering command; kept so the car can hold its turn when it briefly
// loses the wire (off-track).
float last_direction = 0.0;

// latest internal values, kept so the debug view (toggleAnalogPlot) can show
// what the controllers are doing. Very handy for tuning and for finding bugs.
float last_speed    = 0.0;   // measured speed (pulses per second)
int   last_pwm      = 0;     // motor PWM the speed controller commanded
bool  last_on_track = false; // does the car currently see the wire?

void setup() {
  setup_uart();

  servo_setup(servo);
  motor_setup(motor);
  sensor_setup(left_sensor);
  sensor_setup(right_sensor);
  setup_speed_sense(speed_sense);

  reset_integrator(direction_control);
  reset_integrator(speed_control);

  // give the serial commands access to the real objects
  uart_attach_servo(servo);
  uart_attach_motor(motor);
  uart_attach_settings(settings);
}

// interrupt for measuring speed from speed sensor
// -> triggered once per revolution: measure the time since the last edge.
void speed_interrupt() {
  unsigned long now = micros();
  speed_sense.time_diff = now - speed_sense.last_time;   // duration of one revolution
  speed_sense.last_time = now;
}

// Current speed in "pulses per second" (~ revolutions/s). Returns 0 when no
// pulse arrived for a while, i.e. the car is standing still.
float read_speed() {
  // time_diff / last_time are written inside the interrupt -> read atomically
  noInterrupts();
  unsigned long dt = speed_sense.time_diff;
  unsigned long lt = speed_sense.last_time;
  interrupts();

  if (dt == 0 || (micros() - lt) > SPEED_TIMEOUT_US) {
    return 0.0;
  }
  return SPEED_PULSE_SCALE / (float)dt;
}

void loop() {
  static unsigned long last_control = 0;
  float direction = last_direction;

  // ---- run the two controllers at a fixed period T (important for the PID) ----
  if (millis() - last_control >= CONTROL_PERIOD_MS) {
    last_control += CONTROL_PERIOD_MS;

    // read both line sensors (also needed for the debug view / for tuning)
    sensor_read(left_sensor);
    sensor_read(right_sensor);

    // update the values we want to watch (also in test mode: e.g. check the
    // speed sensor by spinning the wheels with setMotorSpeed and reading Speed)
    last_speed = read_speed();
    last_on_track = (left_sensor.value  > settings.off_track_detection) ||
                    (right_sensor.value > settings.off_track_detection);

    if (settings.drive_enabled) {
      // ============================ LENKUNG ============================
      if (last_on_track) {
        // difference of the two coils; setpoint 0 = wire centered under the car.
        // (left - right): a positive controller output steers toward the side
        // with the stronger signal. Flip STEER_SIGN in config.h if reversed.
        float measured = (float)left_sensor.value - (float)right_sensor.value;
        // setpoint = STEER_CENTER_OFFSET so that "wire centered" counts as straight
        direction = STEER_SIGN * pid(direction_control, (float)STEER_CENTER_OFFSET, measured);
        direction = constrain(direction, -100, 100);
        last_direction = direction;
      } else {
        // wire lost -> keep the last steering so the car curves back to it
        direction = last_direction;
      }
      servo_set_position(servo, (int)direction);

      // ========================= GESCHWINDIGKEIT ======================
      float pwm = pid(speed_control, (float)settings.idle_speed, last_speed);
      pwm = constrain(pwm, 0, MOTOR_MAX_SPEED);
      last_pwm = (int)pwm;
      motor_set_speed(motor, (unsigned int)pwm);

    } else {
      // ---- manual / test mode ----
      // Do NOT touch servo & motor here, so the serial commands
      // setServoPos.. / setMotorSpeed.. work for calibration.
      // Keep the controllers reset so autonomous driving starts clean.
      last_pwm = 0;
      reset_integrator(direction_control);
      reset_integrator(speed_control);
    }
  }

  // ---- sensor readout for calibration + debug ----
  // Streams "Left / Right / Diff / OnTrack" so you can read the sensor values
  // in the Serial Monitor. Diff = Left - Right is exactly the steering error:
  // with the wire centered under the car it should be close to 0.
  // Prints ~5x per second (comfortable to read). On/off via PLOT_ANALOG in
  // config.h or the toggleAnalogPlot command.
  static unsigned long last_plot = 0;
  if (settings.plot_analog_readings && (millis() - last_plot >= 200)) {
    last_plot = millis();
    int diff = (int)left_sensor.value - (int)right_sensor.value;
    Serial.print("Left:");      Serial.print(left_sensor.value);
    Serial.print("\tRight:");   Serial.print(right_sensor.value);
    Serial.print("\tDiff:");    Serial.print(diff);
    Serial.print("\tErr:");     Serial.print(diff - (STEER_CENTER_OFFSET));  // ~0 when centered
    Serial.print("\tOnTrack:"); Serial.println(last_on_track ? 1 : 0);
  }

  handle_serial_input();
}
