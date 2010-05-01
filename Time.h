
#ifndef TIME_H
#define TIME_H

/* The months of the year, to be used by TDateTime */

enum TMonth
{
	EJanuary, EFebruary, EMarch, EApril, EMay, EJune,
	EJuly, EAugust, ESeptember, EOctober, ENovember, EDecember
};

/* A class designed to hold the date and time in a human readable form */

class TDateTime
{
	int		iYear;		/* The current year */
	int		iMonth;		/* The month of the current year, starting with 0 */
	int		iDay;		/* The day of the current month */
	int		iHour;		/* The hour of the current day */
	int		iMinute;	/* The minute of the current day */
	int		iSecond;	/* The second of the current minute */

public:

	// TODO: CAW - Initialise + double check all this stuff in the SDL
	TDateTime() { }

	TDateTime(int a_iYear, TMonth a_iMonth, int a_iDay, int a_iHour, int a_iMinute, int a_iSecond, int a_iMicroSecond);

	int Year() const { return(iYear); }

	int Month() const { return(iMonth); }

	int Day() const { return(iDay); }

	int Hour() const { return(iHour); }

	int Minute() const { return(iMinute); }

	int Second() const { return(iSecond); }
};

class TTime
{
	TDateTime	iDateTime;	/* Internal representation of the time */

public:

	// TODO: CAW - Set fields to 0?
	TTime() { }

	TTime(const TDateTime &a_oDateTime);

	TDateTime DateTime() const { return(iDateTime); }

	TTime &operator=(const TDateTime &a_oDateTime);

	bool operator==(TTime a_oTime) const;
};

#endif /* ! TIME_H */
