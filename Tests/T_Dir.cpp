
#include <StdFuncs.h>
#include <Dir.h>
#include <File.h>
#include <FileUtils.h>
#include <Test.h>
#include <string.h>

static RDir g_oDir;				/* RDir class is global to implicitly test re-use */
static RFileUtils g_oFileUtils;	/* Class for cleaning up the file system */
static RTest Test("T_Dir");		/* Class to use for testing and reporting results */

static void TestScan(const char *a_pccPath, int a_iCount = 0, unsigned int a_iSize = 0)
{
	int Count, Index, FileIndex, Result;

	/* Test opening using the path passed in */

	Result = g_oDir.open(a_pccPath);
	test(Result == KErrNone);

	/* Test that this can be read */

	TEntryArray *Entries;
	Result = g_oDir.read(Entries);
	test(Result == KErrNone);

	Count = Entries->Count();
	FileIndex = 0;
	Test.printf("Path \"%s\" count = %d\n", a_pccPath, Count);

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
			Test.printf("%s, size = %d\n", (*Entries)[Index].iName, (*Entries)[Index].iSize);
			FileIndex = Index;
		}
	}

	/* If a count has been explicitly passed in then ensure that it matches the number found */

	if (a_iCount > 0)
	{
		Test.printf("Ensuring count and size match\n");
		test(Count == a_iCount);
		test((*Entries)[FileIndex].iSize == a_iSize);
	}

	g_oDir.close();
}

int main()
{
	TInt Index, Result;
	RFile File;
	TEntry Entry;

	Test.Title();
	Test.Start("RDir class API test");

	/* Test that close() can handle being called before open() */

	g_oDir.close();

	/* Test #2: Test that open() without wildcards works */

	Test.Next("Test that open() without wildcards works");
	TestScan("");

	/* Test #3: Test that open() with a directory path works */

	Test.Next("Test that open() with a directory path works");
	TestScan("SomeDir", 1, 174);
	TestScan("SomeDir/", 1, 174);

	/* Test #4: Test that open() with wildcards works */

	Test.Next("Test that open() with wildcards works");

#ifdef __amigaos__

	TestScan("#?");
	TestScan("#?.sln", 1, 10393);
	TestScan("SomeDir/#?.txt", 1, 174);

#else /* ! __amigaos__ */

	TestScan("*");
	TestScan("*.sln", 1, 10393);
	TestScan("SomeDir/*.txt", 1, 174);

#endif /* ! __amigaos__ */

	/* Test #5: Test that open() with a filename works */

	Test.Next("Test that open() with a filename works");
	TestScan("T_Dir.cpp");
	TestScan("SomeDir/SomeFile.txt", 1, 174);

	/* Test #6: Test that open() with an invalid path works */

	Test.Next("Test that open() with an invalid path works");
	Result = g_oDir.open("x");
	test(Result == KErrNotFound);

	/* Test #7: Test calling close() after a failed open() */

	Test.Next("Test calling close() after a failed open()");
	g_oDir.close();

	/* Test #8: Test calling close() a second time */

	Test.Next("Test calling close() a second time");
	g_oDir.close();

	/* Test #9: Test similar to Test #7 of T_Utils.cpp, but using RDir to */
	/* confirm results rather than Utils::GetFileInfo() */

	Test.Next("Ensure that date and time can be read by RDir");

	/* Delete any old file hanging around from prior runs, create a new one */
	/* and set its time and attributes to be the same as the source code */
	/* for this test */

	Result = g_oFileUtils.deleteFile("TimeFile.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	Result = File.Create("TimeFile.txt", EFileWrite);
	test(Result == KErrNone);
	File.close();

	Result = Utils::GetFileInfo("T_Dir.cpp", &Entry);
	test(Result == KErrNone);

	Result = Utils::setFileDate("TimeFile.txt", Entry);
	test(Result == KErrNone);

	Result = Utils::setProtection("TimeFile.txt", Entry.iAttributes);
	test(Result == KErrNone);

	Result = g_oDir.open("TimeFile.txt");
	test(Result == KErrNone);

	TEntryArray *Entries;
	Result = g_oDir.read(Entries);
	test(Result == KErrNone);

	test((*Entries)[0].iModified == Entry.iModified);
	test((*Entries)[0].iAttributes == Entry.iAttributes);

	g_oDir.close();

	/* Test #10: Ensure an empty directory can be scanned without problem */

	Test.Next("Ensure an empty directory can be scanned without problem");

	Result = Utils::CreateDirectory("EmptyDirectory");
	test((Result == KErrNone) || (Result == KErrAlreadyExists));

	test(g_oDir.open("EmptyDirectory") == KErrNone);
	test(g_oDir.read(Entries) == KErrNone);
	test(Entries->Count() == 0);

	g_oDir.close();

	/* Test #11: Ensure calling RDir::read() on an unopened RDir fails gracefully */

	Test.Next("Ensure calling RDir::read() on an unopened RDir fails gracefully");

	test(g_oDir.read(Entries) == KErrGeneral);

	/* The framework usually works identically on all platforms, but as there are */
	/* subtle differences between different platforms when it comes to paths, this */
	/* is one area where we don't try to retain identical behaviour across platforms. */
	/* So only test these paths on non Amiga OS systems */

#ifdef __amigaos__

	/* Test #12: Test scanning RAM: to ensure that the Disk.info link is correct */

	Test.Next("Test scanning RAM: to ensure that the Disk.info link is correct");
	TestScan("RAM:");

#else /* ! __amigaos__ */

	/* Test #12: Test some paths that only work on Windows and UNIX */

	Test.Next("Test some paths that only work on Windows and UNIX");
	TestScan(".");
	TestScan("../Tests/SomeDir", 1, 174);
	TestScan("../Tests/SomeDir/", 1, 174);

#endif /* ! __amigaos__ */

	/* Clean up after ourselves */

	test(g_oFileUtils.deleteFile("TimeFile.txt") == KErrNone);
	test(Utils::DeleteDirectory("EmptyDirectory") == KErrNone);

	/* Test #13: Ensure that scanning PROGDIR: works */

	Test.Next("Ensure that scanning PROGDIR: works");

	/* Obtain a listing of the PROGDIR: directory and ensure that it contains at least one */
	/* entry.  If not then it must failed as at least the test executable should be there */

	test(g_oDir.open("PROGDIR:") == KErrNone);
	test(g_oDir.read(Entries) == KErrNone);
	test(Entries->Count() > 0);

	/* Scan through the entries and find the test executable, which by definition *must* be present */

	for (Index = 0; Index < Entries->Count(); ++Index)
	{

#ifdef WIN32

		if (strcmp((*Entries)[Index].iName, "T_Dir.exe") == 0)

#else /* ! WIN32 */

		if (strcmp((*Entries)[Index].iName, "T_Dir") == 0)

#endif /* ! WIN32 */

		{
			Test.printf("Found file \"%s\"\n", (*Entries)[Index].iName);

			break;
		}
	}

	test(Index < Entries->Count());

	g_oDir.close();

	Test.End();

	return(RETURN_OK);
}
