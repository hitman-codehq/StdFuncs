
#include "StdFuncs.h"
#include "StdThread.h"

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 13-Nov-2026 7:55 am, Rose Park Hotel, Tokyo Minami-Funabashi TODO
 * @param	Parameter		Description
 * @return	Return value
 */

RStdThread::~RStdThread()
{
	close();
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Friday 13-Nov-2026 7:59 am, Rose Park Hotel, Tokyo Minami-Funabashi TODO
 * @param	Parameter		Description
 * @return	Return value
 */

void RStdThread::close()
{
	if (m_thread.joinable())
	{
		m_thread.join();
	}
}

/**
 * Short description.
 * Long multi line description.
 *
 * @pre		Some precondition here
 *
 * @date	Thursday 12-Nov-2026 6:27 am, Rose Park Hotel, Tokyo Minami-Funabashi TODO
 * @param	Parameter		Description
 * @return	Return value
 */

bool RStdThread::start(Callback a_callback)
{
	if (m_thread.joinable() || !a_callback)
	{
		return false;
	}

	auto finished = std::make_shared<std::atomic<bool>>(false);

#ifdef __amigaos__

#else /* ! __amigaos__ */

	try
	{
		m_thread = std::thread([callback = std::move(a_callback), finished]()
		{
			callback();
			*finished = true;
		});
	}
	catch (const std::system_error &)
	{
		return false;
	}

#endif /* ! __amigaos__ */

	m_finished = std::move(finished);

	return true;
}
