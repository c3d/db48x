// ****************************************************************************
//  datetime.cc                                                   DB48X project
// ****************************************************************************
//
//   File Description:
//
//    Date and time related functions and commands
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

#include "datetime.h"

#include "arithmetic.h"
#include "dmcp.h"
#include "program.h"
#include "renderer.h"
#include "tag.h"
#include "unit.h"


// ============================================================================
//
//   Date and time utilities
//
// ============================================================================

bool to_time(object_p tobj, tm_t &tm, bool error)
// ----------------------------------------------------------------------------
//   Convert a HH.MMSS time value to a time
// ----------------------------------------------------------------------------
{
    if (!tobj)
        return false;
    tobj = object::strip(tobj);

    algebraic_g time = nullptr;
    uint scale = 100;
    if (unit_p u = unit::get(tobj))
        if (object_p uexpr = u->uexpr())
            if (symbol_p sym = uexpr->as_quoted<symbol>())
                if (sym->matches("hms"))
                    time = u->value();
    if (time)
        scale = 60;
    else
        time = tobj->as_real();
    if (!time)
    {
        if (error)
            rt.type_error();
        return false;
    }

    algebraic_g factor = integer::make(scale);
    uint hour = time->as_uint32(0, false);
    time = (time * factor) % factor;
    uint min = time->as_uint32(0, false);
    time = (time * factor) % factor;
    uint sec = time->as_uint32(0, false);
    factor = integer::make(100);
    time = (time * factor) % factor;
    uint csec = time->as_uint32(0, false);
    if (hour >= 24 || min >= 60 || sec >= 60)
    {
        if (error)
            rt.invalid_time_error();
        return false;
    }
    tm.hour = hour;
    tm.min = min;
    tm.sec = sec;
    tm.csec = csec;

    return true;
}


uint to_date(object_p dtobj, dt_t &dt, tm_t &tm, bool error)
// ----------------------------------------------------------------------------
//   Convert a YYYYMMDD.HHMMSS value to a date and optional time
// ----------------------------------------------------------------------------
{
    if (!dtobj)
        return 0;
    dtobj = object::strip(dtobj);

    algebraic_g date = nullptr;
    if (unit_p u = unit::get(dtobj))
        if (object_p uexpr = u->uexpr())
            if (symbol_p sym = uexpr->as_quoted<symbol>())
                if (sym->matches("date"))
                    date = u->value();
    if (!date)
        date = dtobj->as_real();
    if (!date)
    {
        if (error)
            rt.type_error();
        return 0;
    }

    algebraic_g factor = integer::make(100);
    algebraic_g time = integer::make(1);
    time = date % time;

    uint d = date->as_uint32(0, false) % 100;
    date = date / factor;
    uint m = date->as_uint32(0, false) % 100;
    date = date / factor;
    uint y = date->as_uint32(0,false);

    const uint days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    bool bisext = m == 2 && y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
    if (m < 1 || m > 12 || d < 1 || d > days[m-1] + bisext)
    {
        if (error)
            rt.invalid_date_error();
        return 0;
    }

    dt.year  = y;
    dt.month = m;
    dt.day   = d;

    if (time && !time->is_zero())
    {
        time = time * factor;
        uint hour = time->as_uint32(0, false);
        time = (time * factor) % factor;
        uint min = time->as_uint32(0, false);
        time = (time * factor) % factor;
        uint sec = time->as_uint32(0, false);
        time = (time * factor) % factor;
        uint csec = time->as_uint32(0, false);
        if (hour >= 24 || min >= 60 || sec >= 60)
        {
            if (error)
                rt.invalid_time_error();
            return 0;
        }
        tm.hour = hour;
        tm.min = min;
        tm.sec = sec;
        tm.csec = csec;
        return 2;
    }

    return 1;
}


algebraic_p to_days(object_p dobj, bool error)
// ----------------------------------------------------------------------------
//   Return the value interpreted as a number of days
// ----------------------------------------------------------------------------
{
    if (!dobj)
        return nullptr;
    dobj = object::strip(dobj);

    algebraic_p dval = nullptr;
    if (unit_g u = unit::get(dobj))
    {
        unit_g day = unit::make(integer::make(1), +symbol::make("d"));
        if (day->convert(u, false))
            dval = u->value();
    }
    if (!dval)
    {
        dval = dobj->as_real();
        if (!dval && error)
            rt.type_error();
    }
    return dval;
}


