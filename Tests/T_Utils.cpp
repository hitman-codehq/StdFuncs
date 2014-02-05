
#include <StdFuncs.h>
#include <BaUtils.h>
#include <File.h>
#include <Test.h>
#include <string.h>

#include <stdio.h>

static RTest Test("T_Utils");

int main()
{
	char *ProgDirName;
	const char *Extension, *FileName;
	TInt Result, Value;
	RFile File;
	TEntry Entry, OldEntry, NewEntry;

	Test.Title();
	Test.Start("Utils class API test");

	/* Test #1: Extract a file from a path with Utils::FilePart() */

	Test.Next("Extract a file from a path with Utils::FilePart()");

	FileName = Utils::FilePart("*.txt");
	test(strlen(FileName) > 0);
	test(strcmp(FileName, "*.txt") == 0);

	FileName = Utils::FilePart("some_path/*.txt");
	test(strlen(FileName) > 0);
	test(strcmp(FileName, "*.txt") == 0);

	FileName = Utils::FilePart("some_volume:*.txt");
	test(strlen(FileName) > 0);
	test(strcmp(FileName, "*.txt") == 0);

	/* Test #2: Extract an extension from a file name with Utils::Extension() */

	Test.Next("Extract an extension from a file name with Utils::Extension()");

	Extension = Utils::Extension("*.txt");
	test(Extension != NULL);
	test(strcmp(Extension, "txt") == 0);

	Extension = Utils::Extension(".");
	test(Extension != NULL);
	test(strcmp(Extension, "") == 0);

	Extension = Utils::Extension("*txt");
	test(Extension == NULL);

	/* Test #3: Trimming white space from the start & end of a string */

	Test.Next("Trimming white space from the start & end of a string");

	/* Ensure white space is trimmed from both ends of the string */

	char String[] = { " hello world " };
	Utils::TrimString(String);
	test(strlen(String) == 11);
	test(String[0] == 'h');
	test(String[strlen(String) - 1] == 'd');

	/* Ensure white space is trimmed from the start of the string */

	char String2[] = { "\thello world" };
	Utils::TrimString(String2);
	test(strlen(String2) == 11);
	test(String2[0] == 'h');
	test(String2[strlen(String2) - 1] == 'd');

	/* Ensure white space is trimmed from the end of the string */

	char String3[] = { "hello world\t" };
	Utils::TrimString(String3);
	test(strlen(String3) == 11);
	test(String3[0] == 'h');
	test(String3[strlen(String3) - 1] == 'd');

	/* Test #4: Ensure that Utils::CountTokens() functions correctly */

	Test.Next("Ensure that Utils::CountTokens() functions correctly");
	test(Utils::CountTokens("") == 0);
	test(Utils::CountTokens(" ") == 0);
	test(Utils::CountTokens("One") == 1);
	test(Utils::CountTokens("One Two") == 2);
	test(Utils::CountTokens("One \"Two\"") == 2);
	test(Utils::CountTokens("\"One\" \"Two\"") == 2);
	test(Utils::CountTokens("\"One\" Two") == 2);
	test(Utils::CountTokens("\"One\"\"Two\"") == 2);
	test(Utils::CountTokens("\"One\"Two") == 2);
	test(Utils::CountTokens("One\"Two\"") == 2);

	/* A subset of the above tests but using tabs instead of spaces */

	test(Utils::CountTokens("\t") == 0);
	test(Utils::CountTokens("One\tTwo") == 2);
	test(Utils::CountTokens("One\t\"Two\"") == 2);
	test(Utils::CountTokens("\"One\"\t\"Two\"") == 2);
	test(Utils::CountTokens("\"One\"\tTwo") == 2);

	/* Some checks for handling extra white space */

	test(Utils::CountTokens("\tOne\t\tTwo\t") == 2);
	test(Utils::CountTokens(" One  Two ") == 2);

	/* Test #5: Ensure Utils::CreateDirectory() and Utils::DeleteDirectory() work */

	Test.Next("Ensure Utils::CreateDirectory() and Utils::DeleteDirectory() work");
	Result = Utils::DeleteDirectory("SomeDirectory");
	test((Result == KErrNone) || (Result == KErrNotFound));

	test(Utils::CreateDirectory("SomeDirectory") == KErrNone);
	test(Utils::CreateDirectory("SomeDirectory") == KErrAlreadyExists);
	test(Utils::DeleteDirectory("SomeDirectory") == KErrNone);
	test(Utils::CreateDirectory("x/SomeDirectory") == KErrNotFound);

	/* Test #6: Ensure that we can set the file date and time on a file */

	Test.Next("Ensure that we can set the file date and time on a file");

	/* Delete any old file hanging around from prior runs, create a new one */
	/* and set its time and attributes to be the same as the source code */
	/* for this test */

	Result = BaflUtils::DeleteFile("TimeFile.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	Result = File.Create("TimeFile.txt", EFileWrite);
	test(Result == KErrNone);
	File.Close();

	Result = Utils::GetFileInfo("T_Utils.cpp", &OldEntry);
	test(Result == KErrNone);

	Result = Utils::SetFileDate("TimeFile.txt", OldEntry);
	test(Result == KErrNone);

	Result = Utils::SetProtection("TimeFile.txt", OldEntry.iAttributes);
	test(Result == KErrNone);

	Result = Utils::GetFileInfo("TimeFile.txt", &NewEntry);
	test(Result == KErrNone);
	test(OldEntry.iModified == NewEntry.iModified);
	test(OldEntry.iAttributes == NewEntry.iAttributes);

	/* While we are checking these functions, ensure that they return the correct */
	/* return code when a file is not found */

	test(Utils::SetFileDate("UnknownFile.txt", OldEntry) == KErrNotFound);
	test(Utils::SetProtection("UnknownFile.txt", OldEntry.iAttributes) == KErrNotFound);

	/* Test #7: Ensure we can decode attributes successfully */

	Test.Next("Ensure we can decode attributes successfully");

	Result = Utils::GetFileInfo("TimeFile.txt", &Entry);
	test(Result == KErrNone);

	Test.Printf("IsReadable = %d\n", Entry.IsReadable());
	Test.Printf("IsWriteable = %d\n", Entry.IsWriteable());
	Test.Printf("IsExecutable = %d\n", Entry.IsExecutable());
	Test.Printf("IsDeleteable = %d\n", Entry.IsDeleteable());

	Result = Utils::SetDeleteable("TimeFile.txt");
	test(Result == KErrNone);

	Result = Utils::GetFileInfo("TimeFile.txt", &Entry);
	test(Result == KErrNone);

	Test.Printf("After Utils::SetDeleteable(), IsDeleteable = %d\n", Entry.IsDeleteable());
	test(Entry.IsDeleteable());

	/* Test #8: Ensure that trying to delete an object that is in use acts sanely */

	Test.Next("Ensure that trying to delete an object that is in use acts sanely");

	/* Delete any old file hanging around from prior runs, then create a file and try */
	/* to delete it while it is open for writing */

	Result = BaflUtils::DeleteFile("File.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	Result = File.Replace("File.txt", EFileWrite);
	test(Result == KErrNone);

	Result = BaflUtils::DeleteFile("File.txt");
	test((Result == KErrNone) || (Result == KErrInUse));

	File.Close();

	/* Ensure the objects aren't hanging around from last run */

	Result = BaflUtils::DeleteFile("InUseDirectory/File.txt");
	test((Result == KErrNone) || (Result == KErrNotFound) || (Result == KErrPathNotFound));

	Result = Utils::DeleteDirectory("InUseDirectory");
	test((Result == KErrNone) || (Result == KErrNotFound));

	/* Create a directory and try to delete it while it is use */

	Result = Utils::CreateDirectory("InUseDirectory");
	test(Result == KErrNone);

	Result = File.Create("InUseDirectory/File.txt", EFileWrite);
	test(Result == KErrNone);

	Result = Utils::DeleteDirectory("InUseDirectory");
	test(Result == KErrInUse);

	File.Close();

	/* Clean up after ourselves */

	Result = BaflUtils::DeleteFile("InUseDirectory/File.txt");
	test(Result == KErrNone);

	Result = Utils::DeleteDirectory("InUseDirectory");
	test(Result == KErrNone);

	/* Test #9: Test unsuccessful deleting of a file and directory */

	Test.Next("Test unsuccessful deleting of a file and directory");

	Result = BaflUtils::DeleteFile("UnknownFile.txt");
	test(Result == KErrNotFound);

	Result = BaflUtils::DeleteFile("UnknownPath/UnknownFile.txt");
	test(Result == KErrPathNotFound);

	Result = Utils::DeleteDirectory("UnknownDirectory");
	test(Result == KErrNotFound);

	Result = Utils::DeleteDirectory("UnknownDirectory/UnknownDirectory");
	test(Result == KErrPathNotFound);

#ifdef __amigaos4__

	/* Test some special Amiga cases that were causing BaflUtils::DeleteFile() to */
	/* return incorrect return values sometimes */

	Result = BaflUtils::DeleteFile("RAM:UnknownFile.txt");
	test(Result == KErrNotFound);

	Result = BaflUtils::DeleteFile("RAM:UnknownDirectory/UnknownFile.txt");
	test(Result == KErrPathNotFound);

#endif /* __amigaos4__ */

	/* Test #10: Ensure that Utils::ResolveFileName() works */

	Test.Next("Ensure that Utils::ResolveFileName() works");

#ifdef WIN32

#ifdef _DEBUG

	FileName = Utils::ResolveFileName("T_Utils_Debug/T_Utils.exe");

#else /* ! _DEBUG */

	FileName = Utils::ResolveFileName("T_Utils_Release/T_Utils.exe");

#endif /* ! _DEBUG */

#else /* ! WIN32 */

#ifdef _DEBUG

	FileName = Utils::ResolveFileName("Debug/T_Utils");

#else /* ! _DEBUG */

	FileName = Utils::ResolveFileName("Release/T_Utils");

#endif /* ! _DEBUG */

#endif /* ! WIN32 */

	test(FileName != NULL);
	Test.Printf("Resolved name is %s\n", FileName);

	delete [] (char *) FileName;

	/* Test #11: Ensure the PROGDIR: prefix works with Utils::ResolveProgDirName() */

	Test.Next("Ensure the PROGDIR: prefix works with Utils::ResolveProgDirName()");

	ProgDirName = Utils::ResolveProgDirName("PROGDIR:T_Utils");
	test(ProgDirName != NULL);
	Test.Printf("Resolved name is %s\n", ProgDirName);

	delete [] ProgDirName;

	/* Test #12: Ensure that the PROGDIR: prefix works with Utils::GetFileInfo() */

	Test.Next("Ensure that the PROGDIR: prefix works with Utils::GetFileInfo()");

#ifdef WIN32

	Result = Utils::GetFileInfo("PROGDIR:T_Utils.exe", &Entry);
	test(strcmp(Entry.iName, "T_Utils.exe") == 0);
	test(Result == KErrNone);

#else /* ! WIN32 */

	Result = Utils::GetFileInfo("PROGDIR:T_Utils", &Entry);
	test(strcmp(Entry.iName, "T_Utils") == 0);
	test(Result == KErrNone);

#endif /* ! WIN32 */

	/* Test #13: Ensure that Utils::GetFileInfo() correctly returns failure if wildcards are used */

	Test.Next("Ensure that Utils::GetFileInfo() correctly returns failure if wildcards are used");

 #ifdef __amigaos4__

	Result = Utils::GetFileInfo("#?", &Entry);
	test(Result == KErrNotFound);

#else /* ! __amigaos4__ */

	Result = Utils::GetFileInfo("*", &Entry);
	test(Result == KErrNotFound);

	Result = Utils::GetFileInfo("?", &Entry);
	test(Result == KErrNotFound);

#endif /* ! __amigaos4__ */

	/* Test #14: Basic Utils::StringToInt() tests */

	Test.Next("Basic Utils::StringToInt() tests");

	Result = Utils::StringToInt("1", &Value);
	test(Result == KErrNone);
	test(Value == 1);

	Result = Utils::StringToInt("a", &Value);
	test(Result == KErrCorrupt);

	Result = Utils::StringToInt("", &Value);
	test(Result == KErrCorrupt);

	/* Clean up after ourselves */

	Result = BaflUtils::DeleteFile("TimeFile.txt");
	test(Result == KErrNone);

	Result = BaflUtils::DeleteFile("File.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	Test.End();

	return(RETURN_OK);
}
