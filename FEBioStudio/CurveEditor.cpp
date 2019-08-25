#include "ui_curveeditor.h"
#include "MainWindow.h"
#include "Document.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include "DlgFormula.h"
#include <MathLib/MathParser.h>
#include <FEMLib/FESurfaceLoad.h>
#include <MeshTools/FEMultiMaterial.h>

CCmdAddPoint::CCmdAddPoint(FELoadCurve* plc, LOADPOINT& pt) : CCommand("Add point")
{
	m_lc = plc;
	m_pt = pt;
	m_index = -1;
}

void CCmdAddPoint::Execute()
{
	m_index = m_lc->Add(m_pt);
}

void CCmdAddPoint::UnExecute()
{
	m_lc->Delete(m_index);
}

CCmdRemovePoint::CCmdRemovePoint(FELoadCurve* plc, int index) : CCommand("Remove point")
{
	m_lc = plc;
	m_index = index;
}

void CCmdRemovePoint::Execute()
{
	m_pt = m_lc->Item(m_index);
	m_lc->Delete(m_index);
}

void CCmdRemovePoint::UnExecute()
{
	int m = m_lc->Add(m_pt);
	assert(m == m_index);
}

CCmdMovePoint::CCmdMovePoint(FELoadCurve* plc, int index, LOADPOINT to) : CCommand("Move point")
{
	m_lc = plc;
	m_index = index;
	m_p = to;
}

void CCmdMovePoint::Execute()
{
	LOADPOINT tmp = m_lc->Item(m_index);
	m_lc->Item(m_index) = m_p;
	m_p = tmp;
}

void CCmdMovePoint::UnExecute()
{
	Execute();
}

//=============================================================================

CCurveEditor::CCurveEditor(CMainWindow* wnd) : m_wnd(wnd), QMainWindow(wnd), ui(new Ui::CCurveEdior)
{
	m_currentItem = 0;
	m_plc_copy = 0;
	m_nflt = FLT_ALL;
	m_nselect = -1;
	ui->setupUi(this);
	resize(600, 400);
}

