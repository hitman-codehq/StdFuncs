
#ifndef STDRENDEZVOUS_H
#define STDRENDEZVOUS_H

/**
 * Mixin class for indicating when a rendezvous has been received.
 * Client code that is interested in receiving callbacks when a rendezvous has been received
 * from another program should derive from this class and implement the RendezvousReceived()
 * method.  I should then register an instance of the class with the RRendezvous class, which
 * will then call the overloaded method when a rendezvous is received.
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
 * An example of this behaviour is executing a text editor with a file name argument on the
 * command line.  This causes an already running instance of the text editor to load the given
 * file name and to bring itself to the front.  Meanwhile, the second instance of the text editor
 * will quietly exit, making it seem to the user that the two editors have "merged" into one.  As
 * well as for this purpose, the class can be used any time that one program wants to send a
 * message to another application that is running in another address space (as long as the target
 * application also has an instance of the RRendezvous class embedded in it).
 */

class RRendezvous
{
	const char			*m_pccName;		/**< Name of this rendezvous port */
	MRendezvousObserver	*m_poObserver;	/**< Pointer to client to notify when rendezvous received */

public:

	TBool Rendezvous(const char *a_pccName, const unsigned char *a_pcucData, TInt a_iDataSize);

	void RendezvousReceived(const unsigned char *a_pcucData, TInt a_iDataSize);

	void SetObserver(MRendezvousObserver *a_poObserver);
};

extern RRendezvous g_oRendezvous;

#endif /* ! STDRENDEZVOUS_H */
