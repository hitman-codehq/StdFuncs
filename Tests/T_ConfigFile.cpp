
#include <StdFuncs.h>
#include <StdConfigFile.h>
#include <Test.h>

static RTest Test("T_ConfigFile");	/* Class to use for testing and reporting results */

/* Written: Friday 08-Feb-2013 6:55 am, Code HQ Ehinger Tor */
/* @param	a_poConfigFile	Instance of RConfigFile from which to read the numbers */
/*			a_pccKeyName	Name of the key to be read */
/* Reads a specified integral key from the specified configuration file using */
/* RConfigFile::GetInteger() and confirms that it matches the expected value */

static void ReadValidNumber(RConfigFile *a_poConfigFile, const char *a_pccKeyName)
{
	TInt Result, IntegerValue;

	Test.Printf("Reading number from key %s\n", a_pccKeyName);

	/* Read in the number from the requested key and ensure that it is read in correctly */

	IntegerValue = 0;
	Result = a_poConfigFile->GetInteger("ConfigFile", "General", a_pccKeyName, &IntegerValue);
	test(Result == KErrNone);
	test(IntegerValue == 42);
}

/* Written: Saturday 09-Feb-2013 1:13 pm, Code HQ Ehinger Tor */
/* @param	a_poConfigFile	Instance of RConfigFile from which to read the numbers */
/* Performs tests on the functions that enable more dynamic reading of the sections */
/* present in the configuration file */

static void TestDynamicReads(RConfigFile *a_poConfigFile)
{
}

int main()
{
	char *StringValue;
	TInt Result;
	RConfigFile	ConfigFile;

	Test.Title();
	Test.Start("RConfigFile class API test");

	/* Test #2: Test that Close() can handle being called before Open() */

	Test.Next("Test that Close() can handle being called before Open()");

	ConfigFile.Close();

	/* Test #3: Ensure that Open() can handle an invalid filename being passed in */

	Test.Next("Ensure that Open() can handle an invalid filename being passed in");

	Result = ConfigFile.Open("UnknownFile.ini");
	test(Result = KErrNotFound);

	/* Test #4: Ensure that Open() can parse a mostly valid .ini file */

	Test.Next("Ensure that Open() can parse a mostly valid .ini file");

	Result = ConfigFile.Open("TestFiles/StdConfigFile.ini");
	test(Result == KErrNone);

	/* Test #5: Ensure invalid key reads fail and valid ones pass */

	Test.Next("Ensure invalid key reads fail and valid ones pass");

	/* There are two instances of the NotFound1 key in the test file.  Ensure that the first */
	/* really isn't found and that the second is, and that the correct instance is found */

	Result = ConfigFile.GetString("InvalidSection", "General", "InvalidInteger1", StringValue);
	test(Result == KErrNotFound);

	Result = ConfigFile.GetString("ConfigFile", "General", "InvalidInteger1", StringValue);
	test(Result == KErrNone);
	test(strcmp(StringValue, "Found") == 0);

	delete [] StringValue;

	/* There are two instances of "ValidString1" in the .ini file so ensure the correct one is found */

	Result = ConfigFile.GetString("ConfigFile", "General", "ValidString1", StringValue);
	test(Result == KErrNone);
	test(strcmp(StringValue, "Valid1") == 0);

	delete [] StringValue;

	/* There are two instances of "ValidString2" in the .ini file so ensure the correct one is found */

	Result = ConfigFile.GetString("ConfigFile", "NextSubSection", "ValidString2", StringValue);
	test(Result == KErrNone);
	test(strcmp(StringValue, "Valid2") == 0);

	delete [] StringValue;

	/* Try various combinations of invalid section and/or subsection names */

	Result = ConfigFile.GetString("NonExisting", "General", "ValidString1", StringValue);
	test(Result == KErrNotFound);

	Result = ConfigFile.GetString("ConfigFile", "NonExisting", "ValidString1", StringValue);
	test(Result == KErrNotFound);

	Result = ConfigFile.GetString("NonExisting", "NonExisting", "ValidString1", StringValue);
	test(Result == KErrNotFound);

	/* Ensure that we don't match partial sections, subsections or keys.  ie.  Searching for */
	/* "ValidString" should not return a match for "ValidString1" */

	Result = ConfigFile.GetString("ConfigFil", "General", "ValidString1", StringValue);
	test(Result == KErrNotFound);

	Result = ConfigFile.GetString("ConfigFile", "Genera", "ValidString1", StringValue);
	test(Result == KErrNotFound);

	Result = ConfigFile.GetString("ConfigFile", "General", "ValidString", StringValue);
	test(Result == KErrNotFound);

	/* Malformed section or subsection names should fail to be recognised */

	Result = ConfigFile.GetString("MalformedSection", "ValidSubSection", "ValidKey", StringValue);
	test(Result == KErrNotFound);

	Result = ConfigFile.GetString("MalformedSectio", "ValidSubSection", "ValidKey", StringValue);
	test(Result == KErrNotFound);

	Result = ConfigFile.GetString("ValidSection", "MalformedSubSectio", "ValidKey", StringValue);
	test(Result == KErrNotFound);

	Result = ConfigFile.GetString("ValidSection", "MalformedSubSection", "ValidKey", StringValue);
	test(Result == KErrNotFound);

	/* Test #6: Read a number in from a variety of formatted key = value pairs */

	Test.Next("Read a number in from a variety of formatted key = value pairs");

	ReadValidNumber(&ConfigFile, "ValidNumber1");
	ReadValidNumber(&ConfigFile, "ValidNumber2");
	ReadValidNumber(&ConfigFile, "ValidNumber3");
	ReadValidNumber(&ConfigFile, "ValidNumber4");
	ReadValidNumber(&ConfigFile, "ValidNumber5");

	/* Now test the dynamic reading routines */

	TestDynamicReads(&ConfigFile);

	ConfigFile.Close();

	Test.End();

	return(RETURN_OK);
}
