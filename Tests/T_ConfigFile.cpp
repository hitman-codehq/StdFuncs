
#include <StdFuncs.h>
#include <StdConfigFile.h>
#include <Test.h>

static RTest Test("T_ConfigFile");	/* Class to use for testing and reporting results */

int main()
{
	int Result;

	Test.Title();
	Test.Start("RConfigFile class API test");

	Test.End();

	return(RETURN_OK);
}
