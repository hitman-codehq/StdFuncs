
#ifndef STDHTTP_H
#define STDHTTP_H

#include <string>
#include <vector>

class RSocket;
class RStdSSLSocket;

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
	bool			m_chunked = false;
	bool			m_newChunk = false;
	bool			m_haveContentLength = false;
	char		*	m_buffer = nullptr;
	int				m_bodySize = 0;
	int				m_statusCode = 0;
	std::string		m_body;
	std::string		m_headers;
	RSocket			*m_socket;
	RStdSSLSocket	*m_ssl;

private:

	bool appendBody(const char *a_newBodyData, int a_size);

	int request(const std::string &a_url, const std::vector<THTTPHeader> &a_headers, const std::string &a_body,
		TMethod a_method, bool a_keepAlive);

public:

	RStdHTTP() : m_socket(nullptr), m_ssl(nullptr)
	{
		m_chunked = m_newChunk = m_haveContentLength = false;
		m_buffer = nullptr;
		m_bodySize = m_statusCode = 0;
	}

	~RStdHTTP()
	{
		close();
	}

	void open(RSocket *a_socket, RStdSSLSocket *a_sslSocket); // TODO: CAW - Rename a_ssl and m_ssl everywhere

	void close()
	{
		delete[] m_buffer;
		m_buffer = nullptr;
	}

	int get(const std::string &a_url, const std::vector<THTTPHeader> &a_headers, bool a_keepAlive = false)
	{
		return request(a_url, a_headers, "", EMethodGet, a_keepAlive);
	}

	int post(const std::string &a_url, const std::vector<THTTPHeader> &a_headers, const std::string &a_body, bool a_keepAlive = false)
	{
		return request(a_url, a_headers, a_body, EMethodPost, a_keepAlive);
	}

	const std::string &body() const { return m_body; }

	const std::string &headers() const { return m_headers; }

	int statusCode() const { return m_statusCode; }
};

#endif /* ! STDHTTP_H */
