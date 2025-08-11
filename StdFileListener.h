
#ifndef STDFILELISTENER_H
#define STDFILELISTENER_H

#include <Qt/QtFileListener.h>
#include <functional>

class CStdFileListener
{
public:

	using Callback = std::function<void(const std::string&)>;

	void listen(Callback callback);

private:

	Callback m_callback;
	CQtFileListener	m_listener; // TODO: CAW - Naming and ordering
};

#endif /* ! STDFILELISTENER_H */
