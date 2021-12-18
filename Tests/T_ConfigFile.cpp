
#include <StdFuncs.h>
#include <StdConfigFile.h>
#include <Test.h>
#include <string.h>

static RTest Test("T_ConfigFile");	/* Class to use for testing and reporting results */

/* Written: Friday 08-Feb-2013 6:55 am, Code HQ Ehinger Tor */
/* @param	a_poConfigFile	Instance of RConfigFile from which to read the number */
/*			a_pccKeyName	Name of the key to be read */
/* Reads a specified integral key from the specified configuration file using */
/* RConfigFile::GetInteger() and confirms that it matches the expected value */

static void ReadValidNumber(RConfigFile *a_poConfigFile, const char *a_pccKeyName)
{
	TInt Result, IntegerValue;

	Test.printf("Reading number from key %s\n", a_pccKeyName);

	/* Read in the number from the requested key and ensure that it is read in correctly */

	IntegerValue = 0;
	Result = a_poConfigFile->GetInteger("Config File", "General", a_pccKeyName, &IntegerValue);
	test(Result == KErrNone);
	test(IntegerValue == 42);
}

/* Written: Saturday 09-Feb-2013 1:43 pm, Code HQ Ehinger Tor */
/* @param	a_poSection		Instance of CSection from which to read the number */
/*			a_pccKeyName	Name of the key to be read */
/* Reads a specified integral key from the already parsed configuration file using */
/* CSection::FindKey() and confirms that it matches the expected value */

static void ReadValidNumber(CSection *a_poSection, const char *a_pccKeyName)
{
	CKey *Key;

	Test.printf("Reading number from key %s\n", a_pccKeyName);

	/* Search for the requested key and ensure that its value is as expected */

	Key = a_poSection->FindKey(a_pccKeyName);
	test(Key != NULL);
	test(strcmp(Key->m_pcValue, "42") == 0);
}

/* Written: Saturday 09-Feb-2013 1:13 pm, Code HQ Ehinger Tor */
/* @param	a_poConfigFile	Instance of RConfigFile from which to read the numbers */
/* Performs tests on the functions that enable more dynamic reading of the sections */
/* present in the configuration file */

static void TestDynamicReads(RConfigFile *a_poConfigFile)
{
	CKey *Key;
	CSection *Group, *Section, *SubSection;

	/* Test #7: Query for basic values in a particular subsection */

	Test.Next("Query for basic values in a particular subsection");

	/* First find the "General" subsection in the "Config File" section */

	Section = a_poConfigFile->FindSection("Config File");
	test(Section != NULL);

	SubSection = Section->FindSection("General");
	test(SubSection != NULL);

	/* Now query for the keys we know are present (with various amounts of white space) */

	ReadValidNumber(SubSection, "ValidNumber1");
	ReadValidNumber(SubSection, "ValidNumber2");
	ReadValidNumber(SubSection, "ValidNumber3");
	ReadValidNumber(SubSection, "ValidNumber4");
	ReadValidNumber(SubSection, "ValidNumber5");

	/* Test #8: Query for known keys in a known section::subsection::group */

	Test.Next("Query for known keys in a known section::subsection::group");

	Section = a_poConfigFile->FindSection("ValidSection");
	test(Section != NULL);

	SubSection = Section->FindSection("SyntaxHighlighting");
	test(SubSection != NULL);

	Group = SubSection->FindSection("C / C++");
	test(SubSection != NULL);

	Key = Group->FindKey("SyntaxExtensions");
	test(Key != NULL);
	test(strcmp(Key->m_pcValue, ".cpp;.c;.h") == 0);

	Key = Group->FindKey("CommentDelimiters");
	test(Key != NULL);
	test(strcmp(Key->m_pcValue, "//") == 0);

	Key = Group->FindNextKey(Key, "CommentDelimiters");
	test(Key != NULL);
	test(strcmp(Key->m_pcValue, "/* */") == 0);

	Key = Group->FindNextKey(Key, "CommentDelimiters");
	test(Key == NULL);

	Group = SubSection->FindSection("Python");
	test(SubSection != NULL);

	Key = Group->FindKey("CommentDelimiters");
	test(Key != NULL);
	test(strcmp(Key->m_pcValue, "#") == 0);

	Key = Group->FindNextKey(Key, "CommentDelimiters");
	test(Key == NULL);

	Key = Group->FindKey("SyntaxExtensions");
	test(Key != NULL);
	test(strcmp(Key->m_pcValue, ".py") == 0);

	/* Test #8: Query for known keys in an unknown group (well we know it is there but only */
	/* for the purposes of this test */

	Test.Next("Query for known keys in an unknown group");

	Group = SubSection->FindSection();
	test(Group != NULL);
	test(strcmp(Group->m_pcName, "C / C++") == 0);

	Group = SubSection->FindNextSection(Group);
	test(Group != NULL);
	test(strcmp(Group->m_pcName, "Python") == 0);

	Group = SubSection->FindNextSection(Group);
	test(Group == NULL);
}

