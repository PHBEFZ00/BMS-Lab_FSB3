// Include necessary libraries
#include <SPI.h>              // SPI communication library
#include <Adafruit_ILI9341.h> // Display library
#include <TouchScreen.h>      // Display touch functionality

//Konstantendefinition
#define INTERVAL200 200         // set getVoltage interval 200ms
#define INTERVAL1000 1000   // set Display interval 1000ms 
#define INTERVAL400 400         // set check interval 400ms
#define INTERVAL800 800 // set OverVoltageInterval 800ms

// Declare variables
extern volatile uint32_t pwmDuration;

long prev_time;      // previous Time getVolage
long prev_time_display;

bool b_state_overvoltage = 0;
bool b_state_undervoltage = 0;
bool b_state_overtemp = 0;   // status Temperature
bool b_state_overcurrent = 0;

long currenttime_status;
long currenttime_v;
long currenttime_t;
long currenttime_c;
long currenttime_s;
long currenttime_d;
long currenttime_cb;

bool b[5]={0,0,0,0,0}; //First Round Definition

/*----------------------------------------------------------------------------------------------
    Temperature
----------------------------------------------------------------------------------------------*/

void Temperature(long firsttime, bool &b_state_ot)
{
  float ct_act = 0;               // current cell temperature
  int i = 0;

  if (! b[0])
  {
    currenttime_t = firsttime;
    b[0]++;
    Serial.println("bugfix temp");
  }

  if ((millis() - currenttime_status) >= INTERVAL400 && b_state_ot) // 400ms
  {
    setWarningOvertemp(false);

    Serial.print(millis() - currenttime_status);
    Serial.println("   Intervall OV");

    b_state_ot = false;
  } // end if

  if ((millis() - currenttime_t) >= INTERVAL200) // 200ms
  {
    b_state_ot = false;       // set state false --We have NOT  a Warning--
    Serial.print(millis() - currenttime_t);
    Serial.print("ms");
    Serial.println("   Intervall getTemperature"); // time interval 200ms

    currenttime_t = millis();

    for (i = 1; i <= 4; i++)      // cell 1 to 4
    {
      //BMS14
      ct_act = getCellTemp(i);    // read cell temp

      // BMS14 & BMS15 & BMS15.1
      if (ct_act > 60) // if OverTemp
      {
        setWarningOvertemp(true); // temperature warning to VCU
        Serial.print("Cell No. ");
        Serial.print(i);
        Serial.println("   Overtemp");
        b_state_ot = true;    // set state true --We have a Warning--
        currenttime_status = millis();
      } // end if
    } // end for
  } // end if
} // end void Temperature*/

/*----------------------------------------------------------------------------------------------
    Voltage
----------------------------------------------------------------------------------------------*/

void Voltage(long firsttime, bool &b_state_ov, bool &b_state_uv)
{
  float cv_act = 0;               // current cell voltage
  int i = 0;                      // for - counter

  if (! b[1])
  {
    currenttime_v = firsttime;
    b[1]++;
    Serial.println("bugfix volt");
  } //


  if ((millis() - currenttime_status) >= INTERVAL400 && b_state_ov) // 400ms
  {
    setWarningOvervoltage(false);

    Serial.print(millis() - currenttime_status);
    Serial.print("ms");
    Serial.println("   Intervall OverVoltage");

    b_state_ov = false;
  } // end if

  if ((millis() - currenttime_status) >= INTERVAL400 && b_state_uv) // 400ms
  {
    setWarningUndervoltage(false);

    Serial.print(millis() - currenttime_status);
    Serial.print("ms");
    Serial.println("   Intervall UnderVoltage");

    b_state_uv = false;
  } // end if

  if ((millis() - currenttime_v) >= INTERVAL200) // 200ms
  {
    Serial.print(millis() - currenttime_v);
    Serial.print("ms");
    Serial.println("   Intervall getVoltage"); // time interval 200ms

    currenttime_v = millis(); // Timer reset

    for (i = 1; i <= 4; i++)      // cell 1 to 4
    {
      //BMS11
      cv_act = getCellVoltage(i); // read cell voltage

      // BMS12
      if (cv_act > 4.2) // if OverVoltage
      {
        setWarningOvervoltage(true);        // over voltage warning to VCU
        b_state_ov = true;            // set boolean true

        Serial.print("Cell No. ");
        Serial.print(i);
        Serial.print("    ");
        Serial.print(b_state_ov);
        Serial.println("   Overvoltage");
        currenttime_status = millis();
      } // end if

      // BMS13
      if (cv_act < 2.5) // if UnderVoltage
      {
        setWarningUndervoltage(true);       // under voltage warning to VCU
        b_state_uv = true;

        Serial.print("Cell No. ");
        Serial.print(i);
        Serial.print("    ");
        Serial.print(b_state_uv);
        Serial.println("   Undervoltage");
        currenttime_status = millis();
      } // end if
    } // end for

  } // end if
} // end bool Voltage

