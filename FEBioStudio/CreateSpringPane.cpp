#include "stdafx.h"
#include "CreateSpringPane.h"
#include <QBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include "Document.h"
#include "MainWindow.h"
#include "SelectionBox.h"
#include <MeshTools/SpringGenerator.h>
#include <sstream>
#include <QGroupBox>
#include <GeomLib/GCurveMeshObject.h>
#include <MeshLib/FECurveMesh.h>
#include "GLHighlighter.h"
using namespace std;

class Ui::CCreateSpringPane
{
public:
	::CSelectionBox*	m_node[2];
	QComboBox*			m_list;
	int					m_type[2];
	FEMeshBase*			m_mesh[2];
	QComboBox*			m_method;

	GCurveMeshObject*	m_curves;

public:
	void setup(QWidget* w)
	{
		m_curves = 0;

		m_type[0] = m_type[1] = -1;
		m_mesh[0] = m_mesh[1] = 0;

		QHBoxLayout* h = new QHBoxLayout;
		QLabel* label = new QLabel("Group:");
		label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		h->addWidget(label);

		m_list = new QComboBox;
		h->addWidget(m_list);

		QPushButton* pb = new QPushButton("New..."); pb->setObjectName("newSet");
		pb->setFixedWidth(50);
		h->addWidget(pb);

		QGroupBox* g1 = new QGroupBox("Nodeset 1");
		g1->setFlat(true);

		m_node[0] = new ::CSelectionBox; 
		m_node[0]->setObjectName("node1");
		m_node[0]->showNameType(false);
		QVBoxLayout* l1 = new QVBoxLayout;
		l1->setMargin(0);
		l1->addWidget(m_node[0]);
		g1->setLayout(l1);

		QGroupBox* g2 = new QGroupBox("Nodeset 2");
		g2->setFlat(true);

		m_node[1] = new ::CSelectionBox; 
		m_node[1]->setObjectName("node2");
		m_node[1]->showNameType(false);
		QVBoxLayout* l2 = new QVBoxLayout;
		l2->setMargin(0);
		l2->addWidget(m_node[1]);
		g2->setLayout(l2);

		m_method = new QComboBox; m_method->setObjectName("method");
		m_method->addItem("connect by closest point");
		m_method->addItem("connect by closest line projection");

		QVBoxLayout* l = new QVBoxLayout;
		l->setMargin(0);
		l->addLayout(h);
		l->addWidget(g1);
		l->addWidget(g2);
		l->addWidget(m_method);
		l->addStretch();

		w->setLayout(l);

		QMetaObject::connectSlotsByName(w);
	}
};

//=============================================================================
CCreateSpringPane::CCreateSpringPane(CCreatePanel* parent) : CCreatePane(parent), ui(new Ui::CCreateSpringPane)
{
	ui->setup(this);
}

void CCreateSpringPane::showEvent(QShowEvent* ev)
{
	CDocument* doc = m_parent->GetDocument();
	GModel& geom = doc->GetFEModel()->GetModel();

	ui->m_type[0] = ui->m_type[1] = -1;
	ui->m_mesh[0] = ui->m_mesh[1] = 0;
	ui->m_node[0]->clearData();
	ui->m_node[1]->clearData();

	ui->m_list->clear();
	int n = 0;
	for (int i = 0; i<geom.DiscreteObjects(); ++i)
	{
		GDiscreteElementSet* ps = dynamic_cast<GDiscreteElementSet*>(geom.DiscreteObject(i));
		if (ps)
		{
			ui->m_list->addItem(QString::fromStdString(ps->GetName()), i);
		}
	}

	if (ui->m_curves == 0) 
	{
		FECurveMesh* mesh = new FECurveMesh;
		ui->m_curves = new GCurveMeshObject(mesh);
	}

	updateTempObject();
}

void CCreateSpringPane::hideEvent(QHideEvent* ev)
{
	m_parent->SetTempObject(0);
}

void CCreateSpringPane::on_method_currentIndexChanged(int n)
{
	updateTempObject();
}

