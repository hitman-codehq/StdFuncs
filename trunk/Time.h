
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
	TInt	iYear;		/* The current year */
	TInt	iMonth;		/* The month of the current year, starting with 0 */
	TInt	iDay;		/* The day of the current month */
	TInt	iHour;		/* The hour of the current day */
	TInt	iMinute;	/* The minute of the current day */
	TInt	iSecond;	/* The second of the current minute */

public:

	// TODO: CAW - Initialise + double check all this stuff in the SDL
	TDateTime() { }

	TDateTime(TInt a_iYear, TMonth a_iMonth, TInt a_iDay, TInt a_iHour, TInt a_iMinute, TInt a_iSecond, TInt a_iMicroSecond);

	TInt Year() const { return(iYear); }

	TInt Month() const { return(iMonth); }

	TInt Day() const { return(iDay); }

	TInt Hour() const { return(iHour); }

	TInt Minute() const { return(iMinute); }

	TInt Second() const { return(iSecond); }
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

	TBool operator==(TTime a_oTime) const;
};

#endif /* ! TIME_H */