void CCurveEditor::Update()
{
	// clear the tree
	ui->tree->clear();
	ui->plot->clearData();
	m_currentItem = 0;

	QTreeWidgetItem* t1 = new QTreeWidgetItem(ui->tree);
	t1->setExpanded(true);
	t1->setText(0, "Model");

	CDocument* doc = m_wnd->GetDocument();
	FEModel& fem = *doc->GetFEModel();
	GModel& model = fem.GetModel();

	QTreeWidgetItem *t2, *t3;

	// add the discrete objects
	if (Filter(FLT_GEO))
	{
		t2 = ui->addTreeItem(t1, "Geometry");
		for (int i = 0; i<model.DiscreteObjects(); ++i)
		{
			GLinearSpring* po = dynamic_cast<GLinearSpring*>(model.DiscreteObject(i));
			if (po)
			{
				t3 = ui->addTreeItem(t2, QString::fromStdString(po->GetName()));
				FELoadCurve* plc = po->GetParam(GLinearSpring::MP_E).GetLoadCurve();
				if (plc) ui->addTreeItem(t3, "E", plc);
			}

			GGeneralSpring* ps = dynamic_cast<GGeneralSpring*>(model.DiscreteObject(i));
			if (ps)
			{
				t3 = ui->addTreeItem(t2, QString::fromStdString(ps->GetName()));
				FELoadCurve* plc = ps->GetParam(GGeneralSpring::MP_F).GetLoadCurve();
				if (plc) ui->addTreeItem(t3, "F", plc);
			}

			GLinearSpringSet* pg = dynamic_cast<GLinearSpringSet*>(model.DiscreteObject(i));
			if (pg)
			{
				t3 = ui->addTreeItem(t2, QString::fromStdString(pg->GetName()));
				FELoadCurve* plc = pg->GetParam(GLinearSpringSet::MP_E).GetLoadCurve();
				if (plc) ui->addTreeItem(t3, "E", plc);
			}

			GNonlinearSpringSet* pn = dynamic_cast<GNonlinearSpringSet*>(model.DiscreteObject(i));
			if (pn)
			{
				t3 = ui->addTreeItem(t2, QString::fromStdString(pn->GetName()));
				FELoadCurve* plc = pn->GetParam(GNonlinearSpringSet::MP_F).GetLoadCurve();
				if (plc) ui->addTreeItem(t3, "F", plc);
			}
		}
	}

	// add the materials
	if (Filter(FLT_MAT))
	{
		t2 = ui->addTreeItem(t1, "Materials");
		for (int i = 0; i<fem.Materials(); ++i)
		{
			GMaterial* pgm = fem.GetMaterial(i);
			t3 = ui->addTreeItem(t2, QString::fromStdString(pgm->GetName()));

			FEMaterial* pm = pgm->GetMaterialProperties();
			if (pm)
			{
				int n = pm->Parameters();
				for (int j = 0; j<n; ++j)
				{
					Param& p = pm->GetParam(j);
					if (p.IsEditable() && (p.GetParamType() == Param_FLOAT))
					{
						FELoadCurve* plc = p.GetLoadCurve();
						ui->addTreeItem(t3, p.GetLongName(), plc, &p);
					}
				}
				AddMultiMaterial(pm, t3);
			}
		}
	}

	// add the boundary condition data
	if (Filter(FLT_BC))
	{
		t2 = ui->addTreeItem(t1, "BCs");
		for (int i = 0; i<fem.Steps(); ++i)
		{
			FEStep* pstep = fem.GetStep(i);
			int nbc = pstep->BCs();
			for (int j = 0; j<nbc; ++j)
			{
				FEPrescribedDisplacement*  pdbc = dynamic_cast<FEPrescribedDisplacement* >(pstep->BC(j)); if (pdbc) ui->addTreeItem(t2, QString::fromStdString(pdbc->GetName()), pdbc->GetLoadCurve());
				FEPrescribedRotation*      prbc = dynamic_cast<FEPrescribedRotation*     >(pstep->BC(j)); if (prbc) ui->addTreeItem(t2, QString::fromStdString(prbc->GetName()), prbc->GetLoadCurve());
				FEPrescribedFluidPressure* ppbc = dynamic_cast<FEPrescribedFluidPressure*>(pstep->BC(j)); if (ppbc) ui->addTreeItem(t2, QString::fromStdString(ppbc->GetName()), ppbc->GetLoadCurve());
				FEPrescribedTemperature*   ptbc = dynamic_cast<FEPrescribedTemperature*  >(pstep->BC(j)); if (ptbc) ui->addTreeItem(t2, QString::fromStdString(ptbc->GetName()), ptbc->GetLoadCurve());
				FEPrescribedConcentration* pcbc = dynamic_cast<FEPrescribedConcentration*>(pstep->BC(j)); if (pcbc) ui->addTreeItem(t2, QString::fromStdString(pcbc->GetName()), pcbc->GetLoadCurve());
                FEPrescribedFluidVelocity*  pvbc = dynamic_cast<FEPrescribedFluidVelocity*>(pstep->BC(j)); if (pvbc) ui->addTreeItem(t2, QString::fromStdString(pvbc->GetName()), pvbc->GetLoadCurve());
				FEPrescribedFluidDilatation*  pebc = dynamic_cast<FEPrescribedFluidDilatation*>(pstep->BC(j)); if (pebc) ui->addTreeItem(t2, QString::fromStdString(pebc->GetName()), pebc->GetLoadCurve());
			}
		}
	}

	// add the load data
	if (Filter(FLT_LOAD))
	{
		t2 = ui->addTreeItem(t1, "Loads");
		for (int i = 0; i<fem.Steps(); ++i)
		{
			FEStep* pstep = fem.GetStep(i);
			int nbc = pstep->Loads();
			for (int j = 0; j<nbc; ++j)
			{
				FEBoundaryCondition* plj = pstep->Load(j);

                FEFluidFlowResistance* pfr = dynamic_cast<FEFluidFlowResistance*>(pstep->Load(j));
                if (pfr) {
                    ui->addTreeItem(t2, QString::fromStdString("R"), pfr->GetLoadCurve());
                    ui->addTreeItem(t2, QString::fromStdString("pressure_offset"), pfr->GetPOLoadCurve());
                }
				else {
					FEBodyForce* pbl = dynamic_cast<FEBodyForce*>(pstep->Load(j));
					if (pbl)
					{
						t3 = ui->addTreeItem(t2, QString::fromStdString(pbl->GetName()));
						if (pbl->GetLoadCurve(0)) ui->addTreeItem(t3, "x-force", pbl->GetLoadCurve(0));
						if (pbl->GetLoadCurve(1)) ui->addTreeItem(t3, "y-force", pbl->GetLoadCurve(1));
						if (pbl->GetLoadCurve(2)) ui->addTreeItem(t3, "z-force", pbl->GetLoadCurve(2));
					}
					else {
						FEHeatSource* phs = dynamic_cast<FEHeatSource*>(pstep->Load(j));
						if (phs)
						{
							if (phs->GetLoadCurve()) ui->addTreeItem(t2, QString::fromStdString(phs->GetName()), phs->GetLoadCurve());
						}
						else
						{
							FEPrescribedBC* pbc = dynamic_cast<FEPrescribedBC*>(plj);
							if (pbc) ui->addTreeItem(t2, QString::fromStdString(pbc->GetName()), pbc->GetLoadCurve());
						}
					}
				}
			}
		}
	}

	// add contact interfaces
	if (Filter(FLT_CONTACT))
	{
		t2 = ui->addTreeItem(t1, "Contact");
		for (int i = 0; i<fem.Steps(); ++i)
		{
			FEStep* pstep = fem.GetStep(i);
			for (int j = 0; j<pstep->Interfaces(); ++j)
			{
				FEInterface* pi = pstep->Interface(j);
				FERigidWallInterface* pw = dynamic_cast<FERigidWallInterface*>(pi);
				if (pw) 
				{
					t3 = ui->addTreeItem(t2, QString::fromStdString(pw->GetName()));
					ui->addTreeItem(t3, "displacement", pw->GetLoadCurve());
				}
				else
				{
					FERigidSphereInterface* prs = dynamic_cast<FERigidSphereInterface*>(pi);
					if (prs)
					{
						t3 = ui->addTreeItem(t2, QString::fromStdString(prs->GetName()));
						ui->addTreeItem(t3, "ux", prs->GetLoadCurve(0));
						ui->addTreeItem(t3, "uy", prs->GetLoadCurve(1));
						ui->addTreeItem(t3, "uz", prs->GetLoadCurve(2));
					}
					else
					{
						FEInterface* pc = pstep->Interface(j);
						int NP = pc->Parameters();
						if (NP > 0)
						{
							t3 = ui->addTreeItem(t2, QString::fromStdString(pc->GetName()));
							for (int n = 0; n<NP; ++n)
							{
								Param& p = pc->GetParam(n);
								if (p.IsEditable())
								{
									FELoadCurve* plc = p.GetLoadCurve();
									if (plc) ui->addTreeItem(t3, p.GetLongName(), plc);
								}
							}
						}
					}
				}
			}
		}
	}

	// add constraints
	if (Filter(FLT_CONSTRAINT))
	{
		t2 = ui->addTreeItem(t1, "Constraints");
		for (int i = 0; i<fem.Steps(); ++i)
		{
			FEStep* pstep = fem.GetStep(i);
			for (int j = 0; j<pstep->RCs(); ++j)
			{
				FERigidPrescribed* pc = dynamic_cast<FERigidPrescribed*>(pstep->RC(j));
				if (pc)
				{
					t3 = ui->addTreeItem(t2, QString::fromStdString(pc->GetName()));
					ui->addTreeItem(t3, "value", pc->GetLoadCurve());
				}
			}
		}
	}

	// add rigid connectors
	if (Filter(FLT_CONNECTOR))
	{
		t2 = ui->addTreeItem(t1, "Connectors");
		for (int i = 0; i<fem.Steps(); ++i)
		{
			FEStep* pstep = fem.GetStep(i);
			for (int j = 0; j<pstep->Connectors(); ++j)
			{
				FEConnector* pc = pstep->Connector(j);
				int NP = pc->Parameters();
				if (NP > 0)
				{
					t3 = ui->addTreeItem(t2, QString::fromStdString(pc->GetName()));
					for (int n = 0; n<NP; ++n)
					{
						Param& p = pc->GetParam(n);
						if (p.IsEditable() && (p.GetParamType() == Param_FLOAT))
						{
							FELoadCurve* plc = p.GetLoadCurve();
							ui->addTreeItem(t3, p.GetLongName(), plc, &p);
						}
					}
				}
			}
		}
	}

	// must point curves
	if (Filter(FLT_STEP))
	{
		t2 = ui->addTreeItem(t1, "Steps");
		for (int i = 0; i<fem.Steps(); ++i)
		{
			FEStep* pstep = fem.GetStep(i);
			t3 = ui->addTreeItem(t2, QString::fromStdString(pstep->GetName()));
			FEAnalysisStep* pas = dynamic_cast<FEAnalysisStep*>(pstep);
			if (pas && pas->GetSettings().bmust) ui->addTreeItem(t3, "must point", pas->GetMustPointLoadCurve());
		}
	}
}


