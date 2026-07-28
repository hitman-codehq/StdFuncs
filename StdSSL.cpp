
#include <StdFuncs.h>
#include "StdSocket.h"
#include "StdSSL.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

#ifdef __amigaos__

#include <libraries/amisslmaster.h>
#include <libraries/amissl.h>
#include <proto/amisslmaster.h>
#include <proto/amissl.h>
#include <proto/socket.h>

/*WEAK*/ struct Library *AmiSSLMasterBase;
/*WEAK*/ struct Library *AmiSSLBase;
/*WEAK*/ struct Library *AmiSSLExtBase;
extern struct Library *SocketBase;

//extern struct Library *_SocketBase;
//extern struct Library *__SocketBase;

#endif /* __amigaos__ */

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

// TODO: CAW - Hostname verification: with 3.x you can have OpenSSL check that the certificate matches the
// host: SSL_set1_host(ssl, "api.anthropic.com") together with SSL_VERIFY_PEER. That’s better than rolling
// your own check, and worth doing before this handles real API keys.

// TODO: CAW - Entropy: 3.x seeds its own DRBG from OS sources, and the port has to supply one on a system
// without /dev/urandom. Check RAND_status() returns 1 before connecting. If it doesn’t, seed manually with RAND_seed().

// TODO: CAW - Handshake cost: TLS 1.3 needs fewer round trips than 1.2, which helps on a slow link, but the public-key
// maths still lands on the 68k. Keep-alive and session resumption (reusing the SSL_SESSION between connections) are
// the real fixes, so the keep-alive work remains the main optimisation to plan for.

// TODO: CAW - I’d try the handshake against a simple HTTPS site first, as you did on MSYS, and print SSL_get_version()
// and SSL_get_cipher() afterwards. The negotiated cipher tells you what the handshake is costing your CPU, and it’s a
// useful number to have when you look at the keep-alive design.

#ifdef __amigaos4__

struct AmiSSLMasterIFace *IAmiSSLMaster;
struct AmiSSLIFace *IAmiSSL;
//struct SocketIFace *ISocketIFace;

#endif /* __amigaos4__ */