int main()
{
	char *StringValue;
	TInt Result;
	RConfigFile	ConfigFile;

	Test.Title();
	Test.Start("RConfigFile class API test");

	/* Test #2: Test that close() can handle being called before open() */

	Test.Next("Test that close() can handle being called before open()");

	ConfigFile.close();

	/* Test #3: Ensure that open() can handle an invalid filename being passed in */

	Test.Next("Ensure that open() can handle an invalid filename being passed in");

	Result = ConfigFile.open("UnknownFile.ini");
	test(Result = KErrNotFound);

	/* Test #4: Ensure that open() can parse a mostly valid .ini file */

	Test.Next("Ensure that open() can parse a mostly valid .ini file");

	Result = ConfigFile.open("TestFiles/StdConfigFile.ini");
	test(Result == KErrNone);

	/* Test #5: Ensure invalid key reads fail and valid ones pass */

	Test.Next("Ensure invalid key reads fail and valid ones pass");

	/* There are two instances of the NotFound1 key in the test file.  Ensure that the first */
	/* really isn't found and that the second is, and that the correct instance is found */

	Result = ConfigFile.GetString("InvalidSection", "General", "InvalidInteger1", StringValue);
	test(Result == KErrNotFound);

	Result = ConfigFile.GetString("Config File", "General", "InvalidInteger1", StringValue);
	test(Result == KErrNone);
	test(strcmp(StringValue, "Found") == 0);

	delete [] StringValue;

	/* There are two instances of "ValidString1" in the .ini file so ensure the correct one is found */

	Result = ConfigFile.GetString("Config File", "General", "ValidString1", StringValue);
	test(Result == KErrNone);
	test(strcmp(StringValue, "Valid1") == 0);

	delete [] StringValue;

	/* There are two instances of "ValidString2" in the .ini file so ensure the correct one is found */

	Result = ConfigFile.GetString("Config File", "Next Sub Section", "ValidString2", StringValue);
	test(Result == KErrNone);
	test(strcmp(StringValue, "Valid2") == 0);

	delete [] StringValue;

	/* Try various combinations of invalid section and/or subsection names */

	Result = ConfigFile.GetString("NonExisting", "General", "ValidString1", StringValue);
	test(Result == KErrNotFound);

	Result = ConfigFile.GetString("Config File", "NonExisting", "ValidString1", StringValue);
	test(Result == KErrNotFound);

	Result = ConfigFile.GetString("NonExisting", "NonExisting", "ValidString1", StringValue);
	test(Result == KErrNotFound);

	/* Ensure that we don't match partial sections, subsections or keys.  ie.  Searching for */
	/* "ValidString" should not return a match for "ValidString1" */

	Result = ConfigFile.GetString("Config Fil", "General", "ValidString1", StringValue);
	test(Result == KErrNotFound);

	Result = ConfigFile.GetString("Config File", "Genera", "ValidString1", StringValue);
	test(Result == KErrNotFound);

	Result = ConfigFile.GetString("Config File", "General", "ValidString", StringValue);
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

	ConfigFile.close();

	Test.End();

	return(RETURN_OK);
}
