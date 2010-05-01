
#include <StdFuncs.h>
#include <Dir.h>
#include "Test.h"

// TODO: CAW - Use Test.Printf() and get rid of this
#include <stdio.h>

static RDir g_oDir;				/* RDir class is global to implicitly test re-use */
static RTest Test("T_Dir");		/* Class to use for testing and reporting results */

static void TestScan(const char *a_pccPath)
{
	int Count, Index, Result;

	/* Test opening using the path passed in */

	Result = g_oDir.Open(a_pccPath);
	test(Result == KErrNone);

	/* Test that this can be read */

	TEntryArray *Entries;
	Result = g_oDir.Read(Entries);
	test(Result == KErrNone);

	Count = Entries->Count();
	printf("Path \"%s\" count = %d\n", a_pccPath, Count);

	for (Index = 0; Index < Count; ++Index)
	{
		printf("%s\n", (*Entries)[Index].iName);
	}

	g_oDir.Close();
}

int main()
{
	int Result;

	Test.Title();
	Test.Start("RDir class API test");
	Test.Next("Reading directory");

	/* Test that Close() can handle being called before Open() */

	g_oDir.Close();

	/* Test that Open() without wildcards works */

	TestScan("");

	/* Test that Open() with wildcards works */

	TestScan("*");

	/* Test that Open() with a directory path works */

	TestScan("Debug");

	/* Test that Open() with a directory path and pattern works */

	TestScan("T_D*");

	/* Test that Open() with a file path works */

	TestScan("T_Dir.cpp");

	/* Test that Open() with an invalid path works */

	Result = g_oDir.Open("x:");
	test(Result == KErrNotFound);

	/* Test that Close() can be called after a failed Open() without causing problems */

	g_oDir.Close();

	/* Test that Close() can be called a second time without causing problems */

	g_oDir.Close();

	Test.End();

	return(RETURN_OK);
}