algebraic_p julian_day_number(const dt_t &dt, const tm_t &tm)
// ----------------------------------------------------------------------------
//   Compute julian day number for a dt_t structure
// ----------------------------------------------------------------------------
{
    ularge csecs = (tm.hour * 3600 + tm.min * 60 + tm.sec) * 100 + tm.csec;
    ularge jval = julian_day_number(dt.day, dt.month, dt.year);
    algebraic_g jdn = integer::make(jval);
    if (csecs)
    {
        algebraic_g frac = +fraction::make(integer::make(csecs),
                                           +integer::make(8640000));
        jdn = jdn + frac;
    }
    return jdn;
}


algebraic_p julian_day_number(algebraic_p dtobj, bool error)
// ----------------------------------------------------------------------------
//   Compute the Julian day number associate with a date value
// ----------------------------------------------------------------------------
{
    dt_t dt;
    tm_t tm{};
    if (to_date(dtobj, dt, tm, error))
        return julian_day_number(dt, tm);
    return 0;
}


ularge julian_day_number(int d, int m, int y)
// ----------------------------------------------------------------------------
//   Compute the Julian day number given day, month and year
// ----------------------------------------------------------------------------
{
    uint rm = (m-14)/12;
    ularge jdn = ((1461 * (y + 4800 + rm)) / 4
                  + (367 * (m - 2 - 12 * rm)) / 12
                  - (3 * ((y + 4900 + rm) / 100)) / 4
                  + d
                  - 32075);
    return jdn;
}


algebraic_p date_from_julian_day(object_p jdn, bool error)
// ----------------------------------------------------------------------------
//   Create a day from a Julian day object
// ----------------------------------------------------------------------------
{
    if (!jdn)
        return nullptr;

    if (algebraic_g jval = jdn->as_real())
    {
        large jdn = jval->as_int64(0, error);

        enum
        {
            y = 4716,
            j = 1401,
            m = 2,
            n = 12,
            r = 4,
            p = 1461,
            v = 3,
            u = 5,
            s = 153,
            w = 2,
            B = 274277,
            C = -38
        };

        large f = jdn + j + (((4 * jdn + B) / 146097) * 3) / 4 + C;
        large e = r * f + v;
        large g = (e % p) / r;
        large h = u * g + w;
        uint day = (h % s) / u + 1;
        uint month = (h / s + m ) % n + 1;
        uint year = e / p - y + (n + m - month) / n;
        ularge dval = year * 10000 + month * 100 + day;

        algebraic_g date = integer::make(dval);
        algebraic_g fp = integer::make(1);
        fp = jval % fp;
        if (!fp->is_zero())
        {
            algebraic_g factor = integer::make(86400);
            fp = fp * factor;
            ularge hval = fp->as_uint64(0, false);
            uint hour = hval / 3600;
            uint min = (hval / 60) % 60;
            uint sec = hval % 60;
            hval = hour * 10000 + min * 100 + sec;
            fp = +fraction::make(integer::make(hval),
                                 integer::make(1000000));
            date = date + fp;
        }
        date = unit::make(date, +symbol::make("date"));
        return date;
    }

    if (error)
        rt.type_error();
    return nullptr;
}


algebraic_p days_between_dates(object_p date1, object_p date2, bool error)
// ----------------------------------------------------------------------------
//   Compute the number of days between two dates
// ----------------------------------------------------------------------------
{
    dt_t dt1, dt2;
    tm_t tm1{}, tm2{};
    if (to_date(date1, dt1, tm1, error) && to_date(date2, dt2, tm2, error))
    {
        algebraic_g day1 = julian_day_number(dt1, tm1);
        algebraic_g day2 = julian_day_number(dt2, tm2);
        if (algebraic_g diff = day1 - day2)
            if (unit_p days= unit::make(+diff, +symbol::make("d")))
                return days;
    }
    return nullptr;
}


static algebraic_p days_after(object_p date, object_p days, bool error, int s)
// ----------------------------------------------------------------------------
//   Compute the number of days after a given date
// ----------------------------------------------------------------------------
{
    if (algebraic_g num = to_days(days, error))
    {
        dt_t dt;
        tm_t tm{};
        if (to_date(date, dt, tm, error))
        {
            algebraic_g jdn = julian_day_number(dt, tm);
            jdn = s > 0 ? jdn + num : jdn - num;
            num = date_from_julian_day(jdn);
            return num;
        }
    }
    return nullptr;
}


