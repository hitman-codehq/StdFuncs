
#include <StdFuncs.h>
#include <StdWildcard.h>
#include <Test.h>

static RTest Test("T_Wildcard");

int main()
{
	Test.Title();
	Test.Start("Wildcard class API test");

	/* Test #2: Basic match of a name containing no wildcards */

	Test.Next("Match a name containing no wildcards");

	RWildcard wildcard;
	test(wildcard.Open("test.txt") == KErrNone);
	test(wildcard.Match("test.txt") == ETrue);
	wildcard.Close();

	/* Test #3: Match of a name containing simple wildcards */

	Test.Next("Match a name containing simple wildcards");

	test(wildcard.Open("*.txt") == KErrNone);
	test(wildcard.Match("test.txt") == ETrue);
	wildcard.Close();

	test(wildcard.Open("test.*") == KErrNone);
	test(wildcard.Match("test.txt") == ETrue);
	wildcard.Close();

	/* Test #4: Match of a name containing simple wildcards */

	Test.Next("Match a name containing more complex wildcards");

	test(wildcard.Open("test.*xt") == KErrNone);
	test(wildcard.Match("test.txt") == ETrue);
	wildcard.Close();

	test(wildcard.Open("*test.*xt") == KErrNone);
	test(wildcard.Match("sometest.txt") == ETrue);
	test(wildcard.Match("test.txt") == ETrue);
	wildcard.Close();

	test(wildcard.Open("test*.*xt") == KErrNone);
	test(wildcard.Match("testthis.txt") == ETrue);
	test(wildcard.Match("test.txt") == ETrue);
	wildcard.Close();

	test(wildcard.Open("*test*.*xt") == KErrNone);
	test(wildcard.Match("sometestthis.txt") == ETrue);
	wildcard.Close();

	/* Test #5: Negative tests */

	Test.Next("Negative tests");

	test(wildcard.Open("test.*xt") == KErrNone);
	test(wildcard.Match("test.txt3") == EFalse);
	test(wildcard.Match("test.tx") == EFalse);
	wildcard.Close();

	test(wildcard.Open("*test.*xt") == KErrNone);
	test(wildcard.Match("sometesta.txt") == EFalse);
	test(wildcard.Match("sometest.txt1") == EFalse);
	wildcard.Close();

	test(wildcard.Open("test*.*xt") == KErrNone);
	test(wildcard.Match("thistest.txt") == EFalse);
	test(wildcard.Match("tes.txt") == EFalse);
	test(wildcard.Match("est.txt") == EFalse);
	wildcard.Close();

	Test.End();

	return(RETURN_OK);
}
