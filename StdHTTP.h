
#ifndef STDHTTP_H
#define STDHTTP_H

#include <string>
#include <vector>

class RSocket;
class RStdSSL;

struct THTTPHeader
{
	std::string	m_name;
	std::string	m_value;
};

enum TMethod
{
	EMethodGet,
	EMethodPost
};

class RStdHTTP
{
	// TODO: CAW - These are nice but mess up comments
	bool		m_chunked = false;
	bool		m_newChunk = false;
	bool		m_haveContentLength = false;
	char		*m_buffer = nullptr;
	int			m_bodySize = 0;
	int			m_statusCode = 0;
	std::string	m_body;
	std::string	m_headers;
	RSocket		*m_socket;
	RStdSSL		*m_ssl;

private:

	bool appendBody(const char *a_newBodyData, int a_size);

public:

	RStdHTTP(RSocket *a_socket) : m_socket(a_socket), m_ssl(nullptr)
	{
		m_chunked = m_newChunk = m_haveContentLength = false;
	}

	RStdHTTP(RStdSSL *a_ssl) : m_socket(nullptr), m_ssl(a_ssl) { }

	~RStdHTTP()
	{
		delete [] m_buffer;
	}

	int request(const std::string &a_url, const std::vector<THTTPHeader> &a_headers, const std::string &a_body, TMethod a_method);

	int get(const std::string &a_url, const std::vector<THTTPHeader> &a_headers)
	{
		return request(a_url, a_headers, "", EMethodGet);
	}

	int post(const std::string &a_url, const std::vector<THTTPHeader> &a_headers, const std::string &a_body)
	{
		return request(a_url, a_headers, a_body, EMethodPost);
	}

	const std::string &body() const { return m_body; }

	const std::string &headers() const { return m_headers; }

	int statusCode() const { return m_statusCode; }
};

#endif /* ! STDHTTP_H */
