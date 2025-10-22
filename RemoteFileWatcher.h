
#ifndef REMOTEFILEWATCHER_H
#define REMOTEFILEWATCHER_H

#include "FileWatcher.h"
#include "StdSocket.h"

/* Forward declaration to reduce the # of includes required */
class RRemoteFactory;

/**
 * A class for remotely watching for changes in directories.
 * Using an instance of this class, client software can be notified if changes are made to files within a specified
 * directory. A client notification callback will be called if a file is added to, deleted from or changed inside
 * the directory.
 */

class RRemoteFileWatcher : public RFileWatcherObject
{
	RRemoteFactory	*m_remoteFactory;	/**< Pointer to the remote factory that owns this instance */
	RSocket			*m_socket;			/**< Socket for communicating with remote RADRunner */

public:

	~RRemoteFileWatcher()
	{
	}

	void pauseWatching();

	void resumeWatching();

	bool startWatching(const std::string &a_directoryName, const std::string *a_fileName, Callback a_callback);

	void stopWatching();

	void setFactory(RRemoteFactory *a_remoteFactory, RSocket *a_socket)
	{
		m_remoteFactory = a_remoteFactory;
		m_socket = a_socket;
	}
};

#endif /* ! REMOTEFILEWATCHER_H */
