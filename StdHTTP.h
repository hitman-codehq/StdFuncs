
#ifndef STDHTTP_H
#define STDHTTP_H

#include <string>
#include <vector>
//#include "StdSocket.h"

class RSocket;

struct THTTPHeader
{
	std::string	m_name;
	std::string	m_value;
};

class RStdHTTP
{
	char		*m_buffer = nullptr;
	RSocket		&m_socket;

protected:

	// String get body
	std::string	m_body;
	// final Map<String, String> headers
	std::string	m_headers;
	// final int statusCode;

public:

	RStdHTTP(RSocket &a_socket) : m_socket(a_socket) { }

	~RStdHTTP()
	{
		delete [] m_buffer;
	}

	int get(const std::string &a_url, const std::string &a_body, const std::vector<THTTPHeader> &a_headers);

	std::string &body() { return m_body; }

	std::string &headers() { return m_headers; }
};

#endif /* ! STDHTTP_H */