algebraic_p days_after(object_p date, object_p days, bool error)
// ----------------------------------------------------------------------------
//   Compute the number of days after a given date
// ----------------------------------------------------------------------------
{
    return days_after(date, days, error, 1);
}


algebraic_p days_before(object_p date, object_p days, bool error)
// ----------------------------------------------------------------------------
//   Compute the number of days before a given date
// ----------------------------------------------------------------------------
{
    return days_after(date, days, error, -1);
}



// ============================================================================
//
//   Date and time related RPL commands
//
// ============================================================================

COMMAND_BODY(DateTime)
// ----------------------------------------------------------------------------
//   Return current date and time
// ----------------------------------------------------------------------------
{
    dt_t dt;
    tm_t tm;
    rtc_read(&tm, &dt);

    ularge tval = tm.hour * 10000 + tm.min * 100 + tm.sec;
    ularge dval = dt.year * 10000 + dt.month * 100 + dt.day;
    dval = dval * 1000000ULL + tval;
    if (decimal_g date   = decimal::make(dval, -6))
        if (unit_g result = unit::make(+date, +symbol::make("date")))
            if (rt.push(+result))
                return OK;
    return ERROR;
}


COMMAND_BODY(Date)
// ----------------------------------------------------------------------------
//   Return current date
// ----------------------------------------------------------------------------
{
    dt_t dt;
    tm_t tm;
    rtc_read(&tm, &dt);

    ularge dval = dt.year * 10000 + dt.month * 100 + dt.day;
    if (integer_g date   = integer::make(dval))
        if (unit_g result = unit::make(+date, +symbol::make("date")))
            if (rt.push(+result))
                return OK;
    return ERROR;
}


static bool setDate(object_p dobj)
// ----------------------------------------------------------------------------
//   Set the system date based on input
// ----------------------------------------------------------------------------
{
    dt_t dt;
    tm_t tm;
    rtc_read(&tm, &dt);
    if (!to_date(dobj, dt, tm))
        return false;
    rtc_write(&tm, &dt);
    return true;
}


COMMAND_BODY(SetDate)
// ----------------------------------------------------------------------------
//   Set the current date
// ----------------------------------------------------------------------------
{
    if (object_p d = rt.top())
        if (setDate(d))
            if (rt.drop())
                return OK;

    return ERROR;
}


COMMAND_BODY(Time)
// ----------------------------------------------------------------------------
//   Return the current time
// ----------------------------------------------------------------------------
{
    dt_t dt;
    tm_t tm;
    rtc_read(&tm, &dt);

    ularge tval = tm.hour * 10000 + tm.min * 100 + tm.sec;
    if (integer_g itime = integer::make(tval))
      if (integer_g ratio = integer::make(10000))
          if (fraction_g time = fraction::make(itime, ratio))
              if (algebraic_g sexag = from_hms_dms(+time, ""))
                  if (unit_g result = unit::make(+sexag, +symbol::make("hms")))
                      if (rt.push(+result))
                          return OK;
    return ERROR;
}


COMMAND_BODY(ChronoTime)
// ----------------------------------------------------------------------------
//   Return the current time with a precision of 1/100th of a second
// ----------------------------------------------------------------------------
{
    dt_t dt;
    tm_t tm;
    rtc_read(&tm, &dt);

    ularge tval = tm.hour * 1000000 + tm.min * 10000 + tm.sec * 100 + tm.csec;
    if (integer_g itime = integer::make(tval))
      if (integer_g ratio = integer::make(1000000))
          if (fraction_g time = fraction::make(itime, ratio))
              if (algebraic_g sexag = from_hms_dms(+time, ""))
                  if (unit_g result = unit::make(+sexag, +symbol::make("hms")))
                      if (rt.push(+result))
                          return OK;
    return ERROR;
}





static bool setTime(object_p tobj)
// ----------------------------------------------------------------------------
//   Set the system date based on input
// ----------------------------------------------------------------------------
{
    dt_t dt;
    tm_t tm;
    rtc_read(&tm, &dt);
    if (!to_time(tobj, tm))
        return false;
    rtc_write(&tm, &dt);
    return true;
}


