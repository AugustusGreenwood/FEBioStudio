#include "stdafx.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "DlgFind.h"
#include "DlgSelectRange.h"
#include "DlgTransform.h"
#include "ModelDocument.h"
#include "DlgCloneObject.h"
#include "DlgCloneGrid.h"
#include "DlgCloneRevolve.h"
#include "DlgMergeObjects.h"
#include "DlgDetachSelection.h"
#include "DlgPurge.h"
#include "DlgEditProject.h"
#include "PostDocument.h"
#include "Commands.h"
#include <MeshTools/GModel.h>
#include <QMessageBox>
#include <QInputDialog>
#include <GeomLib/GPrimitive.h>
#include <PostGL/GLModel.h>

void CMainWindow::on_actionUndo_triggered()
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	if (doc->CanUndo())
	{
		doc->UndoCommand();
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionRedo_triggered()
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	if (doc->CanRedo())
	{
		doc->RedoCommand();
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionInvertSelection_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FESelection* ps = doc->GetCurrentSelection();
	if (ps)
	{
		doc->DoCommand(new CCmdInvertSelection(doc));
		Update();
	}
}

void CMainWindow::on_actionClearSelection_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FESelection* ps = doc->GetCurrentSelection();
	if (ps && ps->Size())
	{
		int item = doc->GetItemMode();
		int nsel = doc->GetSelectionMode();
		GModel* mdl = doc->GetGModel();
		GObject* po = doc->GetActiveObject();
		FEMesh* pm = (po ? po->GetFEMesh() : 0);
		FEMeshBase* pmb = (po ? po->GetEditableMesh() : 0);
		FELineMesh* pml = (po ? po->GetEditableLineMesh() : 0);
		switch (item)
		{
		case ITEM_MESH:
		{
			switch (nsel)
			{
			case SELECT_OBJECT: doc->DoCommand(new CCmdSelectObject(mdl, 0, false)); break;
			case SELECT_PART: doc->DoCommand(new CCmdSelectPart(mdl, 0, 0, false)); break;
			case SELECT_FACE: doc->DoCommand(new CCmdSelectSurface(mdl, 0, 0, false)); break;
			case SELECT_EDGE: doc->DoCommand(new CCmdSelectEdge(mdl, 0, 0, false)); break;
			case SELECT_NODE: doc->DoCommand(new CCmdSelectNode(mdl, 0, 0, false)); break;
			case SELECT_DISCRETE: doc->DoCommand(new CCmdSelectDiscrete(mdl, 0, 0, false)); break;
			}
		}
		break;
		case ITEM_ELEM: doc->DoCommand(new CCmdSelectElements(pm, 0, 0, false)); break;
		case ITEM_FACE: doc->DoCommand(new CCmdSelectFaces(pmb, 0, 0, false)); break;
		case ITEM_EDGE: doc->DoCommand(new CCmdSelectFEEdges(pml, 0, 0, false)); break;
		case ITEM_NODE: doc->DoCommand(new CCmdSelectFENodes(pml, 0, 0, false)); break;
		}

		Update();
	}
}

