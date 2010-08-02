
#include <StdFuncs.h>
#include <Test.h>

static RTest Test("T_Utils");

int main()
{
	const char *Extension, *FileName;

	Test.Title();
	Test.Start("Utils class API test");

	/* Test #2: Extract a file from a path */

	Test.Next("Extracting file names with Utils::FilePart()");

	FileName = Utils::FilePart("*.txt");
	test(strlen(FileName) > 0);
	test(strcmp(FileName, "*.txt") == 0);

	FileName = Utils::FilePart("some_path/*.txt");
	test(strlen(FileName) > 0);
	test(strcmp(FileName, "*.txt") == 0);

	FileName = Utils::FilePart("some_volume:*.txt");
	test(strlen(FileName) > 0);
	test(strcmp(FileName, "*.txt") == 0);

	/* Test #2: Extract an extension from a file name */

	Test.Next("Extracting extensions with Utils::Extension()");

	Extension = Utils::Extension("*.txt");
	test(Extension != NULL);
	test(strcmp(Extension, "txt") == 0);

	Extension = Utils::Extension(".");
	test(Extension != NULL);
	test(strcmp(Extension, "") == 0);

	Extension = Utils::Extension("*txt");
	test(Extension == NULL);

	/* Test #2: Trimming white space from the start & end of a string */

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

	Test.End();

	return(RETURN_OK);
}
