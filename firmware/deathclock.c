#include <avr/io.h>      
#include <string.h>
#include <avr/interrupt.h>   // Interrupts and timers
#include <util/delay.h>      // Blocking delay functions
#include <avr/pgmspace.h>    // So we can store the 'font table' in ROM
#include <avr/eeprom.h>      // Date/time/pref backup in permanent EEPROM
#include <avr/wdt.h>     // Watchdog timer to repair lockups

#include "deathclock.h"

uint8_t normal_bmi_male[6][23] = {
  { 20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 40,0,0,0,0,1,1,1,1,2,2,2,3,3,3,4,4,4,5,5,6,6,8 },
  { 50,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,4,4,4,5,5,5,7 },
  { 60,0,0,0,0,0,0,1,1,1,1,2,2,2,3,3,3,4,4,4,5,5,6 },
  { 70,0,0,0,0,1,1,1,1,1,2,2,2,2,3,3,3,4,4,4,4,0,0 },
  { 80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5 }
};

uint8_t normal_bmi_female[7][23] = {
  { 20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 30,0,0,0,1,1,1,1,2,2,3,3,3,4,4,5,5,6,6,7,7,8,13 },
  { 40,0,0,0,1,1,1,1,2,2,2,3,3,3,4,4,5,5,6,6,6,7,11 },
  { 50,0,0,0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,5,5,6,6,10 },
  { 60,0,0,0,0,0,0,0,1,1,1,1,2,2,2,3,3,3,4,4,4,5,7 },
  { 70,0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,2,3,3,3,3,5 },
  { 80,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2 }
};

uint8_t sadistic_bmi_male[2][23] = {
  { 20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 40,0,0,0,0,0,0,0,0,1,1,1,1,1,1,2,2,2,2,2,3,3,4 }
};

