
#ifndef REMOTE_COMMANDS_H
#define REMOTE_COMMANDS_H

/** @file */

#include <StdSocket.h>

#ifdef __amigaos__

#define SWAP(number)
#define SWAP64(number)

#else /* ! __amigaos__ */

#define SWAP(number) Utils::swap32(number)
#define SWAP64(number) Utils::swap64(number)

#endif /* ! __amigaos__ */

/* The protocol version supported by the current build */
#define PROTOCOL_MAJOR 0
#define PROTOCOL_MINOR 2

/* The size of the buffer used for capturing stdout */
#define STDOUT_BUFFER_SIZE 1024

/* The size of the stack used on Amiga OS if no size is explicitly specified */
#define DEFAULT_STACK_SIZE 20480

#define READ_INT(Dest, Source) \
	Dest = (*Source << 24) | \
	(*(Source + 1) << 16) | \
	(*(Source + 2) << 8) | \
	*(Source + 3)

#define READ_INT_64(Dest, Source) \
	Dest = ((TInt64) *Source << 56) | \
	((TInt64) *(Source + 1) << 48) | \
	((TInt64) *(Source + 2) << 40) | \
	((TInt64) *(Source + 3) << 32) | \
	((TInt64) *(Source + 4) << 24) | \
	((TInt64) *(Source + 5) << 16) | \
	((TInt64) *(Source + 6) << 8) | \
	(TInt64) *(Source + 7)

#define WRITE_INT(Dest, Value) \
	*Dest = (Value >> 24) & 0xff; \
	*(Dest + 1) = (Value >> 16) & 0xff; \
	*(Dest + 2) = (Value >> 8) & 0xff; \
	*(Dest + 3) = Value & 0xff

#define WRITE_INT_64(Dest, Value) \
	*Dest = (Value >> 56) & 0xff; \
	*(Dest + 1) = (Value >> 48) & 0xff; \
	*(Dest + 2) = (Value >> 40) & 0xff; \
	*(Dest + 3) = (Value >> 32) & 0xff; \
	*(Dest + 4) = (Value >> 24) & 0xff; \
	*(Dest + 5) = (Value >> 16) & 0xff; \
	*(Dest + 6) = (Value >> 8) & 0xff; \
	*(Dest + 7) = Value & 0xff

/**
 * The commands supported by the program.
 * Each of these commands is implemented by a matching CHandler derived class.
 */

enum TCommands
{
	/* EVersion is first so that other commands can be added without breaking the version check */
	EVersion,
	EDelete,
	EDir,
	EExecute,
	EFileInfo,
	EGet,
	ERename,
	ESend,
	EShutdown
};

/**
 * Basic command structure.
 * A structure to be used for transfer between the local client and remote server, in order to specifiy a
 * command to be executed.  The structure uses fixed sized integers, for compatibility across different CPU
 * architectures.  It is the responsibility of the code using this structure to perform byte order swapping as
 * appropriate, and to send or receive the payload.  Only the size of the payload is sent with this structure.
 */

struct SCommand
{
public:

	uint32_t	m_command;	/**< Command to be performed by the server */
	uint32_t	m_size;		/**< Size in bytes of payload after structure */
};

/**
 * Structure containing information about the application being executed.
 * Contains the file's name and the stack size requested by the client, if any. If this is 0, the default size for
 * the target system will be used. This is only used by the Amiga OS versions of the library, as stack handling is
 * automatic on other platforms.
 */

struct SExecuteInfo
{
	int32_t			m_stackSize;		/**< The stack size to be used on the target machine */
	char			m_fileName[1];		/**< The name of the file to be executed */
};

/**
 * Structure containing information about the file being transferred.
 * Contains information such as a file's name and its timestamp, to be transferred as the header of any file
 * that is tranferred between hosts.  Instances of this structure are dynamically allocated, with their size
 * depending on the size of the file's name.
 */

struct SFileInfo
{
	TInt64			m_microseconds;	/**< Timestamp of the file, in microseconds since 01.01.01 */
	unsigned int	m_isDir;		/**< True if the file system object is a directory */
	unsigned int	m_isLink;		/**< True if the file system object is a link */
	TInt64			m_size;			/**< File size in bytes */
	char			m_fileName[1];	/**< The file's name, without a path component */
};

/**
 * Class containing the response of a command execution.
 * A class used for returning results from the execution of a command.  The class uses fixed sized integers, for
 * for compatibility across different CPU architectures.  It is the responsibility of the code using this class
 * to perform byte order swapping as appropriate, and to send or receive the payload.  Only the size of the payload
 * is sent with this class.
 */

