#ifndef DATETIME_H
#define DATETIME_H
// ****************************************************************************
//  datetime.h                                                    DB48X project
// ****************************************************************************
//
//   File Description:
//
//     Date and time related functions
//
//
//
//
//
//
//
//
// ****************************************************************************
//   (C) 2024 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the terms outlined in LICENSE.txt
// ****************************************************************************
//   This file is part of DB48X.
//
//   DB48X is free software: you can redistribute it and/or modify
//   it under the terms outlined in the LICENSE.txt file
//
//   DB48X is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// ****************************************************************************

#include "algebraic.h"
#include "command.h"
#include "dmcp.h"
#include "types.h"


// Convert time and date to their components
bool to_time(object_p time, tm_t &tm, bool error = true);
uint to_date(object_p date, dt_t &dt, tm_t &tm, bool error = true);
algebraic_p to_days(object_p days, bool error = true);

// Convert date value to Julian day number, or 0 if fails
algebraic_p julian_day_number(algebraic_p date, bool error = true);
ularge julian_day_number(int d, int m, int y);

// Convert Julian day number to date
algebraic_p date_from_julian_day(object_p jdn, bool error = true);

// Convert algebraic to HMS or DMS
algebraic_p to_hms_dms(algebraic_r x, cstring name);

// Convert the top of stack ot HMS or DMS unit
object::result to_hms_dms(cstring name);

// Convert a value from HMS or DMS input
algebraic_p from_hms_dms(algebraic_g x, cstring name);

// Convert the top of stack from HMS or DMS unit
object::result from_hms_dms(cstring name);

// Rendering
void render_time(renderer &r, algebraic_g &value,
                 cstring hrs, cstring min, cstring sec,
                 uint base, bool ampm);
size_t render_dms(renderer &r, algebraic_g value,
                  cstring deg, cstring min, cstring sec);
size_t render_date(renderer &r, algebraic_g date);

// Difference between two dates
algebraic_p days_between_dates(object_p date1, object_p date2, bool error=true);

// Adding and subtracting days form a date
algebraic_p days_after(object_p date1, object_p days, bool error = true);
algebraic_p days_before(object_p date1, object_p days, bool error = true);


// System date and time
COMMAND_DECLARE(Date,0);        // Return current date
COMMAND_DECLARE(SetDate,1);     // Set current date
COMMAND_DECLARE(Time,0);        // Return current time
COMMAND_DECLARE(SetTime,1);     // Set current time
COMMAND_DECLARE(DateTime,0);    // Return current date and time
COMMAND_DECLARE(ChronoTime,0);  // Return current date and time
COMMAND_DECLARE(TimedEval,1);   // Timed evaluation

// HMS and DMS operations
COMMAND_DECLARE(ToHMS,1);       // Convert from decimal to H:MM:SS format
COMMAND_DECLARE(FromHMS,1);     // Convert from H:MM:SS format to decimal
COMMAND_DECLARE(ToDMS,1);       // Convert from decimal to H:MM:SS format
COMMAND_DECLARE(FromDMS,1);     // Convert from H:MM:SS format to decimal
COMMAND_DECLARE(HMSAdd,2);      // Add numbers in HMS format
COMMAND_DECLARE(HMSSub,2);      // Subtract numbers in HMS format
COMMAND_DECLARE(DMSAdd,2);      // Add numbers in HMS format
COMMAND_DECLARE(DMSSub,2);      // Subtract numbers in HMS format
COMMAND_DECLARE(DateAdd,2);     // Add a date and a number of days
COMMAND_DECLARE(DateSub,2);     // Count days between two dates
COMMAND_DECLARE(JulianDayNumber,1);// Return JDN for given date
COMMAND_DECLARE(DateFromJulianDayNumber,1); // Date from JDN
COMMAND_DECLARE(InsertHms,-1);  // Insert _hms, optionnaly replacing _dms

#endif // DATETIME_H
