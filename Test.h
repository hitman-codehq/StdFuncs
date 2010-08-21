
#ifndef TEST_H
#define TEST_H

#define test(x) Test(x, __LINE__, __FILE__)

class RTest
{
	const char	*iTitle;	/* Title to be printed by Title() */
	TInt		iLevel;		/* # of currently executing test */

public:

	RTest(const char *a_pccTitle);

	void Title();

	void Printf(const char *a_ccString, ...);

	void Start(const char *a_pccHeading);

	void Next(const char *a_pccHeading);

	void End();

	void operator()(TInt a_iResult, TInt a_iLineNumber, const char *a_pccFileName);
};

#endif /* ! TEST_H */
