
#ifndef STDGADGETS_H
#define STDGADGETS_H

/** @file */

#ifdef __amigaos__

#include <intuition/classes.h>

#elif defined(QT_GUI_LIB)

#include "Qt/QtGadgetSlider.h"
#include "Qt/QtGadgetTree.h"

#endif /* QT_GUI_LIB */

/* Forward declarations to reduce the # of includes required */

class CStdGadgetLayout;
class CWindow;
class MStdGadgetLayoutObserver;
class MStdGadgetSliderObserver;
class QBoxLayout;
class QLabel;
class QWidget;

/* Types of gadgets that can be created */

enum TStdGadgetType
{
	EStdGadgetVerticalLayout,	/* Vertical dynamic layout gadget */
	EStdGadgetHorizontalLayout,	/* Horizontal dynamic layout gadget */
	EStdGadgetVerticalSlider,	/* Vertical scroller */
	EStdGadgetHorizontalSlider,	/* Horizontal scroller */
	EStdGadgetStatusBar,		/* Status bar */
	EStdGadgetTree				/* Tree gadget */
};

/* The abstract base class used for all gadgets */

class CStdGadget
{
protected:

	bool					m_bHidden;			/**< true if the gadget is hidden */
	TInt					m_iGadgetID;		/**< Unique ID of the gadget */
	TInt					m_iX;				/**< X and Y positions of the gadget, relative */
	TInt					m_iY;				/**< to the top left hand corner of the client area */
	TInt					m_iWidth;			/**< Width of the gadget in pixels */
	TInt					m_iHeight;			/**< Height of the gadget in pixels */
	TInt					m_iMinHeight;		/**< Minimum height of the gadget in pixels */
	CStdGadgetLayout		*m_poParentLayout;	/**< Ptr to CStdGadgetLayout that owns this gadget */
	CWindow					*m_poParentWindow;	/**< Ptr to window that owns this gadget */
	enum TStdGadgetType		m_iGadgetType;		/**< Type of gadget */

#ifdef __amigaos__

	Object					*m_poGadget;		/**< Ptr to underlying BOOPSI gadget */

#elif defined(QT_GUI_LIB)

	QWidget					*m_poGadget;		/**< Ptr to underlying Qt widget */

#elif defined(WIN32)

	HWND					m_poGadget;			/**< Ptr to the underlying Windows control */

#endif /* WIN32 */

public:

	StdListNode<CStdGadget>	m_oStdListNode;		/**< Standard list node */

private:

	void SetPosition(TInt a_iX, TInt a_iY);

protected:

	virtual void SetSize(TInt a_iWidth, TInt a_iHeight);

public:

	virtual ~CStdGadget();

	/* Getters and setters */

#ifdef __amigaos__

	Object *GetGadget()
	{
		return(m_poGadget);
	}

#endif /* __amigaos__ */

	TInt GetGadgetID()
	{
		return(m_iGadgetID);
	}

	enum TStdGadgetType GadgetType()
	{
		return(m_iGadgetType);
	}

	CWindow *GetParentWindow()
	{
		return(m_poParentWindow);
	}

	virtual void SetFocus() { };

	virtual void SetVisible(bool a_bVisible);

	bool Visible()
	{
		return(!m_bHidden);
	}

	virtual TInt X();

	virtual TInt Y();

	virtual TInt Width();

	virtual TInt Height();

	virtual TInt MinHeight();

	virtual void Updated(ULONG /*a_ulData*/ = 0) { }

	/* CStdGadgetLayout classe needs to be able to access this class's internals in order to */
	/* manage the gadgets' positions etc. */

	friend class CStdGadgetLayout;
};

/* A special gadget that can automatically layout other gadgets inside itself */

class CStdGadgetLayout : public CStdGadget
{
private:

	static CStdGadgetLayout		*m_poRethinker;		/**< Pointer to layout gadget currently rethinking, if any */

	TInt						m_iWeight;			/**< Weight of the layout gadget */
	MStdGadgetLayoutObserver	*m_poClient;		/**< Pointer to client to notify when gadget changes */
	StdList<CStdGadget>			m_oGadgets;			/**< List of gadgets added to the layout */
	StdList<CStdGadgetLayout>	m_oLayoutGadgets;	/**< List of layout gadgets added to the layout */

#ifdef __amigaos__

	Object					*m_poLayout;			/**< Pointer to the underlying Amiga OS gadget */

#elif defined(QT_GUI_LIB)