//-----------------------------------------------------------------------------
void CCurveEditor::AddMultiMaterial(FEMaterial* pm, QTreeWidgetItem* tp)
{
	for (int k = 0; k<pm->Properties(); ++k)
	{
		FEMaterialProperty& pmc = pm->GetProperty(k);
		for (int l=0; l<pmc.Size(); ++l)
		{
			FEMaterial* pmat = pmc.GetMaterial(l);
			if (pmat)
			{
				QTreeWidgetItem* tc = ui->addTreeItem(tp, FEMaterialFactory::TypeStr(pmat));
				int n = pmat->Parameters();
				for (int j = 0; j<n; ++j)
				{
					Param& p = pmat->GetParam(j);
					if (p.IsEditable())
					{
						FELoadCurve* plc = p.GetLoadCurve();
						ui->addTreeItem(tc, p.GetLongName(), plc, &p);
					}
				}
				AddMultiMaterial(pmat, tc);
			}
		}
	}
}

void CCurveEditor::on_tree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* prev)
{
	m_currentItem = dynamic_cast<CCurveEditorItem*>(current);
	m_cmd.Clear();
	if (m_currentItem)
	{
		FELoadCurve* plc = m_currentItem->GetLoadCurve();
		SetLoadCurve(plc);
	}
	else SetLoadCurve(0);
}

