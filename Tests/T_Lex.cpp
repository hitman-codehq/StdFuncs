
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

#define QUOTES_AND_SPACE_COUNT 7
static char g_acQuotesAndSpaceList[] = "  \"One\" \"two\" \"three\"  ";
static const char *g_apccQuotesAndSpaceListResults[] = { "  ", "\"One\"", " ", "\"two\"", " ", "\"three\"", "  " };

#define QUOTES_COUNT 3
static const char *g_apccQuotesListResults[] = { "\"One\"", "\"two\"", "\"three\"" };

#define SPACES_COUNT 7
static const char *g_apccSpacesListResults[] = { "  ", "One", " ", "two", " ", "three", "  " };

#define SEMICOLON_COUNT 7
static char g_acSemiColonList[] = ";\"One\";;\"two\";;\"three\";";
static const char *g_apccQuotesAndSpacesSemiColonListResults[] = { ";", "\"One\"", ";;", "\"two\"", ";;", "\"three\"", ";" };

#define QUOTES_SEMICOLON_COUNT 3
static const char *g_apccQuotesSemiColonListResults[] = { "\"One\"", "\"two\"", "\"three\"" };

#define SPACES_SEMICOLON_COUNT 7
static const char *g_apccSpacesSemiColonListResults[] = { ";", "One", ";;", "two", ";;", "three", ";" };

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

/* Written: Wednesday 28-Nov-2012 6:32 pm, Vis à Vis Hotel, Lindau */
/* @param	a_roLex			Reference to the initialised TLex object to be tested */
/*			a_apccList		Array of ptrs to strings containing the exepcted results */
/*			a_iListCount	Number of items in a_apccList */
/* Iterates through an initialised TLex, extracts the tokens from it and ensures that */
/* they match up with the expected results.  This is the non destructive version of the */
/* CheckList() function */

static void CheckListNonDestructive(TLex &a_roLex, const char *a_apccList[], TInt a_iListCount)
{
	const char *Token;
	TInt Index, Length;

	/* Iterate through all tokens and ensure they are as expected */

	for (Index = 0; Index < a_iListCount; ++Index)
	{
		Token = a_roLex.NextToken(&Length);
		test(Token != NULL);
		Test.Printf("Index %d = %s\n", Index, a_apccList[Index]);
		test(strncmp(Token, a_apccList[Index], strlen(a_apccList[Index])) == 0);
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

	/* Test #3: Basic non destructive test */

	Test.Next("Basic non destructive test");

	TLex Lex(g_acNonDestructiveList, strlen(g_acNonDestructiveList));

	for (Index = 0; Index < NON_DESTRUCTIVE_LIST_COUNT; ++Index)
	{
		Token = Lex.NextToken(&Offset);
		test(Token != NULL);
		Test.Printf("Index %d = %s\n", Index, g_apccNonDestructiveListResults[Index]);
		test(strncmp(Token, g_apccNonDestructiveListResults[Index], strlen(g_apccNonDestructiveListResults[Index])) == 0);
	}

	/* Test #4: Test including spaces and quotes */

	Test.Next("Test including spaces and quotes");

	TLex QuotesAndSpace(g_acQuotesAndSpaceList, strlen(g_acQuotesAndSpaceList));
	QuotesAndSpace.SetConfig(ETrue, ETrue);
	CheckListNonDestructive(QuotesAndSpace, g_apccQuotesAndSpaceListResults, QUOTES_AND_SPACE_COUNT);

	TLex Quotes(g_acQuotesAndSpaceList, strlen(g_acQuotesAndSpaceList));
	Quotes.SetConfig(ETrue, EFalse);
	CheckListNonDestructive(Quotes, g_apccQuotesListResults, QUOTES_COUNT);

	TLex Spaces(g_acQuotesAndSpaceList, strlen(g_acQuotesAndSpaceList));
	Spaces.SetConfig(EFalse, ETrue);
	CheckListNonDestructive(Spaces, g_apccSpacesListResults, SPACES_COUNT);

	/* Test #5: Test alternate white space separator */

	Test.Next("Test alternate white space separator");

	TLex QuotesAndSpacesSemiColon(g_acSemiColonList, strlen(g_acSemiColonList));
	QuotesAndSpacesSemiColon.SetConfig(ETrue, ETrue);
	QuotesAndSpacesSemiColon.SetWhitespace(";");
	CheckListNonDestructive(QuotesAndSpacesSemiColon, g_apccQuotesAndSpacesSemiColonListResults, SEMICOLON_COUNT);

	TLex QuotesSemiColon(g_acSemiColonList, strlen(g_acSemiColonList));
	QuotesSemiColon.SetConfig(ETrue, EFalse);
	QuotesSemiColon.SetWhitespace(";");
	CheckListNonDestructive(QuotesSemiColon, g_apccQuotesSemiColonListResults, QUOTES_SEMICOLON_COUNT);

	TLex SpacesSemiColon(g_acSemiColonList, strlen(g_acSemiColonList));
	SpacesSemiColon.SetConfig(EFalse, ETrue);
	SpacesSemiColon.SetWhitespace(";");
	CheckListNonDestructive(SpacesSemiColon, g_apccSpacesSemiColonListResults, SPACES_SEMICOLON_COUNT);

	Test.End();

	return(RETURN_OK);
}
