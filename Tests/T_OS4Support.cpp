
#include <StdFuncs.h>
#include <OS4Support.h>
#include <Test.h>
#include <string.h>

static RTest Test("T_OS4Support");

static const char g_simpleString[] = "\5hello";
#define SIMPLE_STRING_LENGTH 5

int main()
{
	char cString[100];
	ULONG result;

	Test.Title();
	Test.Start("OS4Support class API test");

	/* Test #2: Convert simple BCPL string to C string */

	Test.Next("Convert simple BCPL string to C string");

	result = CopyStringBSTRToC(MKBADDR(g_simpleString), cString, sizeof(cString));
	test(result == SIMPLE_STRING_LENGTH);
	test(strlen(cString) == SIMPLE_STRING_LENGTH);
	Test.printf("%s\n", cString);

	/* Test #3: Get length of BCPL string */

	Test.Next("Get length of BCPL string");

	CopyStringBSTRToC(MKBADDR(g_simpleString), nullptr, 0);
	test(result == SIMPLE_STRING_LENGTH);

	CopyStringBSTRToC(MKBADDR(g_simpleString), nullptr, 5);
	test(result == SIMPLE_STRING_LENGTH);

	/* Test #4: Try to convert ZERO BCPL string */

	Test.Next("Try to convert ZERO BCPL string");

	CopyStringBSTRToC(ZERO, cString, sizeof(cString));
	test(result == SIMPLE_STRING_LENGTH);
	test(cString[0] == '\0');

	/* Test #5: Convert ZERO BCPL string into too small buffers */

	Test.Next("Convert ZERO BCPL string into too small buffers");

	result = CopyStringBSTRToC(MKBADDR(g_simpleString), cString, 4);
	test(result == SIMPLE_STRING_LENGTH);
	test(strlen(cString) == 3);
	Test.printf("%s\n", cString);

	result = CopyStringBSTRToC(MKBADDR(g_simpleString), cString, 5);
	test(result == SIMPLE_STRING_LENGTH);
	test(strlen(cString) == 4);
	Test.printf("%s\n", cString);

	result = CopyStringBSTRToC(MKBADDR(g_simpleString), cString, 6);
	test(result == SIMPLE_STRING_LENGTH);
	test(strlen(cString) == 5);
	Test.printf("%s\n", cString);

	Test.End();

	return(RETURN_OK);
}
