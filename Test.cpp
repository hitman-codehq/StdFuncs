
#include "StdFuncs.h"
#include "Test.h"
#include <stdio.h>

/* Written: Friday 05-Jun-2009 11:47 am */

RTest::RTest(const char *a_pccTitle)
{
	m_pccTitle = a_pccTitle;
	m_iLevel = 0;
}

/* Written: Friday 05-Jun-2009 11:48 am */

void RTest::Title()
{
	printf("RTEST TITLE: %s\n", m_pccTitle);
}

/* Written: Monday 06-Jul 2009 3:56 pm */

void RTest::printf(const char *a_pccString, ...)
{
	va_list Args;

	va_start(Args, a_pccString);
	vprintf(a_pccString, Args);
	va_end(Args);
}

/* Written: Friday 05-Jun-2009 1:14 pm */

void RTest::Start(const char *a_pccHeading)
{
	printf("RTEST: Level %03d\n", ++m_iLevel);
	printf("Next test - %s\n", a_pccHeading);
}

/* Written: Friday 05-Jun-2009 1:34 pm */

void RTest::Next(const char *a_pccHeading)
{
	printf("RTEST: Level %03d\n", ++m_iLevel);
	printf("Next test - %s\n", a_pccHeading);
}

/* Written: Friday 05-Jun-2009 1:35 pm */

void RTest::End()
{
	printf("RTEST: SUCCESS : %s test completed O.K.\n", m_pccTitle);
}

void RTest::operator()(TInt a_iResult, TInt a_iLineNumber, const char *a_pccFileName)
{
	if (!(a_iResult))
	{
		oMungWall.EnableOutput(FALSE);

		printf("RTEST: FAIL : %s failed check at line %d of %s\n", m_pccTitle, a_iLineNumber, a_pccFileName);
		exit(RETURN_ERROR);
	}
}
