
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
		Utils::info("KErrCorrupt");
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
		Utils::info("Adding Content-Length of %d bytes\n", a_body.size());
		request += "Content-Length: " + std::to_string(a_body.size()) + "\r\n";
	}

	// TODO: CAW - Inefficient - profile how many allocations are done in this method
	for (const THTTPHeader &header : a_headers)
	{
		request += header.m_name + ": " + header.m_value + "\r\n";
	}

	request += "\r\n";
	//Utils::info("size = %d\n", request.size());
	request += a_body;
	//Utils::info("Now size = %d (%d)\n", request.size(), a_body.size());
	//request += "***";
	//Utils::info(request.c_str());
	printf("--- request = ***%s***\n", request.c_str());

	//return KErrNone;

	int length;

	if (m_socket != nullptr)
	{
		printf("1\n");
		fflush(stdout);
		length = m_socket->write(request.c_str(), static_cast<int>(request.size()));
		printf("1a\n");
		fflush(stdout);
	}
	else
	{
		printf("2\n");
		fflush(stdout);
		length = m_ssl->write(request.c_str(), static_cast<int>(request.size()));
		printf("2a\n");
		fflush(stdout);
	}

	printf("*** Wrote %d bytes\n", length);

	if (length <= 0)
	{
		// TODO: CAW - Correct?
		Utils::info("RSocket.write() returned %d\n", length);
		retVal = length;
		return retVal;
	}

	if (m_buffer == nullptr)
	{
		if ((m_buffer = new char[BUFFER_SIZE]) == nullptr)
		{
			Utils::info("Not enough memory");
			return KErrNoMemory;
		}
	}

	bool foundHeaders = false;
	int total = 0, totalLength = 0;

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

		if (length > 0)
		{
			m_buffer[length] = '\0';
			printf("*** m_buffer = %s\n", m_buffer);

			// TODO: CAW - This will break if more than one read is required to get the headers.
			//             We need to buffer the data and parse it for the headers before writing to the file
			if (!foundHeaders)
			{
				const char *headersEnd = strstr(m_buffer, "\r\n\r\n");

				if (headersEnd)
				{
					int headersSize = static_cast<int>((headersEnd + 4) - m_buffer);
					int writeSize = length - headersSize;
					std::string headers(m_buffer, headersSize);
					printf("headers = ***%s***\n", headers.c_str());
					total += writeSize;
					m_headers += std::string(m_buffer, headersSize);
					m_body += std::string(m_buffer + headersSize, length - headersSize);
					foundHeaders = true;

					const char *contentLengthStart = strstr(m_buffer, "Content-Length: ");
					const char *transferEncodingStart = strstr(m_buffer, "Transfer-Encoding: ");

					if (contentLengthStart != nullptr)
					{
						const char *contentLengthEnd = strstr(contentLengthStart, "\r\n");

						if (contentLengthEnd != nullptr)
						{
							std::string contentLength((contentLengthStart + 16), (contentLengthEnd - contentLengthStart - 16));
							totalLength = std::stoi(contentLength);
						}
					}
					else if (transferEncodingStart != nullptr)
					{
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
						totalLength = BUFFER_SIZE - 1; // TODO: CAW - Hack!
						// TODO: CAW - Return error here
					}
				}
			}
			else
			{
				total += length;
				m_body += std::string(m_buffer, length);
				printf("body = ***%s***\n", m_body.c_str());
				// TODO: CAW - Make an overload for this
				//file.write(reinterpret_cast<unsigned char *>(g_imageBuffer), length);
			}
		}
		else
		{
			printf("read() returned %d\n", length);
			if (length == 0) // TODO: CAW - Hack for chunked bodies
			{
				break;
			}
		}
	}
	while (total < totalLength);

	printf("total = %d, totalLength = %d\n", total, totalLength);

	if (total == totalLength)
	{
		Utils::info("Read entire body!");
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