void CCreateSpringPane::on_newSet_clicked()
{
	CNewDiscreteSetDlg dlg;
	if (dlg.exec())
	{
		CDocument* doc = m_parent->GetDocument();
		GModel& geom = doc->GetFEModel()->GetModel();

		QString name = dlg.m_name;
		int ntype = dlg.m_type;

		GDiscreteElementSet* po = 0;
		if (ntype == 0)
		{
			po = new GLinearSpringSet;
		}
		else if (ntype == 1)
		{
			po = new GNonlinearSpringSet;
		}
		else return;

		std::string sname;
		if (name.isEmpty())
		{
			int N = geom.DiscreteObjects();

			stringstream ss; ss << "DiscreteSet" << N + 1;
			sname = ss.str();
		}
		else
		{
			sname = name.toStdString();
		}

		po->SetName(sname);

		geom.AddDiscreteObject(po);

		name = QString::fromStdString(sname);

		ui->m_list->addItem(name);
		ui->m_list->setCurrentText(name);
	}
}

bool CCreateSpringPane::updateTempObject()
{
	ui->m_curves->ClearMesh();

	CDocument* doc = m_parent->GetDocument();
	GModel& geom = doc->GetFEModel()->GetModel();

	if (ui->m_list->count() == 0) return true;

	// get the nodes from the first node set
	vector<vec3d> node1, node2;
	if (ui->m_type[0] == SELECT_FE_NODES)
	{
		vector<int> fenodes;
		ui->m_node[0]->getAllItems(fenodes);

		if (ui->m_mesh[0] == 0) return 0;

		GMeshObject* po = dynamic_cast<GMeshObject*>(ui->m_mesh[0]->GetGObject());
		if (po == 0) return false;

		FEMesh* m = po->GetFEMesh();
		if (m == 0) return false;

		for (int i = 0; i<(int)fenodes.size(); ++i)
		{
			vec3d ri = m->NodePosition(fenodes[i] - 1);
			node1.push_back(ri);
		}
	}
	else
	{
		vector<int> gnode1;
		ui->m_node[0]->getAllItems(gnode1);

		for (int i=0; i<(int)gnode1.size(); ++i)
		{
			vec3d ri = geom.FindNode(gnode1[i])->Position();
			node1.push_back(ri);
		}
	}

	if (ui->m_type[1] == SELECT_FE_NODES)
	{
		vector<int> fenodes;
		ui->m_node[1]->getAllItems(fenodes);

		if (ui->m_mesh[1] == 0) return 0;

		GMeshObject* po = dynamic_cast<GMeshObject*>(ui->m_mesh[1]->GetGObject());
		if (po == 0) return false;

		FEMesh* m = po->GetFEMesh();
		if (m == 0) return false;

		for (int i = 0; i<(int)fenodes.size(); ++i)
		{
			vec3d ri = m->NodePosition(fenodes[i] - 1);
			node2.push_back(ri);
		}
	}
	else
	{
		vector<int> gnode2;
		ui->m_node[1]->getAllItems(gnode2);

		for (int i = 0; i<(int)gnode2.size(); ++i)
		{
			vec3d ri = geom.FindNode(gnode2[i])->Position();
			node2.push_back(ri);
		}
	}

	// create all the springs
	CSpringGenerator gen(geom);

	int n = ui->m_method->currentIndex();
	if (n == 1) gen.ProjectToLines(true);

	vector<pair<int, int> > springs;
	bool ret = gen.generate(node1, node2, springs);
	if (ret)
	{
		FECurveMesh* cm = ui->m_curves->GetCurveMesh();
		int n1 = node1.size();
		int n2 = node2.size();
		for (int i=0; i<node1.size(); ++i) cm->AddNode(node1[i], false);
		for (int i=0; i<node2.size(); ++i) cm->AddNode(node2[i], false);
		for (auto s : springs)
		{
			cm->AddEdge(s.first, s.second + n1);
		}
		cm->Update();
		ui->m_curves->Update();

		m_parent->SetTempObject(ui->m_curves);
	}

	return true;
}


