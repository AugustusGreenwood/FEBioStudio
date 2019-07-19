#pragma once
#include "ModelViewer.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QListWidget>
#include <QToolButton>
#include <QScrollArea>
#include "ModelTree.h"
#include "ModelPropsPanel.h"
#include "ModelSearch.h"

class CMainWindow;

class Ui::CModelViewer
{
public:
	QStackedWidget*		m_stack;
	CModelSearch*		m_search;

	CModelTree*			tree;
	::CModelPropsPanel*	props;

	QToolButton* srcButton;

public:
	void setupUi(::CMainWindow* mainWnd, ::CModelViewer* wnd)
	{
		QToolButton* selectButton = new QToolButton; 
		selectButton->setIcon(QIcon(":/icons/select.png")); 
		selectButton->setObjectName("selectButton"); 
		selectButton->setAutoRaise(true);
		selectButton->setToolTip("<font color=\"black\">Select in Graphics View");

		QToolButton* deleteButton = new QToolButton; 
		deleteButton->setIcon(QIcon(":/icons/delete.png")); 
		deleteButton->setObjectName("deleteButton"); 
		deleteButton->setAutoRaise(true);
		deleteButton->setToolTip("<font color=\"black\">Delete item");

		srcButton = new QToolButton; 
		srcButton->setIcon(QIcon(":/icons/search.png")); 
		srcButton->setObjectName("searchButton"); 
		srcButton->setAutoRaise(true);
		srcButton->setToolTip("<font color=\"black\">Toggle search panel");
		srcButton->setCheckable(true);

		QToolButton* syncButton = new QToolButton; 
		syncButton->setIcon(QIcon(":/icons/sync.png")); 
		syncButton->setObjectName("syncButton"); 
		syncButton->setAutoRaise(true);
		syncButton->setToolTip("<font color=\"black\">Sync selection");

		// model tree
		tree = new CModelTree(wnd);
		tree->setObjectName("modelTree");
		tree->setColumnCount(1);
		tree->setHeaderHidden(true);

		// search widget
		m_search = new CModelSearch(wnd, tree);

		// stacked widget
		m_stack = new QStackedWidget;

		m_stack->addWidget(tree);
		m_stack->addWidget(m_search);

		// props panel
		props = new ::CModelPropsPanel(mainWnd);
		props->setObjectName("props");

		QScrollArea* scroll = new QScrollArea;
		scroll->setWidget(props);
		scroll->setWidgetResizable(true);
//		scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		//	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
		//	setBackgroundRole(QPalette::Light);
		scroll->setFrameShape(QFrame::NoFrame);

		QSplitter* splitter = new QSplitter(Qt::Vertical);
		splitter->addWidget(m_stack);
		splitter->addWidget(scroll);
		splitter->setContentsMargins(0, 0, 0, 0);

		QHBoxLayout* buttonLayout = new QHBoxLayout;
		buttonLayout->addWidget(selectButton);
		buttonLayout->addWidget(deleteButton);
		buttonLayout->addWidget(srcButton);
		buttonLayout->addWidget(syncButton);
		buttonLayout->addStretch();
		buttonLayout->setMargin(0);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(buttonLayout);
		mainLayout->addWidget(splitter);
		mainLayout->setMargin(0);

		wnd->setLayout(mainLayout);

		QMetaObject::connectSlotsByName(wnd);
	}

	void showSearchPanel(bool b)
	{
		m_stack->setCurrentIndex(b ? 1 : 0);
	}

	void unCheckSearch()
	{
		if (srcButton->isChecked())
		{
			srcButton->setChecked(false);
		}
	}
};
