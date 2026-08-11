
#include <StdFuncs.h>
#include <StdHTTP.h>
#include <StdSocket.h>
#include <Test.h>

static RTest Test("T_StdHTTP");	/* Class to use for testing and reporting results */

enum TTestFeature
{
	EFeatureContentLength,
	EFeatureSplitChunkedEnd,
	EFeatureSplitChunkedMid,
	EFeatureChunked
};

static const char g_contentLengthResponse[] =
	"(HTTP/1.1 200 OK\r\n"
	"Content-Length: 13\r\n"
	"\r\n" // TODO: CAW - Is a blank line a problem?
	"Hello, world!";

static const char g_chunkedResponse[] =
	"HTTP/1.1 200 OK\r\n"
	"Transfer-Encoding: chunked\r\n"
	"\r\n"
	"d\r\n"
	"Hello, world!\r\n"
	"0\r\n"
	"\r\n";

static const char g_splitChunkedEndResponse1[] =
	"HTTP/1.1 200 OK\r\n"
	"Transfer-Encoding: chunked\r\n"
	"\r\n"
	"5\r\n"
	"Hello\r\n";

static const char g_splitChunkedEndResponse2[] =
	"8\r\n"
	", world!\r\n"
	"0\r\n"
	"\r\n";

static const char g_splitChunkedMidResponse1[] =
	"HTTP/1.1 200 OK\r\n"
	"Transfer-Encoding: chunked\r\n"
	"\r\n"
	"3\r\n"
	"Hel\r\n";

static const char g_splitChunkedMidResponse2[] =
	"lo8\r\n"
	", world!\r\n"
	"0\r\n"
	"\r\n";

class RSocketStub : public RSocket
{
	int				m_iteration;
	TTestFeature	m_testFeature;

public:

	int open(const char *a_host, unsigned short a_port)
	{
		(void) a_host;
		(void) a_port;

		/* Ensure that concrete RSocket methods do not assert on the socket being closed */
		m_socket = 0;

		return KErrNone;
	}

	void close()
	{
		m_socket = INVALID_SOCKET;
	}

	void setTestFeature(TTestFeature a_testFeature)
	{
		m_iteration = 0;
		m_testFeature = a_testFeature;
	}

	int read(void *a_buffer, int a_size, bool a_readAll)
	{
		(void) a_readAll;

		int retVal;

		printf("In read() for size %d, m_testFeature = %d\n", a_size, m_testFeature);

		switch (m_testFeature)
		{
			case EFeatureContentLength :
			{
				printf("Copying content length\n");
				retVal = strlen(g_contentLengthResponse);
				memcpy(a_buffer, g_contentLengthResponse, retVal);

				break;
			}

			case EFeatureChunked :
			{
				printf("Copying chunked\n");
				retVal = strlen(g_chunkedResponse);
				memcpy(a_buffer, g_chunkedResponse, retVal);

				break;
			}

			case EFeatureSplitChunkedEnd :
			{
				printf("Copying chunked iteration %d\n", m_iteration);

				if (m_iteration == 0)
				{
					retVal = strlen(g_splitChunkedEndResponse1);
					memcpy(a_buffer, g_splitChunkedEndResponse1, retVal);
				}
				else if (m_iteration == 1)
				{
					retVal = strlen(g_splitChunkedEndResponse2);
					printf("Eh: %d %s***\n", retVal, g_splitChunkedEndResponse2);
					memcpy(a_buffer, g_splitChunkedEndResponse2, retVal);
				}
				else
				{
					retVal = -1;
				}

				++m_iteration;

				break;
			}

			case EFeatureSplitChunkedMid :
			{
				printf("Copying chunked iteration %d\n", m_iteration);

				if (m_iteration == 0)
				{
					retVal = strlen(g_splitChunkedMidResponse1);
					memcpy(a_buffer, g_splitChunkedMidResponse1, retVal);
				}
				else if (m_iteration == 1)
				{
					retVal = strlen(g_splitChunkedMidResponse2);
					printf("Eh: %d %s***\n", retVal, g_splitChunkedMidResponse2);
					memcpy(a_buffer, g_splitChunkedMidResponse2, retVal);
				}
				else
				{
					retVal = -1;
				}

				++m_iteration; // TODO: CAW - Handle outside

				break;
			}

			default:
			{
				printf("Doing nuffink\n");
				retVal = -1;
			}
		}

		printf("Returning %d\n", retVal);
		return retVal;
	}

	int write(const void *a_buffer, int a_size)
	{
		(void) a_buffer;

		return a_size;
	}
};

// TODO: CAW - Tests
// 200 OK 404 Not Found
// Read small with Content-Length (one read required for header and body)
// Read small with chunked (one read required for header and body)
// Read larger with Content-Length (two reads required with read split in headers)
// Read larger with Content-Length (two reads required with read split in body)
// Read failure due to not having Content-Length or Transfer-Encoding
// Check arbitrary headers from the headers vector, array or list
// Read larger with chunked (split in the chunk-size line itself) ? e.g., the read boundary lands mid-way through "1a3" before the terminating \r\n of the size line. This is a nasty one to get right and worth its own explicit test, since it's a different failure mode from splitting mid-data.
// Read larger with chunked (split in chunk data mid chunk) ? analogous to your Content-Length body-split case.
// Read larger with chunked (split in chunk data at end of chunk) ? analogous to your Content-Length body-split case.
// Multiple chunks in one response ? your current examples only exercise a single chunk followed by the terminating 0. A response with 2-3 real chunks back to back would catch any bug where your loop doesn't correctly return to "read next chunk-size line" after consuming one chunk's trailing \r\n.

int main()
{
	int result;

	Test.Title();
	Test.Start("RStdHTTP class API test");

	/* Test GET with Content-Length works correctly */

	Test.Next("Test GET with Content-Length works correctly");

	RSocketStub socket;

	result = socket.open("localhost", 80);
	test(result == KErrNone);

	socket.setTestFeature(EFeatureContentLength);

	RStdHTTP http(&socket);
	std::vector<THTTPHeader> headers;

	//result = http.get("http://localhost/", "", headers);
	//test(result == KErrNone);

	printf("Headers = %s\n", http.headers().c_str());
	printf("Body = %s\n", http.body().c_str());
	//test(http.body() == "Hello, world!");

	/* Test GET with chunked response works correctly */

	Test.Next("Test GET with chunked response works correctly");

	socket.setTestFeature(EFeatureChunked);

	result = http.get("http://localhost/", "", headers);
	test(result == KErrNone);

	printf("Headers = %s\n", http.headers().c_str());
	printf("Body = %s\n", http.body().c_str());
	test(http.body() == "Hello, world!");

	/* Test GET with split chunked end response works correctly */

	Test.Next("Test GET with split chunked end response works correctly");

	socket.setTestFeature(EFeatureSplitChunkedEnd);

	result = http.get("http://localhost/", "", headers);
	test(result == KErrNone);

	printf("Headers = %s\n", http.headers().c_str());
	printf("Body = %s\n", http.body().c_str());
	test(http.body() == "Hello, world!");

	/* Test GET with split chunked mid response works correctly */

	Test.Next("Test GET with split chunked mid response works correctly");

	socket.setTestFeature(EFeatureSplitChunkedMid);

	result = http.get("http://localhost/", "", headers);
	test(result == KErrNone);

	printf("Headers = %s\n", http.headers().c_str());
	printf("Body = %s\n", http.body().c_str());
	test(http.body() == "Hello, world!");

	Test.End();

	return(RETURN_OK);
}