void CCurveEditor::SetLoadCurve(FELoadCurve* plc)
{
	ui->plot->clear();

	if (plc)
	{
		CPlotData* data = new CLoadCurveData(plc);
		for (int i=0; i<plc->Size(); ++i)
		{
			LOADPOINT pt = plc->Item(i);
			data->addPoint(pt.time, pt.load);
		}
		ui->plot->addPlotData(data);
		data->setColor(QColor(64, 192, 128));

		ui->setCurveType(plc->GetType(), plc->GetExtend());
	}
	ui->plot->repaint();
}

void CCurveEditor::UpdateLoadCurve()
{
	if (m_currentItem == 0) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();
	if (plc == 0) return;

	CPlotData& data = ui->plot->getPlotData(0);
	assert(data.size() == plc->Size());
	for (int i = 0; i<data.size(); ++i)
	{
		LOADPOINT& pi = plc->Item(i);
		QPointF& po = data.Point(i);

		po.setX(pi.time);
		po.setY(pi.load);
	}
	ui->plot->repaint();
}


void CCurveEditor::on_filter_currentIndexChanged(int n)
{
	m_nflt = n;
	Update();
}

void CCurveEditor::on_plot_pointClicked(QPointF p, bool shift)
{
	if (m_currentItem == 0) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	if (plc == 0)
	{
		Param* pp = m_currentItem->GetParam();
		if (pp)
		{
			assert(pp->GetLoadCurve() == 0);
			pp->SetLoadCurve();
			plc = pp->GetLoadCurve();
			plc->Clear();
			m_currentItem->SetLoadCurve(plc);
		}
	}

	if (plc && (ui->isAddPointChecked() || shift))
	{
		if (ui->isSnapToGrid()) p = ui->plot->SnapToGrid(p);

		LOADPOINT lp(p.x(), p.y());

		CCmdAddPoint* cmd = new CCmdAddPoint(plc, lp);
		m_cmd.DoCommand(cmd);

		SetLoadCurve(plc);
		ui->plot->selectPoint(0, cmd->Index());
	}
}

