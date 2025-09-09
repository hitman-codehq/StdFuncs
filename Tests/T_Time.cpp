
#include <StdFuncs.h>
#include <Test.h>

static RTest Test("T_Time");	/* Class to use for testing and reporting results */

int main()
{
	Test.Title();
	Test.Start("Time related functionality tests");

	/* Test #2: Test encoding to microseconds and back results in correct time */
	Test.Next("Test encoding to microseconds and back results in correct time");

	TTime now;
	now.HomeTime();

	/* Extract the TDateTime structure directly from the start time, and create a second TDateTime that is */
	/* a rebuild of the microsecond count of the timestamp */
	TDateTime originalTime = now.DateTime();
	TDateTime rebuiltTime(now.Int64());

	/* If the timestamp was converted microsecond and back again successfully, all fields should match */
	test(originalTime.Year() == rebuiltTime.Year());
	test(originalTime.Month() == rebuiltTime.Month());
	test(originalTime.Day() == rebuiltTime.Day());
	test(originalTime.Hour() == rebuiltTime.Hour());
	test(originalTime.Minute() == rebuiltTime.Minute());
	test(originalTime.Second() == rebuiltTime.Second());

	/* Test #3: Check file datestamp obtained from Utils::GetFileInfo() */
	Test.Next("Check file datestamp obtained from Utils::GetFileInfo()");

	TEntry entry;
	test(Utils::GetFileInfo("T_Time.cpp", &entry) == KErrNone);

	TDateTime dateStamp = entry.iModified.DateTime();
	printf("Year = %d\n", dateStamp.Year());
	printf("Month = %d\n", dateStamp.Month());
	printf("Day = %d\n", dateStamp.Day());
	printf("Hour = %d\n", dateStamp.Hour());
	printf("Minute = %d\n", dateStamp.Minute());
	printf("Second = %d\n", dateStamp.Second());
	printf("MilliSecond = %d\n", dateStamp.MilliSecond());

	/* Test #4: Check file datestamp obtained from RDir */
	Test.Next("Check file datestamp obtained from RDir");

	RDir dir;
	TEntryArray *dirEntries;

	test(dir.open("T_Time.cpp") == KErrNone);
	test(dir.read(EDirSortNone) == KErrNone);
	dirEntries = dir.getEntries();
	test(dirEntries->Count() == 1);

	TDateTime dirDateStamp = dirEntries->getHead()->iModified.DateTime();
	printf("Year = %d\n", dirDateStamp.Year());
	printf("Month = %d\n", dirDateStamp.Month());
	printf("Day = %d\n", dirDateStamp.Day());
	printf("Hour = %d\n", dirDateStamp.Hour());
	printf("Minute = %d\n", dirDateStamp.Minute());
	printf("Second = %d\n", dirDateStamp.Second());
	printf("MilliSecond = %d\n", dirDateStamp.MilliSecond());

	dir.close();

	Test.End();

	return(RETURN_OK);
}