/*----------------------------------------------------------------------------------------------
    Current
----------------------------------------------------------------------------------------------*/

void Current(long firsttime, bool &b_state_iBat)
{
  float iBat = 0;

  if (! b[2])
  {
    currenttime_c = firsttime;
    b[2]++;
    Serial.println("bugfix current");
  } //

  if ((millis() - currenttime_c) >= INTERVAL200) // 200ms
  {
    b_state_iBat = false;
    Serial.print(millis() - currenttime_c);
    Serial.print("ms");
    Serial.println("   Intervall getPackCurrent"); // time interval 200ms

    iBat = getPackCurrent();

    currenttime_c = millis(); // Timer reset

    if (iBat > 400)
    {
      b_state_iBat = true;       // set state true --We have a Warning--
    } // end if
  } // end if
} // end void Current

/*----------------------------------------------------------------------------------------------
    Shutdown
----------------------------------------------------------------------------------------------*/

void Shutdown(long firsttime, bool b_state_ov, bool b_state_uv, bool b_state_ot, bool b_state_cu)
{

  if (b_state_ov || b_state_uv || b_state_ot || b_state_cu)
  {
    if (currenttime_s == 0)
    {
      currenttime_s = millis();
      Serial.println("-----------------Set Timer-------------------");
    }

    if (millis() - currenttime_s >= INTERVAL800 && currenttime_s != 0)
    {
      setBDU_Activation(false);
      Serial.print(currenttime_s);
      Serial.print("    ");
      Serial.print(millis() - currenttime_s);
      Serial.println("     ---------BDU Shutdown---------");
    } // end if
  } // end if

  if (! b_state_ov && ! b_state_uv && ! b_state_ot && ! b_state_cu)
  {
    currenttime_s = 0;
  } // end if

} // end void Shutdown

/*----------------------------------------------------------------------------------------------
    Display
----------------------------------------------------------------------------------------------*/

void Display(long firsttime)
{
  if (! b[3])
  {
    currenttime_d = firsttime;
    b[3]++;
    Serial.println("bugfix display");
  } // end if

  if (millis() - currenttime_d >= INTERVAL1000)
  {
    Serial.print(millis() - currenttime_d);
    Serial.print("ms");
    Serial.println("   Intervall Display");
    showMeasurementValues();      // Stellt Messwerte numerisch dar
    drawMeasurementCurves(10);  // Messkurven - Parameter defines Minutes for full scale of X-Axis
    //showOCVcurve();             // Stellt OCV Kurve der Li-Ionen Zellen dar
    currenttime_d = millis();      // Timer Reset
  } // end if
} // end void

/*----------------------------------------------------------------------------------------------
    Cell-Balancing
----------------------------------------------------------------------------------------------*/

