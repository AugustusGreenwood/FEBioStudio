#include "stdafx.h"
#include "ModelSearch.h"
#include <QLineEdit>
#include <QListWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QContextMenuEvent>
#include "Document.h"
#include <PreViewLib/FEModel.h>
#include "ModelViewer.h"
#include "ModelTree.h"

CModelSearch::CModelSearch(CModelViewer* view, CModelTree* tree, QWidget* parent) : QWidget(parent), m_view(view), m_tree(tree)
{
	QLabel* label = new QLabel("Filter:");
	m_flt = new QLineEdit; label->setBuddy(m_flt);

	QHBoxLayout* h = new QHBoxLayout;
	h->addWidget(label);
	h->addWidget(m_flt);
	h->setMargin(0);

	m_list = new QListWidget;
	m_list->setSelectionMode(QAbstractItemView::ExtendedSelection);

	QVBoxLayout* l = new QVBoxLayout;
	l->addLayout(h);
	l->addWidget(m_list);
	l->setMargin(0);

	setLayout(l);

	QObject::connect(m_flt, SIGNAL(textEdited(const QString&)), this, SLOT(onFilterChanged(const QString&)));
	QObject::connect(m_list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onItemDoubleClicked(QListWidgetItem*)));
	QObject::connect(m_list, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(onItemClicked(QListWidgetItem*)));
}

void CModelSearch::Build(CDocument* doc)
{
	m_list->clear();
	UpdateList();
}

void CModelSearch::onFilterChanged(const QString& t)
{
	UpdateList();
}

void CModelSearch::onItemClicked(QListWidgetItem* item)
{
	if (item)
	{
		int n = item->data(Qt::UserRole).toInt();
		m_view->SetCurrentItem(n);
	}
	else
		m_view->SetCurrentItem(-1);
}

void CModelSearch::onItemDoubleClicked(QListWidgetItem* item)
{
	int n = item->data(Qt::UserRole).toInt();
	CModelTreeItem& treeItem = m_tree->GetItem(n);
	m_view->Select(treeItem.obj);
}

void CModelSearch::contextMenuEvent(QContextMenuEvent* ev)
{
	QPoint pt = ev->globalPos();

	// clear the selection
	m_view->ClearSelection();

	QListWidgetItem* item = m_list->currentItem();
	if (item == 0) return;

	// get the selection
	QList<QListWidgetItem*> sel = m_list->selectedItems();
	if (sel.isEmpty()) return;

	if (sel.size() == 1)
	{
		int index = item->data(Qt::UserRole).toInt();
		CModelTreeItem& data = m_tree->GetItem(index);

		m_view->SetSelection(data.obj);
		m_view->ShowContextMenu(&data, pt);
	}
	else
	{
		int index = item->data(Qt::UserRole).toInt();
		CModelTreeItem& data = m_tree->GetItem(index);

		int ntype = data.type;
		if (ntype == 0) return;

		// only show the context menu if all objects are the same
		vector<FEObject*> objList;
		QList<QListWidgetItem*>::iterator it = sel.begin();
		while (it != sel.end())
		{
			index = (*it)->data(Qt::UserRole).toInt();
			CModelTreeItem& di = m_tree->GetItem(index);

			if (di.type != ntype) return;

			objList.push_back(di.obj);

			++it;
		}

		// okay, we should only get here if the type is the same for all types
		m_view->SetSelection(objList);
		m_view->ShowContextMenu(&data, pt);
	}
}

void CModelSearch::GetSelection(std::vector<FEObject*>& objList)
{
	// get the selection
	QList<QListWidgetItem*> sel = m_list->selectedItems();
	if (sel.isEmpty()) return;

	QList<QListWidgetItem*>::iterator it = sel.begin();
	while (it != sel.end())
	{
		int index = (*it)->data(Qt::UserRole).toInt();
		CModelTreeItem& di = m_tree->GetItem(index);

		if (di.obj && 
			((di.flag & CModelTree::OBJECT_NOT_EDITABLE) == 0) &&
			((di.flag & CModelTree::DUPLICATE_ITEM)== 0))
				objList.push_back(di.obj);

		++it;
	}
}

void CModelSearch::UpdateList()
{
	m_list->clear();

	QString flt = m_flt->text();

	bool nofilter = flt.isEmpty();

	int N = m_tree->Items();
	for (int i=0; i<N; ++i)
	{
		CModelTreeItem& item = m_tree->GetItem(i);
		FEObject* o = item.obj;
		if (item.obj &&
			((item.flag & CModelTree::OBJECT_NOT_EDITABLE) == 0) &&
			((item.flag & CModelTree::DUPLICATE_ITEM) == 0))
		{
			QString s = QString::fromStdString(o->GetName());

			// make sure the string is not empty
			if (s.isEmpty() == false)
			{
				if (nofilter || s.contains(flt, Qt::CaseInsensitive))
				{
					QListWidgetItem* it = new QListWidgetItem;
					it->setText(s);
					it->setData(Qt::UserRole, i);
					m_list->addItem(it);
				}
			}
		}
	}

	m_list->sortItems();

	m_view->SetCurrentItem(-1);
}
