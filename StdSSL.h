
#ifndef STDSSL_H
#define STDSSL_H

#include <openssl/ssl.h>

class RSocket;

// TODO: CAW - Double check use cases of this and RStdHTTP to ensure that the really should be R classes
// TODO: CAW - Rename this to RStdSSLSocket and RSocket to RStdSocket?
class RStdSSL
{
	SSL_CTX		*m_context = nullptr;
	RSocket		&m_socket;
	SSL			*m_ssl = nullptr;

public:

	RStdSSL(RSocket &a_socket) : m_socket(a_socket) { }

	int open(const char *a_ssi);

	void close();

	int read(void *a_buffer, int a_size, bool a_readAll = true);

	int write(const void *a_buffer, int a_size);
};

#endif /* ! STDSSL_H */
