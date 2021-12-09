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
