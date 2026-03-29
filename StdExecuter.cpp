
#include <StdFuncs.h>
#include "StdApplication.h"
#include "StdWindow.h"
#include "StdExecuter.h"

#ifdef __amigaos__

#include <dos/dostags.h>

#endif /* __amigaos__ */

/* The size of the buffer used for capturing stdout */
#define STDOUT_BUFFER_SIZE 1024

/* The size of the stack used on Amiga OS if no size is explicitly specified */
#define DEFAULT_STACK_SIZE 20480

/**
 * Amiga OS client exit callback function.
 * This function is called in the context of a child process that has been launched by the
 * CreateNewProcTags() function when that child process is terminated.
 *
 * @date	Friday 10-Nov-2023 6:00 am, Code HQ Tokyo Tsukuda
 * @param	a_returnCode	The return code of the child process
 * @param	a_exitData		Pointer to a structure containing exit data for the child process
 */

#ifdef __amigaos4__

void ExitFunction(int32_t a_returnCode, RStdExecuter::SExitData *a_exitData)
{
	a_exitData->m_exitCode = a_returnCode;

	Signal(a_exitData->m_parentTask, (1L << a_exitData->m_sigBit));
}

#elif defined(__amigaos__)

void ExitFunction(int32_t a_returnCode __asm("d0"), RStdExecuter::SExitData *a_exitData __asm("d1"))
{
	CommandLineInterface *cli = Cli();

	if (cli != nullptr)
	{
		a_exitData->m_exitCode = cli->cli_ReturnCode;
	}
	else
	{
		a_exitData->m_exitCode = a_returnCode;
	}

	Signal(a_exitData->m_parentTask, (1L << a_exitData->m_sigBit));
}

#elif defined(WIN32) && !defined(QT_GUI_LIB)

 /**
  * Short description.
  * Long multi line description.
  *
  * @pre		Some precondition here
  *
  * @date	Saturday 28-Mar-2026 7:25 am, Code HQ Tokyo Tsukuda
  * @param	Parameter		Description
  * @return	Return value
  */

VOID CALLBACK ReadCompletionRoutine(
	DWORD dwErrorCode,
	DWORD dwBytesTransferred,
	LPOVERLAPPED lpOverlapped)
{
	if (dwErrorCode == 0 && dwBytesTransferred > 0)
	{
		Utils::info("*** Got data from child process: %d", dwBytesTransferred);
		if (dwBytesTransferred > 0)
		{
			RStdExecuter *executer = (RStdExecuter *) lpOverlapped->hEvent;

			/* NULL terminate and print the child's output, and send it to the client for processing */
			executer->readComplete(dwBytesTransferred);
			//executer->m_buffer[bytesRead] = '\0';
			//executer->m_callback(executer->m_buffer, bytesRead);
			/* And read the next block of data */
			//read();
		}
		else
		{
			//close();
		}
	}
}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */

/**
 * Frees any resources associated with the class.
 * This method performs the same functions as the destructor, but allows the user to call it when manual
 * deinitialisation of the class is required. After completion, the class instance can be reused by calling open()
 * again.
 *
 * @date	Saturday 01-Nov-2025 7:00 am, Code HQ Tokyo Tsukuda
 */

void RStdExecuter::close()
{

#ifdef __amigaos__

	/* Remove this executer from the list of active executers */
	CWindow::GetRootWindow()->GetApplication()->RemoveExecuter(this);

	/* Free the signal used for waiting for the child process to exit, and reset the exit data structure members */
	FreeSignal(m_exitData.m_sigBit);

	m_exitData.m_exitCode = 0;
	m_exitData.m_sigBit = (ULONG) -1;
	m_exitData.m_parentTask = nullptr;

	delete [] m_buffer;
	m_buffer = nullptr;

	m_signal = 0;

	if (m_stdInRead != 0)
	{
		Close(m_stdInRead);
		m_stdInRead = 0;
	}

	if (m_stdOutWrite != 0)
	{
		Close(m_stdOutWrite);
		m_stdOutWrite = 0;
	}

	if (m_stdOutRead != 0)
	{
		Close(m_stdOutRead);
		m_stdOutRead = 0;
	}

	if (m_packet != nullptr)
	{
		FreeDosObject(DOS_STDPKT, m_packet);
		m_packet = nullptr;
	}

	if (m_port != nullptr)
	{
		DeleteMsgPort(m_port);
		m_port = nullptr;
	}

#elif defined(WIN32)

	delete[] m_buffer;
	m_buffer = nullptr;

#endif /* ! __amigaos__ */

}

