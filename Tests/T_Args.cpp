
#include <StdFuncs.h>
#include <Args.h>
#include <Test.h>

/* Fake command lines used for testing, with parameters shuffled up to exercise RArgs class */

const char *g_pccArgV[] =
{
	"T_Args", "SourceDir", "DestDir", "copy", "delete", "deletedirs", "nocase", "noerrors", "noprotect"
};

#define ARGV_COUNT 9

/* Fake command line to test a missing argument */

const char *g_pccMissingArgV[] =
{
	"T_Args", "SourceDir"
};

#define MISSING_ARGV_COUNT 2

/* Strings for testing white space and " handling */

const char *g_pccOneString = "\"Source Dir\" \"Dest Dir\" copy";
const char *g_pccOneStringExtraWhite = " \"Source Dir\" \t \"Dest Dir\"  copy \t ";
const char *g_pccOneStringNoWhite = "\"Source Dir\"\"Dest Dir\"copy";

#define ONESTRING_ARGV_COUNT 3

/* Template for use in obtaining command line parameters */

static const char g_accTemplate[] = "SOURCE/A,DEST/A,COPY/S,DELETE/S,DELETEDIRS/S,NOCASE/S,NOERRORS/S,NOPROTECT/S";

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

#define CHECK_ARG(a, b) Test.Printf("Checking \"%s\" against \"%s\"\n", a, b); test(strcmp(a, b) == 0);

static RDir g_oDir;				/* RDir class is global to implicitly test re-use */
static RTest Test("T_Args");	/* Class to use for testing and reporting results */

int main()
{
	char *OneString;
	int Index, Result;
	RArgs Args;

	Test.Title();
	Test.Start("RArgs class API test");

	/* Test that Close() can handle being called before Open() */

	Test.Next("Check calling close before open");
	Args.Close();

	/* Parse simple command line parameters that contain no white space */

	Test.Next("Parse simple command line arguments");
	Result = Args.Open(g_accTemplate, ARGS_NUM_ARGS, g_pccArgV, ARGV_COUNT);
	test(Result == KErrNone);
	test(Args.Count() == ARGS_NUM_ARGS);
	test(Args.Valid() == ARGS_NUM_ARGS);

	/* Ensure that the entries have been read as expected */

	for (Index = 0; Index < (ARGV_COUNT - 1); ++Index)
	{
		Test.Printf("Checking \"%s\" against \"%s\"\n", Args[Index], g_pccArgV[Index + 1]);
		test(strcmp(Args[Index], g_pccArgV[Index + 1]) == 0);
	}

	Args.Close();

	/* Parse more complex command line parameters that contain white space */

	Test.Next("Parse complex command line arguments");

	OneString = new char[strlen(g_pccOneString) + 1];
	test(OneString != NULL);
	strcpy(OneString, g_pccOneString);

	Result = Args.Open(g_accTemplate, ARGS_NUM_ARGS, OneString);
	test(Result == KErrNone);
	test(Args.Count() == ARGS_NUM_ARGS);
	test(Args.Valid() == ONESTRING_ARGV_COUNT);

	/* Ensure that the entries have been read as expected */

	CHECK_ARG(Args[0], "Source Dir");
	CHECK_ARG(Args[1], "Dest Dir");
	CHECK_ARG(Args[2], "copy");

	Args.Close();

	/* Parse more complex command line parameters that contain extra white space */

	Test.Next("Parse complex command line arguments with extra white space");

	strcpy(OneString, g_pccOneStringExtraWhite);
	Result = Args.Open(g_accTemplate, ARGS_NUM_ARGS, OneString);
	test(Result == KErrNone);
	test(Args.Count() == ARGS_NUM_ARGS);
	test(Args.Valid() == ONESTRING_ARGV_COUNT);

	/* Ensure that the entries have been read as expected */

	CHECK_ARG(Args[0], "Source Dir");
	CHECK_ARG(Args[1], "Dest Dir");
	CHECK_ARG(Args[2], "copy");

	Args.Close();

	/* Parse more complex command line parameters that contain white space without space separaters */
	/* between " argument terminators */

	Test.Next("Parse complex command line arguments lacking white space");

	strcpy(OneString, g_pccOneStringNoWhite);
	Result = Args.Open(g_accTemplate, ARGS_NUM_ARGS, OneString);
	test(Result == KErrNone);
	test(Args.Count() == ARGS_NUM_ARGS);
	test(Args.Valid() == ONESTRING_ARGV_COUNT);

	/* Ensure that the entries have been read as expected */

	CHECK_ARG(Args[0], "Source Dir");
	CHECK_ARG(Args[1], "Dest Dir");
	CHECK_ARG(Args[2], "copy");

	Args.Close();

	/* Test not passing in a parameter for an /A option */

	Test.Next("Not passing in a parameter for an /A option");

	Result = Args.Open(g_accTemplate, ARGS_NUM_ARGS, g_pccMissingArgV, MISSING_ARGV_COUNT);
	test(Result == KErrNotFound);

	delete [] OneString;
	Test.End();

	return(RETURN_OK);
}
