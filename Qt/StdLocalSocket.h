
#ifndef QTSTDLOCALSOCKET_H
#define QTSTDLOCALSOCKET_H

#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

/* Forward declaration to reduce the # of includes required */

class RApplication;

/**
 * Mixin class for signalling when a message has been received.
 * Client code that is interested in receiving callbacks when a message has been received
 * from another process should derive from this class and implement the MessageReceived()
 * method.  It should then register an instance of the class with the RLocalSocket class
 * using RLocalSocket::SetObserver().
 */

class MLocalSocketObserver
{
public:

	virtual void MessageReceived(const unsigned char *a_pcucData, int a_iDataSize) = 0;
};

class RLocalSocket : public QObject
{
	Q_OBJECT;

private:

	bool					m_bIsServer;		/**< true if the socket is a server socket */
	MLocalSocketObserver	*m_poObserver;		/**< Pointer to client to notify when a message is received */
	QLocalServer			*m_poLocalServer;	/**< Qt class used for listening for local socket connections */
	QLocalSocket			m_oLocalSocket;		/**< Qt class used for sending messages to the server */

public:

	RLocalSocket();

	~RLocalSocket();

	bool IsServer();

	int Open(const char *a_pccName, RApplication *a_poApplication);

	void SetObserver(MLocalSocketObserver *a_poObserver);

	int Write(const unsigned char *a_pcucData, int a_iDataSize);

public slots:

	void Connected();
};

#endif /* ! QTSTDLOCALSOCKET_H */