COMMAND_BODY(SetTime)
// ----------------------------------------------------------------------------
//   Set the current time
// ----------------------------------------------------------------------------
{
    if (object_p t = rt.top())
        if (setTime(t))
            if (rt.drop())
                return OK;
    return ERROR;
}



void render_time(renderer &r, algebraic_g &value,
                 cstring hrs, cstring min, cstring sec,
                 uint base, bool ampm)
// ----------------------------------------------------------------------------
//   Render a time (or an angle) as hours/minutes/seconds
// ----------------------------------------------------------------------------
{
    if (!value || !value->is_real())
        return;
    bool as_time = *hrs == ':';
    settings::SaveNumericalResults snr(false);
    settings::SaveFractionDigits sfd(12);
    settings::SaveFractionIterations sfi(15);
    algebraic::to_fraction(value);
    uint h = value->as_uint32(0, false);
    r.flush();
    r.printf("%u", h);
    r.put(hrs);

    algebraic_g one = integer::make(1);
    algebraic_g factor = integer::make(base);
    value = (value * factor) % factor;
    uint m = value ? value->as_uint32(0, false) : 0;
    r.printf("%02u", m);
    r.put(min);

    value = (value * factor) % factor;
    uint s = value ? value->as_uint32(0, false) : 0;
    r.printf("%02u", s);
    r.put(sec);

    value = value % one;
    if (value && !value->is_zero(false))
    {
        if (as_time && algebraic::to_decimal(value))
        {
            settings::SaveLeadingZero slz(false);
            auto dm = Settings.DisplayMode();
            if (dm == object::ID_Sci || dm == object::ID_Eng)
                dm = object::ID_Fix;
            settings::SaveDisplayMode sdm(dm);
            value->render(r);
        }
        else if (algebraic::to_fraction(value))
        {
            value->render(r);
        }
    }
    if (ampm)
        r.put(h < 12 ? 'A' : 'P');
}


size_t render_dms(renderer &r, algebraic_g value,
                  cstring deg, cstring min, cstring sec)
// ----------------------------------------------------------------------------
//   Render a number as "degrees / minutes / seconds"
// ----------------------------------------------------------------------------
{
    if (!value || !value->is_real())
        return 0;
    bool neg = value->is_negative(false);
    if (neg)
    {
        r.put('-');
        value = -value;
    }
    render_time(r, value, deg, min, sec, 60, false);
    return r.size();
}


size_t render_date(renderer &r, algebraic_g date)
// ----------------------------------------------------------------------------
//   Render a number as "degrees / minutes / seconds"
// ----------------------------------------------------------------------------
{
    if (!date || !date->is_real())
        return 0;
    bool neg = date->is_negative();
    if (neg)
    {
        r.put('-');
        date = -date;
    }

    algebraic_g factor = integer::make(100);
    algebraic_g time = integer::make(1);
    time = date % time;
    uint day = date->as_uint32(0, false) % 100;
    date = date / factor;
    uint month = date->as_uint32(0, false) % 100;
    date = date / factor;
    uint year = date->as_uint32(0, false);

    char mname[4];
    if (Settings.ShowMonthName() && month >=1 && month <= 12)
        snprintf(mname, 4, "%s", get_month_shortcut(month));
    else
        snprintf(mname, 4, "%u", month);

    char ytext[6];
    if (Settings.TwoDigitYear())
        snprintf(ytext, 6, "%02u", year % 100);
    else
        snprintf(ytext, 6, "%u", year);

    if (Settings.ShowDayOfWeek())
    {
        ularge jdn = julian_day_number(day, month, year);
        uint dow = jdn % 7;
        r.printf("%s ", get_wday_shortcut(dow));
    }

    char sep   = Settings.DateSeparator();
    uint index = 2 * Settings.YearFirst() + Settings.MonthBeforeDay();
    switch(index)
    {
    case 0: r.printf("%u%c%s%c%s", day,   sep, mname, sep, ytext); break;
    case 1: r.printf("%s%c%u%c%s", mname, sep, day,   sep, ytext); break;
    case 2: r.printf("%s%c%u%c%s", ytext, sep, day,   sep, mname); break;
    case 3: r.printf("%s%c%s%c%u", ytext, sep, mname, sep, day);   break;
    }

    if (time && !time->is_zero())
    {
        r.put(", ");
        time = time * factor;
        render_time(r, time, ":", ":", "", 100, Settings.Time12H());
    }

    return r.size();
}