#ifdef __amigaos__

/**
 * Start an asynchronous read.
 * Sends a message to the underlying DOS file system handler to request it to read whatever bytes are available, up to
 * a maximum size. This is an asynchronous operation. Upon completeion, a signal will be handled in the RApplication
 * class and the readComplete() callback will be called with the data read.
 *
 * @date	Saturday 01-Nov-2025 6:52 am, Code HQ Tokyo Tsukuda
 */

void RStdExecuter::read()
{
	struct FileHandle *fileHandle = (FileHandle *) BADDR(m_stdOutRead);
	StandardPacket *stdPacket = (StandardPacket *) m_packet->dp_Link;

#ifdef __amigaos4__

	struct MsgPort *filePort = fileHandle->fh_MsgPort;

#else /* ! __amigaos4__ */

	struct MsgPort *filePort = fileHandle->fh_Type;

#endif /* ! __amigaos4__ */

	/* Send a read packet to he file system handler to read as much output from the child's stdout as possible. */
	/* When the child exits, the pipe will be closed and Read() will fail */
	m_packet->dp_Port = m_port;
	m_packet->dp_Type = ACTION_READ;
	m_packet->dp_Arg1 = (LONG) m_stdOutRead;
	m_packet->dp_Arg2 = (LONG) m_buffer;
	m_packet->dp_Arg3 = (STDOUT_BUFFER_SIZE - 1);
	PutMsg(filePort, (struct Message *) stdPacket);
}

/**
 * Process a block of data read from the file system handler.
 * This method is called by the RApplication class when a message is received from the file system handler. It will
 * call the client's callback method and pass it the block of data read, and will kick off another call to read(), to
 * read the next block of data from the file system handler.
 *
 * If the all data has been read, it will close the pipe used for communication with the child process, and will remove
 * this executer from the list of active executers. The class instance can then be reused by calling open() again.
 *
 * @date	Saturday 01-Nov-2025 6:36 am, Code HQ Tokyo Tsukuda
 * @param	a_dataAvailable		True if data is available to read, or false if the child process has exited
 */

void RStdExecuter::readComplete(bool a_dataAvailable)
{
	/* If data is available then read it and pass it onto the client */
	if (a_dataAvailable)
	{
		struct Message *Message;

		if ((Message = GetMsg(m_port)) != nullptr)
		{
			int bytesRead = m_packet->dp_Res1;

			if (bytesRead > 0)
			{
				/* NULL terminate and print the child's output, and send it to the client for processing */
				m_buffer[bytesRead] = '\0';
				m_callback(m_buffer, bytesRead, TResult{KErrNone, 0});

				/* And read the next block of data */
				read();
			}
			else
			{
				close();
			}
		}
	}
	/* Otherwise the child process has exited, so indicate this to the client */
	else
	{
		m_callback(nullptr, 0, TResult{KErrNone, m_exitData.m_exitCode});
		close();
	}
}

#elif defined(QT_GUI_LIB)

/**
 * Process a block of data read from the Qt-specific helper.
 * This method is called by the Qt-specific helper class when data becomes available asynchronously, or when the end
 * of the file has been reached. It will simply pass the data and result to the client's callback method.
 *
 * On a successful read of data, a_buffer will point to the data read, a_size will indicate the size of the data read
 * in bytes, and a_result.m_result will be KErrNone. When the end of the file is reached, a_buffer will be nullptr,
 * a_size will be 0, and a_result.m_result will indicate whether the process exited normally or crashed. If the
 * process exited normally, a_result.m_subResult will contain the exit code of the process.
 *
 * @date	Friday 27-Mar-2026 7:01 am, Code HQ Tokyo Tsukuda
 * @param	a_buffer		Pointer to the buffer containing the data read
 * @param	a_size			The size of the data read, in bytes
 * @param	a_result		The result of the read operation
 */

