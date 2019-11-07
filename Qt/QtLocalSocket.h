
#ifndef QTLOCALSOCKET_H
#define QTLOCALSOCKET_H

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

/**
 * Class for making local socket connections for IPC use.
 * This class allows applications to connect to one another in order to send and receive arbitrary
 * data.  This data can be anything but will usually be used for some sort of IPC using a custom
 * protocol.  Sockets are identified by a unique name.  Whichever instance of the application first
 * opens a socket of a given unique name will become the server.  Future instances of the application
 * that open the same socket will automatically connect to the server and will become clients.
 */

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

	void Close();

	void SetObserver(MLocalSocketObserver *a_poObserver);

	int Write(const unsigned char *a_pcucData, int a_iDataSize);

public slots:

	void Connected();
};

#endif /* ! QTLOCALSOCKET_H */
