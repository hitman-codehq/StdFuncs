
#include "../StdFuncs.h"
#include "../StdExecuter.h"
#include "QtExecuter.h"

/**
 * Launches a command using the Qt QProcess class.
 * Asynchronously launches the given command and streams the stdout and stderr results to the calling client. It
 * will also detect when the child process has finished, and will report this to the calling client, along with the
 * process's exit status.
 *
 * @date	Friday 27-Mar-2026 6:13 am, Code HQ Tokyo Tsukuda
 * @param	a_commandName	The name of the command to be launched
 * @param	a_arguments		The arguments to be passed to the command, or nullptr if no arguments
 * @return	KErrNone if the command was launched successfully
 * @return	KErrNotFound if the command could not be found
 * @return	KErrInUse if a command is already being executed
 */

int RQtExecuter::launchCommand(const char *a_commandName, const char *a_arguments)
{
	/* Only launch the command if a command is not already running */
	if (m_process != nullptr)
	{
		return KErrInUse;
	}

	m_process = new QProcess(this);

	/* Connect signals for asynchronous reading */
	connect(m_process, &QProcess::readyReadStandardOutput, this, &RQtExecuter::onReadyRead);
	connect(m_process, &QProcess::readyReadStandardError, this, &RQtExecuter::onReadyRead);
	connect(m_process, &QProcess::finished, this, &RQtExecuter::onFinished);

	/* Append any arguments passed in onto the command line */
	QStringList arguments;

	if (a_arguments != nullptr)
	{
		QString argsStr(a_arguments);
		arguments = argsStr.split(' ', Qt::SkipEmptyParts);
	}

	/* Start the process asynchronously and wait for it to actually start */
	m_process->start(a_commandName, arguments);

	if (m_process->waitForStarted())
	{
		return KErrNone;
	}
	else
	{
		return KErrNotFound;
	}
}

/**
 * Callback for when the child process has finished.
 * Checks the result of the execution of the child process and reports it to the calling client. It will handle
 * both the normal exit case and the case where the child process has crashed. In the normal exit case, the
 * exit code of the child process is written to the result.
 *
 * @date	Tuesday 31-Mar-2026 4:58 am, Code HQ Tokyo Tsukuda
 * @param	a_exitCode		The exit code of the child process
 * @param	a_exitStatus	The result of the execution of the child process
 */

void RQtExecuter::onFinished(int a_exitCode, QProcess::ExitStatus a_exitStatus)
{
	TResult result;

	if (a_exitStatus == QProcess::NormalExit)
	{
		result.m_subResult = a_exitCode;
	}
	else
	{
		result.m_result = KErrChildCrashed;
	}

	m_parentExecuter.readComplete(nullptr, 0, result);

	/* Delete the QProcess object, to indicate that a launch is no longer in progress */
	delete m_process;
	m_process = nullptr;
}

/**
 * Callback for when output from the child process is available.
 * Checks whether output from either the stdout or the stderr streams is available, and if so, reports it to the
 * calling client.
 *
 * @date	Friday 27-Mar-2026 6:45 am, Code HQ Tokyo Tsukuda
 */

void RQtExecuter::onReadyRead()
{
	/* Reads are always successful */
	TResult result;

	/* Check if there is any stdout output available */
	QByteArray data = m_process->readAllStandardOutput();

	if (data.size() > 0)
	{
		m_parentExecuter.readComplete(data.constData(), data.size(), result);
	}

	/* And check if there is any stderr output available */
	QByteArray stdErrData = m_process->readAllStandardError();

	if (stdErrData.size() > 0)
	{
		m_parentExecuter.readComplete(stdErrData.constData(), stdErrData.size(), result);
	}
}
