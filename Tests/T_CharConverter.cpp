
#include <StdFuncs.h>
#include <StdCharConverter.h>
#include <Test.h>

static RTest Test("T_CharConverter");	/* Class to use for testing and reporting results */

int main()
{
	Test.Title();
	Test.Start("CStdCharConverter class API test");

	Test.Next("Test CStdCharConverter::charSize() with various UTF-8 characters");

	/* 1 - byte UTF - 8 character A (U + 0041) */
	const unsigned char utf8_1[] = {0x41};
	Test.printf("Testing 1-byte character A\n");
	test(CStdCharConverter::charSize(utf8_1) == 1);

	/* 2-byte UTF - 8 character Yen (U + 00A5) */
	const unsigned char utf8_2[] = {0xC2, 0xA5};
	Test.printf("Testing 2-byte character Yen\n");
	test(CStdCharConverter::charSize(utf8_2) == 2);

	/* 3-byte UTF - 8 character Euro (U + 20AC) */
	const unsigned char utf8_3[] = {0xE2, 0x82, 0xAC};
	Test.printf("Testing 3-byte character Euro\n");
	test(CStdCharConverter::charSize(utf8_3) == 3);

	/* 4-byte UTF-8 character Smiley (U+1F600) */
	const unsigned char utf8_4[] = {0xF0, 0x9F, 0x98, 0x80};
	Test.printf("Testing 4-byte character Smiley\n");
	test(CStdCharConverter::charSize(utf8_4) == 4);

	Test.End();

	return(RETURN_OK);
}