int RStdSSLSocket::open(const char *a_ssi)
{

#ifdef __amigaos__

	if (AmiSSLMasterBase == nullptr)
	{
		if ((AmiSSLMasterBase = OpenLibrary("amisslmaster.library", AMISSLMASTER_MIN_VERSION)) == nullptr)
		{
			return KErrNotFound;
		}
	}

#ifdef __amigaos4__

	if (!m_open)
	{
		if(IAmiSSLMaster = (struct AmiSSLMasterIFace *)
						GetInterface((struct Library *)AmiSSLMasterBase,
										"main", 1, NULL))
		{
			if(OpenAmiSSLTags(AMISSL_CURRENT_VERSION,
							AmiSSL_UsesOpenSSLStructs, TRUE or FALSE,
							AmiSSL_GetIAmiSSL, &IAmiSSL,
							AmiSSL_ISocket, ISocket,
							AmiSSL_ErrNoPtr, &errno,
							TAG_DONE) == 0)
			{
				m_open = true;
			}
			else printf("ERROR: Couldn't open AmiSSL!");
		}
		else printf("ERROR: Couldn't obtain AmiSSLMaster interface!"); fflush(stdout);
	}

#else /* ! __amigaos4__ */

	if (!m_open)
	{
		LONG AmiSSLAPIVersion = AMISSL_CURRENT_VERSION;
		LONG UsesOpenSSLStructs = TRUE;

		LONG err = OpenAmiSSLTags(AmiSSLAPIVersion,
			AmiSSL_UsesOpenSSLStructs, UsesOpenSSLStructs,
			AmiSSL_GetAmiSSLBase, (ULONG) &AmiSSLBase,
			AmiSSL_GetAmiSSLExtBase, (ULONG) &AmiSSLExtBase,
			AmiSSL_SocketBase, SocketBase,
			AmiSSL_ErrNoPtr, (ULONG) &errno,
			TAG_DONE);

		// TODO: CAW - This is a temporary debug output to check if AmiSSL is initialised correctly
		switch (err)
		{
			case 1:
				printf("Couldn't initialise amisslmaster.library!\n"); fflush(stdout);
				break;
			case 2:
				printf("Couldn't open AmiSSL!\n"); fflush(stdout);
				break;
			case 3:
				printf("Couldn't initialise AmiSSL!\n"); fflush(stdout);
				break;
			default:
				m_open = true;
				break;
		}

		if (err != 0)
		{
			// TODO: CAW - Close library
			return KErrGeneral;
		}
	}

#endif /* ! __amigaos4__ */

#endif /* __amigaos__ */

	// --- one-time global setup (once per process) ---
	// TODO: CAW - Use a static m_initialised variable?
	SSL_library_init(); // TODO: CAW - Check if this is needed in OpenSSL 1.1.0 and later
	SSL_load_error_strings(); // TODO: CAW - Check if this is needed in OpenSSL 1.1.0 and later

	//InitAmiSSLMaster(AMISSL_CURRENT_VERSION, 1);

	// --- per-connection setup ---
	const SSL_METHOD *method = TLS_client_method(); // negotiates highest mutually-supported TLS version
	m_context = SSL_CTX_new(method);

	// wrap the connected socket in an SSL object
	m_ssl = SSL_new(m_context);
	SSL_set_fd(m_ssl, static_cast<int>(m_socket->m_socket));

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
	//SSL_CTX_load_verify_locations(m_context, "ca-bundle.crt", nullptr); // path to a CA cert bundle

	// perform the TLS handshake over the already-connected socket
	int result = SSL_connect(m_ssl);
	//return KErrGeneral;
	if (result != 1) // TODO: CAW - SSL_ERROR_SSL
	{
		printf("SSL_connect failed with result: %d\n", result); fflush(stdout);

		// handle handshake failure
		//int error = SSL_get_error(m_ssl, result);

		//unsigned long queued = ERR_peek_error(); // 0 = nothing in the OpenSSL error queue
		//int sysErr = Errno();/* errno on the Amiga: see below */
		//printf("OpenSSL error: %d, error queue: %lu, system errno: %d\n", error, queued, sysErr); fflush(stdout);
		//ERR_print_errors_fp(stderr); // dumps the whole queue, if any

		//char err_buf[256];
		//const char *file;
		//const char *data;
		//int line, flags;

		//ERR_error_string_n(error, err_buf, sizeof(err_buf));
		//printf("OpenSSL Error was: %s at %s:%d\n", err_buf, file, line); fflush(stdout);

		return KErrGeneral; // TODO: CAW - Better error
	}
	else
	{
		// TODO: CAW - For your Amiga work, the cipher and the key-exchange group are the useful results, since they
		//             determine how much the handshake costs on a 68k. An elliptic-curve exchange such as X25519 is
		//             much cheaper than finite-field Diffie-Hellman, and ChaCha20 is often faster than AES in software
		//             when there’s no hardware acceleration. If the port lets you set the preference order, you could
		//             favour the cheaper ones with SSL_CTX_set_ciphersuites() (TLS 1.3 suites) and SSL_CTX_set1_groups_list().
		//             Measuring first and then trying alternatives would show you what actually helps.

#if 0
#ifdef __amigaos__
		const SSL_CIPHER *cipher = SSL_get_current_cipher(m_ssl);   // NULL if no handshake has completed
		const char *mac = SSL_CIPHER_get_mac(cipher);
		printf("mac = %s\n", mac);
		const char *encryption = SSL_CIPHER_get_encryption(cipher);
		printf("encryption = %s\n", encryption);
		const char *authentication = SSL_CIPHER_get_authentication(cipher);
		printf("authentication = %s\n", authentication);
		const char *exchange = SSL_CIPHER_get_key_exchange(cipher);
		printf("exchange = %s\n", exchange);
		//const char *_export = SSL_CIPHER_get_export(cipher);
		//printf("_export = %s\n", _export);
#endif
		const SSL_CIPHER *c = SSL_get_current_cipher(m_ssl);

		printf("Protocol:  %s\n", SSL_get_version(m_ssl));                  // e.g. "TLSv1.3"
		printf("Cipher:    %s\n", SSL_CIPHER_get_name(c));                // OpenSSL name
		printf("Standard:  %s\n", SSL_CIPHER_standard_name(c));           // IANA name, e.g. TLS_AES_256_GCM_SHA384
		printf("Version:   %s\n", SSL_CIPHER_get_version(c));
		int algBits = 0;
		int bits = SSL_CIPHER_get_bits(c, &algBits);                      // secret bits, and algorithm bits via the pointer
		printf("Bits:      %d (alg %d)\n", bits, algBits);

		// the component breakdown you were after, as NIDs:
		printf("Encryption: %s\n", OBJ_nid2ln(SSL_CIPHER_get_cipher_nid(c)));
		printf("MAC/digest: %s\n", OBJ_nid2ln(SSL_CIPHER_get_digest_nid(c)));
		printf("Key exch:   %s\n", OBJ_nid2ln(SSL_CIPHER_get_kx_nid(c)));
		printf("Auth:       %s\n", OBJ_nid2ln(SSL_CIPHER_get_auth_nid(c)));
		char buffer[512];
		SSL_CIPHER_description(c, buffer, sizeof(buffer));
		printf("Description: %s\n", buffer);

		// TODO: CAW
		const char *version = SSL_get_version(m_ssl);
		printf("Version = %s\n", version);
		const char *name = SSL_CIPHER_get_name(c);
		printf("Cipher name = %s\n", name);
#endif
	}

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

void RStdSSLSocket::close()
{
	if (m_ssl != nullptr)
	{
		SSL_shutdown(m_ssl);
		SSL_free(m_ssl);
		m_ssl = nullptr;
	}

	if (m_context != nullptr)
	{
		SSL_CTX_free(m_context);
		m_context = nullptr;
	}
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Monday 26-Oct-2026 8:38 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

bool RStdSSLSocket::m_open; // TODO: CAW - Is this needed?

// TODO: CAW - What is the socket shutdown?
void RStdSSLSocket::_shutdown()
{

#ifdef __amigaos__

	if (m_open)
	{
		CloseAmiSSL();
		m_open = false;
	}

#ifdef __amigaos4__

	if (IAmiSSLMaster != nullptr)
	{
		DropInterface((struct Interface *) IAmiSSLMaster);
		IAmiSSLMaster = nullptr;
	}

#endif /* __amigaos4__ */

	if (AmiSSLMasterBase)
	{
		CloseLibrary(AmiSSLMasterBase);
		AmiSSLMasterBase = nullptr;
	}

#endif /* __amigaos__ */

}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Tuesday 22-Sep-2026 6:41 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

int RStdSSLSocket::get_error(int a_result) const
{
	int error = SSL_get_error(m_ssl, a_result);

	/* SSL_ERROR_ZERO_RETURN means the server has cleanly closed the connection, and is the equivalent of a socket */
	/* returning 0. SSL_ERROR_SYSCALL means the server has not sent a close_notify first, which is technically not */
	/* correct, but many servers do it, so we will treat it as a successful close */
	if (error == SSL_ERROR_ZERO_RETURN || error == SSL_ERROR_SYSCALL)
	{
		return KErrNone;
	}
	else
	{
		return KErrEof;
	}
}

/**
 * Indicates whether the SSL connection is open.
 * Returns a boolean indicating whether the SSL connection is open and connected to a remote server.
 *
 * @date	Tuesday 20-Oct-2026 6:41 am, Code HQ Tokyo Tsukuda
 * @return	true if the SSL connection is open, else false
 */

bool RStdSSLSocket::isOpen()
{
	return m_ssl != nullptr;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		setSocket() must have been called to assign an open socket for use
 *
 * @date	Monday 10-Aug-2026 7:12 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

int RStdSSLSocket::read(void *a_buffer, int a_size, bool a_readAll)
{
	(void) a_readAll; // TODO: CAW - Implement this

	ASSERTM((m_socket != nullptr), "RStdSSLSocket::read() => setSocket() has not been called");

	// TODO: CAW - Check Claude's comments on this
	int retVal = SSL_read(m_ssl, a_buffer, a_size); // TODO: CAW - Map onto StdLib errors

	if (retVal <= 0)
	{
		// TODO: CAW - Check Claude's other comments on this
		//int error = SSL_get_error(m_ssl, retVal);
	}
	else
	{
		// printf("retVal = %d\n", retVal);
	}

	return retVal;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		setSocket() must have been called to assign an open socket for use
 *
 * @date	Monday 10-Aug-2026 7:17 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

int RStdSSLSocket::write(const void *a_buffer, int a_size)
{
	ASSERTM((m_socket != nullptr), "RStdSSLSocket::write() => setSocket() has not been called");

	return SSL_write(m_ssl, a_buffer, a_size);
}
