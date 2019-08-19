#include "stdafx.h"
#include "ModelTree.h"
#include "Document.h"
#include <MeshTools/FEModel.h>
#include <MeshTools/FEMultiMaterial.h>
#include <FEMLib/FEAnalysisStep.h>
#include <QTreeWidgetItemIterator>
#include <QContextMenuEvent>
#include <QDesktopServices>
#include "ObjectProps.h"
#include "FEObjectProps.h"
#include "ModelViewer.h"
#include "PostDoc.h"
#include <PostGL/GLModel.h>
#include <PostLib/ImageModel.h>
#include <PostLib/GLImageRenderer.h>
#include <QMessageBox>
#include "MainWindow.h"
#include "GImageObject.h"

class CObjectValidator
{
public:
	CObjectValidator(){}
	virtual ~CObjectValidator(){}

	virtual QString GetErrorString() const = 0;

	virtual bool IsValid() = 0;
};

class CGObjectValidator : public CObjectValidator
{
public:
	CGObjectValidator(GObject* po) : m_po(po){}

	QString GetErrorString() const { return "Object is not meshed"; }

	bool IsValid()
	{
		if ((m_po == 0) || (m_po->GetFEMesh() == 0)) return false;
		return true;
	}

private:
	GObject* m_po;
};

class CBCValidator : public CObjectValidator
{
public:
	CBCValidator(FEBoundaryCondition* pbc) : m_pbc(pbc), m_err(0) {}

	QString GetErrorString() const 
	{ 
		if      (m_err == 1) return "No selection assigned"; 
		else if (m_err == 2) return "Contains invalid references";
		else if (m_err == 3) return "no degree of freedom selected";
		return "No problems";
	}

	bool IsValid()
	{
		m_err = 0;
		if (m_pbc == 0) { m_err = 1; return false; }
		FEItemListBuilder* item = m_pbc->GetItemList();
		if ((item==0) || (item->size() == 0)) { m_err = 1; return false; }
		if (item->IsValid() == false) { m_err = 2; return false; }

		FEFixedDOF* fix = dynamic_cast<FEFixedDOF*>(m_pbc);
		if (fix)
		{
			if (fix->GetBC() == 0)
			{
				m_err = 3;
				return false;
			}
		}
		return true;
	}

private:
	FEBoundaryCondition* m_pbc;
	int	m_err;
};

class CStepValidator : public CObjectValidator
{
public:
	CStepValidator(FEModel* fem) : m_fem(fem) {}

	QString GetErrorString() const { return "No steps defined"; }

	bool IsValid()
	{
		if (m_fem == 0) return false;
		int nsteps = m_fem->Steps() - 1; // subtract one for initial step
		return (nsteps > 0);
	}

private:
	FEModel*	m_fem;
};

class CMaterialValidator : public CObjectValidator
{
public:
	CMaterialValidator(FEModel* fem, GMaterial* mat) : m_fem(fem), m_mat(mat), m_err(0) {}

	QString GetErrorString() const 
	{ 
		if (m_err == 0) return "";
		if (m_err == 1) return "Material not assigned yet"; 
		if (m_err == 2) return "No material properties";
		return "unknown error";
	}

	bool IsValid()
	{
		m_err = 0;
		if ((m_fem == 0) || (m_mat == 0)) 
		{
			m_err = -1;
			return false;
		}

		if (m_mat->GetMaterialProperties() == 0)
		{
			m_err = 2;
			return false;
		}

		GModel& mdl = m_fem->GetModel();
		for (int i=0; i<mdl.Objects(); ++i)
		{
			GObject* po = mdl.Object(i);
			for (int i=0; i<po->Parts(); ++i)
			{
				GPart& p = *po->Part(i);
				if (p.GetMaterialID() == m_mat->GetID()) return true;
			}
		}

		m_err = 1;
		return false;
	}

private:
	FEModel*	m_fem;
	GMaterial*	m_mat;
	int			m_err;
};

class CContactValidator : public CObjectValidator
{
public:
	CContactValidator(FEPairedInterface* pci) : m_pci(pci) {}

	QString GetErrorString() const { return "master/slave not specified"; }

	bool IsValid()
	{
		if (m_pci == 0) return false;
		FEItemListBuilder* master = m_pci->GetMasterSurfaceList();
		FEItemListBuilder* slave  = m_pci->GetSlaveSurfaceList();
		if ((master == 0) || (master->size() == 0)) return false;
		if ((slave  == 0) || (slave ->size() == 0)) return false;
		return true;
	}

private:
	FEPairedInterface*	m_pci;
};

class CRigidConstraintValidator : public CObjectValidator
{
public:
	CRigidConstraintValidator(FERigidConstraint* rc) : m_rc(rc){}

	QString GetErrorString() const { return "No rigid material assigned"; }

	bool IsValid() { return (m_rc->GetMaterialID() != -1); }

private:
	FERigidConstraint*	m_rc;	
};

class CFEBioJobProps : public CPropertyList
{
public:
	CFEBioJobProps(CMainWindow* wnd, CFEBioJob* job) : m_wnd(wnd), m_job(job)
	{
		addProperty("FEBio File:", CProperty::String)->setFlags(CProperty::Visible);
		addProperty("Status:", CProperty::Enum)->setEnumValues(QStringList() << "NONE" << "NORMAL TERMINATION" << "ERROR TERMINATION" << "CANCELLED" << "RUNNING").setFlags(CProperty::Visible);
		addProperty("Plot File:" , CProperty::String)->setFlags(CProperty::Visible);
		addProperty("", CProperty::Action)->info = QString("Open in FEBio Studio");
		addProperty("", CProperty::Action)->info = QString("Open in PostView");
	}

