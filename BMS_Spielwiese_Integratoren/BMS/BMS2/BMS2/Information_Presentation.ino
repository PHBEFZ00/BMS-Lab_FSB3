
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
//---------------------------------------------------------------------------------------------------------------------
