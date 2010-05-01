
#ifndef TEST_H
#define TEST_H

#define test(x) Test(x, __LINE__, __FILE__)

class RTest
{
	const char	*iTitle;	/* Title to be printed by Title() */
	TInt		iLevel;		/* # of currently executing test */

public:

	RTest(const char *aTitle);

	void Title();

	void Printf(const char *aString, ...);

	void Start(const char *aHeading);

	void Next(const char *aHeading);

	void End();

	void operator()(TInt aResult, TInt aLineNumber, const char *aFileName);
};

#endif /* ! TEST_H */
