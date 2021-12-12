// Include necessary libraries
#include <SPI.h>              // SPI communication library
#include <Adafruit_ILI9341.h> // Display library
#include <TouchScreen.h>      // Display touch functionality

// constant definition
#define INTERVAL200 200         // set interval 200ms
#define INTERVAL400 400         // set interval 400ms
#define INTERVAL800 800         // set interval 800ms
#define INTERVAL1000 1000       // set interval 1000ms 
// declare variables
extern volatile uint32_t pwmDuration;

long prev_time;                 // previous Time

bool b_state_overvoltage = 0;   // bool state WarningOverVoltage FALSE
bool b_state_undervoltage = 0;  // bool state WarningUnderVoltage FALSE
bool b_state_overtemp = 0;      // bool state WarningOverTemp FALSE
bool b_state_overcurrent = 0;   // bool state OverCurrent FALSE
long currenttime_status;        // current Time status
long currenttime_v;             // current Time Voltage
long currenttime_t;             // current Time Temp
long currenttime_c;             // current Time Current
long currenttime_s;             // current Time Shutdown
long currenttime_d;             // current Time Display
long currenttime_cb200;         // current Time Cell-Balancing 200ms
long currenttime_cb400;         // current Time Cell-Balancing 400ms

bool b[5]={0,0,0,0,0}; // bool Array first round definition

// global variables for Cell-Balancing
float difference_mean_cells_000 [4];  // first measurement
float difference_mean_cells_200 [4];  // second measurement
float difference_mean_cells_400 [4];  // third (actual) measurement 
float standard_deviation[3];          // standard deviation = { <first measurement> , <second measurement> , < third (actual) measurement> }  
float sum_sd;                         // for calculating standard deviation
int counter = 1;                      // counter for Cell-Balancing "200ms"
float cell_voltage [4];               // array storage for cell voltage / global for testing
bool Balancing;                       // for testing: true = Balancing ON, false = Balancing OFF
float voltage_limits [2];             // for testing: voltage limits; structure: {min, max}


void setup() 
{
  setupBSW();
  prev_time = millis();       // initialize millis-Timer
}

void loop() 
{ 
  setBDU_Activation(true);   // schaltet BDU ein
  setDriveMode(1);           // 1-Cycle Test 2-Slow Driver 3-Fast Driver 4-Power Mode
  receiveAndParseCommands();   // Empfängt Befehle über den Serial Monitor und führt diese aus

  //----------------------------------------------------------------------------------------------------------------------------
  Cell_Balancing(prev_time);
  Display(prev_time);
  Shutdown(prev_time, b_state_overvoltage, b_state_undervoltage, b_state_overtemp, b_state_overcurrent);
  Current(prev_time, b_state_overcurrent);  
  Temperature(prev_time, b_state_overtemp);
  Voltage(prev_time, b_state_overvoltage, b_state_undervoltage);
  
 //----------------------------------------------------------------------------------------------------------------------------

}
