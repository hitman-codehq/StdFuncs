
#include "StdFuncs.h"
#include "StdApplication.h"
#include "StdRendezvous.h"

RRendezvous g_oRendezvous;	/* Class to allow communication between instances of programs */

/**
 * Rendezvous with a program of a given name.
 * This function will rendezvous with particularly named program.  Upon reception of the rendezvous,
 * the target program will wake up and process that rendezvous.  Data can be sent to the target
 * program and this will be mapped or copied into the target program's address space so that it
 * can be accessed.
 *
 * @pre		a_pccName must be non NULL
 * @date	Sunday 09-Feb-2014 10:44 am, on board RE 57179 train to Munich Deutsches Museum
 * @param	a_poApplication	Pointer to the parent application under which the program is running
 * @param	a_pccName		Name that identifies the target program to which to send the message
 * @param	a_pcucData		Pointer to the data to be sent to the program
 * @param	a_iDataSize		Size of the data to be sent, in bytes
 * @return	ETrue if the message was sent successfully, else EFalse
 */

TBool RRendezvous::Rendezvous(RApplication *a_poApplication, const char *a_pccName, const unsigned char *a_pcucData, TInt a_iDataSize)
{
	TBool RetVal;

	ASSERTM((a_pccName != NULL), "RRendezvous::Rendezvous() => Name of target program must not be NULL");

	/* Assume failure */

	RetVal = EFalse;

#ifdef __amigaos4__

	// TODO: CAW - Implement and remove warning prevention casts
	(void) a_pccName;
	(void) a_pcucData;
	(void) a_iDataSize;

#elif defined(QT_GUI_LIB)

	/* Open a local socket with which to read and write messages and let it know to call us when a */
	/* message is received */

	if (m_oLocalSocket.Open(a_pccName, a_poApplication) == KErrNone)
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

	/* Find a handle to the main window of the target program, based on the name passed in */

	if ((Window = FindWindow(a_pccName, a_pccName)) != NULL)
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