void CCurveEditor::on_plot_pointSelected(int n)
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();
	m_nselect = n;
	if (n >= 0)
	{
		ui->enablePointEdit(true);
		LOADPOINT pt = plc->Item(n);
		ui->setPointValues(pt.time, pt.load);
	}
	else
	{
		ui->enablePointEdit(false);
	}
}

void CCurveEditor::on_plot_pointDragged(QPoint p)
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	if (m_nselect>=0)
	{
		QPointF pf = ui->plot->ScreenToView(p);
		if (ui->isSnapToGrid()) pf = ui->plot->SnapToGrid(pf);

		LOADPOINT& lp = plc->Item(m_nselect);
		lp.time = pf.x();
		lp.load = pf.y();

		ui->plot->getPlotData(0).Point(m_nselect) = pf;

		ui->setPointValues(pf.x(), pf.y());

		ui->plot->repaint();
	}
}

void CCurveEditor::on_plot_draggingStart(QPoint p)
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	if (m_nselect >= 0)
	{
		LOADPOINT& lp = plc->Item(m_nselect);
		ui->m_p0.setX(lp.time);
		ui->m_p0.setY(lp.load);
	}
}

void CCurveEditor::on_plot_draggingEnd(QPoint p)
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	if (m_nselect >= 0)
	{
		LOADPOINT lp0;
		lp0.time = ui->m_p0.x();
		lp0.load = ui->m_p0.y();

		LOADPOINT lp = plc->Item(m_nselect);
		plc->Item(m_nselect) = lp0;

		m_cmd.DoCommand(new CCmdMovePoint(plc, m_nselect, lp));

		UpdateLoadCurve();
	}
}

void CCurveEditor::on_open_triggered()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "All files (*)");
	if (fileName.isEmpty() == false)
	{
		std::string sfile = fileName.toStdString();
		const char* szfile = sfile.c_str();
		if (plc->LoadData(szfile))
		{
			SetLoadCurve(plc);
		}
		else QMessageBox::critical(this, "Open File", QString("Failed loading curve data from file %1").arg(szfile));
	}
}

void CCurveEditor::on_save_triggered()
{
}

void CCurveEditor::on_clip_triggered()
{
	ui->plot->OnCopyToClipboard();
}

void CCurveEditor::on_copy_triggered()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	if (m_plc_copy == 0) m_plc_copy = new FELoadCurve;
	*m_plc_copy = *plc;
}

void CCurveEditor::on_paste_triggered()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	if (m_plc_copy == 0) return;
	*plc = *m_plc_copy;
	SetLoadCurve(plc);
}

void CCurveEditor::on_xval_textEdited()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	if (m_nselect >= 0)
	{
		QPointF p = ui->getPointValue();
		LOADPOINT& it = plc->Item(m_nselect);
		it.time = p.x();
		it.load = p.y();
		UpdateLoadCurve();
	}
}

void CCurveEditor::on_yval_textEdited()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	if (m_nselect >= 0)
	{
		QPointF p = ui->getPointValue();
		LOADPOINT& it = plc->Item(m_nselect);
		it.time = p.x();
		it.load = p.y();
		UpdateLoadCurve();
	}
}

