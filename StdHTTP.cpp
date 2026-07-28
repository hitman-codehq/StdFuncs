
#include <StdFuncs.h>
#include <string.h>
#include <sstream> // TODO: CAW
#include "Lex.h"
#include "StdHTTP.h"
#include "StdSocket.h"
#include "StdSSL.h"

#define BUFFER_SIZE 4096

static const char g_contentLength[] = "Content-Length: ";
#define CONTENT_LENGTH_SIZE (sizeof(g_contentLength) - 1)

static const char g_transferEncoding[] = "Transfer-Encoding: ";
#define TRANSFER_ENCODING_SIZE (sizeof(g_transferEncoding) - 1)

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Thursday 13-Aug-2026 5:52 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

bool RStdHTTP::appendBody(const char *a_newBodyData, int a_size)
{
	if (m_chunked)
	{
		while (a_size > 0)
		{
			if (m_newChunk)
			{
				const char *chunkSizeEnd = strstr(a_newBodyData, "\r\n");

				if (chunkSizeEnd != nullptr)
				{
					int chunkSizeSize = static_cast<int>(chunkSizeEnd - a_newBodyData);

					// TODO: CAW - Use Utils::StringToInt() to avoid memory allocation.
					std::string chunkSizeString(a_newBodyData, chunkSizeSize);
					//int chunkSize = std::stoi(chunkSizeString, nullptr, 16);
					int chunkSize = strtol(chunkSizeString.c_str(), nullptr, 16);

					m_bodySize = chunkSize; // TODO: CAW

					// Skip past the chunk size and the trailing \r\n
					a_newBodyData += chunkSizeSize + 2;
					a_size -= chunkSizeSize + 2;

					if (chunkSize == 0)
					{
						return true;
					}

					// If the entire chunk has arrived in this read, append it to the body and move the pointer to the
					// next chunk size
					if (chunkSize < a_size)
					{
						m_body.append(a_newBodyData, chunkSize);

						// Skip past the chunk data and the trailing \r\n
						a_newBodyData += chunkSize + 2;
						a_size -= chunkSize + 2;
						m_bodySize = 0;
					}
					// Otherwise, append just the partial chunk data
					else
					{
						m_body.append(a_newBodyData, a_size);

						m_bodySize -= a_size;
						a_size = 0;
						m_newChunk = false;
					}
				}
				else
				{
					// TODO: CAW - Better handling of this
					break;
				}
			}
			else
			{
				// If the entire remainder of the chunk has arrived in this read, append it to the body and move the
				// pointer to the next chunk size
				if (m_bodySize < a_size)
				{
					m_body.append(a_newBodyData, m_bodySize);

					// Skip past the chunk data and the trailing \r\n
					a_newBodyData += m_bodySize + 2; // TODO: CAW - This could overflow if the \r\n is not present
					a_size -= m_bodySize + 2;
					m_bodySize = 0;

					m_newChunk = true;
				}
				// Otherwise, append just the partial chunk data
				else
				{
					m_body.append(a_newBodyData, a_size);

					m_bodySize -= a_size;
					a_size = 0;
				}
			}
		}
	}
	else
	{
		m_body.append(a_newBodyData, a_size);
		m_bodySize -= a_size;

		ASSERTM((m_bodySize >= 0), "RStdHTTP::appendBody() => Body size does not match expected body size");

		/* For downwards compatibility with HTTP/1.0, we need to handle the case where the server does not send a */
		/* Content-Length header and instead just closes the connection when it is done sending the body. In this case, */
		/* even if we have read the entire body for *this* read, there may be more data available on the socket, so we */
		/* cannot return true here, but must wait until the next socket read returns 0 */
		return (m_bodySize == 0 && m_haveContentLength);
	}

	return false;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Wednesday 28-Oct-2026 6:39 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

void RStdHTTP::open(RSocket *a_socket, RStdSSLSocket *a_sslSocket)
{
	m_socket = a_socket;
	m_ssl = a_sslSocket;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Socket referenced by m_socket must be open and connected
 *
 * @date	Sunday 18-Apr-2021 3:45 pm, Code HQ Bergmannstrasse // TODO: CAW - Huh?
 * @param	Parameter		Description
 * @return	Return value
 */

int RStdHTTP::request(const std::string &a_url, const std::vector<THTTPHeader> &a_headers, const std::string &a_body,
	TMethod a_method, bool a_keepAlive)
{
	ASSERTM((m_socket != nullptr || m_ssl != nullptr), "RStdHTTP::request() => Neither Socket nor SSL instance has been passed in");
	ASSERTM((m_socket && m_socket->isOpen()) || (m_ssl && m_ssl->isOpen()), "RStdHTTP::request() => Neither Socket nor SSL is open");

	int retVal = KErrGeneral; // TODO: CAW

	// TODO: CAW - Test this on Amiga OS3 and OS4
	printf("Before: m_headers.capacity() = %d, m_body.capacity() = %d\n", (int) m_headers.capacity(), (int) m_body.capacity());
	m_chunked = m_newChunk = m_haveContentLength = false;
	m_headers.clear();
	m_body.clear();
	m_bodySize = m_statusCode = 0;
	printf("After: m_headers.capacity() = %d, m_body.capacity() = %d\n", (int) m_headers.capacity(), (int) m_body.capacity());

	std::string request = (a_method == EMethodPost) ? "POST" : "GET";
	request += " /v1/messages HTTP/1.1\r\n";

	std::string hostname;

	if (a_url.compare(0, 7, "http://", 0, 7) == 0)
	{
		hostname = a_url.substr(7);
	}
	else if (a_url.compare(0, 8, "https://", 0, 8) == 0)
	{
		hostname = a_url.substr(8);
	}
	else
	{
		return KErrCorrupt;
	}

	size_t hostNameOffset = hostname.find('/');

	if (hostNameOffset == std::string::npos)
	{
		return KErrCorrupt;
	}

	hostname.erase(hostNameOffset);
	request += "Host: " + hostname + "\r\n";

	if (!a_keepAlive)
	{
		request += "Connection: close\r\n";
	}

	if (a_body.size() > 0)
	{

#ifdef __amigaos4__

		// TODO: CAW - This awfulness is absolutely temporary!
		std::stringstream StringStream;

		/* Although the OS4 compiler supports C++11, std::to_string() is mysteriously missing so use a */
		/* StringStream to convert the integer to a string */

		StringStream << g_contentLength << a_body.size() << "\r\n";
		request += StringStream.str();

#else /* ! __amigaos4__ */

		request += g_contentLength + std::to_string(a_body.size()) + "\r\n";

#endif /* ! __amigaos4__ */

	}

	// TODO: CAW - Inefficient - profile how many allocations are done in this method
	for (const THTTPHeader &header : a_headers)
	{
		request += header.m_name + ": " + header.m_value + "\r\n";
	}

	request += "\r\n";
	request += a_body;

	int size;

	if (m_socket != nullptr)
	{
		size = m_socket->write(request.c_str(), static_cast<int>(request.size()));
	}
	else
	{
		size = m_ssl->write(request.c_str(), static_cast<int>(request.size()));
	}

	if (size <= 0)
	{
		// TODO: CAW - Map this onto a StdFuncs error code and test it, just like for read()
		return size;
	}

	if (m_buffer == nullptr)
	{
		if ((m_buffer = new char[BUFFER_SIZE]) == nullptr)
		{
			return KErrNoMemory;
		}
	}

	bool bodyComplete = false, foundHeaders = false;

	do
	{
		if (m_socket != nullptr)
		{
			size = m_socket->read(m_buffer, (BUFFER_SIZE - 1), false);
		}
		else
		{
			size = m_ssl->read(m_buffer, (BUFFER_SIZE - 1), false);
		}

		if (size > 0)
		{
			m_buffer[size] = '\0';

			// TODO: CAW - This will break if more than one read is required to get the headers.
			//             We need to buffer the data and parse it for the headers before writing to the file
			if (!foundHeaders)
			{
				// TODO: CAW - For parsing into a map later, we could use std::getline()
				//             https://www.fluentcpp.com/2017/04/21/how-to-split-a-string-in-c/
				const char *headersEnd = strstr(m_buffer, "\r\n\r\n");

				if (headersEnd != nullptr)
				{
					int headersSize = static_cast<int>((headersEnd + 4) - m_buffer);
					int bodySize = size - headersSize;
					std::string headers(m_buffer, headersSize);
					m_headers += std::string(m_buffer, headersSize);

					size_t statusLineEnd = m_headers.find("\r\n");

					if (statusLineEnd != std::string::npos)
					{
						int tokenLength;
						TLex lex(m_headers.c_str(), static_cast<int>(statusLineEnd));

						const char *token = lex.NextToken(&tokenLength);

						if (token != nullptr && strncmp(token, "HTTP/", 5) == 0)
						{
							if ((token = lex.NextToken(&tokenLength)) != nullptr)
							{
								// TODO: CAW - Try using the destructive version of TLex to avoid the string copy, and profile this
								//             entire routine for memory allocations
								std::string statusCode(token, tokenLength);

								if (Utils::StringToInt(statusCode.c_str(), &m_statusCode) != KErrNone)
								{
									return KErrCorrupt;
								}
							}
						}
					} // TODO: CAW - Else, we have a corrupt response, so return an error

					foundHeaders = true;

					// TODO: CAW - When we have a map of headers, we can just look up the Content-Length and Transfer-Encoding headers,
					//             rather than searching for them in the string
					size_t contentLengthStart = m_headers.find(g_contentLength);
					size_t transferEncodingStart = m_headers.find(g_transferEncoding);

					// TODO: CAW - Check for Connection: close and respect it, even if we are using keep-alive
					if (contentLengthStart != std::string::npos)
					{
						size_t contentLengthEnd = m_headers.find("\r\n", contentLengthStart);

						if (contentLengthEnd != std::string::npos)
						{
							// TODO: CAW - Add a length parameter to StringToInt() to avoid the string copy
							std::string contentLength(m_headers.c_str() + contentLengthStart + CONTENT_LENGTH_SIZE,
								(contentLengthEnd - contentLengthStart - CONTENT_LENGTH_SIZE));

							if (Utils::StringToInt(contentLength.c_str(), &m_bodySize) != KErrNone)
							{
								return KErrCorrupt;
							}

							m_haveContentLength = true;
						}
					}
					else if (transferEncodingStart != std::string::npos)
					{
						m_chunked = m_newChunk = true;
					}
					else
					{
						/* There is no Content-Length header and the body is not chunked, so just save the calculated size of the body */
						/* and we will save just that for now. There may be more data to be read later */
						m_bodySize = bodySize;
					}

					/* If there was any data left after the headers, add it to the m_body member. We do this here, rather */
					/* than above, as we need to first know whether or not the response is chunked */
					bodyComplete = appendBody(m_buffer + headersSize, bodySize);
				}
				/* Some, but not all headers were received, so copy those we received into the m_headers string, and later, we */
				/* will append the reminder and process them */
				else
				{
					m_headers += std::string(m_buffer, size);
				}
			}
			else
			{
				bodyComplete = appendBody(m_buffer, size);
			}
		}
		else if (size == 0)
		{
			/* Raw sockets and SSL connections return their results differently, so we have to handle this */
			if (m_socket != nullptr)
			{
				/* If size is 0, then the server has closed the connection */
				retVal = (size == 0) ? KErrNone : KErrEof;
			}
			else
			{
				retVal = m_ssl->get_error(size);
			}

			break;
		}
		else
		{
			/* The length is negative (most probably -1), so break out and indicate there was an error reading more data */
			retVal = KErrEof;

			break;
		}
	}
	while (!bodyComplete);

	if (bodyComplete)
	{
		retVal = KErrNone;
	}

	return retVal;
}
