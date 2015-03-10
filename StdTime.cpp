
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
 * Initialises the TTime to the current local time.
 * Queries the operating system for the current local time and assigns it to this instance
 * of the TTime class.
 *
 * @date	Wednesday 04-Mar-2015 07:32 am, Code HQ Ehinger Tor
 */

void TTime::HomeTime()
{

#ifdef __amigaos4__

#elif defined(__linux__)

#else /* ! __linux__ */

	SYSTEMTIME SystemTime;

	/* Get the current time from Windows and build up a TDateTime structure representing that time */

	GetLocalTime(&SystemTime);
	TDateTime DateTime(SystemTime.wYear, (TMonth) SystemTime.wMonth, SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, 0);

#endif /* ! __linux__ */

	/* And assign it to both the human readable and internal representations of the date and time */

	Set(DateTime);
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
	return(iTime);
}

/**
 * Initialises the human readable and internal representations of the date and time.
 * This is an internal helper function that will save the human readable TDateTime structure
 * passed in and will then calculate the number of microseconds that have elapsed since 01.01.01,
 * saving the resulting value.  This allows both the human and machine readable versions of
 * the date and time to be accessed without requiring calculations.  Note that this is a
 * relatively expensive routine as it must perform conversion calculations.
 *
 * @date	Thursday 05-Apr-2015 06:25 am, Code HQ Ehinger Tor
 * @param	a_roDateTime	Reference to the structure containing the current date and time
 */

void TTime::Set(const TDateTime &a_roDateTime)
{
	TInt Index;

	/* Save the human readable TDateTime structure passed in */

	iDateTime = a_roDateTime;

	/* Calculate the number of microseconds representing seconds, minutes, hours and days */

	iTime = (iDateTime.Second() * MICROSECONDS_PER_SECOND);
	iTime += (iDateTime.Minute() * MICROSECONDS_PER_MINUTE);
	iTime += (iDateTime.Hour() * MICROSECONDS_PER_HOUR);
	iTime += ((iDateTime.Day() - 1) * MICROSECONDS_PER_DAY);

	/* Each month has a different number of days in it so we must calculate the number of */
	/* microseconds that represent each separate month */

	for (Index = 0; Index < iDateTime.Month(); ++Index)
	{
		iTime += (g_aiDaysPerMonth[Index] * MICROSECONDS_PER_DAY);
	}

	/* And finally add on the number of microseconds that represent the given year */

	iTime += ((iDateTime.Year()) * MICROSECONDS_PER_YEAR);
}

/* Written: Wednesday 17-Jun-2009 7:50 am */

TTime &TTime::operator=(const TDateTime &a_roDateTime)
{
	Set(a_roDateTime);

	return(*this);
}

/* Written: Monday 13-Jul-2009 6:28 am */

TBool TTime::operator==(const TTime &a_roTime) const
{
	return(iTime == a_roTime.Int64());
}

/**
 * Tests whether the given date and time is greater than that passed in.
 * This function will compare the passed in date and time with that of the current instance of this
 * class and will return whether the passed in date and time is greater.
 *
 * @date	Wednesday 16-Jul-2014 6:29 am
 * @param	a_roTime	Date and time with which to compare the current date and time
 * @return	ETrue if the current date and time is greater than that passed in, else EFalse
 */

TBool TTime::operator>(const TTime &a_roTime) const
{
	return(iTime > a_roTime.Int64());
}
