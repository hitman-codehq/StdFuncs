
#include <StdFuncs.h>
#include <BaUtils.h>
#include <Dir.h>
#include <File.h>
#include <Test.h>
#include <string.h>

static RDir g_oDir;				/* RDir class is global to implicitly test re-use */
static RTest Test("T_Dir");		/* Class to use for testing and reporting results */

static void TestScan(const char *a_pccPath, int a_iCount = 0, unsigned int a_iSize = 0)
{
	int Count, Index, FileIndex, Result;

	/* Test opening using the path passed in */

	Result = g_oDir.Open(a_pccPath);
	test(Result == KErrNone);

	/* Test that this can be read */

	TEntryArray *Entries;
	Result = g_oDir.Read(Entries);
	test(Result == KErrNone);

	Count = Entries->Count();
	FileIndex = 0;
	Test.Printf("Path \"%s\" count = %d\n", a_pccPath, Count);

	/* Scan through the entries and list them.  We also want to filter out any instances */
	/* of the .svn directory that appear, as these are present on some operating systems */
	/* and not others */

	for (Index = 0; Index < Entries->Count(); ++Index)
	{
		/* If the .svn directory was found then reduce the count of objects present */

		if (strcmp((*Entries)[Index].iName, ".svn") == 0)
		{
			--Count;
		}
		else
		{
			Test.Printf("%s, size = %d\n", (*Entries)[Index].iName, (*Entries)[Index].iSize);
			FileIndex = Index;
		}
	}

	/* If a count has been explicitly passed in then ensure that it matches the number found */

	if (a_iCount > 0)
	{
		Test.Printf("Ensuring count and size match\n");
		test(Count == a_iCount);
		test((*Entries)[FileIndex].iSize == a_iSize);
	}

	g_oDir.Close();
}

int main()
{
	int Result;
	RFile File;
	TEntry Entry;

	Test.Title();
	Test.Start("RDir class API test");

	/* Test that Close() can handle being called before Open() */

	g_oDir.Close();

	/* Test #2: Test that Open() without wildcards works */

	Test.Next("Test that Open() without wildcards works");
	TestScan("");

	/* Test #3: Test that Open() with current directory works */

	Test.Next("Test that Open() with current directory works");
	TestScan(".");

	/* Test #4: Test that Open() with a directory path works */

	Test.Next("Test that Open() with a directory path works");
	TestScan("SomeDir", 1, 174);
	TestScan("SomeDir/", 1, 174);
	TestScan("../Tests/SomeDir", 1, 174);
	TestScan("../Tests/SomeDir/", 1, 174);

	/* Test #5: Test that Open() with wildcards works */

	Test.Next("Test that Open() with wildcards works");
	TestScan("*");
	TestScan("*.dsw", 1, 2204);
	TestScan("SomeDir/*.txt", 1, 174);

	/* Test #6: Test that Open() with a filename works */

	Test.Next("Test that Open() with a filename works");
	TestScan("T_Dir.cpp");
	TestScan("SomeDir/SomeFile.txt", 1, 174);

	/* Test #7: Test that Open() with an invalid path works */

	Test.Next("Test that Open() with an invalid path works");
	Result = g_oDir.Open("x");
	test(Result == KErrNotFound);

	/* Test #8: Test calling Close() after a failed Open() */

	Test.Next("Test calling Close() after a failed Open()");
	g_oDir.Close();

	/* Test #9: Test calling Close() a second time */

	Test.Next("Test calling Close() a second time");
	g_oDir.Close();

	/* Test #10: Test similar to Test #7 of T_Utils.cpp, but using RDir to */
	/* confirm results rather than Utils::GetFileInfo() */

	Test.Next("Ensure that date and time can be read by RDir");

	/* Delete any old file hanging around from prior runs, create a new one */
	/* and set its time and attributes to be the same as the source code */
	/* for this test */

	Result = BaflUtils::DeleteFile("TimeFile.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	Result = File.Create("TimeFile.txt", EFileWrite);
	test(Result == KErrNone);
	File.Close();

	Result = Utils::GetFileInfo("T_Dir.cpp", &Entry);
	test(Result == KErrNone);

	Result = Utils::SetFileDate("TimeFile.txt", Entry);
	test(Result == KErrNone);

	Result = Utils::SetProtection("TimeFile.txt", Entry.iAttributes);
	test(Result == KErrNone);

	Result = g_oDir.Open("TimeFile.txt");
	test(Result == KErrNone);

	TEntryArray *Entries;
	Result = g_oDir.Read(Entries);
	test(Result == KErrNone);

	test((*Entries)[0].iModified == Entry.iModified);
	test((*Entries)[0].iAttributes == Entry.iAttributes);

	g_oDir.Close();

	/* Test #11: Ensure an empty directory can be scanned without problem */

	Test.Next("Ensure an empty directory can be scanned without problem");

	Result = Utils::CreateDirectory("EmptyDirectory");
	test((Result == KErrNone) || (Result == KErrAlreadyExists));

	test(g_oDir.Open("EmptyDirectory") == KErrNone);
	test(g_oDir.Read(Entries) == KErrNone);
	test(Entries->Count() == 0);

	g_oDir.Close();

	/* Clean up after ourselves */

	test(Utils::DeleteDirectory("EmptyDirectory") == KErrNone);

	/* Test #12: Ensure calling RDir::Read() on an unopened RDir fails gracefully */

	Test.Next("Ensure calling RDir::Read() on an unopened RDir fails gracefully");

	test(g_oDir.Read(Entries) == KErrGeneral);

	Test.End();

	return(RETURN_OK);
}