	QBoxLayout				*m_poLayout;			/**< Pointer to the underlying Qt widget.
														 Usually this is stored in m_poGadget but unfortunately
														 QLayout derived objects do not derive from QWidget */

#endif /* QT_GUI_LIB */

public:

	static TBool				m_bEnableRefresh;	/**< ETrue to disable immediate refresh when adding new
														 gadgets to the layout.  Affects Amiga OS only */

private:

	CStdGadgetLayout(CWindow *a_poParentWindow, CStdGadgetLayout *a_poParentLayout, TBool a_bVertical,
		MStdGadgetLayoutObserver *a_poClient)
	{
		m_iGadgetType = (a_bVertical) ? EStdGadgetVerticalLayout : EStdGadgetHorizontalLayout;
		m_poParentLayout = a_poParentLayout;
		m_poParentWindow = a_poParentWindow;
		m_poClient = a_poClient;
		m_iWeight = 50;
	}

	TInt Construct(bool a_bUseParentWindow);

	CStdGadget *FindNativeGadget(void *a_pvGadget);

	bool SendUpdate(void *a_pvGadget, ULONG a_ulData);

public:

	StdListNode<CStdGadgetLayout>	m_oStdListNode;	/**< Standard list node */

	static CStdGadgetLayout *New(CStdGadgetLayout *a_poParentLayout, TBool a_bVertical,
		MStdGadgetLayoutObserver *a_poClient = nullptr, CWindow *a_poParentWindow = nullptr);

	~CStdGadgetLayout();

	void Attach(CStdGadget *a_poGagdet);

	void Attach(CStdGadgetLayout *a_poLayoutGadget);

#ifdef __amigaos__

	Object *GetLayout()
	{
		return(m_poLayout);
	}

#elif defined(QT_GUI_LIB)

	QBoxLayout *GetLayout()
	{
		return(m_poLayout);
	}

#endif /* QT_GUI_LIB */

	const StdList<CStdGadgetLayout> *GetLayoutGadgets()
	{
		return(&m_oLayoutGadgets);
	}

	TInt GetSpacing();

#ifdef __amigaos__

	void ReAttach(CStdGadget *a_poGadget);

#endif /* __amigaos__ */

	void remove(CStdGadgetLayout *a_poLayoutGadget);

	void rethinkLayout();

	bool SetWeight(TInt a_iWeight);

	TInt Weight()
	{
		return(m_iWeight);
	}

	/* From CStdGadget */

	virtual void SetFocus();

	virtual TInt X();

	virtual TInt Y();

	virtual TInt Width();

	virtual TInt Height();

	virtual TInt MinHeight();

	/* CWindow classe needs to be able to access this class's internals in order to map */
	/* native events onto standard cross platform events */

	friend class CWindow;
};

/* A class representing a slider or proportional gadget */

class CStdGadgetSlider : public CStdGadget
{
private:

	TInt		m_iMaxRange;					/**< Maximum X/Y position of the slider */
	TInt		m_iPageSize;					/**< Number of characters/lines/pixels per page */
	TInt		m_iPosition;					/**< The position of the slider knob within the slider */
	MStdGadgetSliderObserver *m_poClient;		/**< Ptr to client to notify when gadget changes */

#ifdef QT_GUI_LIB

	bool			m_bSettingValue;			/**< true if the slider's value is being changed */
	CQtGadgetSlider	m_oSlider;					/**< Helper class for listening for signals */

#endif /* QT_GUI_LIB */

private:

#ifdef QT_GUI_LIB

	CStdGadgetSlider(CStdGadgetLayout *a_poParentLayout, TBool a_bVertical, MStdGadgetSliderObserver *a_poClient, TInt a_iGadgetID)
		: m_oSlider(this)

#else /* ! QT_GUI_LIB */

	CStdGadgetSlider(CStdGadgetLayout *a_poParentLayout, TBool a_bVertical, MStdGadgetSliderObserver *a_poClient, TInt a_iGadgetID)

#endif /* ! QT_GUI_LIB */

	{
		m_iGadgetType = (a_bVertical) ? EStdGadgetVerticalSlider : EStdGadgetHorizontalSlider;
		m_poParentLayout = a_poParentLayout;
		m_poParentWindow = a_poParentLayout->GetParentWindow();
		m_poClient = a_poClient;
		m_iGadgetID = a_iGadgetID;

		/* Set the default position to the top/left of the slider, which is 1, not 0 */

		m_iPosition = 1;
	}

