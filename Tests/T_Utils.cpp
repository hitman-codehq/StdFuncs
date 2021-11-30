
#include <StdFuncs.h>
#include <BaUtils.h>
#include <File.h>
#include <Test.h>
#include <string.h>

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

	/* Test #1: Extract a file from a path with Utils::filePart() */

	Test.Next("Extract a file from a path with Utils::filePart()");

	FileName = Utils::filePart("*.txt");
	test(strlen(FileName) > 0);
	test(strcmp(FileName, "*.txt") == 0);

	FileName = Utils::filePart("some_path/*.txt");
	test(strlen(FileName) > 0);
	test(strcmp(FileName, "*.txt") == 0);

	FileName = Utils::filePart("/*.txt");
	test(strlen(FileName) > 0);
	test(strcmp(FileName, "*.txt") == 0);

	FileName = Utils::filePart("some_volume:*.txt");
	test(strlen(FileName) > 0);
	test(strcmp(FileName, "*.txt") == 0);

	FileName = Utils::filePart(":~(*.info)");
	test(strlen(FileName) > 0);
	test(strcmp(FileName, "~(*.info)") == 0);

	/* Test #2: Extract an extension from a filename with Utils::Extension() */

	Test.Next("Extract an extension from a filename with Utils::Extension()");

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

	Result = BaflUtils::deleteFile("TimeFile.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	Result = File.Create("TimeFile.txt", EFileWrite);
	test(Result == KErrNone);
	File.close();

	Result = Utils::GetFileInfo("T_Utils.cpp", &OldEntry);
	test(Result == KErrNone);

	Result = Utils::setFileDate("TimeFile.txt", OldEntry);
	test(Result == KErrNone);

	Result = Utils::setProtection("TimeFile.txt", OldEntry.iAttributes);
	test(Result == KErrNone);

	Result = Utils::GetFileInfo("TimeFile.txt", &NewEntry);
	test(Result == KErrNone);
	test(OldEntry.iModified == NewEntry.iModified);
	test(OldEntry.iAttributes == NewEntry.iAttributes);

	/* While we are checking these functions, ensure that they return the correct */
	/* return code when a file is not found */

	test(Utils::setFileDate("UnknownFile.txt", OldEntry) == KErrNotFound);
	test(Utils::setProtection("UnknownFile.txt", OldEntry.iAttributes) == KErrNotFound);

	/* Test #7: Ensure we can decode attributes successfully */

	Test.Next("Ensure we can decode attributes successfully");

	Result = Utils::GetFileInfo("TimeFile.txt", &Entry);
	test(Result == KErrNone);

	Test.printf("IsReadable = %d\n", Entry.IsReadable());
	Test.printf("IsWriteable = %d\n", Entry.IsWriteable());
	Test.printf("IsExecutable = %d\n", Entry.IsExecutable());
	Test.printf("IsDeleteable = %d\n", Entry.IsDeleteable());

	Result = Utils::SetDeleteable("TimeFile.txt");
	test(Result == KErrNone);

	Result = Utils::GetFileInfo("TimeFile.txt", &Entry);
	test(Result == KErrNone);

	Test.printf("After Utils::SetDeleteable(), IsDeleteable = %d\n", Entry.IsDeleteable());
	test(Entry.IsDeleteable());

	/* Test #8: Ensure that trying to delete an object that is in use acts sanely */

	Test.Next("Ensure that trying to delete an object that is in use acts sanely");

	/* Delete any old file hanging around from prior runs, then create a file and try */
	/* to delete it while it is open for writing */

	Result = BaflUtils::deleteFile("File.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	Result = File.Replace("File.txt", EFileWrite);
	test(Result == KErrNone);

	Result = BaflUtils::deleteFile("File.txt");
	test((Result == KErrNone) || (Result == KErrInUse));

	File.close();

	/* Ensure the objects aren't hanging around from last run */

	Result = BaflUtils::deleteFile("InUseDirectory/File.txt");
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

	File.close();

	/* Clean up after ourselves */

	Result = BaflUtils::deleteFile("InUseDirectory/File.txt");
	test(Result == KErrNone);

	Result = Utils::DeleteDirectory("InUseDirectory");
	test(Result == KErrNone);

	/* Test #9: Test unsuccessful deleting of a file and directory */

	Test.Next("Test unsuccessful deleting of a file and directory");

	Result = BaflUtils::deleteFile("UnknownFile.txt");
	test(Result == KErrNotFound);

	Result = BaflUtils::deleteFile("UnknownPath/UnknownFile.txt");
	test(Result == KErrPathNotFound);

	Result = Utils::DeleteDirectory("UnknownDirectory");
	test(Result == KErrNotFound);

	Result = Utils::DeleteDirectory("UnknownDirectory/UnknownDirectory");
	test(Result == KErrPathNotFound);

#ifdef __amigaos__

	/* Test some special Amiga cases that were causing BaflUtils::deleteFile() to */
	/* return incorrect return values sometimes */

	Result = BaflUtils::deleteFile("RAM:UnknownFile.txt");
	test(Result == KErrNotFound);

	Result = BaflUtils::deleteFile("RAM:UnknownDirectory/UnknownFile.txt");
	test(Result == KErrPathNotFound);

#endif /* __amigaos__ */

	/* Test #10: Ensure that Utils::ResolveFileName() works */

	Test.Next("Ensure that Utils::ResolveFileName() works");

	FileName = Utils::ResolveFileName("TestFiles/StdConfigFile.ini");
	test(FileName != NULL);
	test(strcmp(Utils::filePart(FileName), "StdConfigFile.ini") == 0);
	Test.printf("Resolved name is %s\n", FileName);

	delete [] (char *) FileName;

	FileName = Utils::ResolveFileName("TestFiles");
	test(FileName != NULL);
	test(FileName[strlen(FileName) - 1] != '\\');
	test(FileName[strlen(FileName) - 1] != '/');
	test(strcmp(Utils::filePart(FileName), "TestFiles") == 0);
	Test.printf("Resolved name is %s\n", FileName);

	delete [] (char *) FileName;

	FileName = Utils::ResolveFileName("TestFiles/");
	test(FileName != NULL);
	test(FileName[strlen(FileName) - 1] != '\\');
	test(FileName[strlen(FileName) - 1] != '/');
	test(strcmp(Utils::filePart(FileName), "TestFiles") == 0);
	Test.printf("Resolved name is %s\n", FileName);

	delete [] (char *) FileName;

	/* Some special tests for root directories, which are handled specially by Utils::ResolveFileName() */

#ifdef WIN32

	FileName = Utils::ResolveFileName("c:/");
	test(FileName != NULL);
	test(strcmp(FileName, "c:\\") == 0);
	Test.printf("Resolved name is %s\n", FileName);

	delete [] (char *) FileName;

	/* The Windows version will convert a slash by itself to a drive letter so check for this */

	FileName = Utils::ResolveFileName("/");
	test(FileName != NULL);
	test(strcmp(&FileName[1], ":\\") == 0);
	Test.printf("Resolved name is %s\n", FileName);

	delete [] (char *) FileName;

#elif defined(__amigaos__)

	/* Amiga OS uses ':' for the root directory.  Assume that this test is being run on the "Work:" volume */

	FileName = Utils::ResolveFileName(":");
	test(FileName != NULL);
	test(strcmp(FileName, "Work:") == 0);
	Test.printf("Resolved name is %s\n", FileName);

	delete [] (char *) FileName;

#else /* ! __amigaos__ */

	/* Other versions will leave a slash as a slash so check for this */

	FileName = Utils::ResolveFileName("/");
	test(FileName != NULL);
	test(strcmp(FileName, "/") == 0);
	Test.printf("Resolved name is %s\n", FileName);

	delete [] (char *) FileName;

#endif /* ! __amigaos__ */

	/* Test #11: Ensure the PROGDIR: prefix works with Utils::ResolveProgDirName() */

	Test.Next("Ensure the PROGDIR: prefix works with Utils::ResolveProgDirName()");

	ProgDirName = Utils::ResolveProgDirName("PROGDIR:T_Utils");
	test(ProgDirName != NULL);
	Test.printf("Resolved name is %s\n", ProgDirName);

	delete [] ProgDirName;

	/* Test #12: Ensure that the PROGDIR: prefix works with Utils::GetFileInfo() */

	Test.Next("Ensure that the PROGDIR: prefix works with Utils::GetFileInfo()");

#ifdef WIN32

	Result = Utils::GetFileInfo("PROGDIR:T_Utils.exe", &Entry);
	test(Result == KErrNone);
	test(strcmp(Entry.iName, "T_Utils.exe") == 0);

#else /* ! WIN32 */

	Result = Utils::GetFileInfo("PROGDIR:T_Utils", &Entry);
	test(Result == KErrNone);
	test(strcmp(Entry.iName, "T_Utils") == 0);

#endif /* ! WIN32 */

	/* Test #13: Ensure that Utils::GetFileInfo() correctly returns failure if wildcards are used */

	Test.Next("Ensure that Utils::GetFileInfo() correctly returns failure if wildcards are used");

 #ifdef __amigaos__

	Result = Utils::GetFileInfo("#?", &Entry);
	test(Result == KErrNotFound);

#else /* ! __amigaos__ */

	Result = Utils::GetFileInfo("*", &Entry);
	test(Result == KErrNotFound);

	Result = Utils::GetFileInfo("?", &Entry);
	test(Result == KErrNotFound);

#endif /* ! __amigaos__ */

	/* Test #14: Ensure Utils::GetFileInfo() can handle different scenarios */

	Test.Next("Ensure Utils::GetFileInfo() can handle different scenarios");

#ifdef WIN32

	/* Check that the special case of "x:\\" is handled correctly */

	Result = Utils::GetFileInfo("c:/", &Entry);
	test(Result == KErrNone);
	test(strcmp(Entry.iName, "/") == 0);

	Result = Utils::GetFileInfo("c:\\", &Entry);
	test(Result == KErrNone);
	test(strcmp(Entry.iName, "\\") == 0);

	/* And also fully qualified pathnames.  These should not end with a slash */

	Result = Utils::GetFileInfo("c:\\Windows", &Entry);
	test(Result == KErrNone);
	test(strcmp(Entry.iName, "Windows") == 0);

	Result = Utils::GetFileInfo("c:\\Windows\\", &Entry);
	test(Result == KErrNotSupported);

#endif /* WIN32 */

	/* Various random directory and file paths */

#ifdef __amigaos__

	/* Amiga OS uses ':' for the root directory */

	Result = Utils::GetFileInfo(":", &Entry);
	test(Result == KErrNone);
	test(strcmp(Entry.iName, ":") == 0);

#else /* ! __amigaos__ */

	/* All other platforms use '/' */

	Result = Utils::GetFileInfo("/", &Entry);
	test(Result == KErrNone);
	test(strcmp(Entry.iName, "/") == 0);

#endif /* ! __amigaos__ */

	Result = Utils::GetFileInfo("SomeDir", &Entry);
	test(Result == KErrNone);
	test(strcmp(Entry.iName, "SomeDir") == 0);

	Result = Utils::GetFileInfo("SomeDir/SomeFile.txt", &Entry);
	test(Result == KErrNone);
	test(strcmp(Entry.iName, "SomeFile.txt") == 0);

	Result = Utils::GetFileInfo("SomeDir/", &Entry);
	test(Result == KErrNotSupported);

	Result = Utils::GetFileInfo("", &Entry);
	test(Result == KErrNotFound);

#ifndef __unix__

	/* For non case dependent operating systems, ensure that using the incorrect case */
	/* works and that it returns the correctly cased name of the directory or file */

	Result = Utils::GetFileInfo("somedir", &Entry);
	test(Result == KErrNone);
	test(strcmp(Entry.iName, "SomeDir") == 0);

	Result = Utils::GetFileInfo("somedir/somefile.txt", &Entry);
	test(Result == KErrNone);
	test(strcmp(Entry.iName, "SomeFile.txt") == 0);

#endif /* __unix__ */

	/* Test #15: Basic Utils::StringToInt() tests */

	Test.Next("Basic Utils::StringToInt() tests");

	Result = Utils::StringToInt("1", &Value);
	test(Result == KErrNone);
	test(Value == 1);

	Result = Utils::StringToInt("a", &Value);
	test(Result == KErrCorrupt);

	Result = Utils::StringToInt("", &Value);
	test(Result == KErrCorrupt);

	/* Clean up after ourselves */

	Result = BaflUtils::deleteFile("TimeFile.txt");
	test(Result == KErrNone);

	Result = BaflUtils::deleteFile("File.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	Test.End();

	return(RETURN_OK);
}
