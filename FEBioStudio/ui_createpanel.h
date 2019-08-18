#pragma once
#include <QWidget>
#include <QBoxLayout>
#include <QComboBox>
#include <QToolButton>
#include <QGridLayout>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QListWidget>
#include "PropertyListForm.h"
#include "CIntInput.h"
#include <MathLib/math3d.h>
#include <GeomLib/GObject.h>
#include <MeshTools/GDiscreteObject.h>
#include "ToolBox.h"
#include "ClassDescriptor.h"
#include "ObjectProps.h"
#include <MeshLib/FECurveMesh.h>
#include <MeshTools/GObject2D.h>
#include <MeshTools/FESelection.h>
#include <MeshTools/FELoftMesher.h>
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GCurveMeshObject.h>
#include <MeshTools/GPLCObject.h>
#include "Document.h"
#include "GLHighlighter.h"
#include "GLCursor.h"
#include <sstream>
using namespace std;

CCreateButtonPanel::CCreateButtonPanel(QWidget* parent) : QWidget(parent)
{
	buttonGrid = new QGridLayout;
	buttonGroup = new QButtonGroup(parent);
	setLayout(buttonGrid);

	QObject::connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(on_button_clicked(int)));
}

void CCreateButtonPanel::AddCreateButton(const QString& txt, const QIcon& icon, int nid)
{
	QToolButton* tb = new QToolButton;
	tb->setCheckable(true);
	tb->setFixedSize(36, 36);
	tb->setIcon(icon);
	tb->setIconSize(tb->size());
	tb->setAutoRaise(true);
	tb->setToolTip(QString("<font color=\"black\">") + txt);

	int ncount = (int)buttonGroup->buttons().size();
	int y = ncount % 5;
	int x = ncount / 5;

	buttonGrid->addWidget(tb, x, y);
	buttonGroup->addButton(tb, nid);
}

void CCreateButtonPanel::on_button_clicked(int id)
{
	emit buttonClicked(id);
}


//=============================================================================
// Class the allows users to set the initial position of the newly created object
class CCreatePosition : public QGroupBox
{
public:
	CCreatePosition() : QGroupBox("Position")
	{
		QFormLayout* posLayout = new QFormLayout;
		posLayout->addRow("X:", pos[0] = new CFloatInput); pos[0]->setValue(0.0); pos[0]->setFixedWidth(100);
		posLayout->addRow("Y:", pos[1] = new CFloatInput); pos[1]->setValue(0.0); pos[1]->setFixedWidth(100);
		posLayout->addRow("Z:", pos[2] = new CFloatInput); pos[2]->setValue(0.0); pos[2]->setFixedWidth(100);
		setLayout(posLayout);
	}

	vec3d position()
	{
		vec3d r;
		r.x = pos[0]->value();
		r.y = pos[1]->value();
		r.z = pos[2]->value();

		return r;
	}

	void setPosition(const vec3d& r)
	{
		pos[0]->setValue(r.x);
		pos[1]->setValue(r.y);
		pos[2]->setValue(r.z);
	}

private:
	CFloatInput*	pos[3];
};

//=============================================================================
class CCreateParams : public QGroupBox
{
public:
	CPropertyListForm* form;

public:
	CCreateParams(QWidget* parent = 0) : QGroupBox("Parameters")
	{
		QVBoxLayout* paramLayout = new QVBoxLayout;
		form = new CPropertyListForm;
		form->setBackgroundRole(QPalette::Light);
		paramLayout->addWidget(form);
		paramLayout->addStretch();
		setLayout(paramLayout);
	}

	void SetPropertyList(CPropertyList* pl)
	{
		form->setPropertyList(pl);
	}
};

//=============================================================================
// primitives page
CDefaultCreatePane::CDefaultCreatePane(CCreatePanel* parent, ClassDescriptor* pcd) : CCreatePane(parent), m_pcd(pcd)
{
	// object position
	position = new CCreatePosition;

	// create parameters
	params = new CCreateParams;
	
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(position);
	layout->addWidget(params);
	setLayout(layout);

	setLayout(layout);

	QMetaObject::connectSlotsByName(this);

	m_po = 0;
}

