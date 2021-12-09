// BMS21
void Cell_Balancing(long firsttime)
{
  float arithmetic_mean;                                              // arithmetic mean
  int cell;                                                           // variable-count for cells
  bool setter;                                                        // bool for reseting setBalancing()
  float sum_am = 0;                                                   // for calculating arithmetic mean
  int balancing_cell;                                                 // storage for cell number
  float max_cell_voltage = 0;                                         // auxilary variable for determine max. cell voltage
  int cell_num;                                                       // second variable-count for cells

  if ( b[4] == 0 )                  // setup
  {
    currenttime_cb200 = firsttime;  // set currenttime to firsttime
    currenttime_cb400 = firsttime;  // set currenttime to firsttime
    b[4] = 1;

    // first round measurement & calculation
    for ( cell = 0 ; cell < 4 ; cell++ )            // get cell voltages
    {
      cell_voltage[cell] = getCellVoltage(cell + 1); // write cell voltages in storage array
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

    for ( cell = 0 ; cell < 4 ; cell++ )             // get cell voltages
    {
      cell_voltage[cell] = getCellVoltage(cell + 1); // write cell voltages in storage array
      sum_am = sum_am + cell_voltage[cell];          // sum of all voltages
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

    if ( (counter % 2) == 0 )                       // jump in every second run / 400ms
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

    bool balancing_on = false;                                          // auxilary variable for activating Cell-Balancing
    
    for ( cell = 0 ; cell < 4 ; cell++ )
    {
      if ( difference_mean_cells_400[cell] > standard_deviation[2] )     // check third (actual) measurement
      {
        if ( difference_mean_cells_200[cell] > standard_deviation[1] )   // check second measurement
        {
          if ( difference_mean_cells_000[cell] > standard_deviation[0] ) // check first measurement
          {
            for ( cell_num = 0 ; cell_num < 4 ; cell_num++ )             // algorithm for getting cell number of cell with max. voltage
            {
              if ( max_cell_voltage < cell_voltage[cell_num] )
              {
                
                max_cell_voltage = cell_voltage[cell_num];
                balancing_cell = cell_num + 1;
                
              } // end if
            } // end for
            
            balancing_on = true;                                         // bool true for activating Cell-Balancing
            
          } // end if
        } // end if
      } // end if
    } // end for
    
    if (balancing_on) // set Balancing                                                   
    {
      setBalancing(balancing_cell); // activate Cell-Balancing
      Balancing = true;             // for testing: Balancing ON
      
      //Serial.print(currenttime_cb400);
      //Serial.print("    ");
      //Serial.print(millis() - currenttime_cb400);
      //Serial.print("     Cell-Balancing, cell no. "); Serial.println(balancing_cell);
    } // end if
    
    setter = true;                                                        // set every cell in correct range

    for ( cell = 0 ; cell < 4 ; cell++ )
    {
      if (difference_mean_cells_400[cell] > standard_deviation[2])        // check if cell in correct range
      {
        setter = false;                                                   // continue Cell-Balancing
      } // end if
    } // end for

    if (setter)                                                           // if in correct range
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
