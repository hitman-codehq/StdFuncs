
#ifndef STDHTTP_H
#define STDHTTP_H

#include <string>
#include <vector>
//#include "StdSocket.h"

class RSocket;
class RStdSSL;

struct THTTPHeader
{
	std::string	m_name;
	std::string	m_value;
};

class RStdHTTP
{
	bool		m_chunked;
	bool		m_newChunk;
	char		*m_buffer = nullptr;
	int			m_bodySize; // TODO: CAW - Type
	RSocket		*m_socket;
	RStdSSL		*m_ssl;

protected:

	// TODO: CAW - Use getters?
	// String get body
	std::string	m_body;
	// final Map<String, String> headers
	std::string	m_headers;
	// final int statusCode;

private:

	bool appendBody(const char *a_newBodyData, int a_size);

public:

	RStdHTTP(RSocket *a_socket) : m_socket(a_socket), m_ssl(nullptr) { }

	RStdHTTP(RStdSSL *a_ssl) : m_socket(nullptr), m_ssl(a_ssl) { }

	~RStdHTTP()
	{
		delete [] m_buffer;
	}

	int get(const std::string &a_url, const std::string &a_body, const std::vector<THTTPHeader> &a_headers);

	std::string &body() { return m_body; }

	std::string &headers() { return m_headers; }
};

#endif /* ! STDHTTP_H */
