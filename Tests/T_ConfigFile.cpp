
#include <StdFuncs.h>
#include <StdConfigFile.h>
#include <Test.h>

static RTest Test("T_ConfigFile");	/* Class to use for testing and reporting results */

static void ReadValidNumber(RConfigFile *a_poConfigFile, const char *a_pccKeyName)
{
	TInt Result, ValidNumber;

	Test.Printf("Reading number from key %s\n", a_pccKeyName);

	/* Read in the number from the requested key and ensure that it is read in correctly */

	ValidNumber = 0;
	Result = a_poConfigFile->GetInteger("ConfigFile", "General", a_pccKeyName, &ValidNumber);
	test(Result == KErrNone);
	test(ValidNumber == 42);
}

int main()
{
	TInt Result;
	RConfigFile	ConfigFile;

	Test.Title();
	Test.Start("RConfigFile class API test");

	/* Test #2: Test that Close() can handle being called before Open() */

	Test.Next("Test that Close() can handle being called before Open()");

	ConfigFile.Close();

	/* Open and read in the .ini file */

	Result = ConfigFile.Open("StdConfigFile.ini");
	test(Result == KErrNone);

	/* Test #3: Read a number in from a variety of formatted key = value pairs */

	Test.Next("Read a number in from a variety of formatted key = value pairs");

	ReadValidNumber(&ConfigFile, "ValidNumber1");
	ReadValidNumber(&ConfigFile, "ValidNumber2");
	ReadValidNumber(&ConfigFile, "ValidNumber3");
	ReadValidNumber(&ConfigFile, "ValidNumber4");
	ReadValidNumber(&ConfigFile, "ValidNumber5");

	ConfigFile.Close();

	Test.End();

	return(RETURN_OK);
}