// ============================================================================
//
//   HMS and DMS commands
//
// ============================================================================

algebraic_p to_hms_dms(algebraic_r x, cstring name)
// ----------------------------------------------------------------------------
//   Convert an algebraic value to HMS or DMS value (i.e. no unit)
// ----------------------------------------------------------------------------
{
    if (unit_g u = unit::get(x))
    {
        algebraic_g uexpr = u->uexpr();
        if (symbol_p sym = uexpr->as_quoted<symbol>())
        {
            if (sym->matches("dms") || sym->matches("hms") || sym->matches("°"))
                return u->value();
            algebraic::angle_unit amode = object::ID_object;
            if (sym->matches("pir")  || sym->matches("πr"))
                amode = object::ID_PiRadians;
            else if (sym->matches("grad"))
                amode = object::ID_Grad;
            else if (sym->matches("r"))
                amode = object::ID_Rad;
            if (amode)
            {
                algebraic_g angle = u->value();
                return algebraic::convert_angle(angle, amode, object::ID_Deg);
            }
        }
        if (integer_g one = integer::make(1))
            if (symbol_g uname = symbol::make(name))
                if (unit_g to = unit::make(+one, +uname))
                    if (to->convert(u))
                        return u->value();
        rt.inconsistent_units_error();
        return nullptr;
    }
    if (!x->is_real())
    {
        rt.type_error();
        return nullptr;
    }
    return x;
}


object::result to_hms_dms(cstring name)
// ----------------------------------------------------------------------------
//   Convert the top of stack to HMS or DMS unit
// ----------------------------------------------------------------------------
{
    algebraic_g x = rt.top()->as_algebraic();
    algebraic_g xc = to_hms_dms(x, name);
    if (!x || !xc)
        return object::ERROR;

    if (!arithmetic::to_fraction(xc))
    {
        if (!rt.error())
            rt.value_error();
        return object::ERROR;
    }
    algebraic_g sym = +symbol::make(name);
    unit_g unit = unit::make(xc, sym);
    if (!rt.top(unit))
        return object::ERROR;
    return object::OK;
}


algebraic_p from_hms_dms(algebraic_g x, cstring name)
// ----------------------------------------------------------------------------
//   Convert a value from HMS input
// ----------------------------------------------------------------------------
{
    if (x->is_real())
    {
        // Compatibility mode (including behaviour for 1.60->2.00)
        if (!algebraic::to_fraction(x))
            return nullptr;
        algebraic_g hours = IntPart::run(x);
        algebraic_g fp = FracPart::run(x);
        algebraic_g hundred = integer::make(100);
        algebraic_g min = hundred * fp;
        algebraic_g sec = hundred * FracPart::run(min);
        min = IntPart::run(min);
        algebraic_g ratio = +fraction::make(integer::make(100),
                                            integer::make(6000));
        sec = sec * ratio;
        min = (min + sec) * ratio;
        hours = hours + min;
        return hours;
    }
    else if (unit_g u = unit::get(x))
    {
        algebraic_g uexpr = u->uexpr();
        if (symbol_p sym = uexpr->as_quoted<symbol>())
        {
            if (sym->matches(name))
            {
                uexpr = u->value();
                return uexpr;
            }
        }
        rt.inconsistent_units_error();
    }
    else
    {
        rt.type_error();
    }
    return nullptr;
}


object::result from_hms_dms(cstring name)
// ----------------------------------------------------------------------------
//   Convert the top of stack from HMS or DMS unit
// ----------------------------------------------------------------------------
{
    algebraic_g x = rt.top()->as_algebraic();
    x = from_hms_dms(x, name);
    if (x && rt.top(x))
        return object::OK;
    return object::ERROR;
}


COMMAND_BODY(ToHMS)
// ----------------------------------------------------------------------------
//   Convert value to HMS format
// ----------------------------------------------------------------------------
{
    return to_hms_dms("hms");
}


COMMAND_BODY(ToDMS)
// ----------------------------------------------------------------------------
//   Convert value to DMS format
// ----------------------------------------------------------------------------
{
    return to_hms_dms("dms");
}


COMMAND_BODY(FromHMS)
// ----------------------------------------------------------------------------
//   Convert value from HMS format
// ----------------------------------------------------------------------------
{
    return from_hms_dms("hms");
}


