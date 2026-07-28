
#ifndef STDSSL_H
#define STDSSL_H

//#include <openssl/ssl.h>

class RSocket;
struct ssl_st;
struct ssl_ctx_st;
typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;

// TODO: CAW - Double check use cases of this and RStdHTTP to ensure that the really should be R classes
// TODO: CAW - Rename this to StdSSLSocket.cpp and RSocket to RStdSocket
class RStdSSLSocket
{
	static bool		m_open;
	SSL_CTX			*m_context;
	RSocket			*m_socket;
	SSL				*m_ssl;

public:

	RStdSSLSocket() : m_context(nullptr), m_socket(nullptr), m_ssl(nullptr) { }

	int open(const char *a_ssi);

	void close();

	static void _shutdown();

	int get_error(int a_result) const; // TODO: CAW - Dumb name

	virtual bool isOpen();

	void setSocket(RSocket *a_socket)
	{
		m_socket = a_socket;
	}

	virtual int read(void *a_buffer, int a_size, bool a_readAll = true);

	virtual int write(const void *a_buffer, int a_size);
};

#endif /* ! STDSSL_H */
