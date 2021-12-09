// Include necessary libraries
#include <SPI.h>              // SPI communication library
#include <Adafruit_ILI9341.h> // Display library
#include <TouchScreen.h>      // Display touch functionality

// constant definition
#define INTERVAL200 200         // set interval 200ms
#define INTERVAL1000 1000       // set interval 1000ms 
#define INTERVAL400 400         // set interval 400ms
#define INTERVAL800 800         // set interval 800ms

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

//implementation function components

// BMS21
void Cell_Balancing(long firsttime)
{
  float arithmetic_mean;  // arithmetic mean
  int cell;               // variable-count for cells
  bool setter;            // bool for reseting setBalancing()
  float sum_am = 0;       // for calculating arithmetic mean

  if ( b[4] == 0 )                  // setup
  { 
    currenttime_cb200 = firsttime;  // set currenttime to firsttime
    currenttime_cb400 = firsttime;  // set currenttime to firsttime
    b[4]=1;                         

    // first round measurement & calculation 
    for ( cell = 0 ; cell < 4 ; cell++ )            // get cell voltages
    {
      cell_voltage[cell] = getCellVoltage(cell+1);  // write cell voltages in storage array
      sum_am = sum_am + cell_voltage[cell];         // sum of all voltages
    } // end get cell voltages
  
    arithmetic_mean = sum_am / 4.0;                 // calculation arithmetic mean

    // calculation standard deviation
    float subtotal_sd;                              // subtotal for calculating standard deviation
    for ( cell = 0 ; cell < 4 ; cell++ )
    {
      subtotal_sd = ( cell_voltage[cell] - arithmetic_mean ) * ( cell_voltage[cell] - arithmetic_mean ); // formula standard deviation
      sum_sd = sum_sd + subtotal_sd;                                                                     // formula standard deviation
    }
    
    standard_deviation[0] = sqrt ( 1.0 / ( 4.0 - 1.0 ) * sum_sd ); // standard deviation for first measurement
    
    for ( cell = 0 ; cell < 4 ; cell++ )                           // calculate cell voltage difference to arithmetic mean
    {
      difference_mean_cells_000[cell] = abs( arithmetic_mean - cell_voltage[cell] ); 
    }

    voltage_limits [0] = arithmetic_mean - standard_deviation [0];      // for testing: lower limit (first measurement)
    voltage_limits [1] = arithmetic_mean + standard_deviation [0];      // for testing: upper limit (first measurement)

  }

  if (( millis() - currenttime_cb200 ) >= INTERVAL200 ) // 200ms
  {
    //Serial.print( millis() - currenttime_cb200);
    //Serial.print("ms");
    //Serial.println("   Intervall Cellbalancing getVoltage");
    
    for ( cell = 0 ; cell < 4 ; cell++ )            // get cell voltages
    {
      cell_voltage[cell] = getCellVoltage(cell+1);  // write cell voltages in storage array
      sum_am = sum_am + cell_voltage[cell];         // sum of all voltages
    } // end for
  
    arithmetic_mean = sum_am / 4.0;                 // calculation arithmetic mean

    // calculation standard deviation
    float subtotal_sd;                              // subtotal for calculating standard deviation

    sum_sd = 0;                                     // set sum 0

    for ( cell = 0 ; cell < 4 ; cell++)
    {
      subtotal_sd = ( cell_voltage[cell] - arithmetic_mean ) * ( cell_voltage[cell] - arithmetic_mean ); // formula standard deviation
      sum_sd = sum_sd + subtotal_sd;                                                                     // formula standard deviation
    } // end for
    
    counter++;                                      // setup counter to jump in if()
    
    if ( (counter%2) == 0 )                         // jump in every second run / 400ms
    {
      standard_deviation[1] = sqrt ( 1.0 / ( 4.0 - 1.0 ) * sum_sd );  // standard deviation for second measurement
      
      for ( cell = 0 ; cell < 4 ; cell++ )                            // calculate cell voltage difference to arithmetic mean
      {
        difference_mean_cells_200[cell] = abs( arithmetic_mean - cell_voltage[cell] );
      } // end for

      voltage_limits [0] = arithmetic_mean - standard_deviation [1];      // for testing: lower limit (second measurement)
      voltage_limits [1] = arithmetic_mean + standard_deviation [1];      // for testing: upper limit (second measurement)

    } // end if
      
    currenttime_cb200 = millis();                                     // reinitialize timer
    
  } // end if

  if (( millis() - currenttime_cb400 ) >= INTERVAL400)                // start Cell-Balancing 400ms
  {
    //Serial.print( millis() - currenttime_cb400);
    //Serial.print("ms");
    //Serial.println("   Intervall Cellbalancing");

    standard_deviation[2] = sqrt( 1.0 / ( 4.0 - 1.0 ) * sum_sd );     // standard deviation for third (actual) measurement 

    for ( cell = 0 ; cell < 4 ; cell++ )                              // calculate cell voltage difference to arithmetic mean
    {
      difference_mean_cells_400[cell] = abs( arithmetic_mean - cell_voltage[cell] );
    } // end for

    voltage_limits [0] = arithmetic_mean - standard_deviation [2];      // for testing: lower limit (third measurement)
    voltage_limits [1] = arithmetic_mean + standard_deviation [2];      // for testing: upper limit (third measurement)
    
    for ( cell=0 ; cell < 4 ; cell++ )
    {
      if ( difference_mean_cells_400[cell] > standard_deviation[2] )     // check third (actual) measurement
      {
        if ( difference_mean_cells_200[cell] > standard_deviation[1] )   // check second measurement
        {
          if ( difference_mean_cells_000[cell] > standard_deviation[0] ) // check first measurement
          {
            setBalancing(cell+1); // activate Cell-Balancing
            Balancing = true;     // for testing: Balancing ON

            //Serial.print(currenttime_cb400);
            //Serial.print("    ");
            //Serial.print(millis() - currenttime_cb400);
            //Serial.print("     Cell-Balancing, cell no. "); Serial.println(cell+1);
          } // end if
        } // end if
      } // end if
    } // end for
    
    setter = true;                                                        // set every cell in correct range
    
    for ( cell = 0 ; cell < 4 ; cell++ ) 
    {
      if (difference_mean_cells_400[cell] > standard_deviation[2])        // check if cell in correct range
      {
        setter=false;                                                     // continue Cell-Balancing
      } // end if
    } // end for
    
     if (setter)                                                          // if in correct range
     {
      setBalancing(0);                                                    // deactivate Cell-Balancing
      Balancing = false;                                                  // for testing: Balancing OFF
      
      //Serial.println("Cell-Balancing OFF");
     } // end if
        
    // value actualisation
    standard_deviation[0] = standard_deviation[2];                        // set 3rd value from last loop to 1st value next loop
    
    for ( cell = 0 ; cell < 4 ; cell++ )
    {
      difference_mean_cells_000[cell] = difference_mean_cells_400[cell];  // set 3rd value from last loop to 1st value next loop
    } // end for
    
    currenttime_cb400 = millis();                                         // reinitialize timer
  } // end if
} // end void Cell_Balancing

