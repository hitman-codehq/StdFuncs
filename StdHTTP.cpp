
#include <StdFuncs.h>
// TODO: CAW - Use a C++ version?
#include <string.h>
#include "StdHTTP.h"
#include "StdSocket.h"
#include "StdSSL.h"

// TODO: CAW - This doesn't work with Anthropic
//#define BUFFER_SIZE 1024
#define BUFFER_SIZE 1024*5

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
			//printf("appendBody, m_bodySize = %d, a_size = %d, m_newChunk = %d\n", m_bodySize, a_size, m_newChunk);

			if (m_newChunk)
			{
				printf("*** ENTERING, a_size = %d ***\n", a_size);
				const char *chunkSizeEnd = strstr(a_newBodyData, "\r\n");

				if (chunkSizeEnd != nullptr)
				{
					printf("Found chunk end\n");
					int chunkSizeSize = chunkSizeEnd - a_newBodyData;

					// TODO: CAW - Avoid string and use Utils::AToI
					std::string chunkSizeString(a_newBodyData, chunkSizeSize);
					int chunkSize = std::stoi(chunkSizeString, nullptr, 16);
					m_bodySize = chunkSize;
					printf("*** chunkSize = %d (%x) %d\n", chunkSize, chunkSize, a_size);

					a_newBodyData += chunkSizeSize + 2;
					a_size -= chunkSizeSize + 2;
					m_bodySize -= chunkSizeSize + 2;

					if (chunkSize == 0)
					{
						printf("*** Found end chunk of zero size\n");

						return true;
					}

					if (chunkSize < a_size)
					{
						printf("Appending %d bytes\n", chunkSize);
						m_body.append(a_newBodyData, chunkSize);

						a_newBodyData += chunkSize + 2;
						a_size -= chunkSize + 2;
						m_bodySize -= chunkSize + 2;

						printf("Now a_size = %d, a_newBodyData = ***%s***\n", a_size, a_newBodyData);
					}
					else
					{
						printf("Appending remaining %d bytes and breaking out\n", a_size);
						m_body.append(a_newBodyData, a_size);
						m_newChunk = false;

						return false;
					}
				}
				else
				{
					// TODO: CAW - Better handling of this
					printf("No chunk end, breaking out\n");

					break;
				}

				//return true;
			}
			else
			{
				if (m_bodySize < a_size)
				{
					printf("Appending extra %d bytes\n", m_bodySize);
					m_body.append(a_newBodyData, m_bodySize);

					a_newBodyData += m_bodySize + 2;
					a_size -= m_bodySize + 2;
					m_bodySize = 0;//-= chunkSize + 2;

					m_newChunk = true;
				}
			}
		}
	}
	else
	{
		m_body.append(a_newBodyData, a_size);
		printf("*** Appended %d bytes, new size is %d\n", a_size, (int) m_bodySize);
		m_bodySize -= a_size;
		ASSERTM((m_bodySize >= 0), "RStdHTTP::appendBody() => Body size does not match expected body size");

		return (m_bodySize == 0);
	}

	return false;
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Socket referenced by m_socket must be open and connected // TODO: CAW - Assert on this
 *
 * @date	Sunday 18-Apr-2021 3:45 pm, Code HQ Bergmannstrasse
 * @param	Parameter		Description
 * @return	Return value
 */

// GET /somepage.html HTTP/1.1\r\n
// Host: somedomain.org\r\n
// Connection: close\r\n
// \r\n

// POST /v1/messages HTTP/1.1\r\n
// Host: api.anthropic.com\r\n
// Content-Type: application/json\r\n
// Content-Length: 1234\r\n // TODO: CAW - Calculate after substitutions have happened
// x-api-key: sk-ant-...\r\n
// anthropic-version: 2023-06-01\r\n
// Connection: close\r\n
// \r\n
// {...the JSON body itself...}
//
// TODO: CAW - Implement T_HTTP
//
// - Test passing in https:// -vs- no https:// and a_url with length < len(https://)

