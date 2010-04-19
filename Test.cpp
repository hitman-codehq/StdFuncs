
#include "StdFuncs.h"
#include "Test.h"
#include <stdio.h>

/* Written: Friday 05-Jun-2009 11:47 am */

// TODO; CAW - Amiga style parameters?
RTest::RTest(const char *aTitle)
{
	iTitle = aTitle;
	iLevel = 0;
}

/* Written: Friday 05-Jun-2009 11:48 am */

void RTest::Title()
{
	printf("RTEST TITLE: %s\n", iTitle);
}

/* Written: Monday 06-Jul 2009 3:56 pm */

// TODO: CAW - Should be able to pass parameters in
void RTest::Printf(const char *aString)
{
	printf("%s", aString);
}

/* Written: Friday 05-Jun-2009 1:14 pm */

void RTest::Start(const char *aHeading)
{
	printf("RTEST: Level %03d\n", ++iLevel);
	printf("Next test - %s\n", aHeading);
}

/* Written: Friday 05-Jun-2009 1:34 pm */

void RTest::Next(const char *aHeading)
{
	printf("RTEST: Level %03d\n", ++iLevel);
	printf("Next test - %s\n", aHeading);
}

/* Written: Friday 05-Jun-2009 1:35 pm */

void RTest::End()
{
	printf("RTEST: SUCCESS : %s test completed O.K.\n", iTitle);
}

void RTest::operator()(TInt aResult, TInt aLineNumber, const char *aFileName)
{
	if (!(aResult))
	{
		printf("RTEST: FAIL : %s failed check at line %d of %s\n", iTitle, aLineNumber, aFileName);
		exit(1); // TODO: CAW - Use Amiga OS code
	}
}
