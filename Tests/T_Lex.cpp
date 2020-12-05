
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
static char g_acBasicListQuotes[] = "\"One\" two \"'three'\" four \"five\"";
static const char *g_apccBasicListQuotesResults[BASIC_LIST_QUOTES_COUNT] = { "One", "two", "'three'", "four", "five" };

#define BASIC_LIST_SINGLE_QUOTES_COUNT 5
static char g_acBasicListSingleQuotes[] = "'One' two '\"three\"' four 'five'";
static const char *g_apccBasicListSingleQuotesResults[BASIC_LIST_SINGLE_QUOTES_COUNT] = { "One", "two", "\"three\"", "four", "five" };

#define NON_DESTRUCTIVE_LIST_COUNT 5
static const char g_accNonDestructiveList[] = "One two three four five";
static const char *g_apccNonDestructiveListResults[] = { "One", "two", "three", "four", "five" };

#define QUOTES_AND_SPACE_COUNT 7
static const char g_accQuotesAndSpaceList[] = "  \"One\" \"two\" \"three\"  ";
static const char *g_apccQuotesAndSpaceListResults[] = { "  ", "\"One\"", " ", "\"two\"", " ", "\"three\"", "  " };

#define QUOTES_COUNT 3
static const char *g_apccQuotesListResults[] = { "\"One\"", "\"two\"", "\"three\"" };

#define SPACES_COUNT 7
static const char *g_apccSpacesListResults[] = { "  ", "One", " ", "two", " ", "three", "  " };

#define QUOTES_LIST_NO_WHITESPACE_COUNT 2
static const char g_accQuotesListNoWhitespace[] = "\"one\"\"two\"";
static const char *g_apccQuotesListNoWhitespaceResults[] = { "\"one\"", "\"two\"" };

#define SEMICOLON_COUNT 7
static const char g_accSemiColonList[] = ";\"One\";;\"two\";;\"three\";";
static const char *g_apccQuotesAndSpacesSemiColonListResults[] = { ";", "\"One\"", ";;", "\"two\"", ";;", "\"three\"", ";" };

#define QUOTES_SEMICOLON_COUNT 3
static const char *g_apccQuotesSemiColonListResults[] = { "\"One\"", "\"two\"", "\"three\"" };

#define SPACES_SEMICOLON_COUNT 7
static const char *g_apccSpacesSemiColonListResults[] = { ";", "One", ";;", "two", ";;", "three", ";" };

#define CRLF_AT_END_COUNT 1
static const char g_accCRLFAtEndList[] = "One\r\n";
static const char *g_apccCRLFAtEndListResults[] = { "One" };

static const char g_accCRAtEndList[] = "One\r";
static const char g_accLFAtEndList[] = "One\n";

#define SINGLE_QUOTE_COUNT 1
static const char g_accSingleQuote[] = "\"";
static const char *g_accSingleQuoteResults[] = { "\"" };

#define DOUBLE_QUOTE_COUNT 1
static const char g_accDoubleQuote[] = "\"\"";
static const char *g_accDoubleQuoteResults[] = { "\"\"" };

#define SINGLE_QUOTE_SKIP_COUNT 0
#define DOUBLE_QUOTE_SKIP_COUNT 0

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
		Test.printf("Index %d = %s\n", Index, a_apccList[Index]);
		test(strcmp(Token, a_apccList[Index]) == 0);
	}

	/* All tokens extracted so ensure that the next call to TLex::NextToken() */
	/* returns NULL as expected */

	test(a_roLex.NextToken() == NULL);
}

