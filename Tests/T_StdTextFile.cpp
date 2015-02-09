
#include <StdFuncs.h>
#include <StdTextFile.h>
#include <Test.h>
#include <string.h>

static RTest Test("T_StdTextFile");	/* Class to use for testing and reporting results */
static RTextFile TextFile;			/* RTextFile class is global for easy reuse */

/* Written: Sunday 08-Feb-2015 12:27 pm, Code HQ Ehinger Tor */

static void TestReading()
{
	const char *LastLine, *Line;

	Line = TextFile.GetLine();
	test(Line != NULL);
	test(strcmp(Line, "") == 0);

	Line = TextFile.GetLine();
	test(Line != NULL);
	test(strcmp(Line, "Start Line") == 0);

	Line = TextFile.GetLine();
	test(Line != NULL);
	test(strcmp(Line, "") == 0);

	Line = TextFile.GetLine();
	test(Line != NULL);
	test(strcmp(Line, "") == 0);

	Line = TextFile.GetLine();
	test(Line != NULL);
	test(strcmp(Line, "Middle Line") == 0);

	/* Skip over other lines present and read in the last line */

	do
	{
		LastLine = Line;
	}
	while ((Line = TextFile.GetLine()) != NULL);

	test(LastLine != NULL);
	test(strcmp(LastLine, "End Line") == 0);
}

TInt main()
{
	const char *Line;
	TInt Result;

	Test.Title();
	Test.Start("RTextFile class API test");

	/* Test #2: Test reading an empty file */

	Test.Next("Test reading an empty file");

	Result = TextFile.Open("TestFiles/Empty.txt");
	test(Result == KErrNone);

	Line = TextFile.GetLine();
	test(Line == NULL);

	TextFile.Close();

	/* Test #3: Test basic functionality of the class */

	Test.Next("Test basic functionality of the class");

	Result = TextFile.Open("TestFiles/StdTextFile.txt");
	test(Result == KErrNone);

	Test.Printf("Scan text file for known lines\n");

	TestReading();

	Test.Printf("Rewind and perform the scan a second time\n");

	TextFile.Rewind();
	TestReading();

	TextFile.Close();

	Test.End();

	return(RETURN_OK);
}