// BMS16
void Current(long firsttime, bool &b_state_iBat)
{
  float i_Bat = 0;                       // current current

  if (! b[2])                           // setup
  {
    currenttime_c = firsttime;          // set currenttime to firsttime
    b[2]++;
  } // end if

  if ((millis() - currenttime_c) >= INTERVAL200) // 200ms
  {
    b_state_iBat = false;
    
    //Serial.print(millis() - currenttime_c);
    //Serial.print("ms");
    //Serial.println("   Intervall getPackCurrent");

    i_Bat = getPackCurrent();                    // get Batterypack current

    currenttime_c = millis();                   // reinitialize timer

    if (i_Bat > 400 || i_Bat < -400)
    {
      b_state_iBat = true;                      // set bool true
    } // end if
  } // end if
} // end void Current

// BMS41
void Display(long firsttime)
{
  if (! b[3])                                     // setup
  {
    currenttime_d = firsttime;                    // set currenttime to firsttime
    b[3]++;
  } // end if

  if (millis() - currenttime_d >= INTERVAL1000)   // 1000ms
  {
    //Serial.print(millis() - currenttime_d);
    //Serial.print("ms");
    //Serial.println("   Intervall Display");

    showMeasurementValues();                      // show values with defined resolution
    drawMeasurementCurves(10);                    // draw curves - parameter defines minutes for full scale of x-Axis

    //showOCVcurve();                             // show OCV (open circuit voltage) curve of Li-Ion cells
    currenttime_d = millis();                     // reinitialize timer
  } // end if
} // end void Display

// BMS17.1 + BMS17.2 + BMS17.3 + BMS18
void Shutdown(long firsttime, bool b_state_ov, bool b_state_uv, bool b_state_ot, bool b_state_cu)
{
  if (b_state_ov || b_state_uv || b_state_ot || b_state_cu)                 // if warning active
  {
    if (currenttime_s == 0)                                                 // jump in timer
    {
      currenttime_s = millis();                                             // set timer to current time
      //Serial.println("-----------------Set Timer-------------------");
    }

    if (millis() - currenttime_s >= INTERVAL800 && currenttime_s != 0)      // 800ms and timer activated
    {
      setBDU_Activation(false);                                             // Shutdown BDU - BDU OFF
      
      //Serial.print(currenttime_s);
      //Serial.print("    ");
      //Serial.print(millis() - currenttime_s);
      //Serial.println("     ---------BDU Shutdown---------");
      
    } // end if
  } // end if

  if (! b_state_ov && ! b_state_uv && ! b_state_ot && ! b_state_cu)         // no warning active
  {
    currenttime_s = 0;                                                      // reset timer to be ready to jump in
  } // end if
} // end void Shutdown