	QVariant GetPropertyValue(int i) override
	{
		switch (i)
		{
		case 0: 
		{
			QString s = QString::fromStdString(m_job->GetFileName());
			if (s.isEmpty()) return "(none)";
			else return s;
		}
		break;
		case 1: return m_job->GetStatus(); break;
		case 2:
		{
			QString s = QString::fromStdString(m_job->GetPlotFileName());
			if (s.isEmpty()) return "(none)";
			else return s;
		}
		}

		return QVariant();
	}

	void SetPropertyValue(int i, const QVariant& v) override
	{
		if (i == 3)
		{
			// try to open the file
			if (m_job->OpenPlotFile() == false)
			{
				QMessageBox::critical(m_wnd, "FEBio Studio", "Failed to open the plot file.");
				return;
			}

			m_wnd->UpdatePostPanel();
			m_wnd->UpdatePostToolbar();
			m_wnd->SetActivePostDoc(m_job->GetPostDoc());

			SetModified(true);
		}
		else if (i == 4)
		{
			std::string plotFile = m_job->GetPlotFileName();
			plotFile = "file:///" + plotFile;

			// try to open the file
			QDesktopServices::openUrl(QUrl(QString::fromStdString(plotFile)));
		}
	}

private:
	CMainWindow*	m_wnd;
	CFEBioJob*		m_job;
};

//=============================================================================

CModelTree::CModelTree(CModelViewer* view, QWidget* parent) : QTreeWidget(parent), m_view(view)
{
//	setAlternatingRowColors(true);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
}

CModelTreeItem* CModelTree::GetCurrentData()
{
	QTreeWidgetItem* item = currentItem();
	if (item == 0) return 0;

	int n = item->data(0, Qt::UserRole).toInt();
	if ((n>=0)&&(n<m_data.size()))
	{
		return &(m_data[n]);
	}
	else
	{
		assert(false);
		return 0;
	}
}

QTreeWidgetItem* CModelTree::FindItem(FSObject* o)
{
	QTreeWidgetItemIterator it(this);
	while (*it)
	{
		QVariant data = (*it)->data(0, Qt::UserRole);
		int index = data.toInt();
		CModelTreeItem& item = m_data[index];

		if (((item.flag & OBJECT_NOT_EDITABLE) == 0) && (item.obj == o))
		{
			return *it;
		}

		++it;
	}

	return 0;
}

bool CModelTree::GetSelection(std::vector<FSObject*>& objList)
{
	objList.clear();

	QList<QTreeWidgetItem*> sel = selectedItems();
	if (sel.empty()) return false;

	CModelTreeItem* data = GetCurrentData();
	if (data == 0)
	{
		QTreeWidgetItem* it = sel.at(0);
		int index = it->data(0, Qt::UserRole).toInt();
		data = &GetItem(index);
	}

	if (sel.size() == 1)
	{
		objList.push_back(data->obj);
		return true;
	}
	else
	{
		int ntype = data->type;
		if (ntype == 0) return false;

		// only show the context menu if all objects are the same type
		QList<QTreeWidgetItem*>::iterator it = sel.begin();
		while (it != sel.end())
		{
			int index = (*it)->data(0, Qt::UserRole).toInt();
			CModelTreeItem& di = GetItem(index);

			if (di.type != ntype) 
			{
				objList.clear();
				return false;
			}

			objList.push_back(di.obj);

			++it;
		}

		return true;
	}
}

void CModelTree::contextMenuEvent(QContextMenuEvent* ev)
{
	QPoint pt = ev->globalPos();

	// clear the selection
	m_view->ClearSelection();

	QList<QTreeWidgetItem*> sel = selectedItems();
	if (sel.empty()) return;

	CModelTreeItem* data = GetCurrentData();
	if (data == 0)
	{
		QTreeWidgetItem* it = sel.at(0);
		int index = it->data(0, Qt::UserRole).toInt();
		data = &GetItem(index);
	}

	if (sel.size() == 1)
	{
		m_view->SetSelection(data->obj);
		m_view->ShowContextMenu(data, pt);
	}
	else
	{
		int ntype = data->type;
		if (ntype == 0) return;

		// only show the context menu if all objects are the same type
		vector<FSObject*> objList;
		QList<QTreeWidgetItem*>::iterator it = sel.begin();
		while (it != sel.end())
		{
			int index = (*it)->data(0, Qt::UserRole).toInt();
			CModelTreeItem& di = GetItem(index);

			if (di.type != ntype) return;

			objList.push_back(di.obj);

			++it;
		}

		// okay, we should only get here if the type is the same for all types
		m_view->SetSelection(objList);
		m_view->ShowContextMenu(data, pt);
	}
}

