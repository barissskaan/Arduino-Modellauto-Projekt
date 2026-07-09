/////////////////////////////////////////////////////////
///// This file does not need any modification, /////////
///// if needed it can be extended with more handlers ///
///////////  like with the set_motor_speed //////////////
///////////////////////////////////////////////////////// 

#include "uart.hpp"

static struct  {
  char buffer[BUFFER_SIZE] = {0};
  uint8_t bufferPos = 0;
  servo_settings_t* servo = nullptr;
  motor_settings_t* motor = nullptr;
  system_settings_t* settings = nullptr;
} uart;

// Set the motor speed
void toggle_analog_plot(double in){
  Serial.print("Toggle analog plot");
  Serial.flush();
  uart.settings->plot_analog_readings = !uart.settings->plot_analog_readings;
}

// Set the motor speed
void set_motor_speed(double in){
  Serial.print("Try to set motor speed to: ");
  Serial.println(int(in));
  Serial.flush();
  if (uart.motor == nullptr)
    Serial.println("Motor not attached!");
  else
    motor_set_speed(*uart.motor, int(in));
}

// Set the position of the servo
void set_servo_pos(double in){
  Serial.print("Try to set servo position to: ");
  Serial.println(int(in));
  Serial.flush();
  if (uart.servo == nullptr)
    Serial.println("Servo not attached!");
  else
    servo_set_position(*uart.servo, int(in));
}

// Start autonomous driving (line following + speed control)
void start_driving(double in){
  Serial.println("Autonomous driving: START");
  Serial.flush();
  if (uart.settings == nullptr)
    Serial.println("Settings not attached!");
  else
    uart.settings->drive_enabled = true;
}

// Stop autonomous driving: back to manual/test mode and switch the motor off
void stop_driving(double in){
  Serial.println("Autonomous driving: STOP");
  Serial.flush();
  if (uart.settings != nullptr)
    uart.settings->drive_enabled = false;
  if (uart.motor != nullptr)
    motor_set_speed(*uart.motor, 0);
}

void help(double in){
  Serial.println("BEGIN OF HELP");
  Serial.println("");
  Serial.println("Ensure to set line termination to 'No Line Ending' if you want to send a command");  
  Serial.println("");
  Serial.println("Commands must be terminated with ;!");
  Serial.println("With set command the value comes immidiately after command!");
  Serial.println("Example: setServoPos100;");
  Serial.println("");
  Serial.println("Available commands:");
  Serial.println("setServoPos: set the position of servo (-100..100), manual mode");
  Serial.println("setMotorSpeed: set the motor speed (0..255), manual mode");
  Serial.println("toggleAnalogPlot: start and stop plot of analog readings");
  Serial.println("startDrive: start autonomous driving");
  Serial.println("stopDrive: stop autonomous driving (back to manual mode)");
  Serial.println("help: show help");
  Serial.println("");
  Serial.println("END OF HELP");
  Serial.flush();//wait for outgoing serial transmission to end
}

/// List of Commands
///
/// The following two arrays have to have the SAME number of elements each command corresponding to one function pointer

const char *callback_name[] = {"setServoPos","setMotorSpeed","toggleAnalogPlot","startDrive","stopDrive","help"};

/// List of function pointer from above

void (*callback_func[])(double in) = {set_servo_pos,set_motor_speed,toggle_analog_plot,start_driving,stop_driving,help};



//////////////////////////////////////////////////////
//////////before this only handler functions /////////
//////////////////////////////////////////////////////

void setup_uart(){
  Serial.begin(115200); // for serial communication
  while (!Serial){;} //wait for serial to be available
  help(0);
}

void uart_attach_settings(system_settings_t &settings) {
  uart.settings = &settings;
}

void uart_attach_servo(servo_settings_t &servo) {
  uart.servo = &servo;
}

void uart_attach_motor(motor_settings_t &motor) {
  uart.motor = &motor;
}

/// Callback function
///
/// A callback is a callable (see further down) accepted by a class or function, used to customize the current logic
/// depending on that callback.
/// One reason to use callbacks is to write generic code which is independant from the logic in the called function
/// and can be reused with different callbacks.
/// Many functions of the standard algorithms library algorithm use callbacks:

void handle_serial_input(){

  int callback_count = sizeof(callback_func)/sizeof(callback_func[0]);

    
  if(Serial.available()>0){
    char c = Serial.read();
    if (c != CMD_END && uart.bufferPos < BUFFER_SIZE) {
      uart.buffer[uart.bufferPos++] = c;
    } else {
      bool callback_found = false;
      for (int i = 0; i < callback_count; i++) {
        if (strncmp(callback_name[i], uart.buffer, strlen(callback_name[i])) == 0) {
          callback_found = true;
          double num = atof(uart.buffer+strlen(callback_name[i]));
          callback_func[i](num);
          break;
        }
        if (!callback_found) {
          Serial.println("Unkown call!");
          Serial.println("");
          Serial.println(uart.buffer);
          Serial.println("");
          Serial.println("Try to type 'help;' for help!");
        }

      }
      memset(uart.buffer,0,BUFFER_SIZE);
      uart.bufferPos = 0; 
    }
  }
}