bool CCreateSpringPane::getNodeSelection(vector<int>& nodeList, int n)
{
	// make sure the node list is empty
	nodeList.clear();

	// get the document
	CDocument* pdoc = m_parent->GetDocument();

	// get the current selection and make sure it's not empty
	FESelection* ps = pdoc->GetCurrentSelection();
	if ((ps == 0) || (ps->Size() == 0)) return false;

	// make sure this is a node selection
	if ((ps->Type() != SELECT_NODES) && 
		(ps->Type() != SELECT_FE_NODES)) return false;

	// fill the array
	if (ps->Type() == SELECT_NODES)
	{
		GNodeSelection* sel = dynamic_cast<GNodeSelection*>(ps); assert(sel);
		if (sel == 0) return false;

		if (ui->m_type[n] == -1) ui->m_type[n] = SELECT_NODES;
		if (ui->m_type[n] != SELECT_NODES) return false;

		if (ui->m_mesh[n] != 0) return false;

		GNodeSelection::Iterator it(sel);
		int N = sel->Size();
		for (int i=0; i<N; ++i, ++it)
		{
			GNode& node = *it;
			nodeList.push_back(node.GetID());
		}

		return true;
	}
	else if (ps->Type() == SELECT_FE_NODES)
	{
		FENodeSelection* sel = dynamic_cast<FENodeSelection*>(ps); assert(sel);
		if (sel == 0) return false;

		if (ui->m_type[n] == -1) ui->m_type[n] = SELECT_FE_NODES;
		if (ui->m_type[n] != SELECT_FE_NODES) return false;

		FEMeshBase* pm = dynamic_cast<FEMeshBase*>(sel->GetMesh());

		if (ui->m_mesh[n] == 0) ui->m_mesh[n] = pm;
		if (ui->m_mesh[n] != sel->GetMesh()) return false;

		int NN = pm->Nodes();
		for (int i=0; i<NN; ++i) pm->Node(i).m_ntag = i + 1;

		FENodeSelection::Iterator it(pm);
		int N = sel->Size();
		for (int i=0; i<N; ++i, ++it)
		{
			FENode* node = it;
			nodeList.push_back(node->m_ntag);
		}

		return true;
	}

	return false;
}

void CCreateSpringPane::on_node1_addButtonClicked()
{
	vector<int> nodes;
	if (getNodeSelection(nodes, 0) == false)
	{
		QMessageBox::critical(this, "Add to selection", "Cannot add this selection to the current selection type");
		return;
	}
	ui->m_node[0]->addData(nodes);

	updateTempObject();
}

void CCreateSpringPane::on_node1_subButtonClicked()
{
	vector<int> nodes;
	if (getNodeSelection(nodes, 0) == false)
	{
		QMessageBox::critical(this, "Add to selection", "Cannot subtract this selection to the current selection type");
		return;
	}
	ui->m_node[0]->removeData(nodes);

	updateTempObject();
}

void CCreateSpringPane::on_node1_delButtonClicked()
{
	ui->m_node[0]->removeSelectedItems();
	updateTempObject();
}

void CCreateSpringPane::on_node1_selButtonClicked()
{
	vector<int> items;
	ui->m_node[0]->getSelectedItems(items);
	if (items.empty()) return;

	for (int i = 0; i<(int)items.size(); ++i) items[i]--;

	CDocument* doc = m_parent->GetDocument();
	if (ui->m_type[0] == SELECT_NODES)
	{
		doc->DoCommand(new CCmdSelectNode(doc->GetFEModel(), items, false));
	}
	else if (ui->m_type[0] == SELECT_FE_NODES)
	{
		doc->DoCommand(new CCmdSelectFENodes(ui->m_mesh[0], items, false));
	}
	m_parent->GetMainWindow()->RedrawGL();
}

void CCreateSpringPane::on_node2_addButtonClicked()
{
	vector<int> nodes;
	if (getNodeSelection(nodes, 1) == false)
	{
		QMessageBox::critical(this, "Add to selection", "Cannot add this selection to the current selection type");
		return;
	}
	ui->m_node[1]->addData(nodes);
	updateTempObject();

}

