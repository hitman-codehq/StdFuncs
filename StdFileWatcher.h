
#ifndef STDFILEWATCHER_H
#define STDFILEWATCHER_H

#ifdef QT_GUI_LIB

#include <Qt/QtFileWatcher.h>

#endif /* QT_GUI_LIB */

#include <functional>

class CStdFileWatcher
{
	using Callback = std::function<void(const std::string&)>;

	Callback m_callback;

#ifdef QT_GUI_LIB

	CQtFileWatcher	m_watcher;	/**< Helper class for file system monitoring */

#endif /* QT_GUI_LIB */

private:

	void changed(const std::string &a_directoryName);

public:

#ifdef QT_GUI_LIB

	CStdFileWatcher() : m_watcher(*this) { }

#endif /* QT_GUI_LIB */

	void watch(const std::string &a_directoryName, Callback callback);

	friend class CQtFileWatcher;
};

#endif /* ! STDFILEWATCHER_H */
