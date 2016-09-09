
#ifndef STDTIME_H
#define STDTIME_H

/* The months of the year, to be used by TDateTime */

enum TMonth
{
	EJanuary, EFebruary, EMarch, EApril, EMay, EJune,
	EJuly, EAugust, ESeptember, EOctober, ENovember, EDecember
};

/* Various useful tables of time and date related information */

extern const char *g_apccDays[];
extern const TInt g_aiDaysPerMonth[];
extern const char *g_apccMonths[];

/* A class designed to hold the date and time in a human readable form */

class TDateTime
{
private:

	TInt	iYear;			/**< The current year */
	TMonth	iMonth;			/**< The month of the current year, starting with 0 */
	TInt	iDay;			/**< The day of the current month */
	TInt	iHour;			/**< The hour of the current day */
	TInt	iMinute;		/**< The minute of the current day */
	TInt	iSecond;		/**< The second of the current minute */
	TInt	iMilliSecond;	/**< The millisecond of the current second */

public:

	TDateTime()
	{
		iYear = iDay = iHour = iMinute = iSecond = 0;
		iMonth = EJanuary;
	}

	TDateTime(TInt a_iYear, TMonth a_iMonth, TInt a_iDay, TInt a_iHour, TInt a_iMinute, TInt a_iSecond, TInt a_iMilliSecond);

	TInt Year() const { return(iYear); }

	TInt Month() const { return(iMonth); }

	TInt Day() const { return(iDay); }

	TInt DayOfWeek() const;

	TInt Hour() const { return(iHour); }

	TInt Minute() const { return(iMinute); }

	TInt Second() const { return(iSecond); }

	TInt MilliSecond() const { return(iMilliSecond); }

	static TBool IsLeapYear(TInt a_iYear);
};

class TTime
{
private:

	TInt64		iTime;		/**< Time since 01.01.01 in microseconds */
	TDateTime	iDateTime;	/**< Internal representation of the time */

private:

	void Set(const TDateTime &a_roDateTime);

public:

	TTime() { }

	TDateTime DateTime() const { return(iDateTime); }

	void HomeTime();

	TInt64 Int64() const;

	TTime &operator=(const TDateTime &a_roDateTime);

	TBool operator==(const TTime &a_roTime) const;

	TBool operator>(const TTime &a_roTime) const;
};

#endif /* ! STDTIME_H */