void CCreateSpringPane::on_node2_subButtonClicked()
{
	vector<int> nodes;
	if (getNodeSelection(nodes, 1) == false)
	{
		QMessageBox::critical(this, "Add to selection", "Cannot subtract this selection to the current selection type");
		return;
	}
	ui->m_node[1]->removeData(nodes);
	updateTempObject();
}

void CCreateSpringPane::on_node2_delButtonClicked()
{
	ui->m_node[1]->removeSelectedItems();
	updateTempObject();
}

void CCreateSpringPane::on_node2_selButtonClicked()
{
	vector<int> items;
	ui->m_node[1]->getSelectedItems(items);
	if (items.empty()) return;

	for (int i=0; i<(int)items.size(); ++i) items[i]--;

	CDocument* doc = m_parent->GetDocument();
	if (ui->m_type[1] == SELECT_NODES)
	{
		doc->DoCommand(new CCmdSelectNode(doc->GetFEModel(), items, false));
	}
	else if (ui->m_type[1] == SELECT_FE_NODES)
	{
		doc->DoCommand(new CCmdSelectFENodes(ui->m_mesh[1], items, false));
	}
	m_parent->GetMainWindow()->RedrawGL();
}

FEObject* CCreateSpringPane::Create()
{
	CDocument* doc = m_parent->GetDocument();
	GModel& geom = doc->GetFEModel()->GetModel();

	if (ui->m_list->count() == 0)
	{
		QMessageBox::critical(this, "PreView2", "You must create and select a discrete set first.");
		return 0;
	}

	QString txt = ui->m_list->currentText();
	GDiscreteObject* pd = geom.FindDiscreteObject(txt.toStdString()); assert(pd);

	GDiscreteElementSet* po = dynamic_cast<GDiscreteElementSet*>(pd); assert(po);

	if (po)
	{
		// create all the springs
		CSpringGenerator gen(geom);

		int n = ui->m_method->currentIndex();
		if (n == 1) gen.ProjectToLines(true);

		vector<int> node1, node2;
		if (ui->m_type[0] == SELECT_FE_NODES)
		{
			vector<int> fenodes;
			ui->m_node[0]->getAllItems(fenodes);

			if (ui->m_mesh[0] == 0) return 0;

			GMeshObject* po = dynamic_cast<GMeshObject*>(ui->m_mesh[0]->GetGObject());
			if (po == 0) 
			{
				QMessageBox::critical(this, "PreView2", "Unable to generate springs.");
				return 0;
			}

			for (int i=0; i<(int)fenodes.size(); ++i)
			{
				int nid = po->MakeGNode(fenodes[i] - 1);
				node1.push_back(nid);
			}
		}
		else
		{
			ui->m_node[0]->getAllItems(node1);
		}

		if (ui->m_type[1] == SELECT_FE_NODES)
		{
			vector<int> fenodes;
			ui->m_node[1]->getAllItems(fenodes);

			if (ui->m_mesh[1] == 0) return 0;

			GMeshObject* po = dynamic_cast<GMeshObject*>(ui->m_mesh[1]->GetGObject());
			if (po == 0)
			{
				QMessageBox::critical(this, "PreView2", "Unable to generate springs.");
				return 0;
			}

			for (int i = 0; i<(int)fenodes.size(); ++i)
			{
				int nid = po->MakeGNode(fenodes[i] - 1);
				node2.push_back(nid);
			}
		}
		else
		{
			ui->m_node[1]->getAllItems(node2);
		}

		bool ret = gen.generate(po, node1, node2);
		if (ret == false)
		{
			QMessageBox::critical(this, "PreView2", "Was not able to generate all the springs.");
			return 0;
		}

		ui->m_node[0]->clearData();
		ui->m_node[1]->clearData();

		return &(po->element(po->size() - 1));
	}
	else
	{
		QMessageBox::critical(this, "PreView2", "Hmmm, something went wrong ...");
		return 0;
	}

	updateTempObject();
}
