
#include <StdFuncs.h>
#include "StdSocket.h"
#include "StdSSL.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 10-Aug-2026 6:56 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

int RStdSSL::open(const char *a_ssi)
{
	// --- one-time global setup (once per process) ---
	// TODO: CAW - Use a static m_initialised variable?
	SSL_library_init();
	SSL_load_error_strings();

	// --- per-connection setup ---
	const SSL_METHOD *method = TLS_client_method();   // negotiates highest mutually-supported TLS version
	m_context = SSL_CTX_new(method);

	// wrap the connected socket in an SSL object
	m_ssl = SSL_new(m_context);
	SSL_set_fd(m_ssl, m_socket.m_socket);

	// Important: set the hostname for SNI (Server Name Indication) - many servers,
	// including Anthropic's, require this to serve the correct certificate
	// TODO: CAW - This needs to be passed in
	SSL_set_tlsext_host_name(m_ssl, a_ssi);
	//SSL_set_tlsext_host_name(m_ssl, "api.anthropic.com");
	//SSL_set_tlsext_host_name(m_ssl, "google.com");

	// TODO: CAW - Decide how to handle certificate bundle location and implement this
	// Load the system's default trusted CA certificates
	//if (!SSL_CTX_set_default_verify_paths(ctx)) {
		// Handle error loading trust store
	//}
	// Ensure verification is active
	//SSL_CTX_set_verify(m_context, SSL_VERIFY_PEER, nullptr);
	//SSL_CTX_load_verify_locations(m_context, "ca-bundle.crt", nullptr);   // path to a CA cert bundle

	// perform the TLS handshake over the already-connected socket
	int result = SSL_connect(m_ssl);
	if (result != 1) // TODO: CAW - SSL_ERROR_SSL
	{
	    //int err = SSL_get_error(m_ssl, result);
	    // handle handshake failure
	    printf("SSL_connect() returned %d\n", result);
	    int error = SSL_get_error(m_ssl, result);
	    printf("SSL_connect() error is %d\n", error);
	    return KErrGeneral; // TODO: CAW - Better error
	}

/*if (SSL_get_error(ssl, ret) == SSL_ERROR_SSL) {
    unsigned long error_code;
    const char *file, *data;
    int line, flags;

    while ((error_code = ERR_get_error_line_data(&file, &line, &data, &flags))) {
        char err_buf[256];
        ERR_error_string_n(error_code, err_buf, sizeof(err_buf));
        printf("OpenSSL Queue Error: %s in %s:%d\n", err_buf, file, line);
        if (flags & ERR_TXT_STRING) {
            printf("Error data: %s\n", data);
        }
    }
}*/
	return KErrNone;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 10-Aug-2026 7:04 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void RStdSSL::close()
{
	// --- cleanup ---
	SSL_shutdown(m_ssl);
	SSL_free(m_ssl);
	m_ssl = nullptr;
	//close(sockFd);      // or your RSocket equivalent
	SSL_CTX_free(m_context);
	m_context = nullptr;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 10-Aug-2026 7:12 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

int RStdSSL::read(void *a_buffer, int a_size, bool a_readAll)
{
	printf("RStdSSL::read(): %p %p, %d %d\n", m_ssl, a_buffer, a_size, a_readAll);
	fflush(stdout);

	// TODO: CAW - Check Claude's comments on this
	int retVal = SSL_read(m_ssl, a_buffer, a_size); // TODO: CAW - Map onto StdLib errors

	if (retVal <= 0)
	{
		// TODO: CAW - Check Claude's other comments on this
	    int error = SSL_get_error(m_ssl, retVal);
	    printf("SSL_read() result is %d, error is %d\n", retVal, error);
	}
	else
	{
		printf("retVal = %d\n", retVal);
	}

	return retVal;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 10-Aug-2026 7:17 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

int RStdSSL::write(const void *a_buffer, int a_size)
{
	printf("RStdSSL::write(): %p %p, %d\n", m_ssl, a_buffer, a_size);
	fflush(stdout);

	return SSL_write(m_ssl, a_buffer, a_size);
}
