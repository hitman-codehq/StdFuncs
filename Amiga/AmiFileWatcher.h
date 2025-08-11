
#ifndef AMIFILEWATCHER_H
#define AMIFILEWATCHER_H

#include "../Dir.h"

/* Forward declarations to reduce the # of includes required */

class CStdFileWatcher;
class RApplication;

/**
 * Amiga helper class to listen for file change events.
 * This class is internal to the framework and will listen for file change events coming from an Amiga file system
 * watcher. It will then pass these events onto the generic framework file watcher that owns an instance of this
 * class.
 */

class RAmiFileWatcher
{
	ULONG				m_signal;			/**< The signal used to wait for file change events */
	CStdFileWatcher		&m_parentWatcher;	/**< Reference to framework watcher that uses this Amiga watcher */
	MsgPort				*m_port;			/**< The message port used to receive for file change events */
	NotifyRequest		*m_request;			/**< The request used to listen for file change events */
	RDir				m_dir;				/**< List of files in the directory being watched */

private:

	void changed();

	const TEntry *findEntry(const TEntryArray &a_entries, const char *a_name);

public:

	RAmiFileWatcher(CStdFileWatcher &a_parentWatcher) : m_parentWatcher(a_parentWatcher) { };

	struct MsgPort *getMessagePort()
	{
		return m_port;
	}

	ULONG getSignal()
	{
		return m_signal;
	}

	void pauseWatching();

	void resumeWatching();

	bool startWatching(const std::string &a_directoryName);

	void stopWatching();

	friend class RApplication;
};

#endif /* AMIFILEWATCHER_H */
