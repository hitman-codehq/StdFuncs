
#include "StdFuncs.h"
#include "StdApplication.h"
#include "StdRendezvous.h"
#include <string.h>

RRendezvous g_oRendezvous;	/* Class to allow communication between instances of programs */

/**
 * RRendezvous constructor.
 * Initialises the members of the class to their default values.
 *
 * @date	Sunday 03-May-2015 8:53 am, Code HQ Ehinger Tor
 */

RRendezvous::RRendezvous()
{
	m_pcName = NULL;
	m_poObserver = NULL;

#ifdef __amigaos4__

	m_poMsgPort = NULL;
	m_bPortAdded = EFalse;

#endif /*__amigaos4__ */

}

/**
 * RRendezvous destructor.
 * Frees any resources associated with the class.
 *
 * @date	Saturday 09-May-2015 7:26 am, Code HQ Ehinger Tor
 */

RRendezvous::~RRendezvous()
{
	Close();
}

/**
 * Allocates system resources required to make a rendezvous.
 * This method will allocate whatever underlying operating system resources are required in order
 * to make a rendezvous.  The process that calls this method first will automatically become the
 * server, and any subsequent processes that call it will become clients.  Clients can then
 * rendezvous with the server, but not the other way around.
 *
 * @date	Friday 01-May-2015 1:39 pm, Code HQ Ehinger Tor
 * @param	a_pccName		The name to be used for the rendezvous port
 * @return	KErrNone if successful
 * @return	KErrNoMemory if not enough memory was available
 * @return	KErrGeneral if some other unspecified error occurred
 */

TInt RRendezvous::Open(const char *a_pccName)
{
	TInt RetVal;

	/* Assume success */

	RetVal = KErrNone;

	/* Save the name of the port in persistent memory */

	if ((m_pcName = new char[strlen(a_pccName) + 1]) != NULL)
	{
		strcpy(m_pcName, a_pccName);

#ifdef __amigaos4__

		struct MsgPort *MsgPort;

		/* Only try to create a named message port if it does not already exist.  If the port already */
		/* exists then the server is running, so we will be a client */

		if ((MsgPort = IExec->FindPort(m_pcName)) == NULL)
		{
			if ((m_poMsgPort = IExec->CreateMsgPort()) != NULL)
			{
				/* Initialise the message port so that it has a name, a priority (for speedy lookups) and */
				/* will signal our task when a message is received.  Add it to Amiga OS's public port list */

				m_poMsgPort->mp_Node.ln_Name = m_pcName;
				m_poMsgPort->mp_Node.ln_Pri = 1;
				m_poMsgPort->mp_SigTask = IExec->FindTask(NULL);
				IExec->AddPort(m_poMsgPort);
				m_bPortAdded = ETrue;
			}
			else
			{
				RetVal = KErrGeneral;

				delete [] m_pcName;
				m_pcName = NULL;
			}
		}

#endif /* __amigaos4__ */

	}
	else
	{
		RetVal = KErrNoMemory;
	}

	return(RetVal);
}

/**
 * Frees any resources associated with the class.
 * This method performs the same functions as the destructor, but allows the user to call it when
 * manual deinitialisation of the class is required.  After completion, the class instance can be
 * reused by calling RRendezvous::Open() again.
 *
 * @date	Friday 01-May-2015 1:39 pm, Code HQ Ehinger Tor
 */

void RRendezvous::Close()
{
	delete [] m_pcName;
	m_pcName = NULL;

#ifdef __amigaos4__

	/* Remove the message port from the public list and free it, if required.  Reset its variables so */
	/* that the class can be reused again */

	if (m_poMsgPort)
	{
		if (m_bPortAdded)
		{
			m_bPortAdded = EFalse;
			IExec->RemPort(m_poMsgPort);
		}

		IExec->DeleteMsgPort(m_poMsgPort);
		m_poMsgPort = NULL;
	}

#endif /* __amigaos4__ */

}

#ifdef __amigaos4__

/**
 * Returns the message port on which to listen for a rendezvous.
 * This is an Amiga OS specific function that will return the message port on which the server should wait
 * to receive rendezvous from clients.
 *
 * @date	Saturday 09-May-2015 11:27 am, Code HQ Ehinger Tor
 * @return	The message port on which to wait for a rendezvous
 */

struct MsgPort *RRendezvous::GetMessagePort()
{
	return(m_poMsgPort);
}

/**
 * Returns the signal for which to listen for a rendezvous.
 * This is an Amiga OS specific function that will return the signal for which the server should wait
 * to receive rendezvous from clients.
 *
 * @date	Saturday 09-May-2015 11:09 am, Code HQ Ehinger Tor
 * @return	The signal on which to wait for a rendezvous
 */

ULONG RRendezvous::GetSignal()
{
	return((m_poMsgPort) ? (1 << m_poMsgPort->mp_SigBit) : 0);
}

#endif /* __amigaos4__ */

/**
 * Rendezvous with a program of a given name.
 * This function will rendezvous with particularly named program.  Upon reception of the rendezvous,
 * the target program will wake up and process that rendezvous.  Data can be sent to the target
 * program and this will be mapped or copied into the target program's address space so that it
 * can be accessed.
 *
 * @date	Sunday 09-Feb-2014 10:44 am, on board RE 57179 train to Munich Deutsches Museum
 * @param	a_poApplication	Pointer to the parent application under which the program is running
 * @param	a_pcucData		Pointer to the data to be sent to the program
 * @param	a_iDataSize		Size of the data to be sent, in bytes
 * @return	ETrue if the message was sent successfully, else EFalse
 */

