
#include <StdFuncs.h>
#include <StdWildcard.h>
#include <Test.h>

static RTest Test("T_Wildcard");

int main()
{
	Test.Title();
	Test.Start("Wildcard class API test");

	/* Test #2: Basic match of a name containing no wildcards */

	Test.Next("Basic match of a name containing no wildcards");

	RWildcard wildcard;
	test(wildcard.open("test.txt") == KErrNone);
	test(wildcard.Match("test.txt") == ETrue);
	wildcard.close();

	/* Test #3: Match of a name containing simple wildcards */

	Test.Next("Match of a name containing simple wildcards");

	test(wildcard.open("*.txt") == KErrNone);
	test(wildcard.Match("test.txt") == ETrue);
	wildcard.close();

	test(wildcard.open("test.*") == KErrNone);
	test(wildcard.Match("test.txt") == ETrue);
	wildcard.close();

	/* Test #4: Match of a name containing more complex wildcards */

	Test.Next("Match of a name containing more complex wildcards");

	test(wildcard.open("test.*xt") == KErrNone);
	test(wildcard.Match("test.txt") == ETrue);
	wildcard.close();

	test(wildcard.open("*test.*xt") == KErrNone);
	test(wildcard.Match("sometest.txt") == ETrue);
	test(wildcard.Match("test.txt") == ETrue);
	wildcard.close();

	test(wildcard.open("test*.*xt") == KErrNone);
	test(wildcard.Match("testthis.txt") == ETrue);
	test(wildcard.Match("test.txt") == ETrue);
	wildcard.close();

	test(wildcard.open("*test*.*xt") == KErrNone);
	test(wildcard.Match("sometestthis.txt") == ETrue);
	wildcard.close();

	/* Test #5: Negative tests */

	Test.Next("Negative tests");

	test(wildcard.open("test.*xt") == KErrNone);
	test(wildcard.Match("test.txt3") == EFalse);
	test(wildcard.Match("test.tx") == EFalse);
	wildcard.close();

	test(wildcard.open("*test.*xt") == KErrNone);
	test(wildcard.Match("sometesta.txt") == EFalse);
	test(wildcard.Match("sometest.txt1") == EFalse);
	wildcard.close();

	test(wildcard.open("test*.*xt") == KErrNone);
	test(wildcard.Match("thistest.txt") == EFalse);
	test(wildcard.Match("tes.txt") == EFalse);
	test(wildcard.Match("est.txt") == EFalse);
	wildcard.close();

	Test.End();

	return(RETURN_OK);
}
