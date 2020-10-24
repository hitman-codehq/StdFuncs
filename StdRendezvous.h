
#ifndef STDRENDEZVOUS_H
#define STDRENDEZVOUS_H

#ifdef QT_GUI_LIB

#include "Qt/QtLocalSocket.h"

#endif /* QT_GUI_LIB */

/* Forward declaration to reduce the # of includes required */

class RApplication;

/**
 * Mixin class for indicating when a rendezvous has been received.
 * Client code that is interested in receiving callbacks when a rendezvous has been received
 * from another process should derive from this class and implement the MesageReceived()
 * method.  It should then register an instance of the class with the RRendezvous class
 * using RRendezvous::SetObserver().
 */

class MRendezvousObserver
{
public:

	virtual void RendezvousReceived(const unsigned char *a_pcucData, TInt a_iDataSize) = 0;
};

/**
 * A class that allows a program to rendezvous with another instance of itself.
 * This class will allow a program to make a rendezvous either with another instance of itself,
 * or with a completely different program.  This is useful in situations such as when it is desired
 * to have only a single instance of a program program running at any given time.  If a second
 * instance is started, it will not open its window, but will instead rendezvous with the already
 * running instance, telling it to bring itself to the front.
 *
 * An example of this behaviour is executing a text editor with a filename argument on the
 * command line.  This causes an already running instance of the text editor to load the given
 * filename and to bring itself to the front.  Meanwhile, the second instance of the text editor
 * will quietly exit, making it seem to the user that the two editors have "merged" into one.  As
 * well as for this purpose, the class can be used any time that one program wants to send a
 * message to another application that is running in another address space (as long as the target
 * application also has an instance of the RRendezvous class embedded in it).
 */

#ifdef QT_GUI_LIB

class RRendezvous : public MLocalSocketObserver

#else /* ! QT_GUI_LIB */

class RRendezvous

#endif /* ! QT_GUI_LIB */

{
private:

	char				*m_pcName;		/**< Name of this rendezvous port */
	MRendezvousObserver	*m_poObserver;	/**< Pointer to client to notify when rendezvous received */

#ifdef __amigaos4__

	struct MsgPort		*m_poMsgPort;	/**< Port used by the server for rendezvous */
	TBool				m_bPortAdded;	/**< ETrue if the port was added to the public message port list */

#elif defined(QT_GUI_LIB)

	RLocalSocket		m_oLocalSocket;	/**< Local socket for use in communicating between processes */

#endif /* QT_GUI_LIB */

public:

	RRendezvous();

	~RRendezvous();

	TInt open(RApplication *a_poApplication, const char *a_pccName);

	void close();

#ifdef __amigaos4__

	struct MsgPort *GetMessagePort();

	ULONG GetSignal();

#endif /* __amigaos4__ */

	TBool Rendezvous(const unsigned char *a_pcucData, TInt a_iDataSize);

	void MessageReceived(const unsigned char *a_pcucData, TInt a_iDataSize);

	void SetObserver(MRendezvousObserver *a_poObserver);
};

extern RRendezvous g_oRendezvous;

#endif /* ! STDRENDEZVOUS_H */
