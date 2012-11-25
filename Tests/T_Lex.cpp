
#include <StdFuncs.h>
#include <string.h>
#include <Lex.h>
#include <Test.h>

static RTest Test("T_Lex"); /* Class to use for testing and reporting results */

#define BASIC_LIST_COUNT 5
static char g_acBasicList[] = "One two three four five";
static const char *g_apccBasicListResults[] = { "One", "two", "three", "four", "five" };

#define BASIC_LIST_WHITESPACE_COUNT 5
static char g_acBasicListWhitespace[] = " \tOne  two  three  four  five ";
static const char *g_apccBasicListWhitespaceResults[] = { "One", "two", "three", "four", "five" };

#define BASIC_LIST_QUOTES_COUNT 5
static char g_acBasicListQuotes[] = "\"One\" two \"three\" four \"five\"";
static const char *g_apccBasicListQuotesResults[BASIC_LIST_QUOTES_COUNT] = { "One", "two", "three", "four", "five" };

#define NON_DESTRUCTIVE_LIST_COUNT 5
static const char g_acNonDestructiveList[] = "One two three four five";
static const char *g_apccNonDestructiveListResults[] = { "One", "two", "three", "four", "five" };

/* Written: Saturday 17-Nov-2012 7:22 pm, Code HQ Ehinger Tor */
/* @param	a_roLex			Reference to the initialised TLex object to be tested */
/*			a_apccList		Array of ptrs to strings containing the exepcted results */
/*			a_iListCount	Number of items in a_apccList */
/* Iterates through an initialised TLex, extracts the tokens from it and ensures that */
/* they match up with the expected results */

static void CheckList(TLex &a_roLex, const char *a_apccList[], TInt a_iListCount)
{
	const char *Token;
	TInt Index;

	/* Iterate through all tokens and ensure they are as expected */

	for (Index = 0; Index < a_iListCount; ++Index)
	{
		Token = a_roLex.NextToken();
		test(Token != NULL);
		Test.Printf("Index %d = %s\n", Index, a_apccList[Index]);
		test(strcmp(Token, a_apccList[Index]) == 0);
	}
}

int main()
{
	const char *Token;
	TInt Index, Offset;

	Test.Title();
	Test.Start("TLex class API test");

	/* Test #2: Parse basic lists of words, similar to a command line */

	Test.Next("Parse basic lists of words");

	TLex Basic(g_acBasicList);
	CheckList(Basic, g_apccBasicListResults, BASIC_LIST_COUNT);

	TLex BasicWhitespace(g_acBasicListWhitespace);
	CheckList(BasicWhitespace, g_apccBasicListWhitespaceResults, BASIC_LIST_WHITESPACE_COUNT);

	TLex BasicQuotes(g_acBasicListQuotes);
	CheckList(BasicQuotes, g_apccBasicListQuotesResults, BASIC_LIST_QUOTES_COUNT);

	/* Test #3: Basic a non destructive test */

	Test.Next("Basic a non destructive test");

	TLex Lex(g_acNonDestructiveList, strlen(g_acNonDestructiveList));

	for (Index = 0; Index < NON_DESTRUCTIVE_LIST_COUNT; ++Index)
	{
		Token = Lex.NextToken(&Offset);
		test(Token != NULL);
		Test.Printf("Index %d = %s\n", Index, g_apccNonDestructiveListResults[Index]);
		test(strncmp(Token, g_apccNonDestructiveListResults[Index], strlen(g_apccNonDestructiveListResults[Index])) == 0);
	}

	Test.End();

	return(RETURN_OK);
}
