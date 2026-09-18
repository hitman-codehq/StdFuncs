
#ifndef STDJSON_H
#define STDJSON_H

enum class JSONType : uint8_t { Null, Bool, Number, String, Array, Object };

// TODO: CAW - Make a C class so that members are private
struct JSONValue
{
    JSONType m_type;

    union
    {
        bool	m_boolVal;
        int64_t m_intVal;
    };

	// TODO: CAW - Convert spaces to tabs
    bool        m_isInt; // TODO: CAW - Not needed - we can use JSONType::Number

    // TODO: CAW - Call this value? Create StringRef type
    std::string	m_key;            // only meaningful when parent is Object
    std::string	m_str;           // your own string type, not std::string

	JSONValue	*m_firstChild;  // Object/Array: first member/element
	JSONValue	*m_lastChild;   // Object/Array: last member/element - enables O(1) append
	JSONValue	*m_next;        // next sibling in the parent's child list

	JSONValue *findMember(const char* key) const;

	JSONValue* getElement(int index) const;

public:

	JSONValue()
	{
		m_type = JSONType::Null;
	}

    // Non-copyable: JSONValue owns its children via raw pointers and deletes
    // them in its destructor. A shallow copy would create two owners of the
    // same children, leading to a double-delete/dangling-pointer crash.
    // Always use a reference (operator[]) or pointer (findMember/getElement).
    JSONValue(const JSONValue&)            = delete;

    ~JSONValue();

    // Shared sentinel returned whenever a lookup fails anywhere in a chain.
    // One instance for the whole program - never allocated, never mutated.
    static const JSONValue& Null()
    {
        static const JSONValue s_null;   // type defaults to JSONType::Null, no children
        return s_null;
    }

    bool exists() const { return this != &Null(); }

    // Array element access by index
    const JSONValue& operator[](int index) const;

    // Object member access by key
    const JSONValue& operator[](const char* key) const;
};

class RStdJSON
{
	const char	*m_json;
	const char	*m_jsonEnd;

private: // TODO: CAW

	void advance();

	void appendChild(JSONValue* parent, JSONValue* key, JSONValue* child);

	bool expect(char a_expected);

	bool matchLiteral(const char* literal);

	bool parseArray(JSONValue*& outValue);

	bool parseBool(JSONValue*& outValue);

	bool parseNull(JSONValue*& outValue);

	bool parseNumber(JSONValue*& outValue);

	bool parseObject(JSONValue*& outValue);

	bool parseString(JSONValue*& outValue);

	bool parseValue(JSONValue*& outValue);

	char peekChar() const;

	void skipWhitespace();

public:

	JSONValue *parse(const char *a_json, int a_size);
};

#endif /* STDJSON_H */