void CMainWindow::on_actionDeleteSelection_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	// give the build panel a chance to process this event first
	if (ui->buildPanel->OnDeleteEvent())
	{
		return;
	}

	// see if the focus is on the model viewer
	if (ui->modelViewer->IsFocus())
	{
		if (ui->modelViewer->OnDeleteEvent())
			return;
	}

	FESelection* psel = doc->GetCurrentSelection();
	if (psel == 0) return;

	if (dynamic_cast<GDiscreteSelection*>(psel))
	{
		GDiscreteSelection* pds = dynamic_cast<GDiscreteSelection*>(psel);
		CCmdGroup* pcmd = new CCmdGroup("Delete Discrete");
		FEModel* ps = doc->GetFEModel();
		GModel& model = ps->GetModel();
		for (int i = 0; i<model.DiscreteObjects(); ++i)
		{
			GDiscreteObject* po = model.DiscreteObject(i);
			if (po->IsSelected())
				pcmd->AddCommand(new CCmdDeleteDiscreteObject(&model, po));
		}
		doc->DoCommand(pcmd);

		for (int i = 0; i<model.DiscreteObjects(); ++i)
		{
			GDiscreteElementSet* pds = dynamic_cast<GDiscreteElementSet*>(model.DiscreteObject(i));
			if (pds)
			{
				for (int i = 0; i<pds->size();)
				{
					if (pds->element(i).IsSelected()) pds->RemoveElement(i);
					else i++;
				}
			}
		}
	}
	else if (dynamic_cast<GPartSelection*>(psel))
	{
		GModel& m = *doc->GetGModel();
		GPartSelection* sel = dynamic_cast<GPartSelection*>(psel);
		int n = sel->Count();
		if (n == 0) return;
		GPartSelection::Iterator it(sel);
		vector<int> pid(n);
		for (int i = 0; i<n; ++i, ++it)
		{
			pid[i] = it->GetID();
		}

		for (int i = 0; i<n; ++i)
		{
			GPart* pg = m.FindPart(pid[i]); assert(pg);
			if (pg)
			{
				m.DeletePart(pg);
			}
		}
	}
	else
	{
		int item = doc->GetItemMode();
		if (item == ITEM_MESH)
		{
			int nsel = doc->GetSelectionMode();
			if (nsel == SELECT_OBJECT)
			{
				CCmdGroup* pcmd = new CCmdGroup("Delete");
				FEModel* ps = doc->GetFEModel();
				GModel& model = ps->GetModel();
				for (int i = 0; i<model.Objects(); ++i)
				{
					GObject* po = model.Object(i);
					if (po->IsSelected())
						pcmd->AddCommand(new CCmdDeleteGObject(&model, po));
				}
				doc->DoCommand(pcmd);
			}
			else
			{
				QMessageBox::information(this, "FEBio Studio", "Cannot delete this selection.");
			}
		}
		else
		{
			GObject* po = doc->GetActiveObject();
			if (po == 0) return;

			GMeshObject* pgo = dynamic_cast<GMeshObject*>(po);
			if (pgo && pgo->GetFEMesh()) doc->DoCommand(new CCmdDeleteFESelection(pgo));

			GSurfaceMeshObject* pso = dynamic_cast<GSurfaceMeshObject*>(po);
			if (pso && pso->GetSurfaceMesh()) doc->DoCommand(new CCmdDeleteFESurfaceSelection(pso));

			GPrimitive* pp = dynamic_cast<GPrimitive*>(po);
			if (pp)
			{
				QMessageBox::information(this, "FEBio Studio", "Cannot delete mesh selections of a primitive object.");
			}
		}
	}
	Update(0, true);
	ClearStatusMessage();
}