COMMAND_BODY(FromDMS)
// ----------------------------------------------------------------------------
//   Convert value from DMS format
// ----------------------------------------------------------------------------
{
    return from_hms_dms("dms");
}


static object::result hms_dms_add_sub(cstring name, bool sub)
// ----------------------------------------------------------------------------
//   Addition or subtraction of DMS/HMS values
// ----------------------------------------------------------------------------
{
    algebraic_g x = algebraic_p(rt.stack(0));
    algebraic_g y = algebraic_p(rt.stack(1));

    // Convert both arguments to DMS
    x = from_hms_dms(x, name);
    y = from_hms_dms(y, name);
    if (!x || !y)
        return object::ERROR;

    // Add or subtract
    x = sub ? y - x : y + x;

    // Build result
    algebraic_g sym = +symbol::make(name);
    unit_g unit = unit::make(x, sym);
    if (!rt.drop() || !rt.top(unit))
        return object::ERROR;
    return object::OK;
}


COMMAND_BODY(DMSAdd)
// ----------------------------------------------------------------------------
//   Addition of DMS values
// ----------------------------------------------------------------------------
{
    return hms_dms_add_sub("dms", false);
}


COMMAND_BODY(DMSSub)
// ----------------------------------------------------------------------------
//   Subtraction of DMS values
// ----------------------------------------------------------------------------
{
    return hms_dms_add_sub("dms", true);
}


COMMAND_BODY(HMSAdd)
// ----------------------------------------------------------------------------
//   Addition of HMS values
// ----------------------------------------------------------------------------
{
    return hms_dms_add_sub("hms", false);
}


COMMAND_BODY(HMSSub)
// ----------------------------------------------------------------------------
//   Subtraction of HMS values
// ----------------------------------------------------------------------------
{
    return hms_dms_add_sub("hms", true);
}


COMMAND_BODY(DateAdd)
// ----------------------------------------------------------------------------
//   Add a date to a number of days
// ----------------------------------------------------------------------------
{
    if (object_p d1 = rt.stack(1))
    {
        if (object_p d2 = rt.stack(0))
        {
            if (algebraic_p daf = days_after(d1, d2))
                if (rt.drop() && rt.top(daf))
                    return OK;
            if (algebraic_p daf = days_after(d2, d1))
                if (rt.drop() && rt.top(daf))
                    return OK;
        }
    }

    return ERROR;
}


COMMAND_BODY(DateSub)
// ----------------------------------------------------------------------------
//   Compute the number of days between two dates
// ----------------------------------------------------------------------------
{
    if (object_p d1 = rt.stack(1))
        if (object_p d2 = rt.stack(0))
            if (algebraic_p dd = days_between_dates(d1, d2))
                if (rt.drop() && rt.top(dd))
                    return OK;
    return ERROR;
}


COMMAND_BODY(JulianDayNumber)
// ----------------------------------------------------------------------------
//   Return the Julian day number for current date and time
// ----------------------------------------------------------------------------
{
    dt_t dt;
    tm_t tm{};
    if (object_p d = rt.top())
        if (to_date(d, dt, tm))
            if (algebraic_g jdn = julian_day_number(dt, tm))
                if (rt.top(jdn))
                    return OK;
    return ERROR;
}


COMMAND_BODY(DateFromJulianDayNumber)
// ----------------------------------------------------------------------------
//   Return the date for a given Julian day number
// ----------------------------------------------------------------------------
{
    if (object_p jdn = rt.top())
        if (algebraic_p date = date_from_julian_day(jdn))
            if (rt.top(date))
                return OK;
    return ERROR;
}


COMMAND_BODY(InsertHms)
// ----------------------------------------------------------------------------
//   Insert _hms in the editor, potentially replacing _dms
// ----------------------------------------------------------------------------
{
    ui.replace(utf8("_dms"), utf8("_hms"));
    return OK;
}


COMMAND_BODY(TimedEval)
// ----------------------------------------------------------------------------
//   Evaluate and return the time it took
// ----------------------------------------------------------------------------
{
    uint start = program::read_time();
    if (result err = Eval::do_evaluate())
        return err;
    uint end = program::read_time();
    if (integer_g duration = integer::make(end - start))
        if (symbol_g ms = symbol::make("ms"))
            if (unit_g result = unit::make(+duration, +ms))
                if (tag_g tval = tag::make("duration", +result))
                    if (rt.push(+tval))
                        return OK;
    return ERROR;
}