// TODO: CAW - Swap order of a_headers and a_body and make them both optional
// TODO: CAW - This is using POST, so rename it
int RStdHTTP::get(const std::string &a_url, const std::string &a_body, const std::vector<THTTPHeader> &a_headers)
{
	ASSERTM((m_socket != nullptr || m_ssl != nullptr), "RStdHTTP::get() => Neither Socket nor SSL instance has been passed in");

	int retVal = KErrGeneral; // TODO: CAW

	m_chunked = false;
	m_headers.clear();
	m_body.clear();

	printf("*** get: Getting %s\n", a_url.c_str());
	fflush(stdout);

	std::string url;

	// TODO: CAW - Use string literals
	if (a_url.compare(0, 7, "http://", 0, 7) == 0)
	{
		url = a_url.substr(7);
	}
	else if (a_url.compare(0, 8, "https://", 0, 8) == 0)
	{
		url = a_url.substr(8);
	}
	else
	{
		printf("KErrCorrupt\n");
		return KErrCorrupt;
	}

	// TODO: CAW - substr() inefficient + literal 8
	//std::string request = "GET / HTTP/1.1\r\n"; // TODO: CAW - Hack + add test for this
	std::string request = "POST /v1/messages HTTP/1.1\r\n"; // TODO: CAW - Hack + add test for this

	// TODO: CAW - Bodgey hack for now!
	request += "Host: api.anthropic.com\r\nConnection: close\r\n";
	//request += "Host: google.com\r\nConnection: close\r\n";

	if (a_body.size() > 0)
	{
		printf("Adding Content-Length of %d bytes\n", (int) a_body.size());
		request += "Content-Length: " + std::to_string(a_body.size()) + "\r\n";
	}

	// TODO: CAW - Inefficient - profile how many allocations are done in this method
	for (const THTTPHeader &header : a_headers)
	{
		request += header.m_name + ": " + header.m_value + "\r\n";
	}

	request += "\r\n";
	request += a_body;
	printf("--- request = ***%s***\n", request.c_str());

	int length; // TODO: CAW - Should be size

	if (m_socket != nullptr)
	{
		length = m_socket->write(request.c_str(), static_cast<int>(request.size()));
	}
	else
	{
		length = m_ssl->write(request.c_str(), static_cast<int>(request.size()));
	}

	if (length <= 0)
	{
		// TODO: CAW - Correct?
		printf("RSocket.write() returned %d\n", length);
		retVal = length;
		return retVal;
	}

	if (m_buffer == nullptr)
	{
		if ((m_buffer = new char[BUFFER_SIZE]) == nullptr)
		{
			printf("Not enough memory\n");
			return KErrNoMemory;
		}
	}

	bool bodyComplete = false, foundHeaders = false;
	int total = 0;//, totalLength = 0;

	do
	{
		if (m_socket != nullptr)
		{
			length = m_socket->read(m_buffer, (BUFFER_SIZE - 1), false);
			//length = m_socket.read(g_imageBuffer, (sizeof(g_imageBuffer) - 1), false);
		}
		else
		{
			length = m_ssl->read(m_buffer, (BUFFER_SIZE - 1), false);
		}

		printf("length = %d\n", length);

		if (length > 0)
		{
			m_buffer[length] = '\0';
			printf("*** m_buffer = %s***\n", m_buffer);

			// TODO: CAW - This will break if more than one read is required to get the headers.
			//             We need to buffer the data and parse it for the headers before writing to the file
			if (!foundHeaders)
			{
				const char *headersEnd = strstr(m_buffer, "\r\n\r\n");

				if (headersEnd != nullptr)
				{
					int headersSize = static_cast<int>((headersEnd + 4) - m_buffer);
					int bodySize = length - headersSize;
					std::string headers(m_buffer, headersSize);
					m_headers += std::string(m_buffer, headersSize);
					printf("headers = ***%s***\n", headers.c_str());
					total += bodySize;
					printf("total = %d, headerSize = %d, bodySize = %d\n", total, headersSize, bodySize);

					foundHeaders = true;

					const char *contentLengthStart = strstr(m_buffer, "Content-Length: ");
					const char *transferEncodingStart = strstr(m_buffer, "Transfer-Encoding: ");

					if (contentLengthStart != nullptr)
					{
						const char *contentLengthEnd = strstr(contentLengthStart, "\r\n");

						if (contentLengthEnd != nullptr)
						{
							// TODO: CAW - Hard coded
							std::string contentLength((contentLengthStart + 16), (contentLengthEnd - contentLengthStart - 16));
							m_bodySize = /*totalLength =*/ std::stoi(contentLength);
							printf("*** m_bodySize = %d\n", m_bodySize);
						}
					}
					else if (transferEncodingStart != nullptr)
					{
						m_chunked = m_newChunk = true;
						// loop:
						// read a line, ending in \r\n ? parse as hex ? chunkSize
						// if chunkSize == 0:
						// 	read the final \r\n (end of chunked body)
						// break
						// read exactly chunkSize bytes ? this is real data, append to body
						// read 2 more bytes and discard ? this is the \r\n trailing this chunk's data
					}
					else
					{
						printf("*** BOO! ***\n");
						//totalLength = BUFFER_SIZE - 1; // TODO: CAW - Hack!
						// TODO: CAW - Return error here
					}

					/* If there was any data left after the headers, add it to the m_body member. We do this here, rather */
					/* than above, as we need to first know whether or not the response is chunked */
					if ((bodyComplete = appendBody(m_buffer + headersSize, bodySize)))
					{
						//m_body += std::string(m_buffer + headersSize, bodySize); //length - headersSize);
						printf("Body complete, body = ***%s***\n", m_body.c_str());
					}
					else
					{
						printf("Body incomplete, continuing, body = ***%s***\n", m_body.c_str());
					}
				}
			}
			else
			{
				total += length;

				if ((bodyComplete = appendBody(m_buffer, length)))
				{
					//m_body += std::string(m_buffer + headersSize, bodySize); //length - headersSize);
					printf("Body complete, body = ***%s***\n", m_body.c_str());
				}
				else
				{
					printf("Body incomplete, continuing, body = ***%s***\n", m_body.c_str());
				}

				// TODO: CAW - Make an overload for this
				//file.write(reinterpret_cast<unsigned char *>(g_imageBuffer), length);
			}
		}
		else
		{
			printf("read() returned %d\n", length);
			//if (length == 0) // TODO: CAW - Hack for chunked bodies
			{
				break;
			}
		}
	}
	while (!bodyComplete); //(total < totalLength);

	printf("total = %d\n", total);//, totalLength);

	//if (total == totalLength)
	{
		printf("Read entire body!\n");
		retVal = KErrNone;
	}

	/*if (length > 0)
	{
		m_buffer = new char[BUFFER_SIZE];

		do
		{
			length = m_socket.read(m_buffer, (BUFFER_SIZE - 1), false);

			if (length > 0)
			{
				m_buffer[length] = '\0';

				if (!foundHeaders)
				{
					// TODO: CAW - For parsing into a map later, we could use std::getline()
					//             https://www.fluentcpp.com/2017/04/21/how-to-split-a-string-in-c/
					endOfHeaders = strstr(m_buffer, "\r\n\r\n");

					if (endOfHeaders != nullptr)
					{
						*endOfHeaders = '\0';
						foundHeaders = true;

						m_headers.append(m_buffer);

						endOfHeaders += 4;
						m_body.append(std::string(endOfHeaders));
					}
					else
					{
						m_headers.append(m_buffer);
					}
				}
				else
				{
					m_body.append(m_buffer);
				}
			}
			else
			{
				//retVal = KErrGeneral;
				break;
			}
		}
		// TODO: CAW - This will have the same problem as the PNG fetcher. Use Content-Length here as well
		while (length == (BUFFER_SIZE - 1));
	}*/

	return retVal;
}