void CMainWindow::on_actionHideSelection_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc)
	{
		Post::CGLModel& mdl = *postDoc->GetGLModel();
		switch (mdl.GetSelectionMode())
		{
		case Post::SELECT_NODES: mdl.HideSelectedNodes(); break;
		case Post::SELECT_EDGES: mdl.HideSelectedEdges(); break;
		case Post::SELECT_FACES: mdl.HideSelectedFaces(); break;
		case Post::SELECT_ELEMS: mdl.HideSelectedElements(); break;
		}
		postDoc->UpdateFEModel();
		RedrawGL();
	}
	else
	{
		CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());

		doc->HideCurrentSelection();
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionHideUnselected_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc)
	{
		Post::CGLModel& mdl = *postDoc->GetGLModel();
		mdl.HideUnselectedElements();
		postDoc->UpdateFEModel();
		RedrawGL();
	}
	else
	{
		CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
		doc->HideUnselected();
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionUnhideAll_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc)
	{
		Post::CGLModel& mdl = *postDoc->GetGLModel();
		mdl.UnhideAll();
		postDoc->UpdateFEModel();
		RedrawGL();
	}
	else
	{
		CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
		doc->DoCommand(new CCmdUnhideAll(doc));
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionFind_triggered()
{
	CPostDocument* doc = GetPostDocument();
	if (doc == nullptr) return;
	if (doc->IsValid() == false) return;

	Post::CGLModel* model = doc->GetGLModel(); assert(model);
	if (model == 0) return;

	int nview = model->GetSelectionMode();
	int nsel = 0;
	if (nview == Post::SELECT_NODES) nsel = 0;
	if (nview == Post::SELECT_EDGES) nsel = 1;
	if (nview == Post::SELECT_FACES) nsel = 2;
	if (nview == Post::SELECT_ELEMS) nsel = 3;

	CDlgFind dlg(this, nsel);

	if (dlg.exec())
	{
		Post::CGLModel* pm = doc->GetGLModel();

		if (dlg.m_bsel[0]) nview = Post::SELECT_NODES;
		if (dlg.m_bsel[1]) nview = Post::SELECT_EDGES;
		if (dlg.m_bsel[2]) nview = Post::SELECT_FACES;
		if (dlg.m_bsel[3]) nview = Post::SELECT_ELEMS;

		CGLControlBar* pb = ui->glc;
		switch (nview)
		{
		case Post::SELECT_NODES: pb->SetMeshItem(ITEM_NODE); pm->SelectNodes(dlg.m_item, dlg.m_bclear); break;
		case Post::SELECT_EDGES: pb->SetMeshItem(ITEM_EDGE); pm->SelectEdges(dlg.m_item, dlg.m_bclear); break;
		case Post::SELECT_FACES: pb->SetMeshItem(ITEM_FACE); pm->SelectFaces(dlg.m_item, dlg.m_bclear); break;
		case Post::SELECT_ELEMS: pb->SetMeshItem(ITEM_ELEM); pm->SelectElements(dlg.m_item, dlg.m_bclear); break;
		}

		doc->GetGLModel()->UpdateSelectionLists();
		ReportSelection();
		RedrawGL();
	}
}

void CMainWindow::on_actionSelectRange_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc == nullptr) return;
	if (!postDoc->IsValid()) return;

	Post::CGLModel* model = postDoc->GetGLModel(); assert(model);
	if (model == 0) return;

	Post::CGLColorMap* pcol = postDoc->GetGLModel()->GetColorMap();
	if (pcol == 0) return;

	float d[2];
	pcol->GetRange(d);

	CDlgSelectRange dlg(this);
	dlg.m_min = d[0];
	dlg.m_max = d[1];

	if (dlg.exec())
	{
		CDocument* doc = GetDocument();
		switch (model->GetSelectionMode())
		{
		case Post::SELECT_NODES: doc->SetItemMode(ITEM_NODE); model->SelectNodesInRange(dlg.m_min, dlg.m_max, dlg.m_brange); break;
		case Post::SELECT_EDGES: doc->SetItemMode(ITEM_EDGE); model->SelectEdgesInRange(dlg.m_min, dlg.m_max, dlg.m_brange); break;
		case Post::SELECT_FACES: doc->SetItemMode(ITEM_FACE); model->SelectFacesInRange(dlg.m_min, dlg.m_max, dlg.m_brange); break;
		case Post::SELECT_ELEMS: doc->SetItemMode(ITEM_ELEM); model->SelectElemsInRange(dlg.m_min, dlg.m_max, dlg.m_brange); break;
		}

		model->UpdateSelectionLists();
		postDoc->UpdateFEModel();
		ReportSelection();
		UpdateGLControlBar();
		RedrawGL();
	}
}

void CMainWindow::on_actionToggleVisible_triggered()
{
	CPostDocument* postDoc = GetPostDocument();
	if (postDoc)
	{
		Post::CGLModel& mdl = *postDoc->GetGLModel();
		mdl.ToggleVisibleElements();
		postDoc->UpdateFEModel();
		RedrawGL();
	}
	else
	{
		CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());

		int nsel = doc->GetSelectionMode();
		int nitem = doc->GetItemMode();

		GModel* mdl = doc->GetGModel();

		CCommand* cmd = 0;
		if (nitem == ITEM_MESH)
		{
			switch (nsel)
			{
			case SELECT_OBJECT: cmd = new CCmdToggleObjectVisibility(mdl); break;
			case SELECT_PART: cmd = new CCmdTogglePartVisibility(mdl); break;
			case SELECT_DISCRETE: cmd = new CCmdToggleDiscreteVisibility(mdl); break;
			}
		}
		else
		{
			GObject* po = doc->GetActiveObject();
			if (po == 0) return;

			FEMesh* pm = po->GetFEMesh();
			FEMeshBase* pmb = po->GetEditableMesh();

			switch (nitem)
			{
			case ITEM_ELEM: if (pm) cmd = new CCmdToggleElementVisibility(pm); break;
			case ITEM_FACE: if (pmb) cmd = new CCmdToggleFEFaceVisibility(pm); break;
			}
		}

		if (cmd)
		{
			doc->DoCommand(cmd);
			UpdateModel();
			RedrawGL();
		}
	}
}