void CDefaultCreatePane::CreateObject()
{
	m_po = dynamic_cast<GObject*>(m_pcd->Create());
	params->SetPropertyList(new CObjectProps(m_po));
}

void CDefaultCreatePane::setInput(const vec3d& r)
{
	position->setPosition(r);
}

void CDefaultCreatePane::Activate()
{
	vec3d r = GLCursor::Position();
	position->setPosition(r);
	if (m_po == 0) CreateObject();
}

FSObject* CDefaultCreatePane::Create()
{
	static int n = 1;

	GObject* po = m_po;

	char szname[128] = { 0 };
	sprintf(szname, "Object%02d", n++);
	po->SetName(szname);

	vec3d r = position->position();
	po->Transform().SetPosition(r);

	CreateObject();

	return po;
}

//=============================================================================
CNewDiscreteSetDlg::CNewDiscreteSetDlg(QWidget* parent) : QDialog(parent)
{
	QFormLayout* form = new QFormLayout;
	form->addRow("Name:", m_edit = new QLineEdit);
	form->addRow("Type:", m_combo = new QComboBox);

	m_combo->addItem("Linear");
	m_combo->addItem("Nonlinear");

	QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	QVBoxLayout* l = new QVBoxLayout;
	l->addLayout(form);
	l->addWidget(bb);

	setLayout(l);

	QObject::connect(bb, SIGNAL(accepted()), this, SLOT(accept()));
	QObject::connect(bb, SIGNAL(rejected()), this, SLOT(reject()));
}

void CNewDiscreteSetDlg::accept()
{
	m_name = m_edit->text();
	m_type = m_combo->currentIndex();
	QDialog::accept();
}

//=============================================================================
CCreateLoftSurface::CCreateLoftSurface(CCreatePanel* parent) : CCreatePane(parent)
{
	QLabel* pl = new QLabel("Selected curves:");
	m_list = new QListWidget;
	QVBoxLayout* layout = new QVBoxLayout;
	m_combo = new QComboBox;
	m_combo->addItems(QStringList() << "Tri" << "Quad");
	m_divs = new QSpinBox;
	m_divs->setRange(1, 100);
	m_divs->setValue(1);
	layout->addWidget(pl);
	layout->addWidget(m_list);
	layout->addWidget(m_combo);
	layout->addWidget(m_divs);
	layout->addStretch();

	setLayout(layout);

	QMetaObject::connectSlotsByName(this);
}

void CCreateLoftSurface::showEvent(QShowEvent* ev)
{
	GLHighlighter::setTracking(true);
	QObject::connect(GLHighlighter::Instance(), SIGNAL(itemPicked(GItem*)), this, SLOT(itemPicked(GItem*)));
}

void CCreateLoftSurface::hideEvent(QHideEvent* ev)
{
	GLHighlighter::setTracking(false);
	GLHighlighter::Instance()->disconnect(this);
	m_list->clear();
	m_edge.clear();
}

void CCreateLoftSurface::Activate()
{
}

void CCreateLoftSurface::Deactivate()
{
}

void CCreateLoftSurface::itemPicked(GItem* pick)
{
	GLHighlighter& hl = *GLHighlighter::Instance();
	if (dynamic_cast<GEdge*>(pick))
	{
		GEdge* edge = dynamic_cast<GEdge*>(pick);
		m_edge.push_back(edge);
		m_list->addItem(QString::fromStdString(edge->GetName()));
	}
}

FSObject* CCreateLoftSurface::Create()
{
	if (m_edge.size() < 2) return 0;

	vector<FECurveMesh*> curves;
	for (int i=0; i<(int)m_edge.size(); ++i)
	{
		GEdge& edge = *m_edge[i];
		GObject* po = dynamic_cast<GObject*>(edge.Object()); assert(po);
		if (po == 0) return 0;

		FECurveMesh* c = po->GetFECurveMesh(edge.GetLocalID());
		assert(c);
		if (c == 0) return 0;
		curves.push_back(c);
	}

	int nelem = m_combo->currentIndex();

	static int n = 0; n++;

	FELoftMesher loft;
	loft.setElementType(nelem);
	loft.setDivisions(m_divs->value());
	FESurfaceMesh* mesh = loft.Apply(curves);

	GSurfaceMeshObject* po = new GSurfaceMeshObject(mesh);

	char sz[256] = {0};
	sprintf(sz, "LoftObject%02d", n);
	po->SetName(sz);

	m_edge.clear();
	m_list->clear();
	GLHighlighter::ClearHighlights();

	return po;
}