	TInt Construct();

	bool CreateNative();

public:

	static CStdGadgetSlider *New(CStdGadgetLayout *a_poParentLayout, TBool a_bVertical,
		MStdGadgetSliderObserver *a_poClient, TInt a_iGadgetID);

	~CStdGadgetSlider();

	TInt MaxRange()
	{
		return(m_iMaxRange);
	}

	void SetPosition(TInt a_iPosition);

	void SetRange(TInt a_iPageSize, TInt a_iMaxRange);

	/* From CStdGadget */

#ifdef __amigaos__

	void SetVisible(bool a_bVisible) override;

#endif /* __amigaos__ */

	void Updated(ULONG a_ulData);
};

/* A class representing a status bar gadget */

class CStdGadgetStatusBar : public CStdGadget
{
private:

	char		**m_ppcPartsText;				/**< Ptr to an array of ptrs to text strings */
	TInt		m_iNumParts;					/**< Number of parts within the gadget */

#ifdef __amigaos__

	Object		**m_poPartsGadgets;				/**< Ptr to an array of ptrs to parts labels */

#elif defined(QT_GUI_LIB)

	QLabel		**m_poPartsGadgets;				/**< Ptr to an array of ptrs to parts labels */

#else /* ! QT_GUI_LIB */

	TInt		*m_piPartsOffsets;				/**< Pointer to an array of offsets of the parts in
													 the status bar (as percentages) */

#endif /* ! QT_GUI_LIB */

private:

	CStdGadgetStatusBar(CStdGadgetLayout *a_poParentLayout, TInt a_iGadgetID)
	{
		m_iGadgetType = EStdGadgetStatusBar;
		m_poParentLayout = a_poParentLayout;
		m_poParentWindow = a_poParentLayout->GetParentWindow();
		m_iGadgetID = a_iGadgetID;
	}

	TInt Construct(TInt a_iNumParts, TInt *a_piPartsOffsets);

	void SetSize(TInt a_iWidth, TInt a_iHeight);

public:

	~CStdGadgetStatusBar();

	static CStdGadgetStatusBar *New(CStdGadgetLayout *a_poParentLayout, TInt a_iNumParts, TInt *a_piPartsOffsets, TInt a_iGadgetID);

	const char *GetText(TInt a_iPart);

	void SetText(TInt a_iPart, const char *a_pccText);
};

/* A class representing a node in a tree gadget */

class CTreeNode
{
public:

	StdListNode<CTreeNode>	m_oStdListNode;		/**< Standard list node */
	std::string				m_text;				/**< Text to display in the tree gadget */

public:

	CTreeNode(const char *a_text) : m_text(a_text) { };
};

/* A class representing an expandable and collapsible tree gadget */

class CStdGadgetTree : public CStdGadget
{
private:

#ifdef __amigaos__

	std::string		m_title;				/**< Persistent memory for the title string passed in */
	List			m_fileList;				/**< List of items in the tree */

#elif defined(QT_GUI_LIB)

	CQtTreeWidget	m_tree;					/**< Helper class for listening for signals */

#endif /* QT_GUI_LIB */

protected:

#ifdef QT_GUI_LIB

	CStdGadgetTree(CStdGadgetLayout *a_parentLayout, int a_gadgetID) : m_tree(this)

#else /* ! QT_GUI_LIB */

	CStdGadgetTree(CStdGadgetLayout *a_parentLayout, int a_gadgetID)

#endif /* ! QT_GUI_LIB */

	{
		m_iGadgetType = EStdGadgetTree;
		m_poParentLayout = a_parentLayout;
		m_poParentWindow = a_parentLayout->GetParentWindow();
		m_iGadgetID = a_gadgetID;

#ifdef __amigaos__

		NewList(&m_fileList);

#endif /* __amigaos__ */

	}

	int construct(const std::string &a_title);

public:

	std::string getSelectedItem();

	void setContent(StdList<CTreeNode> &a_items);

	void setTitle(const std::string &a_title);
};

/* Mixin class for the slider or proportional gadget to be able to notify its client */
/* when it has been updated */

class MStdGadgetSliderObserver
{
public:

	virtual void SliderUpdated(CStdGadgetSlider *a_poGadget, TInt a_iPosition) = 0;
};

/* Mixin class for the layout gadget to be able to notify its client when it has been updated */

class MStdGadgetLayoutObserver
{
public:

	virtual void Resize() = 0;
};

#endif /* ! STDGADGETS_H */