void CMainWindow::on_actionNameSelection_triggered()
{
	static int nparts = 1;
	static int nsurfs = 1;
	static int nedges = 1;
	static int nnodes = 1;

	char szname[256] = { 0 };

	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FEModel* pfem = doc->GetFEModel();
	GModel* mdl = doc->GetGModel();

	// make sure there is a selection
	FESelection* psel = doc->GetCurrentSelection();
	if (psel->Size() == 0) return;

	// set the name
	int item = doc->GetItemMode();
	switch (item)
	{
	case ITEM_ELEM: sprintf(szname, "Part%02d", nparts); break;
	case ITEM_FACE: sprintf(szname, "Surface%02d", nsurfs); break;
	case ITEM_EDGE: sprintf(szname, "EdgeSet%02d", nedges); break;
	case ITEM_NODE: sprintf(szname, "Nodeset%02d", nnodes); break;
	case ITEM_MESH:
	{
		int nsel = doc->GetSelectionMode();
		switch (nsel)
		{
		case SELECT_PART: sprintf(szname, "Part%02d", nparts); break;
		case SELECT_FACE: sprintf(szname, "Surface%02d", nsurfs); break;
		case SELECT_EDGE: sprintf(szname, "EdgeSet%02d", nedges); break;
		case SELECT_NODE: sprintf(szname, "Nodeset%02d", nnodes); break;
		default:
			return;
		}
	}
	break;
	}

	bool ok;
	QString text = QInputDialog::getText(this, "Name Selection", "Name:", QLineEdit::Normal, szname, &ok);

	if (ok && !text.isEmpty())
	{
		string sname = text.toStdString();
		const char* szname = sname.c_str();

		GObject* po = doc->GetActiveObject();

		// create a new group
		switch (item)
		{
		case ITEM_ELEM:
		{
			assert(po);
			FEElementSelection* pes = dynamic_cast<FEElementSelection*>(psel); assert(pes);
			FEPart* pg = dynamic_cast<FEPart*>(pes->CreateItemList());
			pg->SetName(szname);
			doc->DoCommand(new CCmdAddPart(po, pg));
			++nparts;
			UpdateModel(pg);
		}
		break;
		case ITEM_FACE:
		{
			assert(po);
			FEFaceSelection* pfs = dynamic_cast<FEFaceSelection*>(psel);
			FESurface* pg = dynamic_cast<FESurface*>(pfs->CreateItemList());
			pg->SetName(szname);
			doc->DoCommand(new CCmdAddSurface(po, pg));
			++nsurfs;
			UpdateModel(pg);
		}
		break;
		case ITEM_EDGE:
		{
			assert(po);
			FEEdgeSelection* pes = dynamic_cast<FEEdgeSelection*>(psel);
			FEEdgeSet* pg = dynamic_cast<FEEdgeSet*>(pes->CreateItemList());
			pg->SetName(szname);
			doc->DoCommand(new CCmdAddFEEdgeSet(po, pg));
			++nsurfs;
			UpdateModel(pg);
		}
		break;
		case ITEM_NODE:
		{
			assert(po);
			FENodeSelection* pns = dynamic_cast<FENodeSelection*>(psel);
			FENodeSet* pg = dynamic_cast<FENodeSet*>(pns->CreateItemList());
			pg->SetName(szname);
			doc->DoCommand(new CCmdAddNodeSet(po, pg));
			++nnodes;
			UpdateModel(pg);
		}
		break;
		case ITEM_MESH:
		{
			int nsel = doc->GetSelectionMode();
			switch (nsel)
			{
			case SELECT_PART:
			{
				GPartList* pg = new GPartList(pfem, dynamic_cast<GPartSelection*>(psel));
				pg->SetName(szname);
				doc->DoCommand(new CCmdAddGPartGroup(mdl, pg));
				++nparts;
				UpdateModel(pg);
			}
			break;
			case SELECT_FACE:
			{
				GFaceList* pg = new GFaceList(pfem, dynamic_cast<GFaceSelection*>(psel));
				pg->SetName(szname);
				doc->DoCommand(new CCmdAddGFaceGroup(mdl, pg));
				++nsurfs;
				UpdateModel(pg);
			}
			break;
			case SELECT_EDGE:
			{
				GEdgeList* pg = new GEdgeList(pfem, dynamic_cast<GEdgeSelection*>(psel));
				pg->SetName(szname);
				doc->DoCommand(new CCmdAddGEdgeGroup(mdl, pg));
				++nedges;
				UpdateModel(pg);
			}
			break;
			case SELECT_NODE:
			{
				GNodeList* pg = new GNodeList(pfem, dynamic_cast<GNodeSelection*>(psel));
				pg->SetName(szname);
				doc->DoCommand(new CCmdAddGNodeGroup(mdl, pg));
				++nnodes;
				UpdateModel(pg);
			}
			break;
			}
		}
		}
	}
}

