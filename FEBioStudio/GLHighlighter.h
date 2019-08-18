#pragma once
#include <GeomLib/GItem.h>
#include <QtCore/QObject>

//-------------------------------------------------------------------
// forward declaration
class CGLView;

//-------------------------------------------------------------------
// This class assist in hightlighting items in the CGLView. Highlighting allows
// users to pick items without changing the current selection.
// The highlighter keeps a list of all highlighted items. Only one of those
// items is the "active" item. The active item is the item that is currently
// under the mouse cursor. "Picking" the active item adds it to the list.
class GLHighlighter : public QObject
{
	Q_OBJECT

public:
	// return an instance of this highlighter (this class is a singleton)
	static GLHighlighter* Instance() { return &m_This; }

	// Attach the highlighter to a CGLView
	// This must be done prior to any highlighting.
	static void AttachToView(CGLView* view);

	// set the active item
	static void SetActiveItem(GItem* item);

	// "pick" the active item
	// This adds the active item to the list of highlighted items .
	// This also clears the active item
	static void PickActiveItem();

	// "pick" an item
	static void PickItem(GItem* item);

	// return the currently acite item
	static GItem* GetActiveItem();

	// return the name of the highlighted item
	static QString GetActiveItemName();

	// draw the highlighted item
	static void draw();

	// clear all hightlights
	static void ClearHighlights();

	// set the type of items to highlight (NOT WORKING YET!!)
	static void setHighlightType(int type);

	// if true, the highlighted item will be tracked by the mouse
	static void setTracking(bool b);

	// see if tracking is true or not.
	static bool IsTracking();

	// set the color for active item
	static void SetActiveColor(GLColor c);

	// set the color for the picked items
	static void SetPickColor(GLColor c);

private:
	// constructor
	GLHighlighter();

signals:
	// emitted when the active item changes
	void activeItemChanged();

	// emitted when the active item is "picked"
	void itemPicked(GItem* item);

private:
	CGLView*		m_view;				// pointer to GL view
	GItem*			m_activeItem;		// pointer to the active item (or zero)
	vector<GItem*>	m_item;				// list of hightlighted items (except active item)
	bool			m_btrack;			// set active item via mouse tracking
	GLColor			m_activeColor;		// color of active item
	GLColor			m_pickColor;		// color of picked items

	// the one-and-only highlighter
	static GLHighlighter m_This;
};
