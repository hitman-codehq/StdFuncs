
#include <StdFuncs.h>
#include <StdStringList.h>
#include <Test.h>
#include <string.h>

static RTest Test("T_StringList");	/* Class to use for testing and reporting results */

#define BASIC_STRINGS_COUNT 3
static char g_acBasicStrings[] = "One;two;three with spaces";
static const char *g_apccBasicStringsResults[] = { "One", "two", "three with spaces" };

int main()
{
	TInt Index, Result;
	CStdStringList *StringList;

	Test.Title();
	Test.Start("CStdStringList class API test");

	/* Test #1: Create an instance and ensure it is initialised correctly */

	Test.Next("Create an instance and ensure it is initialised correctly");

	StringList = new CStdStringList;
	test(StringList != NULL);
	test(StringList->Count() == 0);

	/* Test #2: Add some strings and check they are correct */

	Test.Next("Add some strings and check they are correct");

	Result = StringList->Append(g_acBasicStrings);
	test(Result == KErrNone);
	test(StringList->Count() == BASIC_STRINGS_COUNT);

	for (Index = 0; Index < BASIC_STRINGS_COUNT; ++Index)
	{
		Test.Printf("String %d is \"%s\"\n", Index, (*StringList)[Index]);
		test(strcmp((*StringList)[Index], g_apccBasicStringsResults[Index]) == 0);
	}

	/* Now add some more and ensure they are really appended and that the original strings are */
	/* still intact */

	Result = StringList->Append(g_acBasicStrings);
	test(Result == KErrNone);
	test(StringList->Count() == (BASIC_STRINGS_COUNT * 2));

	for (Index = 0; Index < (BASIC_STRINGS_COUNT * 2); ++Index)
	{
		Test.Printf("String %d is \"%s\"\n", Index, (*StringList)[Index]);
		test(strcmp((*StringList)[Index], g_apccBasicStringsResults[(Index % BASIC_STRINGS_COUNT)]) == 0);
	}

	/* Test #3: Reset the class back to its default state */

	Test.Next("Reset the class back to its default state");

	StringList->Reset();
	test(StringList->Count() == 0);

	/* Test #4: Add some strings after a reset */

	Test.Next("Add some strings after a reset");

	Result = StringList->Append(g_acBasicStrings);
	test(Result == KErrNone);
	test(StringList->Count() == BASIC_STRINGS_COUNT);

	for (Index = 0; Index < BASIC_STRINGS_COUNT; ++Index)
	{
		Test.Printf("String %d is \"%s\"\n", Index, (*StringList)[Index]);
		test(strcmp((*StringList)[Index], g_apccBasicStringsResults[Index]) == 0);
	}

	delete StringList;

	Test.End();

	return(RETURN_OK);
}
