
#include <StdFuncs.h>
#include <Dir.h>
#include <Test.h>

// TODO: CAW - Steal some code from T_Utils to create a file and set its time, and then
//             ensure that the time is correct when the file is scanned with RDir

static RDir g_oDir;				/* RDir class is global to implicitly test re-use */
static RTest Test("T_Dir");		/* Class to use for testing and reporting results */

static void TestScan(const char *a_pccPath, int a_iCount = 0, unsigned int a_iSize = 0)
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

	/* If a count has been explicitly passed in then ensure that it matches the number found */

	if (a_iCount > 0)
	{
		Test.Printf("Ensuring count and size match\n");
		test(Count == a_iCount);
		test((*Entries)[0].iSize == a_iSize);
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

	Test.Next("Testing Open() with \"\"");
	TestScan("");

	/* Test that Open() with current directory works */

	Test.Next("Testing Open() with \".\"");
	TestScan(".");

	/* Test that Open() with a directory path works */

	Test.Next("Testing Open() without trailing slash");
	TestScan("SomeDir", 1, 174);

	Test.Next("Testing Open() with trailing slash");
	TestScan("SomeDir/", 1, 174);

	Test.Next("Testing Open() with a relative path without trailing slash");
	TestScan("../Tests/SomeDir", 1, 174);

	Test.Next("Testing Open() with a relative path with trailing slash");
	TestScan("../Tests/SomeDir/", 1, 174);

	/* Test that Open() with wildcards works */

	Test.Next("Testing Open() with a \"*\" wildcard");
	TestScan("*");

	Test.Next("Testing Open() with a partial \"*.dsw\" wildcard");
	TestScan("*.dsw", 1, 2204);

	Test.Next("Testing Open() with a path and partial \"*.txt\" wildcard");
	TestScan("SomeDir/*.txt", 1, 174);

	/* Test that Open() with a filename works */

	Test.Next("Testing Open() with a filename");
	TestScan("T_Dir.cpp");

	Test.Next("Testing Open() with a path and filename");
	TestScan("SomeDir/SomeFile.txt", 1, 174);

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
