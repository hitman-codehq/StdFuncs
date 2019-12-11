
#ifndef QTWINDOW_H
#define QTWINDOW_H

#include <QtWidgets/QMainWindow>

class CWindow;

/**
 * Qt helper class that represents the main window of an application.
 * This custom class is required to intercept certain required Qt messages such as paint
 * events etc. so that we can pass them onto our framework
 */

class CQtWindow : public QMainWindow
{
	Q_OBJECT

private:

	bool		m_bClosing;		/**< true if in the window is in the process of being closed */
	CWindow		*m_poWindow;	/**< Ptr to framework window represented by this Qt window */
	QSize		m_oSize;		/**< Preferred size of the non maximised window */

private:

	void HandleKeyEvent(QKeyEvent *a_poKeyEvent, bool a_bKeyDown);

	void HandlePointerEvent(QMouseEvent *a_poMouseEvent);

protected:

	/* From QMainWindow */

	void closeEvent(QCloseEvent *a_poCloseEvent);

	void focusInEvent(QFocusEvent *a_poFocusEvent);

	void focusOutEvent(QFocusEvent *a_poFocusEvent);

	void keyPressEvent(QKeyEvent *a_poKeyEvent);

	void keyReleaseEvent(QKeyEvent *a_poKeyEvent);

	void mouseDoubleClickEvent(QMouseEvent *a_poMouseEvent);

	void mouseMoveEvent(QMouseEvent *a_poMouseEvent);

	void mousePressEvent(QMouseEvent *a_poMouseEvent);

	void mouseReleaseEvent(QMouseEvent *a_poMouseEvent);

	void resizeEvent(QResizeEvent *a_poResizeEvent);

	void wheelEvent(QWheelEvent *a_poWheelEvent);

	/* From QWidget.h */

	QSize sizeHint() const;

public:

	CQtWindow(CWindow *a_poWindow, QSize &a_roSize);

	CWindow *Window()
	{
		return(m_poWindow);
	}

	void setClosing(bool a_bClosing)
	{
		m_bClosing = a_bClosing;
	}

public slots:

	/* Slot for picking up when the menu bar is activated */

	void aboutToShow();
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

	CWindow		*m_poWindow;	/**< Ptr to the Framework window that uses this widget */

protected:

	/* From QWidget */

	void paintEvent(QPaintEvent *a_poPaintEvent);

public:

	CQtCentralWidget(CWindow *a_poWindow);
};

#endif /* ! QTWINDOW_H */