void CMainWindow::on_actionTransform_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FESelection* ps = doc->GetCurrentSelection();

	if (ps && ps->Size())
	{
		vec3d pos = ps->GetPivot();
		quatd rot = ps->GetOrientation();
		vec3d r = rot.GetVector()*(180 * rot.GetAngle() / PI);
		vec3d scl = ps->GetScale();

		CDlgTransform dlg(this);
		dlg.m_pos = pos;
		dlg.m_relPos = vec3d(0, 0, 0);

		dlg.m_rot = r;
		dlg.m_relRot = vec3d(0, 0, 0);

		dlg.m_scl = scl;
		dlg.m_relScl = vec3d(1, 1, 1);

		dlg.Init();

		if (dlg.exec())
		{
			CCmdGroup* pcmd = new CCmdGroup("Transform");

			// translation
			vec3d dr = dlg.m_pos - pos + dlg.m_relPos;
			pcmd->AddCommand(new CCmdTranslateSelection(doc, dr));

			// rotation
			vec3d r = dlg.m_rot;
			double w = PI*r.Length() / 180;
			r.Normalize();
			rot = quatd(w, r)*rot.Inverse();

			r = dlg.m_relRot;
			w = PI*r.Length() / 180;
			r.Normalize();
			rot = quatd(w, r)*rot;

			pcmd->AddCommand(new CCmdRotateSelection(doc, rot, ui->glview->GetPivotPosition()));

			// scale
			vec3d s1(1, 0, 0);
			vec3d s2(0, 1, 0);
			vec3d s3(0, 0, 1);

			r = ui->glview->GetPivotPosition();

			pcmd->AddCommand(new CCmdScaleSelection(doc, dlg.m_scl.x*dlg.m_relScl.x / scl.x, s1, r));
			pcmd->AddCommand(new CCmdScaleSelection(doc, dlg.m_scl.y*dlg.m_relScl.y / scl.y, s2, r));
			pcmd->AddCommand(new CCmdScaleSelection(doc, dlg.m_scl.z*dlg.m_relScl.z / scl.z, s3, r));

			doc->DoCommand(pcmd);
			UpdateGLControlBar();
			RedrawGL();
		}
	}
}

void CMainWindow::on_actionCollapseTransform_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	GObject* po = doc->GetActiveObject();
	if (po == 0) QMessageBox::critical(this, "FEBio Studio", "Please select an object");
	else
	{
		po->CollapseTransform();
		UpdateModel(po);
		RedrawGL();
	}
}

void CMainWindow::on_actionClone_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	// get the active object
	GObject* po = doc->GetActiveObject();
	if (po == 0)
	{
		QMessageBox::critical(this, "FEBio Studio", "You need to select an object first.");
		return;
	}

	CDlgCloneObject dlg(this);
	if (dlg.exec())
	{
		// get the model
		GModel& m = *doc->GetGModel();

		// clone the object
		GObject* pco = m.CloneObject(po);
		if (pco == 0)
		{
			QMessageBox::critical(this, "FEBio Studio", "Could not clone this object.");
			return;
		}

		// set the name
		QString name = dlg.GetNewObjectName();
		std::string sname = name.toStdString();
		pco->SetName(sname);

		// add and select the new object
		doc->DoCommand(new CCmdAddAndSelectObject(&m, pco));

		// update windows
		Update(0, true);
	}
}