// BMS14 + BMS15 + BMS15.1 + BMS15.2
void Temperature(long firsttime, bool &b_state_ot)
{
  float ct_act[4] = {0,0,0,0};          // current cell temperature
  int i = 0;                            // initialize counter

  if (! b[0])                           // setup 
  {
    currenttime_t = firsttime;          // set currenttime to firsttime
    b[0]++;
  }

  if ((millis() - currenttime_status) >= INTERVAL400 && b_state_ot)   // 400ms and bool true
  {
    setWarningOvertemp(false);    // turn off warning

    //Serial.print(millis() - currenttime_status);
    //Serial.println("   Intervall OverTemp OFF");

    b_state_ot = false;
  } // end if

  if ((millis() - currenttime_t) >= INTERVAL200)    // 200ms
  {
    
    //Serial.print(millis() - currenttime_t);
    //Serial.print("ms");
    //Serial.println("   Intervall getTemp");

    currenttime_t = millis();                       // reinitialize timer

    for (i = 1; i <= 4; i++)            // cell 1 to 4
    {
      //BMS14
      ct_act[i-1] = getCellTemp(i);     // read cell temp

      // BMS14 & BMS15 & BMS15.1
      if (ct_act[i-1] > 60)             // if OverTemp
      {
        setWarningOvertemp(true);       // temperature warning to VCU
        
        //Serial.print("Cell No. ");
        //Serial.print(i);
        //Serial.println("   Overtemp");
        
        b_state_ot = true;              // set bool true
        currenttime_status = millis();  // reinitialize timer
        
      } // end if
    } // end for
  } // end if
} // end void Temperature

// BMS11 + BMS12 + BMS12.1 + BMS12.2 + BMS13 + BMS13.1 + BMS13.2
void Voltage(long firsttime, bool &b_state_ov, bool &b_state_uv)
{
  float cv_act[4] = {0,0,0,0};        // current cell voltage
  int i = 0;                          // initialize counter

  if (! b[1])                         // setup
  {
    currenttime_v = firsttime;        // set currenttime to firsttime
    b[1]++;
  } // end if


  if ((millis() - currenttime_status) >= INTERVAL400 && b_state_ov) // 400ms and bool true
  {
    setWarningOvervoltage(false);        // turn off warning

    //Serial.print(millis() - currenttime_status);
    //Serial.println("   Intervall OverVoltage OFF");

    b_state_ov = false;
  } // end if

  if ((millis() - currenttime_status) >= INTERVAL400 && b_state_uv) // 400ms and bool true
  {
    setWarningUndervoltage(false);       // turn off warning

    //Serial.print(millis() - currenttime_status);
    //Serial.print("ms");
    //Serial.println("   Intervall UnderVoltage OFF");

    b_state_uv = false;
  } // end if

  if ((millis() - currenttime_v) >= INTERVAL200) // 200ms
  {
    //Serial.print(millis() - currenttime_v);
    //Serial.print("ms");
    //Serial.println("   Intervall getVoltage");

    currenttime_v = millis();               // set Timer to current time

    for (i = 1; i <= 4; i++)                // cell 1 to 4
    {
      //BMS11
      cv_act[i-1] = getCellVoltage(i);      // get cell voltage

      // BMS12
      if (cv_act[i-1] > 4.2)                // if OverVoltage
      {
        setWarningOvervoltage(true);        // over voltage warning to VCU
        b_state_ov = true;                  // set bool true

        //Serial.print("Cell No. ");
        //Serial.print(i);
        //Serial.print("    ");
        //Serial.print(b_state_ov);
        //Serial.println("   Overvoltage");
        
        currenttime_status = millis();      // reinitialize timer
      } // end if

      // BMS13
      if (cv_act[i-1] < 2.5)                // if UnderVoltage
      {
        setWarningUndervoltage(true);       // under voltage warning to VCU
        b_state_uv = true;                  // set bool true

        //Serial.print("Cell No. ");
        //Serial.print(i);
        //Serial.print("    ");
        //Serial.print(b_state_uv);
        //Serial.println("   Undervoltage");
        
        currenttime_status = millis();      // reinitialize timer
      } // end if
    } // end for
  } // end if
} // end void Voltage

//end function components



void setup() 
{
  setupBSW();
  prev_time = millis();       // initialize millis-Timer
}

void loop() 
{ 
  setBDU_Activation(true);   // schaltet BDU ein
  setDriveMode(4);           // 1-Cycle Test 2-Slow Driver 3-Fast Driver 4-Power Mode
  receiveAndParseCommands();   // Empfängt Befehle über den Serial Monitor und führt diese aus
  void Cell_Balancing(long prev_time);
  void Current(long prev_time, bool &b_state_iBat);
  Display(prev_time);
  void Shutdown(long prev_time, bool b_state_ov, bool b_state_uv, bool b_state_ot, bool b_state_cu);
  void Temperature(long prev_time, bool &b_state_ot);
  void Voltage(long prev_time, bool &b_state_ov, bool &b_state_uv);
}
