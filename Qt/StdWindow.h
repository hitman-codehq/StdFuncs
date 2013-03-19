
#ifndef QTSTDWINDOW_H
#define QTSTDWINDOW_H

#include <QtGui/QAction>
#include <QtGui/QMainWindow>

class CWindow;

/* This custom class is required to intercept certain required Qt messages such as paint */
/* events etc. so that we can pass them onto our framework */

class CQtWindow : public QMainWindow
{
	Q_OBJECT;

private:

	bool		m_bClosing;		/* true if in the window is in the process of being closed */
	CWindow		*m_poWindow;	/* Ptr to framework window represented by this Qt window */

private:

	void HandleKeyEvent(QKeyEvent *a_poKeyEvent, bool a_bKeyDown);

protected:

	/* From QMainWindow */

	void closeEvent(QCloseEvent *a_poCloseEvent);

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

	void setClosing(bool a_bClosing)
	{
		m_bClosing = a_bClosing;
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

/**
 * Qt helper class that represents a window's central widget.
 * Qt requires that you have a QWidget derived object as the so-called "central widget"
 * and does not handle the window decoration offsets correctly if you don't.  This custom
 * central widget class is used for rendering into, for listening for paint events and for
 * attaching QLayout derived widgets to assist with widget layout.
 */

class CQtCentralWidget : public QWidget
{
private:

	CWindow		*m_poWindow;	/* Ptr to the Framework window that uses this widget */

protected:

	/* From QWidget */

	void paintEvent(QPaintEvent *a_poPaintEvent);

public:

	CQtCentralWidget(CWindow *a_poWindow);
};

#endif /* ! QTSTDWINDOW_H */
