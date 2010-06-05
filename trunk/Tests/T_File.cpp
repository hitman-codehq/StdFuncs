
#include <StdFuncs.h>
#include <BaUtils.h>
#include <File.h>
#include <string.h>
#include "Test.h"

static const char *WriteText = "This is a test of file reading and writing\n";
static RTest Test("T_File");

int main()
{
	char Buffer[256];
	int Result, Length;

	Test.Title();
	Test.Start("RFile class API test");

	RFile File;

	// TODO: CAW - These numbers don't match up with those from RTest
	/* Test #1: Test that RFile.Replace() works as expected */

	Test.Next("Creating file with RFile::Replace()");

	/* The test file may be hanging around from the last time the test was run so delete it */
	/* so that it does not cause behavioural changes in the following tests */

	Result = BaflUtils::DeleteFile("File.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	/* Ensure that RFile::Replace() works both when the file does not exist and when it does */

	Result = File.Replace("File.txt", EFileWrite);
	test(Result == KErrNone);
	File.Close();

	Result = File.Replace("File.txt", EFileWrite);
	test(Result == KErrNone);
	File.Close();

	/* Test #2: Rename a file using BaflUtils::Rename() */

	Test.Next("Renaming a file with BaflUtils::Rename()");

	Result = BaflUtils::DeleteFile("NewFile.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	Result = BaflUtils::RenameFile("File.txt", "NewFile.txt");
	test(Result == KErrNone);

	/* Test #3: Test that RFile::Create() and RFile::Open() work as expected */

	Test.Next("Creating and opening files with RFile::Create() and RFile::Open()");

	/* The test file will be hanging around from the last time the test was run so delete it */
	/* so that it does not cause behavioural changes in the following tests */

	Result = BaflUtils::DeleteFile("NewFile.txt");
	test(Result == KErrNone);

	Result = File.Create("File.txt", EFileWrite);
	test(Result == KErrNone);
	File.Close();

	// TODO: CAW - Why doesn't this fail on Amiga OS?
	Result = File.Create("File.txt", EFileWrite);
	test(Result == KErrAlreadyExists);

	Result = File.Create("x:\\File.txt", EFileWrite);
	test(Result == KErrPathNotFound);

	Result = File.Open("UnknownFile.txt", EFileRead);
	test(Result == KErrNotFound);

	Result = File.Open("File.txt", EFileWrite);
	test(Result == KErrNone);

	Length = strlen(WriteText);
	Result = File.Write((const unsigned char *) WriteText, Length);
	test(Result == Length);

	File.Close();

	Result = File.Open("File.txt", EFileRead);
	test(Result == KErrNone);

	Result = File.Read((unsigned char *) Buffer, (Length * 2));
	test(Result == Length);
	Buffer[Length] = '\0';
	test(!(strcmp(Buffer, WriteText)));

	File.Close();

	Result = BaflUtils::DeleteFile("File.txt");
	test(Result == KErrNone);

	Test.End();

	return(RETURN_OK);
}