TBool RRendezvous::Rendezvous(RApplication *a_poApplication, const unsigned char *a_pcucData, TInt a_iDataSize)
{
	TBool RetVal;

	/* Assume failure */

	RetVal = EFalse;

#ifdef __amigaos4__

	char *Buffer, *Data;
	struct Message *Message;
	struct MsgPort *MsgPort;

	(void) a_poApplication;

	/* Only send the message if we are the client, or we will end up sending the message to ourselves. */
	/* If we are the server then m_poMsgPort will be non NULL */

	if (m_poMsgPort == NULL)
	{
		/* Allocate a buffer large enough to hold the Message structure and the data to be sent */

		if ((Buffer = new char[sizeof(struct Message) + a_iDataSize]) != NULL)
		{
			Message = (struct Message *) Buffer;
			Message->mn_Length = (sizeof(struct Message) + a_iDataSize);

			/* If any data was specified then append it to the message structure */

			if (a_iDataSize > 0)
			{
				Data = (char *) (Message + 1);
				memcpy(Data, a_pcucData, a_iDataSize);
			}

			/* Create a reply port that can be used by the server to indicate that a message has been */
			/* received and processed */

			if ((Message->mn_ReplyPort = IExec->CreateMsgPort()) != NULL)
			{
				/* The FindPort() function must be protected by a Forbid()/Permit() pair.  However, we must only */
				/* send the message with interrupts disabled or the server will never be able to process the */
				/* received message.  So send the message and enable interrupts before waiting for the response */

				IExec->Forbid();

				if ((MsgPort = IExec->FindPort(m_pcName)) != NULL)
				{
					RetVal = ETrue;

					IExec->PutMsg(MsgPort, Message);
				}

				IExec->Permit();

				/* Listen for a response, but only if the message was successfully sent */

				if (RetVal)
				{
					IExec->Wait(1 << Message->mn_ReplyPort->mp_SigBit);
				}

				/* Delete the reply port, now that we are finished with it */

				IExec->DeleteMsgPort(Message->mn_ReplyPort);
			}

			delete [] Buffer;
		}
	}

#elif defined(QT_GUI_LIB)

	/* Open a local socket with which to read and write messages and let it know to call us when a */
	/* message is received */

	if (m_oLocalSocket.Open(m_pcName, a_poApplication) == KErrNone)
	{
		m_oLocalSocket.SetObserver(this);

		/* If the socket is the client then send a message to rendezvous with the server */

		if (!m_oLocalSocket.IsServer())
		{
			if (m_oLocalSocket.Write(a_pcucData, a_iDataSize) == KErrNone)
			{
				RetVal = ETrue;
			}
		}
	}

#else /* ! QT_GUI_LIB */

	COPYDATASTRUCT CopyData;
	HWND Window;

	(void) a_poApplication;

	/* Find a handle to the main window of the target program, based on the name passed in */

	if ((Window = FindWindow(m_pcName, m_pcName)) != NULL)
	{
		/* Indicate success.  Unfortunately SendMessage() does not return a value to indicate that */
		/* is could or could not send the message, so we have to assume that it was successful */

		RetVal = ETrue;

#ifdef _WIN32_WINNT_WIN2K

		/* Normally Windows does not allow processes to bring themselves to the front unless they are */
		/* processing an input related event.  This interferes with our ability to bring ourselves to */
		/* the front when a second instance of the application is launched, so give the target process */
		/* permission to bring itself to the front.  Note that this will only work if the process calling */
		/* this function currently has the "set foreground" ability */

		DWORD ProcessID;

		GetWindowThreadProcessId(Window, &ProcessID);
		DEBUGCHECK((AllowSetForegroundWindow(ProcessID) != FALSE), "RRendezvous::Rendezvous() => Unable to assign set foreground ability to target");

#endif /* _WIN32_WINNT_WIN2K */

		/* Fill out the structure required by the WM_COPYDATA message and send the message, */
		/* waiting for it to be processed before returning */

		CopyData.dwData = 0;
		CopyData.cbData = a_iDataSize;
		CopyData.lpData = (void *) a_pcucData;

		SendMessage(Window, WM_COPYDATA, (WPARAM) NULL, (LPARAM) &CopyData);
	}

#endif /* ! QT_GUI_LIB */

	return(RetVal);
}

/**
 * Called by the underlying operating system when a rendezvous is received.
 * This function is called by the operating system when a rendezvous is received from another
 * program.  If a callback has been registered by a client then this callback will be called.
 * Otherwise, the function will return without doing anything.
 *
 * @date	Sunday 09-Feb-2014 11:10 am, on board RE 57179 train to Munich Deutsches Museum
 * @param	a_pcucData	Pointer to the data sent by the sending program
 * @param	a_iDataSize	Size of the data sent, in bytes
 */

void RRendezvous::MessageReceived(const unsigned char *a_pcucData, TInt a_iDataSize)
{
	if (m_poObserver)
	{
		m_poObserver->RendezvousReceived(a_pcucData, a_iDataSize);
	}
}

/**
 * Registers a function to be called when a rendezvous is received.
 * If a client program is interested in receiving a notification when a rendezvous is received
 * then it should call this function at startup, passing in a pointer to the function to be
 * called when the rendezvous is received.
 *
 * @date	Sunday 09-Feb-2014 10:16 am, on board RE 57179 train to Munich Deutsches Museum
 * @param	a_poObserver	Pointer to client function to be called when rendezvous is received
 */

void RRendezvous::SetObserver(MRendezvousObserver *a_poObserver)
{
	m_poObserver = a_poObserver;
}