uint8_t sadistic_bmi_female[3][23] = {
  { 20,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
  { 30,0,0,0,0,0,0,0,1,1,1,1,1,2,2,2,2,3,3,3,3,4,6 },
  { 40,0,0,0,0,0,0,0,1,1,1,1,1,1,2,2,2,2,3,3,3,3,5 }
};

uint16_t normal_smoking_male[11][2] = {
  { 25,0 },
  { 30,2739 },
  { 35,2703 },
  { 40,2557 },
  { 45,2520 },
  { 50,2410 },
  { 55,2265 },
  { 60,2046 },
  { 65,1790 },
  { 70,1607 },
  { 80,1205 }
};

uint16_t normal_smoking_female[10][2] = {
  { 25, 0 },
  { 30, 2367 },
  { 45, 2331 },
  { 50, 2294 },
  { 55, 2257 },
  { 60, 2192 },
  { 65, 1782 },
  { 70, 1826 },
  { 75, 1381 },
  { 80, 1096 }
};

uint16_t sadistic_smoking_male[3][2] = {
  { 25, 0 },
  { 30, 1278 },
  { 40, 1242 }
};

uint16_t sadistic_smoking_female[3][2] = {
  { 25, 0 },
  { 30, 1424 },
  { 40, 1388 }
};


uint8_t day_in_month[12]={31,28,31,30,31,30,31,31,30,31,30,31};

uint8_t is_leap_year ( uint16_t year )
{
  if ((year % 400) == 0)
    return 1;
  if ((year % 100) == 0)
    return 0;
  if ((year % 4) == 0)
    return 1;
  return 0;
}

int date_diff ( uint8_t month1, uint8_t day1, uint8_t year1, uint8_t month2, uint8_t day2, uint8_t year2 )
{
  int diff = 0;
  int i;

  if((year2 < year1) || ((year2 == year1) && (month2 < month1)))
    return date_diff( month2, day2, year2, month1, day1, year1 ) * -1;

  if((month1 == month2) && (year1 == year2))
    return day2 - day1;
  if(year1==year2)
  {
    diff = day_in_month[month1-1] - day1;
    if(month1 == 2)
      diff += is_leap_year(year1);
    for(i=month1+1;i<month2;i++)
    {
      diff+=day_in_month[i-1];
      if(i==2)
        diff+=is_leap_year(year1);
    }
    diff += day2;
    return diff;
  }
  diff = day_in_month[month1-1] - day1;
  if(month1 == 2)
      diff+=is_leap_year(year1);
  for(i=month1+1;i<=12;i++)
  {
    diff+=day_in_month[i-1];
    if(i==2)
      diff+=is_leap_year(year1);
  }
  for(i=year1+1;i<year2;i++)
    diff+=365+is_leap_year(i);
  for(i=1;i<month2;i++)
  {
    diff+=day_in_month[i-1];
    if(i==2)
      diff+=is_leap_year(year2);
  }
  diff += day2;
  return diff;
}

uint8_t BodyMassIndex ( uint8_t unit, uint16_t height, uint16_t weight )
{
  if ( unit == BMI_Imperial )
  {
    //Imperial, Weight in pounds, Height in inches
    return (weight * 703) / (height * height);
  }
  else
  {
    //Metric, Weight in Kilograms, Height in centimeters
    return (weight * 10000) / (height * height);
  }
}

uint32_t ETD ( uint8_t DOB_month, uint8_t DOB_day, uint8_t DOB_year, uint8_t month, uint8_t day, uint8_t year, uint8_t Gender, uint8_t Mode, uint8_t BMI, uint8_t Smoker )
{
  int diff,y,days,i,bmi;
  diff = date_diff(DOB_month,DOB_day,DOB_year,month,day,year);
  y = (diff * 100) / 36525;
  if (Gender == DC_gender_male)
    days = 2694500;  //Divide by 100 at the end.
  else
    days = 2895900;
  days -= diff*100;
  bmi = BMI;
  if(bmi > 45)
    bmi = 45;
  if(bmi < 24)
    bmi = 24;
  bmi -= 24;
  if(Mode == DC_mode_sadistic)
  {
    if (Gender == DC_gender_male)
    {
      days -= 1351425;  //Divide by 100 at the end.
      if ( Smoker == DC_smoker )
        for(i=0;i<3;i++)
        {
          if( y < sadistic_smoking_male[i][0] )
          {
            days -= sadistic_smoking_male[i][1]*100;
            break;
          }
        }
      for(i=0;i<2;i++)
      {
        if ( y < sadistic_bmi_male[i][0] )
        {
          days -= (sadistic_bmi_male[i][bmi+1] * 36525);
          break;
        }
      }
    }
    else
    {
      days -= 1461000;
      if ( Smoker == DC_smoker )
        for(i=0;i<3;i++)
        {
          if( y < sadistic_smoking_female[i][0] )
          {
            days -= sadistic_smoking_female[i][1]*100;
            break;
          }
        }
      for(i=0;i<3;i++)
      {
        if ( y < sadistic_bmi_female[i][0] )
        {
          days -= (sadistic_bmi_female[i][bmi+1] * 36525);
          break;
        }
      }
    }
  }
  else
  {
    if (Gender == DC_gender_male)
    {
      if ( Smoker == DC_smoker )
        for(i=0;i<11;i++)
        {
          if( y < normal_smoking_male[i][0] )
          {
            days -= normal_smoking_male[i][1]*100;
            break;
          }
        }
      for(i=0;i<6;i++)
      {
        if ( y < normal_bmi_male[i][0] )
        {
          days -= (normal_bmi_male[i][bmi+1] * 36525);
          break;
        }
      }
    }
    else
    {
      if ( Smoker == DC_smoker )
        for(i=0;i<10;i++)
        {
          if( y < normal_smoking_female[i][0] )
          {
            days -= normal_smoking_female[i][1]*100;
            break;
          }
        }
      for(i=0;i<7;i++)
      {
        if ( y < normal_bmi_female[i][0] )
        {
          days -= (normal_bmi_female[i][bmi+1] * 36525);
          break;
        }
      }
    }
  }
  
  if(Mode == DC_mode_optimistic)
  {
    days += 365250;
    //days += random(0,5479)*100;
  }
  if(Mode == DC_mode_pessimistic)
  {
    days -= 547875;
    //days -= random(0,3653)*100;
  }
  
  days *= 864;  //Convert days left into seconds left.
  return days;
}