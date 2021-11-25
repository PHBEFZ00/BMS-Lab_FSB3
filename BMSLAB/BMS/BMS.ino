// Include necessary libraries
#include <SPI.h>              // SPI communication library
#include <Adafruit_ILI9341.h> // Display library
#include <TouchScreen.h>      // Display touch functionality

extern volatile uint32_t pwmDuration;

int gesamtzeit=0;

void cell_balancing()
{
  static uint32_t zeit; zeit=millis();
  

  //static uint32_t oldtimebalancing;

  //if ((millis()-oldtimebalancing)>400)
  //{
    //oldtimebalancing=millis();

    //float max_differencevoltage=0.2;
    float max_differencevoltage; float standard_deviation;//Berechnung Standardabweichung aktivieren!!
    float arithmetic_mean;
    float cell_voltage [4];
    float difference_mean_cells [4];
    float difference_mean_cells_before [4];
    int cell;
    bool setter;
    float sum=0;

    

    for (cell=0;cell<4;cell++) //Get cell voltages
    {
      cell_voltage[cell]=getCellVoltage(cell+1);
      sum=sum+cell_voltage[cell];
    }
    
    arithmetic_mean=sum/4.0;
    static uint32_t micro; micro=micros();
    
    int i;
    float hinterer_teil; float summe=0;
    for (i=0;i<4;i++)
    {
      hinterer_teil=(cell_voltage[i]-arithmetic_mean)*(cell_voltage[i]-arithmetic_mean);
      summe = summe + hinterer_teil;
    }
    
    standard_deviation=sqrt(1.0/(4.0-1.0)*summe);
    Serial.println(standard_deviation);
    max_differencevoltage=standard_deviation;
    Serial.print("Standardabweichung: "); Serial.println(micros()-micro);
    
    for (cell=0;cell<4;cell++) //Calculation of all differences to the arithmetic mean value
    {
      difference_mean_cells[cell]=abs(arithmetic_mean-cell_voltage[cell]);
    }
    
    for (cell=0;cell<4;cell++)
    { 
      if (difference_mean_cells[cell]>=max_differencevoltage)
      {
        
        if (difference_mean_cells_before[cell]>=max_differencevoltage)
        {
          setBalancing(cell+1);
          Serial.println("Cell-Balancing aktiv");
          Serial.print("SetBalancing-Time: "); Serial.println(millis()-zeit);
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
            Serial.println("False");
          }
        }
        if (setter)
        {
          setBalancing(0);
          Serial.println("Cell-Balancing OFF");
        }
      }
      difference_mean_cells_before[cell]=difference_mean_cells[cell];
    }
  //}
  Serial.print("Durchlauf: "); Serial.println(millis()-zeit);
  gesamtzeit=gesamtzeit+(millis()-zeit); Serial.print("Gesamt: "); Serial.println(gesamtzeit);
}

void setup() 
{
  setupBSW();
}

static uint32_t oldtimebalancing;

void loop() 
{ 
  //setBDU_Activation(true);   // schaltet BDU ein
  setDriveMode(1);           // 1-Cycle Test 2-Slow Driver 3-Fast Driver 4-Power Mode
  receiveAndParseCommands();   // Empfängt Befehle über den Serial Monitor und führt diese aus

  {
    if ((millis()-oldtimebalancing)>400)
    {
      cell_balancing();
      oldtimebalancing=millis();
    }
  }
  
  showMeasurementValues();   // Stellt Messwerte numerisch dar
  drawMeasurementCurves(10); // Messkurven - Parameter defines Minutes for full scale of X-Axis
  //showOCVcurve();          // Stellt OCV Kurve der Li-Ionen Zellen dar
}