void CMainWindow::on_actionCloneGrid_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	// get the active object
	GObject* po = doc->GetActiveObject();
	if (po == 0)
	{
		QMessageBox::critical(this, "FEBio Studio", "You need to select an object first.");
		return;
	}

	CDlgCloneGrid dlg(this);
	if (dlg.exec())
	{
		GModel& m = *doc->GetGModel();

		// clone the object
		vector<GObject*> newObjects = m.CloneGrid(po, dlg.m_rangeX[0], dlg.m_rangeX[1], dlg.m_rangeY[0], dlg.m_rangeY[1], dlg.m_rangeZ[0], dlg.m_rangeZ[1], dlg.m_inc[0], dlg.m_inc[1], dlg.m_inc[2]);
		if (newObjects.empty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Failed to grid clone this object");
			return;
		}

		// add all the objects
		CCmdGroup* cmd = new CCmdGroup("Clone grid");
		for (int i = 0; i<(int)newObjects.size(); ++i)
		{
			cmd->AddCommand(new CCmdAddObject(&m, newObjects[i]));
		}
		doc->DoCommand(cmd);

		// update UI
		Update(0, true);
	}
}

void CMainWindow::on_actionCloneRevolve_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	// get the active object
	GObject* po = doc->GetActiveObject();
	if (po == 0)
	{
		QMessageBox::critical(this, "FEBio Studio", "You need to select an object first.");
		return;
	}

	CDlgCloneRevolve dlg(this);
	if (dlg.exec())
	{
		GModel& m = *doc->GetGModel();

		vector<GObject*> newObjects = m.CloneRevolve(po, dlg.m_count, dlg.m_range, dlg.m_spiral, dlg.m_center, dlg.m_axis, dlg.m_rotateClones);
		if (newObjects.empty())
		{
			QMessageBox::critical(this, "FEBio Studio", "Failed to revolve clone this object");
			return;
		}

		// add all the objects
		CCmdGroup* cmd = new CCmdGroup("Clone revolve");
		for (int i = 0; i<(int)newObjects.size(); ++i)
		{
			cmd->AddCommand(new CCmdAddObject(&m, newObjects[i]));
		}
		doc->DoCommand(cmd);

		// update UI
		Update(0, true);
	}
}

void CMainWindow::on_actionMerge_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	CDlgMergeObjects dlg(this);
	if (dlg.exec() == QDialog::Rejected) return;

	// make sure we have an object selection
	FESelection* currentSelection = doc->GetCurrentSelection();
	if (currentSelection->Type() != SELECT_OBJECTS)
	{
		QMessageBox::critical(this, "Merge Objects", "Cannot merge objects");
		return;
	}
	GObjectSelection* sel = dynamic_cast<GObjectSelection*>(currentSelection);

	// merge the objects
	GModel& m = *doc->GetGModel();
	GObject* newObject = m.MergeSelectedObjects(sel, dlg.m_name, dlg.m_weld, dlg.m_tol);
	if (newObject == 0)
	{
		QMessageBox::critical(this, "Merge Objects", "Cannot merge objects");
		return;
	}

	// we need to delete the selected objects and add the new object
	// create the command that will do the attaching
	CCmdGroup* pcmd = new CCmdGroup("Attach");
	for (int i = 0; i<sel->Count(); ++i)
	{
		// remove the old object
		GObject* po = sel->Object(i);
		pcmd->AddCommand(new CCmdDeleteGObject(&m, po));
	}
	// add the new object
	pcmd->AddCommand(new CCmdAddAndSelectObject(&m, newObject));

	// perform the operation
	doc->DoCommand(pcmd);

	// update UI
	Update(0, true);
}

