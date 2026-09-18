
#include <StdFuncs.h>
#include <Lex.h>
#include "StdJSON.h"
#include <ctype.h> // TODO: CAW
#include <string.h>

#define IsDigit(digit) isdigit(static_cast<unsigned char>(digit))

// TODO: CAW
/*bool IsDigit(char c)
{
    return isdigit(static_cast<unsigned char>(c)) != 0;
}

bool IsDigit(char c)
{
    return c >= '0' && c <= '9';
}*/

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Thursday 23-Jul-2026 7:56 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void RStdJSON::advance()
{
	if (m_json < m_jsonEnd)
	{
		++m_json;
	}
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Thursday 23-Jul-2026 9:30 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void RStdJSON::appendChild(JSONValue* parent, JSONValue* key, JSONValue* child)
{
    // key is only meaningful for Object parents; for Array parents pass nullptr
    if (key != nullptr)
        child->m_key = key->m_str;

    child->m_next = nullptr;   // this is always the new last element

    if (parent->m_lastChild == nullptr)
    {
        // first child being added
        parent->m_firstChild = child;
        parent->m_lastChild  = child;
    }
    else
    {
        parent->m_lastChild->m_next = child;
        parent->m_lastChild       = child;
    }
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Thursday 23-Jul-2026 7:58 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

bool RStdJSON::expect(char a_expected)
{
	if (peekChar() != a_expected)
	{
		// small fixed buffer avoids pulling in a heavier string-formatting path
		/*char msg[64];
		snprintf(msg, sizeof(msg), "Expected '%c' but found '%c'",
		expected, PeekChar() ? PeekChar() : '?');
		SetError(msg, m_cur);*/

		return false;
	}

	advance();

	return true;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 24-Jul-2026 8:34 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

bool RStdJSON::matchLiteral(const char* literal)
{
    const char* p = m_json;
    size_t len = strlen(literal);

    if (static_cast<size_t>(m_jsonEnd - p) < len)
        return false;   // not enough remaining input to possibly match

    for (size_t i = 0; i < len; ++i)
    {
        if (p[i] != literal[i])
            return false;   // mismatch - m_cur untouched
    }

    m_json += len;   // full match - consume it
    return true;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Thursday 23-Jul-2026 6:42 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

JSONValue *RStdJSON::parse(const char *a_json, int a_size)
{
	m_json = a_json;
	m_jsonEnd = a_json + a_size;
	//m_size = a_size;

	JSONValue *root = nullptr;

	if (!parseValue(root))
		return nullptr;

	// Optional but recommended: after the root value, only whitespace
	// (or nothing) should remain - trailing garbage means malformed input.
	skipWhitespace();

	if (m_json != m_jsonEnd)
	{
		//SetError("Unexpected trailing data after JSON value", m_cur);
		return nullptr;
	}

	return root;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 25-Jul-2026 8:54 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

bool RStdJSON::parseArray(JSONValue*& outValue)
{
    skipWhitespace();
    if (!expect('['))
        return false;

    JSONValue* array = new JSONValue;
    array->m_type = JSONType::Array;

    skipWhitespace();
    if (peekChar() != ']')
    {
        for (;;)
        {
            JSONValue* element = nullptr;
            if (!parseValue(element))
                return false;

            appendChild(array, nullptr, element);   // no key - this is an array, not an object

            skipWhitespace();
            if (peekChar() == ',')
            {
                advance();
                continue;
            }
            break;
        }
    }

    skipWhitespace();
    if (!expect(']'))
        return false;

    outValue = array;
    return true;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 24-Jul-2026 8:30 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

bool RStdJSON::parseBool(JSONValue*& outValue)
{
    skipWhitespace();

    bool value;

    if (matchLiteral("true"))
    {
        value = true;
    }
    else if (matchLiteral("false"))
    {
        value = false;
    }
    else
    {
        //SetError("Expected 'true' or 'false'", m_cur);
        return false;
    }

    //JSONValue* node = m_pool.AllocBool();
    JSONValue* node = new JSONValue; // m_pool.AllocNumber();
    node->m_type = JSONType::Bool;
    node->m_boolVal = value;
    outValue = node;
    return true;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 25-Jul-2026 6:37 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

bool RStdJSON::parseNull(JSONValue*& outValue)
{
    skipWhitespace();

    if (!matchLiteral("null"))
    {
        //SetError("Expected 'null'", m_cur);
        return false;
    }

    JSONValue* node = new JSONValue; //m_pool.AllocNull();
    node->m_type = JSONType::Null;
    outValue = node;

    return true;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 24-Jul-2026 7:31 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

bool RStdJSON::parseNumber(JSONValue*& outValue)
{
    skipWhitespace();

    const char* start = m_json;

    if (peekChar() == '-')
        advance();

    // --- integer part ---
    if (peekChar() == '0')
    {
        advance();
        if (isdigit(peekChar()))
        {
            //SetError("Leading zeros are not allowed", m_cur);
            return false;
        }
    }
    else if (IsDigit(peekChar()))
    {
        while (IsDigit(peekChar()))
            advance();
    }
    else
    {
        //SetError("Expected digit", m_cur);
        return false;
    }

    bool isFraction = false;

    // --- fractional part: no FPU support, so we don't convert this - just skip over it ---
    if (peekChar() == '.')
    {
        isFraction = true;
        advance();
        if (!IsDigit(peekChar()))
        {
            //SetError("Expected digit after decimal point", m_cur);
            return false;
        }
        while (IsDigit(peekChar()))
            advance();
    }

    // --- exponent part: same story ---
    if (peekChar() == 'e' || peekChar() == 'E')
    {
        isFraction = true;
        advance();
        if (peekChar() == '+' || peekChar() == '-')
            advance();
        if (!IsDigit(peekChar()))
        {
            //SetError("Expected digit in exponent", m_cur);
            return false;
        }
        while (IsDigit(peekChar()))
            advance();
    }

    const char* end = m_json;   // full token is start..end, well-formed

    JSONValue* node = new JSONValue; // m_pool.AllocNumber();
    node->m_type = JSONType::Number;

    if (isFraction)
    {
        // No FPU support - keep the raw text, don't attempt numeric conversion.
        node->m_isInt = false;
        node->m_str   = std::string(start, end - start); // m_pool.MakeStringRef(start, static_cast<size_t>(end - start));
    }
    else
    {
        errno = 0;
        char* endPtr = nullptr;
        long long value = strtoll(start, &endPtr, 10);

        if (errno == ERANGE)
        {
            // Overflowed int64 - still no FPU fallback, so keep as raw text too.
            node->m_isInt = false;
            node->m_str   = std::string(start, end - start); // m_pool.MakeStringRef(start, static_cast<size_t>(end - start));
        }
        else
        {
            node->m_isInt  = true;
            node->m_intVal = value;
        }
    }

    outValue = node;
    return true;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Thursday 23-Jul-2026 9:23 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

bool RStdJSON::parseObject(JSONValue*& outValue)
{
    skipWhitespace();

    if (!expect('{'))
        return false;

    JSONValue* obj = new JSONValue; //m_pool.AllocObject();
    /*node*/obj->m_type = JSONType::Object; // TODO: CAW - Make a proper allocator

    skipWhitespace();
    if (peekChar() != '}')
    {
        for (;;)
        {
            JSONValue* key = nullptr;
            if (!parseString(key))     // ParseString now skips its own leading whitespace
                return false;

            skipWhitespace();
            if (!expect(':'))
                return false;

            JSONValue* value = nullptr;
            if (!parseValue(value))    // ParseValue now skips its own leading whitespace
                return false;

            appendChild(obj, key, value);
            delete key;

            skipWhitespace();
            if (peekChar() == ',')
            {
                advance();
                continue;
            }
            break;
        }
    }

    skipWhitespace();
    if (!expect('}'))
        return false;

    outValue = obj;
    return true;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Thursday 23-Jul-2026 8:38 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

bool RStdJSON::parseString(JSONValue*& outValue)
{
    skipWhitespace();

    if (!expect('"'))
        return false;

    const char* rawStart = m_json;
    const char* scan = m_json;
    while (scan < m_jsonEnd && *scan != '"')
    {
        if (*scan == '\\')
        {
            ++scan;
            if (scan >= m_jsonEnd) // TODO: CAW - Investigate this highlighting
                break;
        }
        ++scan;
    }

    if (scan >= m_jsonEnd)
    {
        //SetError("Unterminated string", m_cur);
        return false;
    }

    size_t maxLen = static_cast<size_t>(scan - rawStart);
    char* out = new char[maxLen]; // m_pool.AllocBytes(maxLen);
    size_t outLen = 0;

    while (m_json < m_jsonEnd && *m_json != '"')
    {
        unsigned char c = static_cast<unsigned char>(*m_json);

        if (c < 0x20)
        {
            //SetError("Unescaped control character in string", m_cur);
            return false;
        }

        if (c != '\\')
        {
            out[outLen++] = static_cast<char>(c);
            advance();
            continue;
        }

        // --- escape sequence ---
        advance();   // consume the backslash
        char esc = peekChar();

        switch (esc)
        {
            case '"':  out[outLen++] = '"';  advance(); break;
            case '\\': out[outLen++] = '\\'; advance(); break;
            case '/':  out[outLen++] = '/';  advance(); break;
            case 'b':  out[outLen++] = '\b'; advance(); break;
            case 'f':  out[outLen++] = '\f'; advance(); break;
            case 'n':  out[outLen++] = '\n'; advance(); break;
            case 'r':  out[outLen++] = '\r'; advance(); break;
            case 't':  out[outLen++] = '\t'; advance(); break;

            case 'u':
            {
                // TODO: proper UTF-8 encoding once the parser is otherwise working.
                // For now just consume the 4 hex digits so string boundaries stay
                // correct, and store a placeholder so it's obvious this is unfinished.
                advance();   // consume 'u'
                //uint32_t codepoint;
                //if (!ParseHex4(codepoint))
                //    return false;

                out[outLen++] = '?';   // placeholder byte - revisit in GetString()
                break;
            }

            default:
                //SetError("Invalid escape sequence", m_json);
                return false;
        }
    }

    advance();   // consume closing '"'

    JSONValue* node = new JSONValue; // m_pool.AllocString();
    node->m_type = JSONType::String;
    node->m_str = std::string(out, outLen); // m_pool.MakeStringRef(out, outLen);
    delete [] out;
    outValue = node;
    return true;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Thursday 23-Jul-2026 7:29 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

// TODO: CAW - Spaces used
bool RStdJSON::parseValue(JSONValue*& outValue)
{
    skipWhitespace();

    char c = peekChar();

    switch (c)
    {
        case '{':
            return parseObject(outValue);

        case '[':
            return parseArray(outValue);

        case '"':
            return parseString(outValue);

        case 't':
        case 'f':
            return parseBool(outValue);

        case 'n':
            return parseNull(outValue);

        case '-':
            return parseNumber(outValue);

        default:
            if (c >= '0' && c <= '9')
                return parseNumber(outValue);

            //SetError("Unexpected character - expected a value", m_cur);
            return false;
    }
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Thursday 23-Jul-2026 7:39 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

char RStdJSON::peekChar() const
{
	return (m_json < m_jsonEnd) ? *m_json : '\0';
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Thursday 23-Jul-2026 7:42 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void RStdJSON::skipWhitespace()
{
	while (m_json < m_jsonEnd)
	{
		char ch = *m_json; // TODO: CAW

		if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
		{
			advance();
		}
		else
		{
			break;
		}
	}
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 25-Jul-2026 7:37 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

JSONValue::~JSONValue()
{
    // delete the linked-list of children (this handles both Object members and Array elements)
    JSONValue* child = m_firstChild;
    while (child != nullptr)
    {
        JSONValue* next = child->m_next;
        delete child;     // recursively invokes this same destructor
        child = next;
    }
    m_firstChild = nullptr;
    m_lastChild  = nullptr;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 25-Jul-2026 7:09 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

// Array element access by index
const JSONValue& JSONValue::operator[](int index) const
{
    if (m_type != JSONType::Array || index < 0)
        return Null();

    JSONValue* child = m_firstChild;
    for (int i = 0; child != nullptr && i < index; ++i)
        child = child->m_next;

    return child ? *child : Null();
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 25-Jul-2026 7:09 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

const JSONValue& JSONValue::operator[](const char* key) const
{
    if (m_type != JSONType::Object)
        return Null();

    for (JSONValue* child = m_firstChild; child != nullptr; child = child->m_next)
    {
        if (child->m_key == key)
            return *child;
    }
    return Null();
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Thursday 23-Jul-2026 9:37 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

// Object member lookup ? linear scan over the linked children.
// Returns nullptr if not found or if this node isn't an object.
JSONValue *JSONValue::findMember(const char* key) const
{
    if (m_type != JSONType::Object)
    {
        return nullptr;
    }

    for (JSONValue* child = m_firstChild; child != nullptr; child = child->m_next)
    {
        if (child->m_key == key)
            return child;
    }

    return nullptr;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 25-Jul-2026 9:57 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

JSONValue* JSONValue::getElement(int index) const
{
    if (m_type != JSONType::Array || index < 0)
        return nullptr;

    JSONValue* child = m_firstChild;
    for (int i = 0; child != nullptr && i < index; ++i)
        child = child->m_next;

    return child;
}