void RStdExecuter::readComplete(const char *a_buffer, int a_size, const TResult &a_result)
{
	m_callback(a_buffer, a_size, a_result);
}

#else /* ! QT_GUI_LIB */

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Saturday 28-Mar-2026 7:36 am, Code HQ Tokyo Tsukuda
 * @param	Parameter		Description
 * @return	Return value
 */

DWORD bytesRead;
HANDLE hReadEvent;
OVERLAPPED overlapped;

void RStdExecuter::readComplete(int a_size) // TODO: CAW - ReadNext flag
{
	if (a_size >= 0)
	{
		m_buffer[a_size] = '\0';
		Utils::info("*** Got data from child process: %s", m_buffer);
		m_callback(m_buffer, a_size, TResult{ KErrNone, 0 });
	}
	else
	{
		memset(&overlapped, 0, sizeof(overlapped));
		overlapped.hEvent = (HANDLE) this; // hReadEvent;

		Utils::info("Restarting the fucker");
		BOOL success = ReadFileEx(m_stdOutRead, m_buffer, (STDOUT_BUFFER_SIZE - 1), &overlapped, ReadCompletionRoutine);
		Utils::info("success: %d", success);
		//read();
	}
}

#endif /* ! QT_GUI_LIB */

/**
 * Launches a command and streams its output to the client.
 * This function launches a command with dedicated stdin, stdout and stderr handles that allow the control and capture
 * of all stdio of the child process that has been launched. It will capture all output from that child process and
 * stream it back to the client that requested the launch.
 *
 * The command name passed in can be either a full path to the executable, or just the name of the executable. If just
 * the name is passed in, the system's path will be searched for the executable to be launched.
 *
 * @date	Monday 15-Feb-2021 7:19 am, Code HQ Bergmannstrasse
 * @param	a_commandName	The name of the command to be launched
 * @param	a_stackSize		The stack size to be used on the target machine (Amiga OS only)
 * @param	a_callback		The callback to be called when data is available
 * @return	KErrNone if the command was launched successfully
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrNotFound if the command executable could not be found
 * @return	KErrInUse if a command is already being executed
 * @return	KErrGeneral if any other error occurred
 */

