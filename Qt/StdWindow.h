
#ifndef QTSTDWINDOW_H
#define QTSTDWINDOW_H

#include <QtGui/QAction>
#include <QtGui/QMainWindow>

class CWindow;

/* This custom class is required to intercept certain required Qt messages such as paint */
/* events etc. so that we can pass them onto our framework */

class CQtWindow : public QMainWindow
{
public:

	Q_OBJECT;

private:

	CWindow		*m_poWindow;	/* Ptr to framework window represented by this Qt window */

private:

	void HandleKeyEvent(QKeyEvent *a_poKeyEvent, bool a_bKeyDown);

protected:

	/* From QMainWindow */

	void keyPressEvent(QKeyEvent *a_poKeyEvent);

	void keyReleaseEvent(QKeyEvent *a_poKeyEvent);

	void resizeEvent(QResizeEvent *a_poResizeEvent);

public:

	CQtWindow(CWindow *a_poWindow)
	{
		m_poWindow = a_poWindow;

		/* Allow the window to accept keyboard input by default */

		setFocusPolicy(Qt::StrongFocus);
	}

	CWindow *Window()
	{
		return(m_poWindow);
	}
};

/* This custom class is required to keep track of Qt menu selection events and send them to */
/* the framework and its client classes */

class CQtAction : public QAction
{
	Q_OBJECT;

private:

	int			m_iCommand;			/* ID of this action/menu item */
	CQtWindow	*m_poWindow;		/* Ptr to framework window to which to send command updates */

public:

	CQtAction(int a_iCommand, const QString &a_roText, CQtWindow *a_poWindow) : QAction(a_roText, a_poWindow)
	{
		m_iCommand = a_iCommand;
		m_poWindow = a_poWindow;

		/* Connect the action's triggered() slot to our actionTriggered() signal so that we know */
		/* when the menu item is selected */

		QObject::connect(this, SIGNAL(triggered()), this, SLOT(actionTriggered()));
	}

	int Command()
	{
		return(m_iCommand);
	}

public slots:

	/* Slot for picking up menu selections */

	void actionTriggered();
};

#endif /* ! QTSTDWINDOW_H */