struct TResponse
{
	int32_t		m_result;			/**< The result of attempting to execute the command */
	int32_t		m_subResult;		/**< The secondary result, valid only if m_result == KErrNone.  Its
										 value and meaning is specific to the command that returns it */
	uint32_t	m_size;				/**< Size in bytes of payload after structure */

	TResponse()
	{
		m_result = m_subResult = KErrNone;
		m_size = 0;
	}

	TResponse(int32_t a_result, int32_t a_subResult) : m_result(a_result), m_subResult(a_subResult), m_size(0) { }
};

/**
 * Class for returning more than one return value from a method or function.
 * Instead of returning an int containing one of the StdFuncs KErr values, code can return an instance of this
 * structure with the return value in m_result, and a secondary return value in m_subResult.
 */

class TResult
{
public:

	int32_t		m_result;			/**< The result of attempting to execute the command */
	int32_t		m_subResult;		/**< The secondary result, valid only if m_result == KErrNone.  Its
										 value and meaning is specific to the method that returns it */

	TResult()
	{
		m_result = m_subResult = KErrNone;
	}

	TResult(int32_t a_result, int32_t a_subResult) : m_result(a_result), m_subResult(a_subResult) { }
};

/**
 * The base class for all command handlers.
 * All command handlers derive from, and implement, this interface, allowing them to be used without
 * concern for their concrete type or implementation.
 */

class CHandler
{
protected:

	RSocket			*m_socket;			/**< Socket on which to process the command */
	SCommand		m_command;			/**< Command to be sent to the remote server */
	TResponse		m_response;			/**< Response from the remote server */
	unsigned char	*m_payload;			/**< Buffer containing packet's payload, if any (server) */
	unsigned char	*m_responsePayload;	/**< Buffer containing response's payload, if any (client) */

protected:

	int getFileInformation(const char *a_fileName, SFileInfo *&a_fileInfo);

	int readFile(const char *a_fileName);

	void readPayload();

	void readResponse();

	TResult sendCommand();

	int sendFile(const char *a_fileName);

	void setFileInformation(const SFileInfo &a_fileInfo);

public:

	/** Constructor to be used when creating client instances */
	CHandler(RSocket *a_socket, uint32_t a_command) : m_socket(a_socket)
	{
		m_command.m_command = a_command;
	}

	/** Constructor to be used when creating server instances */
	CHandler(RSocket *a_socket, const SCommand &a_command) : m_socket(a_socket), m_command(a_command) { }

	virtual ~CHandler()
	{
		delete [] m_payload;
		delete [] m_responsePayload;
	}

	/** Method for executing the handler on the server */
	virtual void execute() = 0;

	/** Method for executing the handler on the client */
	virtual TResult sendRequest() = 0;

	const TResponse *getResponse() { return &m_response; }

	unsigned char *getResponsePayload() { return m_responsePayload; }
};

/**
 * Command for deleting a file from the remote host.
 * Given the name of a remote file, this command requests the remote server to delete that file.
 */

class CDelete : public CHandler
{
	const char	*m_fileName;	/**< The name of the file to be deleted */

public:

	/** Constructor to be used when creating client instances */
	CDelete(RSocket *a_socket, const char *a_fileName) : CHandler(a_socket, EDelete), m_fileName(a_fileName) { }

	/** Constructor to be used when creating server instances */
	CDelete(RSocket *a_socket, const SCommand &a_command) : CHandler(a_socket, a_command) { }

	void execute() override;

	TResult sendRequest() override;
};

/**
 * Command for listing directory contents on the remote host.
 * Given the name of a directory to be listed, this command requests a listing of that directory from the remote host.
 */

class CDir : public CHandler
{
	const char *m_directoryName;	/**< The name of the directory to be listed */

public:

	/** Constructor to be used when creating client instances */
	CDir(RSocket* a_socket, const char *a_directoryName) : CHandler(a_socket, EDir), m_directoryName(a_directoryName) { }

	/** Constructor to be used when creating server instances */
	CDir(RSocket* a_socket, const SCommand &a_command) : CHandler(a_socket, a_command) { }

	void execute() override;

	TResult sendRequest() override;
};

/**
 * Command for remotely executing an executable or script file.
 * Given the name of a file to be executed, this command sends that name to the remote host, where it is
 * executed.
 */

class CExecute : public CHandler
{
	const char	*m_fileName;	/**< The name of the file to be executed */
	int			m_stackSize;	/**< The stack size to be used on the target machine */

#ifdef WIN32

	HANDLE m_stdInRead;			/**< Handle for reading from stdin (child) */
	HANDLE m_stdInWrite;		/**< Handle for writing to stdin (parent) */
	HANDLE m_stdOutRead;		/**< Handle for reading from stdout (parent) */
	HANDLE m_stdOutWrite;		/**< Handle for writing to std out (child) */

#endif /* WIN32 */

public:

