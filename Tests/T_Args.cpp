
#include <StdFuncs.h>
#include <Args.h>
#include <Test.h>
#include <string.h>

/* Fake command line to test a full set of arguments */

static const char *g_pccArgV[] =
{
	"T_Args", "Source Dir", "Dest Dir", "copy", "delete", "deletedirs", "nocase", "noerrors", "noprotect"
};

#define ARGV_COUNT 9

/* Fake command line to test a missing argument */

const char *g_pccMissingArgV[] =
{
	"T_Args", "SourceDir"
};

#define MISSING_ARGV_COUNT 2

/* Fake command lines for testing white space and " handling */

const char *g_pccOneString = "\"Source Dir\" \"Dest Dir\" copy";
const char *g_pccOneStringExtraWhite = " \"Source Dir\" \t \"Dest Dir\"  copy \t ";
const char *g_pccOneStringNoWhite = "\"Source Dir\"\"Dest Dir\"copy";

#define ONESTRING_ARGV_COUNT 3

/* Template for use in obtaining command line parameters */

static const char g_accTemplate[] = "SOURCE/A,DEST/A,COPY/S,DELETE/S,DELETEDIRS/S,NOCASE/S,NOERRORS/S,NOPROTECT/S";

/* Fake command line for testing multiple source filenames */

static const char *g_pccMultiArgV[] =
{
	"T_Args", "First Dir", "Second Dir", "Third Dir"
};

#define MULTI_ARGV_COUNT 4

/* Fake command line for testing 10 or more multiple source options */

static const char *g_pccMagicMultiArgV[] =
{
	"T_Args", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Eleven"
};

#define MULTI_MAGIC_ARGV_COUNT 12

/* Template for use in obtaining multiple source filenames */

static const char g_accMultiSourceTemplate[] = "SOURCE/A/M,DEST/A";

/* Template for use in obtaining multiple destination filenames.  We also use the /M */
/* option in a different place here than in the above string, to test this positioning */

static const char g_accMultiDestTemplate[] = "SOURCE/A,DEST/M/A";

/* Definitions of the number of arguments that can be passed in and their offsets in the */
/* multiple source argument array */

#define ARGS_MULTI_SOURCE 0
#define ARGS_MULTI_DEST 1
#define ARGS_MULTI_NUM_ARGS 2

/* Definitions of the number of arguments that can be passed in and their offsets in the */
/* argument array */

#define ARGS_SOURCE 0
#define ARGS_DEST 1
#define ARGS_COPY 2
#define ARGS_DELETE 3
#define ARGS_DELETEDIRS 4
#define ARGS_NOCASE 5
#define ARGS_NOERRORS 6
#define ARGS_NOPROTECT 7
#define ARGS_NUM_ARGS 8

#define CHECK_ARG(a, b) Test.printf("Checking \"%s\" against \"%s\"\n", a, b); test(strcmp(a, b) == 0);

static RTest Test("T_Args");	/* Class to use for testing and reporting results */