void CCurveEditor::on_deletePoint_clicked()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	if (m_nselect >= 0)
	{
		m_cmd.DoCommand(new CCmdRemovePoint(plc, m_nselect));

		SetLoadCurve(plc);
		m_nselect = -1;
		ui->enablePointEdit(false);
	}
}

void CCurveEditor::on_zoomToFit_clicked()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();
	ui->plot->OnZoomToFit();
}

void CCurveEditor::on_zoomX_clicked()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();
	ui->plot->OnZoomToWidth();
}

void CCurveEditor::on_zoomY_clicked()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();
	ui->plot->OnZoomToHeight();
}

void CCurveEditor::on_undo_triggered()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	if (m_cmd.CanUndo()) m_cmd.UndoCommand();

	QString undo = m_cmd.CanUndo() ? m_cmd.GetUndoCmdName() : "(Nothing to undo)";
	QString redo = m_cmd.CanRedo() ? m_cmd.GetRedoCmdName() : "(Nothing to redo)";
	ui->setCmdNames(undo, redo);

	SetLoadCurve(plc);
	m_nselect = -1;
	ui->enablePointEdit(false);
}

void CCurveEditor::on_redo_triggered()
{	
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	if (m_cmd.CanRedo()) m_cmd.RedoCommand();

	QString undo = m_cmd.CanUndo() ? m_cmd.GetUndoCmdName() : "(Nothing to undo)";
	QString redo = m_cmd.CanRedo() ? m_cmd.GetRedoCmdName() : "(Nothing to redo)";
	ui->setCmdNames(undo, redo);

	SetLoadCurve(plc);
	m_nselect = -1;
	ui->enablePointEdit(false);
}

void CCurveEditor::on_math_triggered()
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();

	CDlgFormula dlg(this);

	dlg.SetMath(plc->GetName());
	
	if (dlg.exec())
	{
		std::vector<LOADPOINT> pts = dlg.GetPoints();
		QString math = dlg.GetMath();
		std::string smath = math.toStdString();

		CMathParser m;
		plc->Clear();
		plc->SetName(smath.c_str());
		if (pts.empty())
		{
			LOADPOINT p0(0, 0), p1(0, 0);
			plc->Add(p0);
			plc->Add(p1);
		}
		else
		{
			for (int i = 0; i < (int)pts.size(); ++i)
			{
				LOADPOINT& pt = pts[i];
				plc->Add(pt);
			}
		}

		SetLoadCurve(plc);
		m_nselect = -1;
		ui->enablePointEdit(false);
	}	
}

void CCurveEditor::on_lineType_currentIndexChanged(int n)
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();
	plc->SetType(n);
	ui->plot->repaint();
}

void CCurveEditor::on_extendMode_currentIndexChanged(int n)
{
	if ((m_currentItem == 0) || (m_currentItem->GetLoadCurve() == 0)) return;
	FELoadCurve* plc = m_currentItem->GetLoadCurve();
	plc->SetExtend(n);
	ui->plot->repaint();
}

//=======================================================================================
void CLoadCurveData::draw(QPainter& painter, CPlotWidget& plt)
{
	if (m_lc == 0) return;

	QBrush b = painter.brush();
	painter.setBrush(Qt::NoBrush);

	int N = size();

	// draw the line
	QRect rt = plt.ScreenRect();
	QPoint p0, p1;
	for (int i=rt.left(); i<rt.right(); i += 2)
	{
		p1.setX(i);
		QPointF p = plt.ScreenToView(p1);
		p.setY(m_lc->Value(p.x()));
		p1 = plt.ViewToScreen(p);

		if (i != rt.left())
		{
			painter.drawLine(p0, p1);
		}

		p0 = p1;
	}

	// draw the marks
	if (plt.showDataMarks())
	{
		painter.setBrush(b);
		for (int i = 0; i<N; ++i)
		{
			p1 = plt.ViewToScreen(Point(i));
			QRect r(p1.x() - 2, p1.y() - 2, 5, 5);
			painter.drawRect(r);
		}
	}
}
