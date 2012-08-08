
#include <StdFuncs.h>
#include <Dir.h>
#include <Test.h>

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
	Test.Printf("Path \"%s\" count = %d\n", a_pccPath, Count);

	for (Index = 0; Index < Count; ++Index)
	{
		Test.Printf("%s, size = %d\n", (*Entries)[Index].iName, (*Entries)[Index].iSize);
	}

	g_oDir.Close();
}

int main()
{
	int Result;

	Test.Title();
	Test.Start("RDir class API test");

	/* Test that Close() can handle being called before Open() */

	g_oDir.Close();

	/* Test that Open() without wildcards works */

	Test.Next("Testing Open() without using \"\"");
	TestScan("");

	/* Test that Open() with current directory works */

	Test.Next("Testing Open() with \".\"");
	TestScan(".");

	/* Test that Open() with a directory path works */

	Test.Next("Testing Open() without trailing slash");
	TestScan("SomeDir");

	// TODO: CAW - Check the count and size are correct here

	Test.Next("Testing Open() with trailing slash");
	TestScan("SomeDir/");

	/* Test that Open() with wildcards works */

	// TODO: CAW - Get these working for UNIX and try with entries that include directory paths

	Test.Next("Testing Open() with a \"*\" wildcard");
	TestScan("*");

	Test.Next("Testing Open() with a partial \"*.cpp\" wildcard");
	TestScan("*.cpp");

	/* Test that Open() with a filename works */

	Test.Next("Test that Open() with a filename works");
	TestScan("T_Dir.cpp");

	Test.Next("Test that Open() with a path and filename works");
	TestScan("SomeDir/SomeFile.txt");

	/* Test that Open() with an invalid path works */

	Result = g_oDir.Open("x");
	test(Result == KErrNotFound);

	/* Test that Close() can be called after a failed Open() without causing problems */

	g_oDir.Close();

	/* Test that Close() can be called a second time without causing problems */

	g_oDir.Close();

	Test.End();

	return(RETURN_OK);
}
