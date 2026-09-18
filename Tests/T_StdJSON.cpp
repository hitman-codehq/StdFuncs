
#include <StdFuncs.h>
#include <Test.h>
#include "StdJSON.h"
#include <string.h>

static RTest Test("T_JSON");	/* Class to use for testing and reporting results */

static const char g_basicBool[] = R"(
{
  "is_awesome": true
})";

static const char g_basicNumber[] = R"(
{
  "the_answer": 42
})";

static const char g_basicString[] = R"(
{
  "message": "Hello"
})";

static const char g_basicArray[] = R"(
{
	"array": [ { "one": 1 } ]
})";

//	"array": [ "one": 1, "two", "2_string", "true": true, "Null": null ]

// TODO: CAW - Crash
/*static const char g_anthropicPayload[] = R"(
{
  "id": "msg_01XFDUDYJgAACzvnptvVoYEL",
  "type": "message",
  "role": "assistant",
  "model": "claude-sonnet-5",
  "stop_reason": "end_turn"
})";*/

static const char g_anthropicPayload[] = R"(
{
  "id": "msg_01XFDUDYJgAACzvnptvVoYEL",
  "type": "message",
  "role": "assistant",
  "content": [
    { "type": "text", "text": "...the actual completion text..." }
  ],
  "model": "claude-sonnet-5",
  "stop_reason": "end_turn",
  "stop_sequence": null,
  "usage": {
    "input_tokens": 12,
    "output_tokens": 34
  }
})";

int main()
{
	Test.Title();
	Test.Start("RStdJSON class API test");

	/* Test #2: Load a single line file (with EOL marker at end of line) and ensure */
	/* it is represented correctly in memory */

	Test.Start("Test parsing simple JSON object with boolean");

	RStdJSON json;

	JSONValue *root;

	root = json.parse(g_basicBool, static_cast<int>(strlen(g_basicBool)));
	test(root != nullptr);

	const JSONValue *result = root->findMember("is_awesome");
	test(result != nullptr);
	test(result->m_type == JSONType::Bool);
	test(result->m_boolVal == true);
	printf("Key = %s, value = %d\n", result->m_key.c_str(), result->m_boolVal);

	// TODO: CAW - Why does file and line reporting work on MSVC, but some leaks are anonymous
	delete root;

	Test.Start("Test parsing simple JSON object with number");

	root = json.parse(g_basicNumber, static_cast<int>(strlen(g_basicNumber)));
	test(root != nullptr);

	result = root->findMember("the_answer");
	test(result != nullptr);
	test(result->m_type == JSONType::Number);
	test(result->m_intVal == 42);
	printf("Key = %s, value = %lld\n", result->m_key.c_str(), result->m_intVal);

	delete root;

	Test.Start("Test parsing simple JSON object with string");

	root = json.parse(g_basicString, static_cast<int>(strlen(g_basicString)));
	test(root != nullptr);

	result = root->findMember("message");
	test(result != nullptr);
	test(result->m_type == JSONType::String);
	test(result->m_str == "Hello");
	printf("Key = %s, value = %s\n", result->m_key.c_str(), result->m_str.c_str());

	delete root;

	Test.Start("Test parsing simple JSON array");

	root = json.parse(g_basicArray, static_cast<int>(strlen(g_basicArray)));
	test(root != nullptr);

	const JSONValue &result2 = (*root)["array"];//[0]["one"];
	//test(result != nullptr);
	test(result2.m_type == JSONType::Array);
	//test(result2.intVal == 1);
	printf("Key = %s, value = %lld\n", result2.m_key.c_str(), result2.m_intVal);

	const JSONValue &result3 = result2[0];
	test(result3.m_type == JSONType::Object);

	const JSONValue &result4 = result3["one"];
	test(result4.m_type == JSONType::Number);
	test(result4.m_intVal == 1);
	printf("Key = %s, value = %lld\n", result4.m_key.c_str(), result4.m_intVal);

	const JSONValue &result5 = (*root)["array"][0]["one"];
	test(result5.m_type == JSONType::Number);
	test(result5.m_intVal == 1);
	printf("Key = %s, value = %lld\n", result5.m_key.c_str(), result5.m_intVal);

	result = root->findMember("array"); // TODO: CAW - Pointer or reference?
	test(result != nullptr);

	result = result->getElement(0);
	test(result != nullptr);
	test(result->m_type == JSONType::Object);

	result = result->findMember("one");
	test(result != nullptr);
	test(result->m_type == JSONType::Number);
	test(result->m_intVal == 1);
	printf("Key = %s, value = %lld\n", result->m_key.c_str(), result->m_intVal);

	const JSONValue &notFound = (*root)["content"][0]["text"]; // TODO: CAW - Test not finding this
	test(notFound.exists() == false);
	//std::string text = content.GetString();

	delete root;

	RStdJSON json2; // TODO: CAW - Use json once we implement open() and close()

	//JSONValue *root2 = json2.parse(g_basicBool, strlen(g_basicBool));
	JSONValue *root2 = json2.parse(g_anthropicPayload, static_cast<int>(strlen(g_anthropicPayload)));
	test(root2 != nullptr);

	test((*root2)["id"].m_str == "msg_01XFDUDYJgAACzvnptvVoYEL");
	test((*root2)["type"].m_str == "message");
	test((*root2)["role"].m_str == "assistant");
	test((*root2)["content"][0]["type"].m_str == "text");
	test((*root2)["content"][0]["text"].m_str == "...the actual completion text...");
	test((*root2)["model"].m_str == "claude-sonnet-5");
	test((*root2)["stop_reason"].m_str == "end_turn");
	test((*root2)["stop_sequence"].m_type == JSONType::Null);
	test((*root2)["usage"]["input_tokens"].m_intVal == 12);
	test((*root2)["usage"]["output_tokens"].m_intVal == 34);

	delete root2;

	printf("%llx %lld %lld %lld\n", SIZE_MAX, sizeof(int), sizeof(size_t), sizeof(long long));

	Test.End();

	return(RETURN_OK);
}
