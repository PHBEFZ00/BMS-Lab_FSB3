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
//__________________________________________________________________________________________________________________
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
      BDU_On = 0;                                            // Shutdown BDU - BDU OFF
      
      //Serial.print(currenttime_s);
      //Serial.print("    ");
      //Serial.print(millis() - currenttime_s);
      //Serial.println("     ---------BDU Shutdown---------");
      
    } // end if
  } // end if

  if (! b_state_ov && ! b_state_uv && ! b_state_ot && ! b_state_cu)         // no warning active
  {
    currenttime_s = 0;  // reset timer to be ready to jump in
    BDU_On = 1;
  } // end if
} // end void Shutdown
//________________________________________________________________________________________________________
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
//_______________________________________________________________________________________________
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
