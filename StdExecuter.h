
#ifndef STDEXECUTER_H
#define STDEXECUTER_H

#include <functional>

/**
 * A class for executing commands and capturing their output.
 * Using this class, it is possible to asynchronously launch a command and capture what it outputs to stdout. The
 * command is launched asynchronously and a callback is called when data becomes available. This allows the client
 * software to continue operation of its main thread without blocking.
 */

class RStdExecuter
{

#ifdef __amigaos__

	char			*m_buffer;		/**< Buffer used to reading stdout data from the child */
	int32_t			m_exitCode;		/**< Exit code of the child process */
	ULONG			m_signal;		/**< Signal used to wait for messages from the DOS Handler */
	BPTR			m_stdInRead;	/**< Handle for reading from stdin (child) */
	BPTR			m_stdOutRead;	/**< Handle for reading from stdout (parent) */
	BPTR			m_stdOutWrite;	/**< Handle for writing to std out (child) */
	MsgPort			*m_port;		/**< Message port used to receive messages from DOS Handler */
	DosPacket		*m_packet;		/**< DOS Packet used to request stdout reads from DOS Handler */

#elif defined(WIN32)

	HANDLE			m_stdInRead;		/**< Handle for reading from stdin (child) */
	HANDLE			m_stdInWrite;		/**< Handle for writing to stdin (parent) */
	HANDLE			m_stdOutRead;		/**< Handle for reading from stdout (parent) */
	HANDLE			m_stdOutWrite;		/**< Handle for writing to std out (child) */

#endif /* WIN32 */

protected:

	/**< Type definition for client callback to invoke when data is received */
	using Callback = std::function<void(const char *a_buffer, int a_size)>;

	Callback		m_callback;			/**< Client callback to invoke when data is received */

public:

	RStdExecuter()
	{

#ifdef __amigaos__

		m_buffer = nullptr;
		m_signal = 0;
		m_stdInRead = m_stdOutRead = m_stdOutWrite = 0;
		m_port = nullptr;
		m_packet = nullptr;

#elif defined(WIN32)

		m_stdInRead = m_stdInWrite = m_stdOutRead = m_stdOutWrite;

#endif /* WIN32 */

	}

	void close();

#ifdef __amigaos__

	void read();

	void readComplete();

#endif /* __amigaos__ */

private:

#ifdef WIN32

	int createChildProcess(const char *a_commandName, HANDLE &a_childProcess);

#endif /* WIN32 */

public:

	TResult launchCommand(const char *a_commandName, int a_stackSize, Callback a_callback);

#ifdef __amigaos__

	ULONG getSignal()
	{
		return m_signal;
	}

#endif /* __amigaos__ */

};

#endif /* ! STDEXECUTER_H */