void CMainWindow::on_actionDetach_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FESelection* sel = doc->GetCurrentSelection();
	if ((sel == 0) || (sel->Size() == 0) || (sel->Type() != SELECT_FE_ELEMENTS))
	{
		QMessageBox::warning(this, "Detach Selection", "Cannot detach this selection");
		return;
	}

	CDlgDetachSelection dlg(this);
	if (dlg.exec())
	{
		GMeshObject* po = dynamic_cast<GMeshObject*>(doc->GetActiveObject()); assert(po);
		if (po == 0) return;

		// create a new object for this mesh
		GMeshObject* newObject = po->DetachSelection();

		// give the object a new name
		string newName = dlg.getName().toStdString();
		newObject->SetName(newName);

		// add it to the pile
		doc->DoCommand(new CCmdAddObject(doc->GetGModel(), newObject));

		UpdateModel(newObject, true);
	}
}


void CMainWindow::on_actionExtract_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FESelection* sel = doc->GetCurrentSelection();
	if ((sel == 0) || (sel->Size() == 0) || (sel->Type() != SELECT_FE_FACES))
	{
		QMessageBox::warning(this, "Extract Selection", "Cannot extract this selection");
		return;
	}

	CDlgExtractSelection dlg(this);
	if (dlg.exec())
	{
		GObject* po = doc->GetActiveObject();
		if (po == 0) return;

		// create a new object for this mesh
		GMeshObject* newObject = ExtractSelection(po);

		// give the object a new name
		string newName = dlg.getName().toStdString();
		newObject->SetName(newName);

		// add it to the pile
		doc->DoCommand(new CCmdAddObject(doc->GetGModel(), newObject));

		UpdateModel(newObject, true);
	}
}

void CMainWindow::on_actionPurge_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	CDlgPurge dlg(this);
	if (dlg.exec())
	{
		FEModel* ps = doc->GetFEModel();
		ps->Purge(dlg.getOption());
		doc->ClearCommandStack();
		doc->SetModifiedFlag(true);
		UpdateModel();
		Update();
	}
}

void CMainWindow::on_actionEditProject_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	CDlgEditProject dlg(doc->GetProject(), this);
	dlg.exec();
	UpdatePhysicsUi();
}

void CMainWindow::on_actionFaceToElem_triggered()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(GetDocument());
	if (doc == nullptr) return;

	FESelection* sel = doc->GetCurrentSelection();
	if (sel->Type() == SELECT_FE_FACES)
	{
		FEFaceSelection* selectedFaces = dynamic_cast<FEFaceSelection*>(sel);
		FEMesh* mesh = dynamic_cast<FEMesh*>(selectedFaces->GetMesh());
		if (mesh)
		{
			vector<int> selectedElems = mesh->GetElementsFromSelectedFaces();
			if (selectedElems.empty() == false)
			{
				CCmdSelectElements* cmd = new CCmdSelectElements(mesh, selectedElems, false);
				doc->DoCommand(cmd);
			}
		}
	}
}

void CMainWindow::on_actionSelect_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	doc->SetTransformMode(TRANSFORM_NONE);
	RedrawGL();
}

void CMainWindow::on_actionTranslate_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	doc->SetTransformMode(TRANSFORM_MOVE);
	RedrawGL();
}

void CMainWindow::on_actionRotate_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	doc->SetTransformMode(TRANSFORM_ROTATE);
	RedrawGL();
}

void CMainWindow::on_actionScale_toggled(bool b)
{
	CDocument* doc = GetDocument();
	if (doc == nullptr) return;

	doc->SetTransformMode(TRANSFORM_SCALE);
	RedrawGL();
}

void CMainWindow::on_selectCoord_currentIndexChanged(int n)
{
	switch (n)
	{
	case 0: ui->glview->SetCoordinateSystem(COORD_GLOBAL); break;
	case 1: ui->glview->SetCoordinateSystem(COORD_LOCAL); break;
	case 2: ui->glview->SetCoordinateSystem(COORD_SCREEN); break;
	}
	Update();
}

//-----------------------------------------------------------------------------
void CMainWindow::on_actionMeasureTool_triggered()
{
	if (ui->measureTool == nullptr) ui->measureTool = new CDlgMeasure(this);
	ui->measureTool->show();
}
