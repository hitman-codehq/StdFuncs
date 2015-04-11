
#include "StdFuncs.h"
#include "StdTime.h"
#include <time.h>

/* Useful constants used for calculating the current date and time in microseconds */

#define MICROSECONDS_PER_SECOND (TInt64) 1000000
#define MICROSECONDS_PER_MINUTE MICROSECONDS_PER_SECOND
#define MICROSECONDS_PER_HOUR (MICROSECONDS_PER_MINUTE * 60)
#define MICROSECONDS_PER_DAY (MICROSECONDS_PER_HOUR * 24)
#define MICROSECONDS_PER_YEAR (MICROSECONDS_PER_DAY * 365)

/** List of names of days of the week */

const char *g_apccDays[] =
{
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

/** List of number of days in each month of the year */

const TInt g_aiDaysPerMonth[] =
{
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/** List of names of months of the year */

const char *g_apccMonths[] =
{
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
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
 * Calculates the current day of the week.
 * This function will calculate the day of the week specified by the current instance of the
 * TDateTime structure, as an index starting from Sunday.  ie. Sunday will return 0, Monday
 * will return 1 etc. up until Saturday, which will return 6.
 *
 * Note that this is an expensive routine as the day of the week is only calculated when this
 * is called and it must count the number of days that have passed since 01.01.01 in order to
 * determine the answer.
 *
 * @date	Wednesday 11-Mar-2015 06:16 am, Code HQ Ehinger Tor
 * @return	The day of the week, starting from 0
 */

TInt TDateTime::DayOfWeek() const
{
	TInt Index, NumDays;

	NumDays = 0;

	/* Determine the number of days that have passed from 01.01.01 to the current year and month */

	for (Index = 1; Index < iYear; ++Index)
	{
		NumDays += (TDateTime::IsLeapYear(Index)) ? 366 : 365;
	}

	for (Index = 0; Index < iMonth; ++Index)
	{
		NumDays += g_aiDaysPerMonth[Index];
	}

	/* Take into account whether the current year is a leap year.  If it is and the current day is after */
	/* February then we need to add 1 day */

	if (TDateTime::IsLeapYear(iYear))
	{
		if (iMonth > EFebruary)
		{
			++NumDays;
		}
	}

	/* And finally the current day of the month */

	NumDays += iDay;

	/* Divide by the number of days per week and take the remainder and we have the current day of the week */

	return(NumDays % 7);
}

/**
 * Returns whether the given year is a leap year.
 * This function will calculate whether the year specified is a leap year.
 *
 * @date	Thursday 05-Feb-2015 07:20 am, Code HQ Ehinger Tor
 * @param	a_iYear		The year to be checked
 * @return	ETrue if the year is a leap year, else EFalse
 */

TBool TDateTime::IsLeapYear(TInt a_iYear)
{
	TBool LeapYear = EFalse;

	/* If the year specified is evenly divisible by four then it is a leap year */

	if ((a_iYear % 4) == 0)
	{
		/* Unless it is the first year of a new century (ie. 1900).  Then it is not a leap year */

		if ((a_iYear % 100) == 0)
		{
			/* But there is the special case that the first year of a century that is divisible by 400 */
			/* (ie. 2000) is a leap year */

			if ((a_iYear % 400) == 0)
			{
				LeapYear = ETrue;
			}
		}
		else
		{
			LeapYear = ETrue;
		}
	}

	return(LeapYear);
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

#if defined(__amigaos4__) || defined(__linux__)

	time_t TimeInSeconds;
	struct tm *LocalTime;

	/* Get the current time using the POSIX function and build up a TDateTime structure representing that time */

	TimeInSeconds = time(NULL);
	LocalTime = localtime(&TimeInSeconds);
	ASSERTM((LocalTime != NULL), "TTime::HomeTime() => Unable to obtain current date and time");
	TDateTime DateTime((LocalTime->tm_year + 1900), (TMonth) LocalTime->tm_mon, LocalTime->tm_mday, LocalTime->tm_hour, LocalTime->tm_min, LocalTime->tm_sec, 0);

#else /* ! defined(__amigaos4__) || defined(__linux__) */

	SYSTEMTIME SystemTime;

	/* Get the current time from Windows and build up a TDateTime structure representing that time */

	GetLocalTime(&SystemTime);
	TDateTime DateTime(SystemTime.wYear, (TMonth) (SystemTime.wMonth - 1), SystemTime.wDay, SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, 0);

#endif /* ! defined(__amigaos4__) || defined(__linux__) */

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
