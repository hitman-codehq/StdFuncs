
#include <StdFuncs.h>
#include <BaUtils.h>
#include <File.h>
#include <string.h>
#include <Test.h>

static const char *g_pccWriteText = "This is a test of file reading and writing\n";
static RTest Test("T_File");

/* Writes text to an already open file, closes and reopens the file and reads the text */
/* back in to ensure that it was written correctly.  Note that this will overwrite any */
/* text already in the file from previous calls! */

static void WriteToFile(RFile &a_roFile)
{
	char Buffer[256];
	int Length, Result;

	Length = strlen(g_pccWriteText);
	Result = a_roFile.Write((const unsigned char *) g_pccWriteText, Length);
	test(Result == Length);

	a_roFile.Close();

	Result = a_roFile.Open("File.txt", EFileRead);
	test(Result == KErrNone);

	Result = a_roFile.Read((unsigned char *) Buffer, Length);
	test(Result == Length);
	Buffer[Length] = '\0';
	test(!(strcmp(Buffer, g_pccWriteText)));
}

int main()
{
	char Buffer[256];
	int Result;

	Test.Title();
	Test.Start("RFile class API test");

	RFile File, File2;

	// TODO: CAW - These numbers don't match up with those from RTest
	/* Test #1: Test that RFile.Replace() works as expected */

	Test.Next("Creating file with RFile::Replace()");

	/* The test file may be hanging around from the last time the test was run */

	Result = BaflUtils::DeleteFile("File.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	/* Ensure that RFile::Replace() works both when the file does not exist and when it does */

	Result = File.Replace("File.txt", EFileWrite);
	test(Result == KErrNone);
	File.Close();

	Result = File.Replace("File.txt", EFileWrite);
	test(Result == KErrNone);
	File.Close();

	/* Test #2: Rename a file using BaflUtils::RenameFile() */

	Test.Next("Renaming a file with BaflUtils::RenameFile()");

	/* The test file may be hanging around from the last time the test was run */

	Result = BaflUtils::DeleteFile("NewFile.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	Result = BaflUtils::RenameFile("File.txt", "NewFile.txt");
	test(Result == KErrNone);

	/* Clean up after ourselves */

	Result = BaflUtils::DeleteFile("NewFile.txt");
	test(Result == KErrNone);

	/* Test #3: Test that RFile::Create() and RFile::Open() work as expected */

	Test.Next("Creating and opening files with RFile::Create() and RFile::Open()");

	Result = File.Create("File.txt", EFileWrite);
	test(Result == KErrNone);

	/* Ensure that files cannot be opened in a shared mode */

	Result = File2.Open("File.txt", EFileRead);
	test(Result == KErrGeneral);

	Result = File2.Open("File.txt", EFileWrite);
	test(Result == KErrGeneral);

	File.Close();

	Result = File.Open("File.txt", EFileWrite);
	test(Result == KErrNone);

	/* Ensure that files cannot be opened in a shared mode */

	Result = File2.Open("File.txt", EFileRead);
	test(Result == KErrGeneral);

	Result = File2.Open("File.txt", EFileWrite);
	test(Result == KErrGeneral);

	File.Close();

	/* Ensure other Create() and Open() errors are as expected */

	Result = File.Create("File.txt", EFileWrite);
	test(Result == KErrAlreadyExists);

	Result = File.Create("UnknownPath/File.txt", EFileWrite);
	test(Result == KErrPathNotFound);

	Result = File.Open("UnknownPath/UnknownFile.txt", EFileRead);
	test(Result == KErrPathNotFound);

	Result = File.Open("UnknownFile.txt", EFileRead);
	test(Result == KErrNotFound);

	/* Test #4: Test that we are able to create and write to files using both supported APIs */

	Test.Next("Creating and writing to files using both supported APIs");

	Result = BaflUtils::DeleteFile("File.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	Result = File.Create("File.txt", EFileWrite);
	test(Result == KErrNone);

	/* Ensure that trying to read 0 bytes is handled sanely */

	Result = File.Read((unsigned char *) Buffer, 0);
	test(Result == 0);

	/* Ensure that trying to write 0 bytes is handled sanely */

	Result = File.Write((const unsigned char *) g_pccWriteText, 0);
	test(Result == 0);

	/* Now write some real data */

	WriteToFile(File);

	File.Close();

	Result = File.Open("File.txt", EFileWrite);
	test(Result == KErrNone);

	/* Ensure that trying to read 0 bytes is handled sanely */

	Result = File.Read((unsigned char *) Buffer, 0);
	test(Result == 0);

	WriteToFile(File);

	File.Close();

	/* Ensure we can't write to a file opened in read only mode */

	Result = File.Open("File.txt", EFileRead);
	test(Result == KErrNone);

	Result = File.Write((const unsigned char *) g_pccWriteText, strlen(g_pccWriteText));
	test(Result == KErrGeneral);

	File.Close();

	/* Clean up after ourselves */

	Result = BaflUtils::DeleteFile("File.txt");
	test(Result == KErrNone);

	Test.End();

	return(RETURN_OK);
}