int RStdExecuter::launchCommand(const char *a_commandName, int a_stackSize, Callback a_callback)
{

#ifdef __amigaos__

	/* Only launch the command if a command is not already running */
	if (m_stdOutRead != 0)
	{
		return KErrInUse;
	}

	ULONG stackSize = a_stackSize > 0 ? a_stackSize : DEFAULT_STACK_SIZE;
	int retVal = KErrNoMemory;

	m_callback = a_callback;

	/* Open the pipes with a buffer size of 1024 bytes. We really need less than this, but it seems to be a minimum */
	/* and specifying smaller values is ignored */
	m_stdInRead = Open("Console:", MODE_OLDFILE);
	m_stdOutWrite = Open("PIPE:RADRunner/1024", MODE_NEWFILE);
	m_stdOutRead = Open("PIPE:RADRunner/1024", MODE_OLDFILE);
	m_port = CreateMsgPort();
	m_packet = (DosPacket *) AllocDosObject(DOS_STDPKT, NULL);
	m_buffer = new char[STDOUT_BUFFER_SIZE];

	if ((m_stdInRead != 0) && (m_stdOutWrite != 0) && (m_stdOutRead != 0) &&
		(m_port != nullptr && m_packet != nullptr && m_buffer != nullptr))
	{
		m_exitData.m_exitCode = 0;
		m_exitData.m_sigBit = AllocSignal(-1);
		m_exitData.m_parentTask = FindTask(nullptr);

		int result = SystemTags(a_commandName, SYS_Input, (ULONG) m_stdInRead, SYS_Output, (ULONG) m_stdOutWrite,
			NP_ExitCode, (ULONG) ExitFunction, NP_ExitData, (ULONG) &m_exitData, SYS_Asynch, TRUE, NP_StackSize, stackSize,
			TAG_DONE);

		if (result == 0)
		{
			/* The shell was launched successfully, but the command specified by the user may not exist. In this */
			/* case, the shell will print an error and we will capture it in the same way as if the command was */
			/* executed successfully */
			retVal = KErrNone;

			/* Add the executer to the list of active executers, so that the RApplication class can call it when */
			/* data becomes available */
			m_signal = 1 << m_port->mp_SigBit;
			CWindow::GetRootWindow()->GetApplication()->AddExecuter(this);

			/* The m_stdInRead and m_stdOutWrite handles are closed by the asynchronous call to SystemTags() when */
			/* the child process exits, but we need to close the m_stdOutRead handle ourselves */
			m_stdInRead = m_stdOutWrite = 0;

			read();
		}
		else
		{
			retVal = KErrNotFound;
		}
	}

	if (retVal != KErrNone)
	{
		close();
	}

#elif defined(QT_GUI_LIB)

	(void) a_stackSize;

	m_callback = a_callback;

	int retVal = m_executer.launchCommand(a_commandName);

#elif defined(WIN32)

	(void) a_stackSize;

	int retVal = KErrGeneral;
	SECURITY_ATTRIBUTES securityAttributes;

	/* Set the bInheritHandle flag so pipe handles are inherited by the child, so that it is able to read  to and */
	/* write from them */
	securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttributes.bInheritHandle = TRUE;
	securityAttributes.lpSecurityDescriptor = NULL;

	char pipeName[MAX_PATH];
	static LONG pipeId = 0;
	sprintf(pipeName, "\\\\.\\pipe\\BrunelBuild");// . %08x. %08x",
		//GetCurrentProcessId(), InterlockedIncrement(&pipeId));

	/* Create pipes that can be used for stdin, stdout and stderr */
#if 0
	if (CreatePipe(&m_stdOutRead, &m_stdOutWrite, &securityAttributes, 0))
	{
		{
#else
	HANDLE hReadPipe = CreateNamedPipe(
		pipeName,
		PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,  // OVERLAPPED is key!
		PIPE_TYPE_BYTE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES, //1,              // Max instances
		4096,              // Out buffer size
		4096,           // In buffer size
		NMPWAIT_USE_DEFAULT_WAIT,// INFINITE,              // Timeout
		&securityAttributes);          // Security

	if (hReadPipe != INVALID_HANDLE_VALUE)
	{
		m_stdOutRead = hReadPipe;

		HANDLE hWritePipe = CreateFile(
			pipeName,
			GENERIC_WRITE,
			0,
			&securityAttributes,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,  // Write end doesn't need overlapped
			NULL);

		if (hWritePipe != INVALID_HANDLE_VALUE)
		{
			m_stdOutWrite = hWritePipe; // TODO: CHILD
#endif
			//HANDLE hReadPipeAsync;
			//DuplicateHandle(GetCurrentProcess(), m_stdOutRead, GetCurrentProcess(),
			//	&hReadPipeAsync, 0, FALSE, DUPLICATE_SAME_ACCESS);
			//CloseHandle(m_stdOutRead);
			//m_stdOutRead = hReadPipeAsync;

			//SetHandleInformation(m_stdOutWrite, HANDLE_FLAG_INHERIT, 0);
			//if (SetHandleInformation(m_stdOutRead, HANDLE_FLAG_INHERIT, 0))
			{
				if (CreatePipe(&m_stdInRead, &m_stdInWrite, &securityAttributes, 0))
				{
					if (SetHandleInformation(m_stdInRead, HANDLE_FLAG_INHERIT, 0))
					//if (SetHandleInformation(m_stdInWrite, HANDLE_FLAG_INHERIT, 0))
					{
						hReadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

						//OVERLAPPED overlapped = { 0 };
						overlapped.hEvent = (HANDLE) this; // hReadEvent;
						//overlapped.hEvent = (HANDLE) hReadEvent;

						HANDLE childProcess;

						/* Create the child process. This will read from and write to the pipes we have created, */
						/* and upon exit will close its end of the pipes, so that we can detect that it has exited */
						if ((retVal = createChildProcess(a_commandName, childProcess)) == KErrNone)
						{
							m_buffer = new char[STDOUT_BUFFER_SIZE];
							BOOL success;
							//DWORD bytesRead, exitCode;

							/* Loop around and read as much from the child's stdout as possible. When the child exits, */
							/* the pipe will be closed and ReadFile() will fail */
							//do
							{
								//success = ReadFile(m_stdOutRead, m_buffer, (STDOUT_BUFFER_SIZE - 1), NULL, &overlapped);
								success = ReadFileEx(m_stdOutRead, m_buffer, (STDOUT_BUFFER_SIZE - 1), &overlapped, ReadCompletionRoutine);
								Utils::info("success: %d, bytesRead: %d", success, bytesRead);

								//if (success)
								//{
								//	/* NULL terminate and print the child's output, and send it to the client for processing */
								//	buffer[bytesRead] = '\0';
								//	a_callback(buffer, bytesRead);
								//}
							}
							//while (success && bytesRead > 0);


							//delete [] buffer;

							//if (GetExitCodeProcess(childProcess, &exitCode))
							//{
							//	retVal.m_subResult = exitCode;
							//}

							/* And finally, close the handle to the child process now that we have its return code */
							//CloseHandle(childProcess);
						}
					}

					/* Ensure that stdin related streams are closed. The read stream may or may not already be closed, */
					/* depending on the success of prior operations */
					//CloseHandle(m_stdInWrite);
					//m_stdInWrite = nullptr;

					//if (m_stdInRead != nullptr)
					//{
					//	CloseHandle(m_stdInRead);
					//	m_stdInRead = nullptr;
					//}
				}
			}
		}

		/* Ensure that stdout related streams are closed. The write stream may or may not already be closed, */
		/* depending on the success of prior operations */
		//CloseHandle(m_stdOutRead);
		//m_stdOutRead = nullptr;

		//if (m_stdOutWrite != nullptr)
		//{
		//	CloseHandle(m_stdOutWrite);
		//	m_stdOutWrite = nullptr;
		//}
	}

#else /* ! WIN32 */

	(void) a_commandName;
	(void) a_stackSize;
	(void) a_callback;

	int retVal = KErrGeneral;

#endif /* ! WIN32 */

	return retVal;
}

#if defined(WIN32) && !defined(QT_GUI_LIB)

/**
 * Launches a child process.
 * Asynchronously launches a child process that uses the previously created pipes for stdin, stdout and stderr. When
 * this method returns, the child process will be executing and may be controlled using the given stdio handles.
 *
 * @date	Monday 15-Feb-2021 7:19 am, Code HQ Bergmannstrasse
 * @param	a_commandName	The name of the command to be launched
 * @return	KErrNone if the command was launched successfully
 * @return	KErrNotFound if the command executable could not be found
 */

int RStdExecuter::createChildProcess(const char *a_commandName, HANDLE &a_childProcess)
{
	int retVal;
	PROCESS_INFORMATION processInfo;
	STARTUPINFO startupInfo;

	ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&startupInfo, sizeof(STARTUPINFO));

	/* Pass the handles for the stdin, stdout and stderr pipes to the client process */
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.hStdError = m_stdOutWrite;
	startupInfo.hStdOutput = m_stdOutWrite;
	startupInfo.hStdInput = m_stdInRead;
	startupInfo.dwFlags |= STARTF_USESTDHANDLES;

	std::string commandName(a_commandName);

	/* Create the requested child process */
	if (CreateProcess(NULL, (char *) commandName.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo))
	{
		DWORD fuck;
		
		DWORD cunt = GetExitCodeProcess(processInfo.hProcess, &fuck);
		Utils::info("%d %x", fuck, cunt);

		retVal = KErrNone;
		a_childProcess = processInfo.hProcess;

		/* Close the handle to the primary thread, which we don't need */
		CloseHandle(processInfo.hThread);

		/* Close handles to the stdin and stdout pipes no longer needed by the child process. If they are not */
		/* explicitly closed, there is no way to recognise that the child process has ended. */
		CloseHandle(m_stdOutWrite);
		m_stdOutWrite = NULL;
		CloseHandle(m_stdInRead);
		m_stdInRead = NULL;
	}
	else
	{
		retVal = Utils::MapLastError();
	}

	return retVal;
}

#endif /* defined(WIN32) && !defined(QT_GUI_LIB) */
