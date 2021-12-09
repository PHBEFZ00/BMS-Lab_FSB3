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