/* Written: Wednesday 28-Nov-2012 6:32 pm, Vis � Vis Hotel, Lindau */
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

	/* First check that the list contains the expected number of tokens.  The TLex.Count() */
	/* routine will reinitialise the TLex's instance data after this so we can iterate */
	/* through its entries */

	test(a_roLex.Count() == a_iListCount);

	/* Iterate through all tokens and ensure they are as expected */

	for (Index = 0; Index < a_iListCount; ++Index)
	{
		Token = a_roLex.NextToken(&Length);
		test(Token != NULL);
		Test.printf("Index %d = %s\n", Index, a_apccList[Index]);
		test(strncmp(Token, a_apccList[Index], strlen(a_apccList[Index])) == 0);
	}

	/* All tokens extracted so ensure that the next call to TLex::NextToken() */
	/* returns NULL as expected */

	test(a_roLex.NextToken(&Length) == NULL);
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

	TLex BasicSingleQuotes(g_acBasicListSingleQuotes);
	CheckList(BasicSingleQuotes, g_apccBasicListSingleQuotesResults, BASIC_LIST_SINGLE_QUOTES_COUNT);

	/* Test #3: Basic non destructive test */

	Test.Next("Basic non destructive test");

	TLex Lex(g_accNonDestructiveList, strlen(g_accNonDestructiveList));

	for (Index = 0; Index < NON_DESTRUCTIVE_LIST_COUNT; ++Index)
	{
		Token = Lex.NextToken(&Offset);
		test(Token != NULL);
		Test.printf("Index %d = %s\n", Index, g_apccNonDestructiveListResults[Index]);
		test(strncmp(Token, g_apccNonDestructiveListResults[Index], strlen(g_apccNonDestructiveListResults[Index])) == 0);
	}

	/* Test #4: Tests including spaces and quotes */

	Test.Next("Test including spaces and quotes");

	TLex QuotesAndSpace(g_accQuotesAndSpaceList, strlen(g_accQuotesAndSpaceList));
	QuotesAndSpace.SetConfig(ETrue, ETrue);
	CheckListNonDestructive(QuotesAndSpace, g_apccQuotesAndSpaceListResults, QUOTES_AND_SPACE_COUNT);

	TLex Quotes(g_accQuotesAndSpaceList, strlen(g_accQuotesAndSpaceList));
	Quotes.SetConfig(ETrue, EFalse);
	CheckListNonDestructive(Quotes, g_apccQuotesListResults, QUOTES_COUNT);

	TLex Spaces(g_accQuotesAndSpaceList, strlen(g_accQuotesAndSpaceList));
	Spaces.SetConfig(EFalse, ETrue);
	CheckListNonDestructive(Spaces, g_apccSpacesListResults, SPACES_COUNT);

	TLex QuotesNoWhitespace(g_accQuotesListNoWhitespace, strlen(g_accQuotesListNoWhitespace));
	QuotesNoWhitespace.SetConfig(ETrue, EFalse);
	CheckListNonDestructive(QuotesNoWhitespace, g_apccQuotesListNoWhitespaceResults, QUOTES_LIST_NO_WHITESPACE_COUNT);

	/* Test #5: Test alternate white space separator */

	Test.Next("Test alternate white space separator");

	TLex QuotesAndSpacesSemiColon(g_accSemiColonList, strlen(g_accSemiColonList));
	QuotesAndSpacesSemiColon.SetConfig(ETrue, ETrue);
	QuotesAndSpacesSemiColon.SetWhitespace(";");
	CheckListNonDestructive(QuotesAndSpacesSemiColon, g_apccQuotesAndSpacesSemiColonListResults, SEMICOLON_COUNT);

	TLex QuotesSemiColon(g_accSemiColonList, strlen(g_accSemiColonList));
	QuotesSemiColon.SetConfig(ETrue, EFalse);
	QuotesSemiColon.SetWhitespace(";");
	CheckListNonDestructive(QuotesSemiColon, g_apccQuotesSemiColonListResults, QUOTES_SEMICOLON_COUNT);

	TLex SpacesSemiColon(g_accSemiColonList, strlen(g_accSemiColonList));
	SpacesSemiColon.SetConfig(EFalse, ETrue);
	SpacesSemiColon.SetWhitespace(";");
	CheckListNonDestructive(SpacesSemiColon, g_apccSpacesSemiColonListResults, SPACES_SEMICOLON_COUNT);

	/* Test #5: Ensure that CR/LF terminators are ignored */

	Test.Next("Ensure that CR/LF terminators are ignored");

	TLex CRLFAtEnd(g_accCRLFAtEndList, strlen(g_accCRLFAtEndList));
	CheckListNonDestructive(CRLFAtEnd, g_apccCRLFAtEndListResults, CRLF_AT_END_COUNT);

	TLex CRAtEnd(g_accCRAtEndList, strlen(g_accCRAtEndList));
	CheckListNonDestructive(CRAtEnd, g_apccCRLFAtEndListResults, CRLF_AT_END_COUNT);

	TLex LFAtEnd(g_accLFAtEndList, strlen(g_accLFAtEndList));
	CheckListNonDestructive(LFAtEnd, g_apccCRLFAtEndListResults, CRLF_AT_END_COUNT);

	/* Test #6: Ensure that quotes without content are handled sanely */

	Test.Next("Ensure that quotes without content are handled sanely");

	/* Parsing a string with a single quote has caused problems in the past so ensure that it is */
	/* handled precisely as expected */

	TLex SingleQuote(g_accSingleQuote, strlen(g_accSingleQuote));
	SingleQuote.SetConfig(ETrue, ETrue);
	CheckListNonDestructive(SingleQuote, g_accSingleQuoteResults, SINGLE_QUOTE_COUNT);

	TLex SingleQuoteSkip(g_accSingleQuote, strlen(g_accSingleQuote));
	CheckListNonDestructive(SingleQuoteSkip, g_accSingleQuoteResults, SINGLE_QUOTE_SKIP_COUNT);

	/* Also check that double quotes by themselves are handled as expected */

	TLex DoubleQuote(g_accDoubleQuote, strlen(g_accDoubleQuote));
	DoubleQuote.SetConfig(ETrue, ETrue);
	CheckListNonDestructive(DoubleQuote, g_accDoubleQuoteResults, DOUBLE_QUOTE_COUNT);

	TLex DoubleQuoteSkip(g_accDoubleQuote, strlen(g_accDoubleQuote));
	CheckListNonDestructive(DoubleQuoteSkip, g_accDoubleQuoteResults, DOUBLE_QUOTE_SKIP_COUNT);

	Test.End();

	return(RETURN_OK);
}