	/** Constructor to be used when creating client instances */
	CExecute(RSocket *a_socket, const char *a_fileName, int a_stackSize) : CHandler(a_socket, EExecute),
		m_fileName(a_fileName), m_stackSize(a_stackSize) { }

	/** Constructor to be used when creating server instances */
	CExecute(RSocket *a_socket, const SCommand &a_command) : CHandler(a_socket, a_command) { }

	void execute() override;

	TResult sendRequest() override;

#ifdef WIN32

	int createChildProcess(char *a_commandName, HANDLE &a_childProcess);

#endif /* WIN32 */

	TResult launchCommand(char *a_commandName, int a_stackSize);
};

/**
 * Command for querying file information on the remote host.
 * Given the name of a remote file, this command requests information about the attributes of that file, and returns
 * it in a @link SFileInfo @endlink structure.
 */

class CFileInfo : public CHandler
{
	const char	*m_fileName;	/**< The name of the file for which to request information */

public:

	/** Constructor to be used when creating client instances */
	CFileInfo(RSocket *a_socket, const char *a_fileName) : CHandler(a_socket, EFileInfo), m_fileName(a_fileName) { }

	/** Constructor to be used when creating server instances */
	CFileInfo(RSocket* a_socket, const SCommand &a_command) : CHandler(a_socket, a_command) { }

	void execute() override;

	TResult sendRequest() override;
};

/**
 * Command for transferring a file from the remote host.
 * Given the name of a file to be fetched, this command requests that file from the remote host and transfers
 * it to the local host.
 */

class CGet : public CHandler
{
	const char	*m_fileName;	/**< The name of the file to be fetched */

public:

	/** Constructor to be used when creating client instances */
	CGet(RSocket *a_socket, const char *a_fileName) : CHandler(a_socket, EGet), m_fileName(a_fileName) { }

	/** Constructor to be used when creating server instances */
	CGet(RSocket *a_socket, const SCommand &a_command) : CHandler(a_socket, a_command) { }

	void execute() override;

	TResult sendRequest() override;
};

/**
 * Command for renaming a file on the remote host.
 * Given the old and new names of a remote file, this command requests the remote server to rename that file.
 */

class CRename : public CHandler
{
	const char	*m_oldName;	/**< The current name of the file to be renamed */
	const char	*m_newName;	/**< The new name to which to rename the file */

public:

	/** Constructor to be used when creating client instances */
	CRename(RSocket *a_socket, const char *a_oldName, const char *a_newName) : CHandler(a_socket, ERename),
		m_oldName(a_oldName), m_newName(a_newName) { }

	/** Constructor to be used when creating server instances */
	CRename(RSocket *a_socket, const SCommand &a_command) : CHandler(a_socket, a_command) { }

	void execute() override;

	TResult sendRequest() override;
};

/**
 * Command for transferring a file to the remote host.
 * Given the name of a file to be sent, this command reads that file from the local host and transfers
 * it to the remote host.
 */

class CSend : public CHandler
{
	const char	*m_fileName;	/**< The name of the file to be sent */

public:

	/** Constructor to be used when creating client instances */
	CSend(RSocket *a_socket, const char *a_fileName) : CHandler(a_socket, ESend), m_fileName(a_fileName) { }

	/** Constructor to be used when creating server instances */
	CSend(RSocket *a_socket, const SCommand &a_command) : CHandler(a_socket, a_command) { }

	void execute() override;

	TResult sendRequest() override;
};

/**
 * Command for shutting down the remote server.
 * Sends a command to the remote server to tell it to shut itself down.
 */

class CShutdown : public CHandler
{
public:

	/** Constructor to be used when creating client instances */
	CShutdown(RSocket *a_socket) : CHandler(a_socket, EShutdown) { }

	/** Empty implementation of unused server side method */
	void execute() override { }

	TResult sendRequest() override;
};

/**
 * Command for requesting the supported protocol version.
 * Sends a command to the remote server to request its supported protocol version.
 */

class CVersion : public CHandler
{
public:

	/** Constructor to be used when creating client instances */
	CVersion(RSocket *a_socket) : CHandler(a_socket, EVersion) { }

	/** Constructor to be used when creating server instances */
	CVersion(RSocket *a_socket, const SCommand &a_command) : CHandler(a_socket, a_command) { }

	void execute() override;

	TResult sendRequest() override;
};

extern const char *g_commandNames[];

#define SIGNATURE_SIZE 4
extern const char g_signature[];

#endif /* ! REMOTE_COMMANDS_H */