void Cell_Balancing(long firsttime)
{
  float max_differencevoltage;
  float standard_deviation;
  float arithmetic_mean;
  float cell_voltage [4];
  float difference_mean_cells [4];
  float difference_mean_cells_400ms [4]; 
  int cell;
  bool setter; // for reseting setBalancing()
  float sum_am=0; // for calculating arithmetic mean
  
  
  if ( b[4] == 0 ) // first round puffer
      {
        b[4] = 1;
        Serial.println("bugfix Cell-Balancing");
        currenttime_cb = firsttime;
      }

  if (( millis() - currenttime_cb ) >= 400) // 400ms
  {
    Serial.print( millis() - currenttime_cb);
    Serial.print("ms");
    Serial.println("   Intervall Cellbalancing"); // time interval 200ms 
    for ( cell = 0 ; cell < 4 ; cell++ ) // get cell voltages
    {
      cell_voltage[cell] = getCellVoltage(cell+1);
      sum_am = sum_am + cell_voltage[cell];
    } // end get cell voltages
  
    arithmetic_mean = sum_am / 4.0; // calculation arithmetic mean

    // calculation standard deviation
    int i; // variable-count
    float subtotal_sd; // subtotal for calculating standard deviation
    float sum_sd=0;
  
    for ( i = 0 ; i < 4 ; i++)
    {
      subtotal_sd = ( cell_voltage[i] - arithmetic_mean ) * ( cell_voltage[i] - arithmetic_mean );
      sum_sd = sum_sd + subtotal_sd;
    }
  
    standard_deviation = sqrt ( 1.0 / ( 4.0 - 1.0 ) * sum_sd ); 
    //Serial.print("standard deviation: "); Serial.println(standard_deviation); // activate for standard deviation print
    max_differencevoltage = standard_deviation; // updating the maximum voltage difference (possibility to choose a fixed limit)
    // end of calculation standard deviation
  
    for ( cell = 0 ; cell < 4 ; cell++ ) // calculating all differences to the arithmetic mean value
    {
      difference_mean_cells[cell] = abs( arithmetic_mean - cell_voltage[cell] );
    } // end calculating all differences to the arithmetic mean value
  
    // start Cell-Balancing
    for (cell=0;cell<4;cell++)
    {
        if ( difference_mean_cells[cell] > max_differencevoltage )
        {
          if ( difference_mean_cells_400ms[cell] > max_differencevoltage )
          {
            setBalancing(cell+1);
            Serial.print(currenttime_cb);
            Serial.print("    ");
            Serial.print(millis() - currenttime_cb);
            Serial.println("     ----------Cell-Balancing----------");
            //Serial.println("Cell-Balancing aktiv");
            //Serial.print("SetBalancing-Time: "); Serial.println(millis()-zeit);
          }
         }
       
        else
        {
          int k;
          for (k=0;k<4;k++)
          {
            setter=true;
            if (difference_mean_cells[k]>=max_differencevoltage)
            {
              setter=false;
              //Serial.println("False");
            }
          }
          if (setter)
          {
            setBalancing(0);
            //Serial.println("Cell-Balancing OFF");
          }
        }
    difference_mean_cells_400ms[cell]=difference_mean_cells[cell];
      //Serial.print("Durchlauf: "); Serial.println(millis()-zeit);
    }
    currenttime_cb = millis();
  }
  //gesamtzeit=gesamtzeit+(millis()-zeit); Serial.print("Gesamt: "); Serial.println(gesamtzeit);
}

void setup() 
{
  setupBSW();
  prev_time = millis();       // initialize millis-Timer
}

static uint32_t oldtimebalancing=0;

void loop() 
{ 
  setBDU_Activation(true);   // schaltet BDU ein
  setDriveMode(1);           // 1-Cycle Test 2-Slow Driver 3-Fast Driver 4-Power Mode
  receiveAndParseCommands();   // Empfängt Befehle über den Serial Monitor und führt diese aus
  

  Temperature(prev_time, b_state_overtemp);

  Voltage(prev_time, b_state_overvoltage, b_state_undervoltage);

  Current(prev_time, b_state_overcurrent);
  
  Cell_Balancing(prev_time);

  Shutdown(prev_time, b_state_overvoltage, b_state_undervoltage, b_state_overtemp, b_state_overcurrent);

  // BMS41 refresh Display every 1000ms
  //Display(prev_time);
  
  //showOCVcurve();          // Stellt OCV Kurve der Li-Ionen Zellen dar
}