void CCreateLoftSurface::setInput(FESelection* sel)
{
	if (sel && (sel->Type() == SELECT_CURVES) && (sel->Size() == 1))
	{
		GEdgeSelection* edgeSel = dynamic_cast<GEdgeSelection*>(sel);
		GEdgeSelection::Iterator it(edgeSel);
		m_edge.push_back(it);
	}
}

//=============================================================================
CCreateExtrude::CCreateExtrude(CCreatePanel* parent) : CCreatePane(parent)
{
	QLabel* pl = new QLabel("Extrude distance:");
	m_distance = new QLineEdit;
	m_distance->setValidator(new QDoubleValidator);
	m_distance->setText("1");

	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(pl);
	layout->addWidget(m_distance);
	layout->addStretch();

	setLayout(layout);

	QMetaObject::connectSlotsByName(this);
}

void CCreateExtrude::Activate()
{
}

void CCreateExtrude::Deactivate()
{
}

FSObject* CCreateExtrude::Create()
{
	static int n = 1;

	CDocument* doc = m_parent->GetDocument();
	if (doc == 0) return 0;

	GObject* activeObject = doc->GetActiveObject();
	if (activeObject == 0) return 0;

	// create a clone of this object
	GPLCObject* newObject = new GPLCObject;
	newObject->Copy(activeObject);

	GExtrudeModifier mod;
	mod.SetFloatValue(0, m_distance->text().toDouble());
	mod.Apply(newObject);

	stringstream ss;
	ss << "Extrude" << n;
	newObject->SetName(ss.str());
	n++;

	return newObject;
}

//=============================================================================
// User interface for create panel
class Ui::CCreatePanel
{
public:
	CCreateButtonPanel*	but;
	QStackedWidget* panes;
	vector<CCreatePane*> page;
	CCreatePane*	activePane;
	QPushButton* createButton;

public:
	void setupUi(::CCreatePanel* parent)
	{
		activePane = 0;

		// stack for button boxes
		but = new CCreateButtonPanel;

		// stack for create panes
		panes = new QStackedWidget;
		panes->addWidget(new QLabel(""));

		// create button
		createButton = new QPushButton("Create");
		createButton->setObjectName("create");
		createButton->hide();

		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(panes);
		l->addWidget(createButton);

		QWidget* dummy = new QWidget;
		dummy->setLayout(l);

		// create the toolbox
		CToolBox* tool = new CToolBox;
		tool->addTool("Create", but);
		tool->addTool("Parameters", dummy);

		// put everything in a layout
		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->setMargin(0);
		mainLayout->addWidget(tool);
		mainLayout->addStretch();

		// set the layout
		parent->setLayout(mainLayout);

		QObject::connect(but, SIGNAL(buttonClicked(int)), parent, SLOT(select_option(int)));
		QObject::connect(createButton, SIGNAL(clicked()), parent, SLOT(on_create_clicked()));
	}

	CCreatePane* currentPage()
	{
		return activePane;
	}

	// Add a create option to the createpanel
	void AddCreateOption(int category, const QString& label, const QIcon& icon, CCreatePane* ui)
	{
		ui->hide();
		// add the ui to the list
		page.push_back(ui);
		panes->addWidget(ui);

		// Add a button for creating this object
		but->AddCreateButton(label, icon, (int) page.size() - 1);
	}

	void setActivePane(int id)
	{
		if (activePane) activePane->Deactivate();

		panes->setCurrentIndex(id + 1);
		activePane = (id >= 0 ? page[id] : 0);
		if (activePane == 0)
		{
			createButton->hide();
		}
		else 
		{
			activePane->Activate();
			if (createButton->isHidden()) createButton->show();
		}
	}
};
