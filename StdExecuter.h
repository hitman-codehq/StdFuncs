
#ifndef STDEXECUTER_H
#define STDEXECUTER_H

#ifdef QT_GUI_LIB

#include <Qt/QtExecuter.h>

#endif /* QT_GUI_LIB */

#include <functional>

/**
 * A class for executing commands and capturing their output.
 * Using this class, it is possible to asynchronously launch a command and capture what it outputs to stdout. The
 * command is launched asynchronously and a callback is called when data becomes available. This allows the client
 * software to continue operation of its main thread without blocking.
 */

class RStdExecuter
{
public:

#ifdef __amigaos__

	/**
	 * Structure used to return the result of an asynchronous launch.
	 * Contains information that can be used when a command that has been asynchronously launched has completed, to
	 * signal the calling process that the command has completed and to provide the exit code of the command.
	 */
	struct SExitData
	{
		int32_t		m_exitCode;		/**< The exit code of the child process */
		ULONG		m_sigBit;		/**< Signal bit used to send exit message to parent process */
		Task		*m_parentTask;	/**< Task to which the signal should be sent */
	};

#endif /* __amigaos__ */

private:

#ifdef __amigaos__

	char			*m_buffer;		/**< Buffer used to reading stdout data from the child */
	SExitData		m_exitData;		/**< Exit data information for the child process */
	ULONG			m_signal;		/**< Signal used to wait for messages from the DOS Handler */
	BPTR			m_stdInRead;	/**< Handle for reading from stdin (child) */
	BPTR			m_stdOutRead;	/**< Handle for reading from stdout (parent) */
	BPTR			m_stdOutWrite;	/**< Handle for writing to std out (child) */
	MsgPort			*m_port;		/**< Message port used to receive messages from DOS Handler */
	DosPacket		*m_packet;		/**< DOS Packet used to request stdout reads from DOS Handler */

#elif defined(QT_GUI_LIB)

	RQtExecuter		m_executer;		/**< Qt-specific helper class for executing commands and capturing output */

#elif defined(WIN32)

	char			*m_buffer;			/**< Buffer used to reading stdout data from the child */
	HANDLE			m_childProcess;		/**< Handle for the child process */
	HANDLE			m_stdInRead;		/**< Handle for reading from stdin (child) */
	HANDLE			m_stdInWrite;		/**< Handle for writing to stdin (parent) */
	HANDLE			m_stdOutRead;		/**< Handle for reading from stdout (parent) */
	HANDLE			m_stdOutWrite;		/**< Handle for writing to std out (child) */
	OVERLAPPED		m_overlapped;		/**< Overlapped structure for overlapped I/O */

#endif /* WIN32 */

protected:

	/**< Type definition for client callback to invoke when data is received */
	using Callback = std::function<void(const char *a_buffer, int a_size, const TResult &a_result)>;

	Callback		m_callback;			/**< Client callback to invoke when data is received */

public:

#ifdef QT_GUI_LIB

	RStdExecuter() : m_executer(*this)

#else /* ! QT_GUI_LIB */

	RStdExecuter()

#endif /* ! QT_GUI_LIB */

	{

#ifdef __amigaos__

		m_exitData = {0, (ULONG) -1, nullptr};

		m_buffer = nullptr;
		m_signal = 0;
		m_stdInRead = m_stdOutRead = m_stdOutWrite = 0;
		m_port = nullptr;
		m_packet = nullptr;

#elif defined(WIN32) && !defined(QT_GUI_LIB)

		m_stdInRead = m_stdInWrite = m_stdOutRead = m_stdOutWrite;

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

	}

	void close();

#ifdef __amigaos__

	void read();

	void readComplete(bool a_dataAvailable);

#elif defined(QT_GUI_LIB)

	void readComplete(const char *a_buffer, int a_size, const TResult &a_result);

#elif defined(WIN32)

	void readComplete(DWORD a_errorCode, int a_size);

#endif /* WIN32 */

private:

#if defined(WIN32) && !defined(QT_GUI_LIB)

	int createChildProcess(const char *a_commandName, const char *a_arguments, HANDLE &a_childProcess);

	static void CALLBACK readCompletionRoutine(DWORD a_errorCode, DWORD a_bytesTransferred, LPOVERLAPPED a_overlapped);

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

public:

	bool isRunning() const
	{

#ifdef __amigaos__

	return (m_stdOutRead != 0);

#elif defined(QT_GUI_LIB)

	return m_executer.isRunning();

#elif defined(WIN32)

	return (m_childProcess != nullptr);

#else /* ! WIN32 */

	return false;

#endif /* ! WIN32 */

	}

	int launchCommand(const char *a_commandName, const char *a_arguments, int a_stackSize, Callback a_callback);

#ifdef __amigaos__

	ULONG getSignal()
	{
		return m_signal;
	}

	ULONG getCompletionSignal()
	{
		return 1 << m_exitData.m_sigBit;
	}

#endif /* __amigaos__ */

};

#endif /* ! STDEXECUTER_H */
