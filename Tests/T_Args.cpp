
#include <StdFuncs.h>
#include <Args.h>
#include "Test.h"

/* Fake command line used for testing, with parameters shuffled up to exercise RArgs class */

const char *a_pccArgV[] =
{
	"T_Args", "SourceDir", "DestDir", "delete", "copy", "noerrors", "deletedirs", "noprotect", "nocase"
};

#define ARGV_COUNT 9

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

static RDir g_oDir;				/* RDir class is global to implicitly test re-use */
static RTest Test("T_Args");	/* Class to use for testing and reporting results */

int main()
{
	int Result;
	RArgs Args;

	Test.Title();
	Test.Start("RArgs class API test");
	Test.Next("Reading directory");

	/* Test that Close() can handle being called before Open() */

	Args.Close();

	/* Parse the command line parameters passed in and make sure they are formatted correctly */

	Result = Args.Open(g_accTemplate, ARGS_NUM_ARGS, a_pccArgV, ARGV_COUNT);
	test(Result == KErrNone);
	test(Args.Count() == ARGS_NUM_ARGS);

	/* Print out the list of argument for manual checking */

	for (int Index = 0; Index < ARGS_NUM_ARGS; ++Index)
	{
		if (Args[Index])
		{
			Test.Printf("%s\n", Args[Index]);
		}
		else
		{
			Test.Printf("Argument %d not specified\n", Index);
		}
	}

	Args.Close();

	Test.End();

	return(RETURN_OK);
}