QTreeWidgetItem* CModelTree::AddTreeItem(QTreeWidgetItem* parent, const QString& name, int ntype, int ncount, FSObject* po, CPropertyList* props, CObjectValidator* val, int flags)
{
	QTreeWidgetItem* t2 = new QTreeWidgetItem(parent);

	QString txt;
	if (ncount == 0) txt = name;
	else txt = QString("%1 (%2)").arg(name).arg(ncount);
	t2->setText(0, txt);

	if (val && (val->IsValid() == false))
	{
		t2->setIcon(0, QIcon(":/icons/warning.png"));
		t2->setToolTip(0, QString("<font color=\"black\">") + val->GetErrorString());
		if (parent) parent->setExpanded(true);
	}

	t2->setData(0, Qt::UserRole, (int)m_data.size());

	CModelTreeItem it = { po, props, val, flags, ntype };
	m_data.push_back(it);

	return t2;
}
void CModelTree::ClearData()
{
	for (size_t i=0; i<m_data.size(); ++i) 
	{
		if (m_data[i].props) delete m_data[i].props;
		if (m_data[i].val  ) delete m_data[i].val;
	}
	m_data.clear();
}

void CModelTree::UpdateObject(FSObject* po)
{
	QTreeWidgetItemIterator it(this);
	while (*it)
	{
		QVariant data = (*it)->data(0, Qt::UserRole);
		int n = data.toInt();
		if (m_data[n].obj == po)
		{
			CObjectValidator* val = m_data[n].val;
			if (val)
			{
				if (val->IsValid() == false)
				{
					(*it)->setIcon(0, QIcon(":/icons/warning.png"));
					(*it)->setToolTip(0, QString("<font color=\"black\">") + val->GetErrorString());
				}
				else
				{
					(*it)->setIcon(0, QIcon());
					(*it)->setToolTip(0, QString());
				}
			}
			return;
		}
		++it;
	}
}

void CModelTree::ShowItem(QTreeWidgetItem* item)
{
	QTreeWidgetItem* parent = item->parent();
	while (parent)
	{
		if (parent->isExpanded() == false) parent->setExpanded(true);
		parent = parent->parent();
	}
}

void CModelTree::Select(FSObject* po)
{
	if (po == 0) { clearSelection(); return; }

	QTreeWidgetItemIterator it(this);
	while (*it)
	{
		QVariant data = (*it)->data(0, Qt::UserRole);
		int n = data.toInt();
		if (m_data[n].obj == po)
		{
			ShowItem(*it);
			this->setCurrentItem(*it);
			return;
		}
		++it;
	}

	assert(false);
}

void CModelTree::Select(const std::vector<FSObject*>& objList)
{
	clearSelection();
	m_view->SetCurrentItem(-1);

	int N = (int)objList.size();
	if (N == 0) return;

	QTreeWidgetItemIterator it(this);
	while (*it)
	{
		QVariant data = (*it)->data(0, Qt::UserRole);
		int index = data.toInt();
		CModelTreeItem& item = m_data[index];

		for (int i=0; i<N; ++i)
		{
			if ((item.obj == objList[i]) && ((item.flag & OBJECT_NOT_EDITABLE) == 0))
			{
				ShowItem(*it);
				setItemSelected(*it, true);
				break;
			}
		}
		++it;
	}
}