int main()
{
	char *OneString;
	int Index, Result;
	RArgs Args;

	Test.Title();
	Test.Start("RArgs class API test");

	/* Test #2: Test that close() can handle being called before open() */

	Test.Next("Test that close() can handle being called before open()");
	Args.close();

	/* Test #3: Parse simple command line arguments */

	Test.Next("Parse simple command line arguments");
	Result = Args.open(g_accTemplate, ARGS_NUM_ARGS, g_pccArgV, ARGV_COUNT);
	test(Result == KErrNone);
	test(Args.Count() == ARGS_NUM_ARGS);

	/* Ensure that the string arguments have been read as expected */

	for (Index = ARGS_SOURCE; Index <= ARGS_DEST; ++Index)
	{
		Test.printf("Checking \"%s\" against \"%s\"\n", Args[Index], g_pccArgV[Index + 1]);
		test(strcmp(Args[Index], g_pccArgV[Index + 1]) == 0);
	}

	/* Ensure that the boolean arguments have been read as expected */

	for (Index = ARGS_COPY; Index < ARGS_NUM_ARGS; ++Index)
	{
		Test.printf("Checking \"%s\"\n", g_pccArgV[Index + 1]);
		test(Args[Index] != 0);
	}

	Args.close();

	/* Test #4: Parse more complex command line parameters that contain white space */

	Test.Next("Parse complex command line arguments that contain white space");

	OneString = new char[strlen(g_pccOneStringExtraWhite) + 1];
	test(OneString != NULL);
	strcpy(OneString, g_pccOneString);

	Result = Args.open(g_accTemplate, ARGS_NUM_ARGS, OneString);
	test(Result == KErrNone);
	test(Args.Count() == ARGS_NUM_ARGS);

	/* Ensure that the arguments have been read as expected */

	CHECK_ARG(Args[0], "Source Dir");
	CHECK_ARG(Args[1], "Dest Dir");
	test(Args[2] != 0);

	Args.close();

	/* Test #5: Parse more complex command line parameters that contain extra white space */

	Test.Next("Parse complex command line arguments that contain extra white space");

	strcpy(OneString, g_pccOneStringExtraWhite);
	Result = Args.open(g_accTemplate, ARGS_NUM_ARGS, OneString);
	test(Result == KErrNone);
	test(Args.Count() == ARGS_NUM_ARGS);

	/* Ensure that the arguments have been read as expected */

	CHECK_ARG(Args[0], "Source Dir");
	CHECK_ARG(Args[1], "Dest Dir");
	test(Args[2] != 0);

	Args.close();

	/* Test #6: Parse complex command line arguments lacking white space */

	Test.Next("Parse complex command line arguments lacking white space");

	strcpy(OneString, g_pccOneStringNoWhite);
	Result = Args.open(g_accTemplate, ARGS_NUM_ARGS, OneString);
	test(Result == KErrNone);
	test(Args.Count() == ARGS_NUM_ARGS);

	/* Ensure that the arguments have been read as expected */

	CHECK_ARG(Args[0], "Source Dir");
	CHECK_ARG(Args[1], "Dest Dir");
	test(Args[2] != 0);

	delete [] OneString;
	Args.close();

	/* Test #7: Test not passing in a parameter for an /A option */

	Test.Next("Not passing in a parameter for an /A option");

	Result = Args.open(g_accTemplate, ARGS_NUM_ARGS, g_pccMissingArgV, MISSING_ARGV_COUNT);
	test(Result == KErrNotFound);

	/* Test #8: Command line parameters with multiple sources and one destination */

	Test.Next("Command line parameters with multiple sources and one destination");

	Result = Args.open(g_accMultiSourceTemplate, ARGS_MULTI_NUM_ARGS, g_pccMultiArgV, MULTI_ARGV_COUNT);
	test(Result == KErrNone);
	test(Args.Count() == ARGS_MULTI_NUM_ARGS);

	/* Ensure that the string arguments have been read as expected */

	CHECK_ARG(Args[0], "First Dir");
	CHECK_ARG(Args[1], "Third Dir");

	/* Now check that the multi arguments have been read ok */

	test(Args.CountMultiArguments() == 2);
	CHECK_ARG(Args.MultiArgument(0), "First Dir");
	CHECK_ARG(Args.MultiArgument(1), "Second Dir");

	Args.close();

	/* Test #9: Command line parameters with one source and multiple destinations */

	Test.Next("Command line parameters with one source and multiple destinations");

	Result = Args.open(g_accMultiDestTemplate, ARGS_MULTI_NUM_ARGS, g_pccMultiArgV, MULTI_ARGV_COUNT);
	test(Result == KErrNone);
	test(Args.Count() == ARGS_MULTI_NUM_ARGS);

	/* Ensure that the string arguments have been read as expected */

	CHECK_ARG(Args[0], "First Dir");
	CHECK_ARG(Args[1], "Second Dir");

	/* Now check that the multi arguments have been read ok */

	test(Args.CountMultiArguments() == 2);
	CHECK_ARG(Args.MultiArgument(0), "Second Dir");
	CHECK_ARG(Args.MultiArgument(1), "Third Dir");

	Args.close();

	/* Test #8: Ensure that passing in 10 or more multiple source options works */

	Test.Next("Ensure that passing in 10 or more multiple source options works");

	Result = Args.open(g_accMultiSourceTemplate, ARGS_MULTI_NUM_ARGS, g_pccMagicMultiArgV, MULTI_MAGIC_ARGV_COUNT);
	test(Result == KErrNone);

	/* Ensure that the string arguments have been read as expected */

	CHECK_ARG(Args[0], "One");
	CHECK_ARG(Args[1], "Eleven");

	/* Now check that the multi arguments have been read ok */

	test(Args.CountMultiArguments() == (MULTI_MAGIC_ARGV_COUNT - 2));

	for (Index = 0; Index < (MULTI_MAGIC_ARGV_COUNT - 2); ++Index)
	{
		CHECK_ARG(Args.MultiArgument(Index), g_pccMagicMultiArgV[Index + 1]);
	}

	Args.close();

	Test.End();

	return(RETURN_OK);
}
