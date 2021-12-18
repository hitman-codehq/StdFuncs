
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
	Result = a_roFile.write((const unsigned char *) g_pccWriteText, Length);
	test(Result == Length);

	a_roFile.close();

	Result = a_roFile.open("File.txt", EFileRead);
	test(Result == KErrNone);

	Result = a_roFile.read((unsigned char *) Buffer, Length);
	test(Result == Length);
	Buffer[Length] = '\0';
	test(!(strcmp(Buffer, g_pccWriteText)));
}

void TestExclusiveMode()
{
	int Result;
	RFile File, File2;

	Result = BaflUtils::deleteFile("File.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	/* Test #4: Test that RFile::Create() and RFile::open() work as expected */

	Test.Next("Creating and opening files with RFile::Create() and RFile::open() in exclusive mode");

	/* Ensure that files cannot be opened in an exclusive mode when created with RFile::Create() */

	Result = File.Create("File.txt", EFileWrite | EFileExclusive);
	test(Result == KErrNone);

	Result = File2.open("File.txt", EFileRead | EFileExclusive);
	test(Result == KErrInUse);

	Result = File2.open("File.txt", EFileWrite | EFileExclusive);
	test(Result == KErrInUse);

	File.close();

	/* Ensure that files cannot be opened in an exclusive mode when opened with RFile::open() */
	/* in writeable mode */

	Result = File.open("File.txt", EFileWrite | EFileExclusive);
	test(Result == KErrNone);

	Result = File2.open("File.txt", EFileRead | EFileExclusive);
	test(Result == KErrInUse);

	Result = File2.open("File.txt", EFileWrite | EFileExclusive);
	test(Result == KErrInUse);

	File.close();

	/* Ensure that files cannot be opened in an exclusive mode when opened with RFile::open() */
	/* in read only mode */

	Result = File.open("File.txt", EFileRead | EFileExclusive);
	test(Result == KErrNone);

	Result = File2.open("File.txt", EFileRead | EFileExclusive);
	test(Result == KErrInUse);

	Result = File2.open("File.txt", EFileWrite | EFileExclusive);
	test(Result == KErrInUse);

	File.close();

	/* Ensure that files cannot be opened in an exclusive mode using RFile::Create().  Note that */
	/* unlike other file opening behaviour, this behaviour is the same regardless of whether the */
	/* file was created in exclusive or shared mode (see shared tests) */

	test(BaflUtils::deleteFile("File.txt") == KErrNone);

	Result = File.Create("File.txt", EFileWrite | EFileExclusive);
	test(Result == KErrNone);

	Result = File2.Create("File.txt", EFileWrite | EFileExclusive);
	test(Result == KErrAlreadyExists);

	File.close();
}

void TestSharedMode()
{
	int Result;
	RFile File, File2, File3;

	Result = BaflUtils::deleteFile("File.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	/* Test #5: Test that RFile::Create() and RFile::open() work as expected */

	Test.Next("Creating and opening files with RFile::Create() and RFile::open() in shared mode");

	/* Ensure that files can be opened in a shared mode when created with RFile::Create() */

	Result = File.Create("File.txt", EFileWrite);
	test(Result == KErrNone);

	Result = File2.open("File.txt", EFileRead);
	test(Result == KErrNone);

	Result = File3.open("File.txt", EFileWrite);
	test(Result == KErrNone);

	File3.close();
	File2.close();
	File.close();

	/* Ensure that files cann be opened in a shared mode when opened with RFile::open() */
	/* in writeable mode */

	Result = File.open("File.txt", EFileWrite);
	test(Result == KErrNone);

	Result = File2.open("File.txt", EFileRead);
	test(Result == KErrNone);

	Result = File3.open("File.txt", EFileWrite);
	test(Result == KErrNone);

	File3.close();
	File2.close();
	File.close();

	/* Ensure that files can be opened in a shared mode when opened with RFile::open() */
	/* in read only mode */

	Result = File.open("File.txt", EFileRead);
	test(Result == KErrNone);

	Result = File2.open("File.txt", EFileRead);
	test(Result == KErrNone);

	Result = File3.open("File.txt", EFileWrite);
	test(Result == KErrNone);

	File3.close();
	File2.close();
	File.close();

	/* Ensure that files cannot be opened in a shared mode using RFile::Create().  Note that */
	/* unlike other file opening behaviour, this behaviour is the same regardless of whether the */
	/* file was created in exclusive or shared mode (see exclusive tests) */

	test(BaflUtils::deleteFile("File.txt") == KErrNone);

	Result = File.Create("File.txt", EFileWrite);
	test(Result == KErrNone);

	Result = File2.Create("File.txt", EFileWrite);
	test(Result == KErrAlreadyExists);

	File.close();
}

int main()
{
	char Buffer[256];
	int Result;

	Test.Title();
	Test.Start("RFile class API test");

	RFile File, File2;

	/* Test #2: Test that RFile.Replace() works as expected */

	Test.Next("Creating file with RFile::Replace()");

	/* The test file may be hanging around from the last time the test was run */

	Result = BaflUtils::deleteFile("File.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	/* Ensure that RFile::Replace() works both when the file does not exist and when it does */

	Result = File.Replace("File.txt", EFileWrite);
	test(Result == KErrNone);
	File.close();

	Result = File.Replace("File.txt", EFileWrite);
	test(Result == KErrNone);
	File.close();

	/* Test #3: Rename a file using BaflUtils::RenameFile() */

	Test.Next("Renaming a file with BaflUtils::RenameFile()");

	/* The test file may be hanging around from the last time the test was run */

	Result = BaflUtils::deleteFile("NewFile.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	Result = BaflUtils::RenameFile("File.txt", "NewFile.txt");
	test(Result == KErrNone);

	/* Clean up after ourselves */

	Result = BaflUtils::deleteFile("NewFile.txt");
	test(Result == KErrNone);

	TestExclusiveMode();
	TestSharedMode();

	/* Ensure other Create() and open() errors are as expected */

	Result = File.Create("File.txt", EFileWrite);
	test(Result == KErrAlreadyExists);

	Result = File.Create("UnknownPath/File.txt", EFileWrite);
	test(Result == KErrPathNotFound);

	Result = File.open("UnknownPath/UnknownFile.txt", EFileRead);
	test(Result == KErrPathNotFound);

	Result = File.open("UnknownFile.txt", EFileRead);
	test(Result == KErrNotFound);

	/* Test #6: Test that we are able to create and write to files using both supported APIs */

	Test.Next("Creating and writing to files using both supported APIs");

	Result = BaflUtils::deleteFile("File.txt");
	test((Result == KErrNone) || (Result == KErrNotFound));

	Result = File.Create("File.txt", EFileWrite);
	test(Result == KErrNone);

	/* Ensure that trying to read 0 bytes is handled sanely */

	Result = File.read((unsigned char *) Buffer, 0);
	test(Result == 0);

	/* Ensure that trying to write 0 bytes is handled sanely */

	Result = File.write((const unsigned char *) g_pccWriteText, 0);
	test(Result == 0);

	/* Now write some real data */

	WriteToFile(File);

	File.close();

	Result = File.open("File.txt", EFileWrite);
	test(Result == KErrNone);

	/* Ensure that trying to read 0 bytes is handled sanely */

	Result = File.read((unsigned char *) Buffer, 0);
	test(Result == 0);

	WriteToFile(File);

	File.close();

	/* Ensure we can't write to a file opened in read only mode */

	Result = File.open("File.txt", EFileRead);
	test(Result == KErrNone);

	Result = File.write((const unsigned char *) g_pccWriteText, strlen(g_pccWriteText));
	test(Result == KErrGeneral);

	File.close();

	/* Test #7: Ensure that the PROGDIR: prefix works with RFile::open() */

	Test.Next("Ensure that the PROGDIR: prefix works with RFile::open()");

#ifdef WIN32

	Result = File.open("PROGDIR:T_File.exe", EFileRead);
	test(Result == KErrNone);

#else /* ! WIN32 */

	Result = File.open("PROGDIR:T_File", EFileRead);
	test(Result == KErrNone);

#endif /* ! WIN32 */

	File.close();

	/* Clean up after ourselves */

	Result = BaflUtils::deleteFile("File.txt");
	test(Result == KErrNone);

	Test.End();

	return(RETURN_OK);
}
