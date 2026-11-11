
#ifndef STDTHREAD_H
#define STDTHREAD_H

#include <atomic>
#include <functional>
#include <memory>
#include <system_error>
#include <thread>
#include <utility>

/**
 * A class for executing arbitrary code in a thread.
 * Using this class, it is possible to create a new thread and execute an arbitrary piece of code on that thread. It
 * can be used in a "fire and forget" manner, or the calling code can wait until that code has completed.
 */

class RStdThread
{
//protected:

	std::thread							m_thread;		/**< The underlying thread */
	std::shared_ptr<std::atomic<bool>>	m_finished;		/**< Set by the thread when its callback returns */

public:

	/**< Type definition for client callback to invoke when thread has started */
	using Callback = std::function<void()>;

	RStdThread() = default;

	~RStdThread();
	/*{
		close();
	}*/

	//RStdThread(const RStdThread&) = delete;

	// TODO: CAW - Spacing on macOS?
	RStdThread& operator=(const RStdThread&) = delete;

	RStdThread(RStdThread&&) noexcept = default;

	RStdThread& operator=(RStdThread&& a_other) noexcept
	{
		if (this != &a_other)
		{
			close();	// A joinable std::thread must not be overwritten

			m_thread   = std::move(a_other.m_thread);
			m_finished = std::move(a_other.m_finished);
		}

		return *this;
	}

	/** Starts the thread and invokes a_callback on it. Returns false if already started or creation failed. */
	bool start(Callback a_callback);

	/** True once the callback has returned. Safe to call from the owning thread at any time. */
	bool isFinished() const { return m_finished && *m_finished; }

	/** Waits for the thread to end. Idempotent. Must not be called from the thread itself. */
	void close();
	/*{
		printf("1\n");
		if (m_thread.joinable())
		{
			printf("2\n");
			m_thread.join();
			printf("3\n");
		}
		printf("4\n");
	}*/
};

#if 0
class CStdThread
{
protected:

	/**< Type definition for client callback to invoke when thread has started */
	using Callback = std::function<void()>;

	Callback		m_callback;			/**< Client callback to invoke when thread has started */

public:

	bool start(Callback a_callback);
};
#endif

#endif /* ! STDTHREAD_H */

