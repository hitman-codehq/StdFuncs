
#include "StdFuncs.h"
#include "StdTime.h"

/* Useful constants used for calculating the current date and time in microseconds */

#define MICROSECONDS_PER_SECOND (TInt64) 1000000
#define MICROSECONDS_PER_MINUTE MICROSECONDS_PER_SECOND
#define MICROSECONDS_PER_HOUR (MICROSECONDS_PER_MINUTE * 60)
#define MICROSECONDS_PER_DAY (MICROSECONDS_PER_HOUR * 24)
#define MICROSECONDS_PER_YEAR (MICROSECONDS_PER_DAY * 365)

/** List of days per month for use by TTime::Int64() */

static const TInt g_aiDaysPerMonth[] =
{
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/* Written: Wednesday 17-Jun-2009 7:30 am */

TDateTime::TDateTime(TInt a_iYear, TMonth a_iMonth, TInt a_iDay, TInt a_iHour, TInt a_iMinute, TInt a_iSecond, TInt /*a_iMicroSecond*/)
{
	iYear = a_iYear;
	iMonth = a_iMonth;
	iDay = a_iDay;
	iHour = a_iHour;
	iMinute = a_iMinute;
	iSecond = a_iSecond;
}

/**
 * Calculates the number of microseconds that have elapsed since 01.01.01.
 * This function will convert the current date and time to a 64 bit count of the number
 * of microseconds that have elapsed since the start of the 1st millenium.  It is a
 * relatively CPU intensive function due to the amount of calculation involved, but will
 * result in an integer that can easily be used for fast comparison of dates and times.
 *
 * @date	Wednesday 16-Jul-2014 6:40 am
 * @return	Number of microseconds that have elapsed since 01.01.01
 */

TInt64 TTime::Int64() const
{
	TInt Index;
	TInt64 RetVal;

	/* Calculate the number of microseconds representing seconds, minutes, hours and days */

	RetVal = (iDateTime.Second() * MICROSECONDS_PER_SECOND);
	RetVal += (iDateTime.Minute() * MICROSECONDS_PER_MINUTE);
	RetVal += (iDateTime.Hour() * MICROSECONDS_PER_HOUR);
	RetVal += ((iDateTime.Day() - 1) * MICROSECONDS_PER_DAY);

	/* Each month has a different number of days in it so we must calculate the number of */
	/* microseconds that represent each separate month */

	for (Index = 0; Index < iDateTime.Month(); ++Index)
	{
		RetVal += (g_aiDaysPerMonth[Index] * MICROSECONDS_PER_DAY);
	}

	/* And finally add on the number of microseconds that represent the given year */

	RetVal += ((iDateTime.Year()) * MICROSECONDS_PER_YEAR);

	return(RetVal);
}

/* Written: Wednesday 17-Jun-2009 7:50 am */

TTime &TTime::operator=(const TDateTime &a_roDateTime)
{
	iDateTime = a_roDateTime;

	return(*this);
}

/* Written: Monday 13-Jul-2009 6:28 am */

TBool TTime::operator==(const TTime &a_roTime) const
{
	return((iDateTime.Year() == a_roTime.iDateTime.Year()) && (iDateTime.Month() == a_roTime.iDateTime.Month()) &&
		(iDateTime.Day() == a_roTime.iDateTime.Day()) && (iDateTime.Hour() == a_roTime.iDateTime.Hour()) &&
		(iDateTime.Minute() == a_roTime.iDateTime.Minute()) && (iDateTime.Second() == a_roTime.iDateTime.Second()));
}

/**
 * Tests whether this date and time is greater than that passed in.
 * This function will calculate the current date and time as a microsecond count from 01.01.01, and also
 * that of the time passed in, and will use the calculated values to determine whether the current date
 * and time is greater than that passed in.  Due to the amount of calculation involved, this is a
 * relatively expensive routine in terms of CPU cycles.
 *
 * @date	Wednesday 16-Jul-2014 6:29 am
 * @param	a_roTime	Date and time with which to compare the current date and time
 * @return	ETrue if the current date and time is greater than that passed in, else EFalse
 */

TBool TTime::operator>(const TTime &a_roTime) const
{
	return(Int64() > a_roTime.Int64());
}
