
#include "StdFuncs.h"
#include "Time.h"

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

/* Written: Wednesday 17-Jun-2009 7:50 am */

TTime &TTime::operator=(const TDateTime &a_oDateTime)
{
	iDateTime = a_oDateTime;

	return(*this);
}

/* Written: Monday 13-Jul-2009 6:28 am */

TBool TTime::operator==(TTime a_oTime) const
{
	return((iDateTime.Year() == a_oTime.iDateTime.Year()) && (iDateTime.Month() == a_oTime.iDateTime.Month()) &&
		(iDateTime.Day() == a_oTime.iDateTime.Day()) && (iDateTime.Hour() == a_oTime.iDateTime.Hour()) &&
		(iDateTime.Minute() == a_oTime.iDateTime.Minute()) && (iDateTime.Second() == a_oTime.iDateTime.Second()));
}