void CModelTree::Build(CDocument* doc)
{
	// clear the tree
	clear();
	ClearData();

	// get the model
	FEProject& prj = doc->GetProject();
	FEModel& fem = *doc->GetFEModel();
	GModel& mdl = fem.GetModel();

	QTreeWidgetItem* t1 = new QTreeWidgetItem(this);
	t1->setText(0, "Model");
	t1->setExpanded(true);
	t1->setData(0, Qt::UserRole, (int)m_data.size());

	CModelTreeItem it = {0, 0};
	m_data.push_back(it);

	// add data variables
	QTreeWidgetItem* t2 = AddTreeItem(t1, "Model Data", 0, 0, 0, new CObjectProps(&fem));
	UpdateModelData(t2, fem);

	// add the objects
	t2 = AddTreeItem(t1, "Geometry", MT_OBJECT_LIST, mdl.Objects());
	UpdateObjects(t2, fem);

	// add the groups
	int nsel = mdl.CountNamedSelections();
	t2 = AddTreeItem(t1, "Named Selections", MT_NAMED_SELECTION, nsel);
	UpdateGroups(t2, fem);

	// add the materials
	t2 = AddTreeItem(t1, "Materials", MT_MATERIAL_LIST, fem.Materials());
	UpdateMaterials(t2,fem);

	// add the boundary conditions
	int nbc = 0;
	for (int i=0; i<fem.Steps(); ++i) nbc += fem.GetStep(i)->BCs();
	t2 = AddTreeItem(t1, "Boundary Conditions", MT_BC_LIST, nbc);
	UpdateBC(t2, fem, 0);

	// add the boundary loads
	int nload = 0;
	for (int i=0; i<fem.Steps(); ++i) nload += fem.GetStep(i)->Loads();
	t2 = AddTreeItem(t1, "Loads", MT_LOAD_LIST, nload);
	UpdateLoads(t2, fem, 0);

	// add the initial conditions
	int nic = 0;
	for (int i = 0; i<fem.Steps(); ++i) nic += fem.GetStep(i)->ICs();
	t2 = AddTreeItem(t1, "Initial Conditions", MT_IC_LIST, nic);
	UpdateICs(t2, fem, 0);

	// add the interfaces
	int nint = 0;
	for (int i=0; i<fem.Steps(); ++i) nint += fem.GetStep(i)->Interfaces();
	t2 = AddTreeItem(t1, "Contact", MT_CONTACT_LIST, nint);
	UpdateContact(t2, fem, 0);

	// add the constraints
	int nlc = 0;
	for (int i=0; i<fem.Steps(); ++i) nlc += fem.GetStep(i)->RCs();
	t2 = AddTreeItem(t1, "Constraints", MT_CONSTRAINT_LIST, nlc);
	UpdateRC(t2, fem, 0);

	// add the connectors
	int nconn = 0;
	for (int i=0; i<fem.Steps(); ++i) nconn += fem.GetStep(i)->Connectors();
	t2 = AddTreeItem(t1, "Connectors", MT_CONNECTOR_LIST, nconn);
	UpdateConnectors(t2, fem, 0);

	// add the discrete objects
	t2 = AddTreeItem(t1, "Discrete", MT_DISCRETE_LIST, mdl.DiscreteObjects());
	UpdateDiscrete(t2, fem);

	// add the steps
	t2 = AddTreeItem(t1, "Steps", MT_STEP_LIST, fem.Steps() - 1, 0, 0, new CStepValidator(&fem));
	UpdateSteps(t2, fem);

	// add the output
	t2 = AddTreeItem(t1, "Output", MT_PROJECT_OUTPUT);
	UpdateOutput(t2, prj);

	// add the jobs
	if (doc->FEBioJobs())
	{
		QTreeWidgetItem* t1 = new QTreeWidgetItem(this);
		t1->setText(0, "Jobs");
		t1->setExpanded(true);
		t1->setData(0, Qt::UserRole, (int)m_data.size());

		CModelTreeItem it = { 0, 0 };
		m_data.push_back(it);

		UpdateJobs(t1, doc);
	}

	// add the image stacks
	if (doc->ImageObjects())
	{
		QTreeWidgetItem* t1 = new QTreeWidgetItem(this);
		t1->setText(0, "3D Images");
		t1->setExpanded(true);
		t1->setData(0, Qt::UserRole, (int)m_data.size());

		CModelTreeItem it = { 0, 0 };
		m_data.push_back(it);

		UpdateImages(t1, doc);
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateJobs(QTreeWidgetItem* t1, CDocument* doc)
{
	for (int i=0; i<doc->FEBioJobs(); ++i)
	{
		CFEBioJob* job = doc->GetFEBioJob(i);
		QTreeWidgetItem* t2 = AddTreeItem(t1, QString::fromStdString(job->GetName()), MT_JOB, 0, job, new CFEBioJobProps(m_view->GetMainWindow(), job), 0, SHOW_PROPERTY_FORM);

		CPostDoc* doc = job->GetPostDoc();
		if (doc)
		{
			Post::CGLModel* glm = doc->GetGLModel();
			if (glm)
			{
				Post::CGLColorMap* cm = glm->GetColorMap();
				if (cm)
				{
					AddTreeItem(t2, QString::fromStdString(cm->GetName()), MT_POST_PLOT, 0, cm, new CObjectProps(cm));
				}

				Post::CGLDisplacementMap* dm = glm->GetDisplacementMap();
				if (dm)
				{
					AddTreeItem(t2, QString::fromStdString(dm->GetName()), MT_POST_PLOT, 0, dm, new CObjectProps(dm));
				}

				int plots = glm->Plots();
				for (int j = 0; j < plots; ++j)
				{
					Post::CGLPlot* plt = glm->Plot(j);
					string name = plt->GetName();
					AddTreeItem(t2, QString::fromStdString(name), MT_POST_PLOT, 0, plt, new CObjectProps(plt));
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateImages(QTreeWidgetItem* t1, CDocument* doc)
{
	for (int i = 0; i < doc->ImageObjects(); ++i)
	{
		GImageObject* img = doc->GetImageObject(i);
		QTreeWidgetItem* t2 = AddTreeItem(t1, QString::fromStdString(img->GetName()), MT_3DIMAGE, 0, img, new CObjectProps(img), 0);

		Post::CImageModel* imgModel = img->GetImageModel();
		if (imgModel)
		{
			for (int j = 0; j < imgModel->ImageRenderers(); ++j)
			{
				Post::CGLImageRenderer* imgRender = imgModel->GetImageRenderer(j);
				AddTreeItem(t2, QString::fromStdString(imgRender->GetName()), MT_3DIMAGE_RENDER, 0, 0, new CObjectProps(imgRender), 0);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateModelData(QTreeWidgetItem* t1, FEModel& fem)
{
	int NSOL = fem.Solutes();
	if (NSOL > 0)
	{
		QTreeWidgetItem* t2 = AddTreeItem(t1, "Solutes", MT_SOLUTES_LIST, NSOL);
		for (int i=0; i<NSOL; ++i)
		{
			FESoluteData& s = fem.GetSoluteData(i);
			AddTreeItem(t2, QString::fromStdString(s.GetName()), MT_SOLUTE, 0, 0, new CObjectProps(&s));
		}	
	}

	int NSBM = fem.SBMs();
	if (NSBM > 0)
	{
		QTreeWidgetItem* t2 = AddTreeItem(t1, "Solid-bound Molecules", MT_SBM_LIST, NSBM);
		for (int i = 0; i<NSBM; ++i)
		{
			FESoluteData& s = fem.GetSBMData(i);
			AddTreeItem(t2, QString::fromStdString(s.GetName()), MT_SBM, 0, 0, new CObjectProps(&s));
		}
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateDiscrete(QTreeWidgetItem* t1, FEModel& fem)
{
	GModel& model = fem.GetModel();
	for (int i = 0; i<model.DiscreteObjects(); ++i)
	{
		GDiscreteObject* po = model.DiscreteObject(i);
		if (dynamic_cast<GDiscreteElementSet*>(po))
		{
			GDiscreteElementSet* pg = dynamic_cast<GDiscreteElementSet*>(po);
			QTreeWidgetItem* t2 = AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_DISCRETE_SET, pg->size(), pg, new CObjectProps(pg));
			for (int j = 0; j<pg->size(); ++j)
			{
				GDiscreteElement& el = pg->element(j);
				AddTreeItem(t2, QString::fromStdString(el.GetName()), MT_DISCRETE, 0, &el);
			}
		}
		else if (dynamic_cast<GDeformableSpring*>(po))
		{
			AddTreeItem(t1, QString::fromStdString(po->GetName()), MT_FEOBJECT, 0, po, new CObjectProps(po));
		}
		else AddTreeItem(t1, QString::fromStdString(po->GetName()));
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateObjects(QTreeWidgetItem* t1, FEModel& fem)
{
	QTreeWidgetItem* t2, *t3, *t4;

	// get the model
	GModel& model = fem.GetModel();

	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);

		t2 = AddTreeItem(t1, QString::fromStdString(po->GetName()), MT_OBJECT, 0, po, 0, new CGObjectValidator(po));

		if (po->IsVisible() == false) t2->setTextColor(0, Qt::gray);

		t3 = AddTreeItem(t2, "Parts", MT_PART_LIST, po->Parts(), po, 0, 0, OBJECT_NOT_EDITABLE);
		for (int j = 0; j<po->Parts(); ++j)
		{
			GPart* pg = po->Part(j);
			t4 = AddTreeItem(t3, QString::fromStdString(pg->GetName()), MT_PART, 0, pg);

			if (pg->IsVisible() == false)
			{
				t4->setTextColor(0, Qt::gray);
			}
		}
		t3->setExpanded(false);

		t3 = AddTreeItem(t2, "Surfaces", MT_FACE_LIST, po->Faces(), po, 0, 0, OBJECT_NOT_EDITABLE);
		for (int j = 0; j<po->Faces(); ++j)
		{
			GFace* pg = po->Face(j);
			t4 = AddTreeItem(t3, QString::fromStdString(pg->GetName()), MT_SURFACE, 0, pg);
			if (pg->IsVisible() == false) t4->setTextColor(0, Qt::gray);
		}
		t3->setExpanded(false);

		t3 = AddTreeItem(t2, "Edges", MT_EDGE_LIST, po->Edges(), po, 0, 0, OBJECT_NOT_EDITABLE);
		for (int j=0; j<po->Edges(); ++j)
		{
			GEdge* pg = po->Edge(j);
			t4 = AddTreeItem(t3, QString::fromStdString(pg->GetName()), MT_EDGE, 0, pg);
			if (pg->IsVisible() == false) t4->setTextColor(0, Qt::gray);
		}
		t3->setExpanded(false);

		t3 = AddTreeItem(t2, "Nodes", MT_NODE_LIST, po->Nodes(), po, 0, 0, OBJECT_NOT_EDITABLE);
		for (int j = 0; j<po->Nodes(); ++j)
		{
			GNode* pg = po->Node(j);
			if ((pg->Type() == 0) || (pg->Type() == NODE_VERTEX))
			{
				t4 = AddTreeItem(t3, QString::fromStdString(pg->GetName()), MT_NODE, 0, pg);
				if (pg->IsVisible() == false) t4->setTextColor(0, Qt::gray);
			}
		}
		t3->setExpanded(false);

		t2->setExpanded(false);
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateGroups(QTreeWidgetItem* t1, FEModel& fem)
{
	// get the model
	GModel& model = fem.GetModel();

	// add the groups
	int gparts = model.PartLists();
	for (int j = 0; j<gparts; ++j)
	{
		GPartList* pg = model.PartList(j);
		AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_PART_GROUP, 0, pg);
	}

	int gsurfs = model.FaceLists();
	for (int j = 0; j<gsurfs; ++j)
	{
		GFaceList* pg = model.FaceList(j);
		AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_FACE_GROUP, 0, pg);
	}

	int gedges = model.EdgeLists();
	for (int j = 0; j<gedges; ++j)
	{
		GEdgeList* pg = model.EdgeList(j);
		AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_EDGE_GROUP, 0, pg);
	}

	int gnodes = model.NodeLists();
	for (int j = 0; j<gnodes; ++j)
	{
		GNodeList* pg = model.NodeList(j);
		AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_NODE_GROUP, 0, pg);
	}

	// add the mesh groups
	// a - node sets
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FEMesh* pm = po->GetFEMesh();
		if (pm)
		{
			int nsets = po->FENodeSets();
			for (int j = 0; j<nsets; ++j)
			{
				FENodeSet* pg = po->GetFENodeSet(j);
				AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_NODE_GROUP, 0, pg);
			}
		}
	}

	// b - surfaces
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FEMesh* pm = po->GetFEMesh();
		if (pm)
		{
			int surfs = po->FESurfaces();
			for (int j = 0; j<surfs; ++j)
			{
				FESurface* pg = po->GetFESurface(j);
				AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_FACE_GROUP, 0, pg);
			}
		}
	}

	// c - edges
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FEMesh* pm = po->GetFEMesh();
		if (pm)
		{
			int edges = po->FEEdgeSets();
			for (int j = 0; j<edges; ++j)
			{
				FEEdgeSet* pg = po->GetFEEdgeSet(j);
				AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_EDGE_GROUP, 0, pg);
			}
		}
	}

	// d - parts
	for (int i = 0; i<model.Objects(); ++i)
	{
		GObject* po = model.Object(i);
		FEMesh* pm = po->GetFEMesh();
		if (pm)
		{
			int parts = po->FEParts();
			for (int j = 0; j<parts; ++j)
			{
				FEPart* pg = po->GetFEPart(j);
				AddTreeItem(t1, QString::fromStdString(pg->GetName()), MT_NODE_GROUP, 0, pg);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateBC(QTreeWidgetItem* t1, FEModel& fem, FEStep* pstep)
{
	QTreeWidgetItem* t2;

	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* ps = fem.GetStep(i);
		if ((pstep == 0) || (ps == pstep))
		{
			for (int j = 0; j<ps->BCs(); ++j)
			{
				FEBoundaryCondition* pbc = ps->BC(j);
				assert(pbc->GetStep() == ps->GetID());

				CPropertyList* pl = 0;

				if      (dynamic_cast<FEFixedDOF*     >(pbc)) pl = new CFixedDOFProps     (dynamic_cast<FEFixedDOF*     >(pbc));
				else pl = new FEObjectProps(pbc, &fem);

				int flags = SHOW_PROPERTY_FORM;
				if (pstep == 0) flags |= DUPLICATE_ITEM;
				t2 = AddTreeItem(t1, QString::fromStdString(pbc->GetName()), MT_BC, 0, pbc, pl, new CBCValidator(pbc), flags);
			}
		}
	}
}

//----------------------------------------------------------------------------
void CModelTree::UpdateLoads(QTreeWidgetItem* t1, FEModel& fem, FEStep* pstep)
{
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* ps = fem.GetStep(i);
		if ((pstep == 0) || (ps == pstep))
		{
			for (int j = 0; j<ps->Loads(); ++j)
			{
				FEBoundaryCondition* pfc = ps->Load(j);
				assert(pfc->GetStep() == ps->GetID());

				int flags = SHOW_PROPERTY_FORM;
				if (pstep == 0) flags |= DUPLICATE_ITEM;
				QTreeWidgetItem* t2 = AddTreeItem(t1, QString::fromStdString(pfc->GetName()), MT_LOAD, 0, pfc, new FEObjectProps(pfc, &fem), new CBCValidator(pfc), flags);

				AddDataMaps(t2, pfc);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateICs(QTreeWidgetItem* t1, FEModel& fem, FEStep* pstep)
{
	QTreeWidgetItem* t2;

	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* ps = fem.GetStep(i);
		if ((pstep == 0) || (ps == pstep))
		{
			for (int j = 0; j<ps->ICs(); ++j)
			{
				FEInitialCondition* pic = ps->IC(j);
				assert(pic->GetStep() == i);
				CPropertyList* pl = new FEObjectProps(pic, &fem);

				int flags = SHOW_PROPERTY_FORM;
				if (pstep == 0) flags |= DUPLICATE_ITEM;
				t2 = AddTreeItem(t1, QString::fromStdString(pic->GetName()), MT_IC, 0, pic, pl, new CBCValidator(pic), flags);
			}
		}
	}
}

void CModelTree::UpdateContact(QTreeWidgetItem* t1, FEModel& fem, FEStep* pstep)
{
	int n, i;
	QTreeWidgetItem* t2;

	// add the rigid interfaces
	for (n = 0; n<fem.Steps(); ++n)
	{
		FEStep* ps = fem.GetStep(n);
		if ((pstep == 0) || (pstep == ps))
		{
			int flags = 0;
			if (pstep == 0) flags |= DUPLICATE_ITEM;

			// add rigid interfaces
			for (i = 0; i<ps->Interfaces(); ++i)
			{
				FERigidInterface* pi = dynamic_cast<FERigidInterface*>(ps->Interface(i));
				if (pi)
				{
					t2 = AddTreeItem(t1, QString::fromStdString(pi->GetName()), MT_CONTACT, 0, pi, new CRigidInterfaceSettings(fem, pi), 0, flags);
//					if (pi->IsActive()) t2->labelfont(FL_HELVETICA); else t2->labelfont(FL_HELVETICA_ITALIC);
				}
			}

			// add the rigid joint
			for (i = 0; i<ps->Interfaces(); ++i)
			{
				FERigidJoint* pi = dynamic_cast<FERigidJoint*>(ps->Interface(i));
				if (pi)
				{
					t2 = AddTreeItem(t1, QString::fromStdString(pi->GetName()), MT_CONTACT, 0, pi, new CObjectProps(pi), 0, flags);
//					if (pi->IsActive()) t2->labelfont(FL_HELVETICA); else t2->labelfont(FL_HELVETICA_ITALIC);
				}
			}

			// add the rigid wall
			for (i = 0; i<ps->Interfaces(); ++i)
			{
				FERigidWallInterface* pi = dynamic_cast<FERigidWallInterface*>(ps->Interface(i));
				if (pi)
				{
					t2 = AddTreeItem(t1, QString::fromStdString(pi->GetName()), MT_CONTACT, 0, pi, new CObjectProps(pi), 0, flags);
//					if (pi->IsActive()) t2->labelfont(FL_HELVETICA); else t2->labelfont(FL_HELVETICA_ITALIC);
				}
			}

			// add the rigid sphere
			for (i = 0; i<ps->Interfaces(); ++i)
			{
				FERigidSphereInterface* pi = dynamic_cast<FERigidSphereInterface*>(ps->Interface(i));
				if (pi)
				{
					t2 = AddTreeItem(t1, QString::fromStdString(pi->GetName()), MT_CONTACT, 0, pi, new CObjectProps(pi), 0, flags);
//					if (pi->IsActive()) t2->labelfont(FL_HELVETICA); else t2->labelfont(FL_HELVETICA_ITALIC);
				}
			}

			// add the volume constraints
			for (i = 0; i<ps->Interfaces(); ++i)
			{
				FEVolumeConstraint* pi = dynamic_cast<FEVolumeConstraint*>(ps->Interface(i));
				if (pi)
				{
					t2 = AddTreeItem(t1, QString::fromStdString(pi->GetName()), MT_CONTACT, 0, pi, new CObjectProps(pi), 0, flags);
//					if (pi->IsActive()) t2->labelfont(FL_HELVETICA); else t2->labelfont(FL_HELVETICA_ITALIC);
				}
			}

			// add the symmetry planes
			for (i = 0; i<ps->Interfaces(); ++i)
			{
				FESymmetryPlane* pi = dynamic_cast<FESymmetryPlane*>(ps->Interface(i));
				if (pi)
				{
					t2 = AddTreeItem(t1, QString::fromStdString(pi->GetName()), MT_CONTACT, 0, pi, new CObjectProps(pi), 0, flags);
					// if (pi->IsActive()) t2->labelfont(FL_HELVETICA); else t2->labelfont(FL_HELVETICA_ITALIC);
				}
			}

            // add the constrained normal fluid flow surfaces
            for (i = 0; i<ps->Interfaces(); ++i)
            {
                FENormalFlowSurface* pi = dynamic_cast<FENormalFlowSurface*>(ps->Interface(i));
                if (pi)
                {
					t2 = AddTreeItem(t1, QString::fromStdString(pi->GetName()), MT_CONTACT, 0, pi, new CObjectProps(pi), 0, flags);
                    // if (pi->IsActive()) t2->labelfont(FL_HELVETICA); else t2->labelfont(FL_HELVETICA_ITALIC);
                }
            }
            
			// add the paired interfaces
			for (i = 0; i<ps->Interfaces(); ++i)
			{
				FEPairedInterface* pi = dynamic_cast<FEPairedInterface*>(ps->Interface(i));
				if (pi)
				{
					AddTreeItem(t1, QString::fromStdString(pi->GetName()), MT_CONTACT, 0, pi, new CObjectProps(pi), new CContactValidator(pi), flags);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateSteps(QTreeWidgetItem* t1, FEModel& fem)
{
	QTreeWidgetItem* t2, *t3;

	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* pstep = fem.GetStep(i);

		QString name = QString::fromStdString(pstep->GetName());

		// add control settings
		FEAnalysisStep* analysis = dynamic_cast<FEAnalysisStep*>(pstep);
		if (analysis)
		{
			t2 = AddTreeItem(t1, name, MT_STEP, 0, analysis, new CAnalysisTimeSettings(analysis), 0, 1);
		}
		else t2 = AddTreeItem(t1, name, 0, 0, pstep);

		// add the boundary conditions
		t3 = AddTreeItem(t2, "BCs", MT_BC_LIST, pstep->BCs());
		UpdateBC(t3, fem, pstep);

		// add the loads
		t3 = AddTreeItem(t2, "Loads", MT_LOAD_LIST, pstep->Loads());
		UpdateLoads(t3, fem, pstep);

		// add the interfaces
		t3 = AddTreeItem(t2, "Contact", MT_CONTACT_LIST, pstep->Interfaces());
		UpdateContact(t3, fem, pstep);

		// add the constraints
		t3 = AddTreeItem(t2, "Constraints", MT_CONSTRAINT_LIST, pstep->RCs());
		UpdateRC(t3, fem, pstep);

		// add the connectors
		t3 = AddTreeItem(t2, "Connectors", MT_CONNECTOR_LIST, pstep->Connectors());
		UpdateConnectors(t3, fem, pstep);
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateRC(QTreeWidgetItem* t1, FEModel& fem, FEStep* pstep)
{
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* ps = fem.GetStep(i);
		if ((pstep == 0) || (ps == pstep))
		{
			for (int j = 0; j<ps->RCs(); ++j)
			{
				FERigidConstraint* prc = ps->RC(j);

				CPropertyList* pl = new CRigidConstraintSettings(fem, prc);

				int flags = SHOW_PROPERTY_FORM;
				if (pstep) flags |= DUPLICATE_ITEM;
				AddTreeItem(t1, QString::fromStdString(prc->GetName()), MT_CONSTRAINT, 0, prc, pl, new CRigidConstraintValidator(prc), flags);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateConnectors(QTreeWidgetItem* t1, FEModel& fem, FEStep* pstep)
{
	for (int i = 0; i<fem.Steps(); ++i)
	{
		FEStep* ps = fem.GetStep(i);
		if ((pstep == 0) || (ps == pstep))
		{
			for (int j = 0; j<ps->Connectors(); ++j)
			{
				FEConnector* prc = ps->Connector(j);
				CPropertyList* pl = new CRigidConnectorSettings(fem, prc);

				int flags = SHOW_PROPERTY_FORM;
				if (pstep) flags |= DUPLICATE_ITEM;
				AddTreeItem(t1, QString::fromStdString(prc->GetName()), MT_CONNECTOR, 0, prc, pl, 0, flags);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateMaterials(QTreeWidgetItem* t1, FEModel& fem)
{
	for (int i = 0; i<fem.Materials(); ++i)
	{
		GMaterial* pm = fem.GetMaterial(i);
		FEMaterial* mat = pm->GetMaterialProperties();
		QString name = QString("%1 [%2]").arg(QString::fromStdString(pm->GetName())).arg(mat->TypeStr());
		AddMaterial(t1, name, pm, mat, fem, true);
	}
}

//-----------------------------------------------------------------------------
void CModelTree::AddMaterial(QTreeWidgetItem* item, const QString& name, GMaterial* gmat, FEMaterial* pmat, FEModel& fem, bool topLevel)
{
	// reaction materials's structure is somewhat cumbersome, so we provide an alternative representation
	if (dynamic_cast<FEReactionMaterial*>(pmat))
	{
		AddReactionMaterial(item, dynamic_cast<FEReactionMaterial*>(pmat), fem);
		return;
	}

	// create the tree widget item
	QTreeWidgetItem* t2 = 0;
	if (topLevel)
		t2 = AddTreeItem(item, name, MT_MATERIAL, 0, gmat, new CMaterialProps(fem, gmat), new CMaterialValidator(&fem, gmat));
	else
		t2 = AddTreeItem(item, name, 0, 0, pmat, new FEObjectProps(pmat, &fem));

	// fill the child items
	if (pmat)
	{
		for (int i = 0; i<pmat->Properties(); ++i)
		{
			FEMaterialProperty& p = pmat->GetProperty(i);
			QString propName = QString::fromStdString(p.GetName());
			if (p.Size() == 1)
			{
				FEMaterial* pj = p.GetMaterial();
				if (pj)
				{
					QString typeName = (pj->TypeStr() ? QString(pj->TypeStr()) : "error");
					QString name;
					if (pj->GetName().empty())
					{
						if (propName == typeName)
							name = QString("%1").arg(propName);
						else
							name = QString("%1 [%2]").arg(propName).arg(typeName);
					}
					else name = QString("%1 [%2]").arg(propName).arg(QString::fromStdString(pj->GetName()));
					AddMaterial(t2, name, 0, pj, fem, false);
				}
			}
			else
			{
				for (int j = 0; j<p.Size(); ++j)
				{
					FEMaterial* pj = p.GetMaterial(j);
					if (pj)
					{
						QString typeName = (pj->TypeStr() ? QString(pj->TypeStr()) : "error");
						QString name;
						if (pj->GetName().empty())
						{
							if (propName == typeName)
								name = QString("%1:%2").arg(propName).arg(j + 1);
							else
								name = QString("%1:%2 [%3]").arg(propName).arg(j + 1).arg(pj->TypeStr());
						}
						else name = QString("%1:%2 [%3]").arg(propName).arg(j + 1).arg(QString::fromStdString(pj->GetName()));
						AddMaterial(t2, name, 0, pj, fem, false);
					}
				}
			}
		}

		// add the material maps
		AddDataMaps(t2, pmat);
	}
}

//-----------------------------------------------------------------------------
void CModelTree::AddDataMaps(QTreeWidgetItem* t2, FEComponent* pc)
{
	int N = pc->DataMaps();
	for (int n = 0; n<N; ++n)
	{
		FEDataMap* map = pc->GetDataMap(n);
		AddTreeItem(t2, QString::fromStdString(map->GetName()), MT_DATAMAP, 0, map, new CDataMapProps(map), 0, SHOW_PROPERTY_FORM | NAME_NOT_EDITABLE);
	}
}

//-----------------------------------------------------------------------------
void CModelTree::AddReactionMaterial(QTreeWidgetItem* item, FEReactionMaterial* mat, FEModel& fem)
{
	// add reaction material first
	QString name;
	if (mat->GetName().empty() == false)
	{
		name = QString("reaction: %1").arg(QString::fromStdString(mat->GetName()));
	}
	else
	{
		string eq = buildReactionEquation(mat, fem);
		name = QString("reaction [%1]").arg(QString::fromStdString(eq));
	}
	QTreeWidgetItem* t2 = AddTreeItem(item, name, 0, 0, 0, 0);

	// add forward rate
	FEMaterial* fwd = mat->GetForwardRate();
	if (fwd) AddMaterial(t2, QString("%1 [%2]").arg("forward rate").arg(fwd->TypeStr()), 0, fwd, fem, false);

	// add reverse rate
	FEMaterial* rev = mat->GetReverseRate();
	if (rev) AddMaterial(t2, QString("%1 [%2]").arg("reverse rate").arg(rev->TypeStr()), 0, rev, fem, false);

	// add reactants and products
	AddTreeItem(t2, "reactants", 0, 0, 0, new CReactionReactantProperties(mat, fem));
	AddTreeItem(t2, "products", 0, 0, 0, new CReactionProductProperties(mat, fem));
}

//-----------------------------------------------------------------------------
void CModelTree::UpdateOutput(QTreeWidgetItem* t1, FEProject& prj)
{
	AddTreeItem(t1, "plotfile", MT_PROJECT_OUTPUT, 0, 0, new CPlotfileProperties(prj), 0, 1);
	AddTreeItem(t1, "logfile", MT_PROJECT_OUTPUT_LOG, 0, 0, new CLogfileProperties(prj), 0, 1);
}
