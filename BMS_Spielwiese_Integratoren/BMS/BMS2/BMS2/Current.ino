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
