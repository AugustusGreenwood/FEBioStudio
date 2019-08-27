#include "stdafx.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <PostGL/GLPlaneCutPlot.h>
#include <PostGL/GLMirrorPlane.h>
#include <PostGL/GLVectorPlot.h>
#include <PostGL/GLTensorPlot.h>
#include <PostGL/GLStreamLinePlot.h>
#include <PostGL/GLParticleFlowPlot.h>
#include <PostGL/GLSlicePLot.h>
#include <PostGL/GLIsoSurfacePlot.h>
#include <PostLib/ImageModel.h>
#include <PostLib/ImageSlicer.h>
#include <PostLib/VolRender.h>
#include <PostLib/MarchingCubes.h>
#include <PostGL/GLVolumeFlowPlot.h>
#include <PostGL/GLModel.h>
#include <QMessageBox>
#include "PostDoc.h"
#include "GraphWindow.h"
#include "SummaryWindow.h"
#include "StatsWindow.h"
#include "IntegrateWindow.h"

Post::CGLModel* CMainWindow::GetCurrentModel()
{
	CFEBioJob* job = dynamic_cast<CFEBioJob*>(ui->modelViewer->GetCurrentObject());
	if (job == nullptr) return nullptr;

	CPostDoc* pd = job->GetPostDoc();
	if (pd == nullptr) return nullptr;

	return pd->GetGLModel();
}

void CMainWindow::on_actionPlaneCut_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	Post::CGLPlaneCutPlot* pp = new Post::CGLPlaneCutPlot(glm);
	glm->AddPlot(pp);

	ui->modelViewer->Update();
	ui->modelViewer->Select(pp);
	ui->modelViewer->parentWidget()->raise();
	RedrawGL();
}

void CMainWindow::on_actionMirrorPlane_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	Post::CGLMirrorPlane* pp = new Post::CGLMirrorPlane(glm);
	glm->AddPlot(pp);

	ui->modelViewer->Update();
	ui->modelViewer->Select(pp);
	ui->modelViewer->parentWidget()->raise();
	RedrawGL();
}

void CMainWindow::on_actionVectorPlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	Post::CGLVectorPlot* pp = new Post::CGLVectorPlot(glm);
	glm->AddPlot(pp);

	ui->modelViewer->Update();
	ui->modelViewer->Select(pp);
	ui->modelViewer->parentWidget()->raise();
	RedrawGL();
}

void CMainWindow::on_actionTensorPlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	Post::GLTensorPlot* pp = new Post::GLTensorPlot(glm);
	glm->AddPlot(pp);

	ui->modelViewer->Update();
	ui->modelViewer->Select(pp);
	ui->modelViewer->parentWidget()->raise();
	RedrawGL();
}

void CMainWindow::on_actionStreamLinePlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	Post::CGLStreamLinePlot* pp = new Post::CGLStreamLinePlot(glm);
	glm->AddPlot(pp);

	ui->modelViewer->Update();
	ui->modelViewer->Select(pp);
	ui->modelViewer->parentWidget()->raise();

	RedrawGL();
}

void CMainWindow::on_actionParticleFlowPlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	Post::CGLParticleFlowPlot* pp = new Post::CGLParticleFlowPlot(glm);
	glm->AddPlot(pp);

	ui->modelViewer->Update();
	ui->modelViewer->Select(pp);
	ui->modelViewer->parentWidget()->raise();

	RedrawGL();
}

void CMainWindow::on_actionVolumeFlowPlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	Post::GLVolumeFlowPlot* pp = new Post::GLVolumeFlowPlot(glm);
	glm->AddPlot(pp);

	ui->modelViewer->Update();
	ui->modelViewer->Select(pp);
	ui->modelViewer->parentWidget()->raise();

	RedrawGL();
}


void CMainWindow::on_actionImageSlicer_triggered()
{
	Post::CImageModel* img = dynamic_cast<Post::CImageModel*>(ui->modelViewer->GetCurrentObject());
	if (img == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "Please select an image data set first.");
		return;
	}

	Post::CImageSlicer* slicer = new Post::CImageSlicer(img);
	slicer->Create();
	img->AddImageRenderer(slicer);
	ui->modelViewer->Update();
	ui->modelViewer->Select(slicer);
	RedrawGL();
}

void CMainWindow::on_actionVolumeRender_triggered()
{
	Post::CImageModel* img = dynamic_cast<Post::CImageModel*>(ui->modelViewer->GetCurrentObject());
	if (img == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "Please select an image data set first.");
		return;
	}

	Post::CVolRender* vr = new Post::CVolRender(img);
	vr->Create();
	img->AddImageRenderer(vr);
	ui->modelViewer->Update();
	ui->modelViewer->Select(vr);
	RedrawGL();
}

void CMainWindow::on_actionMarchingCubes_triggered()
{
	Post::CImageModel* img = dynamic_cast<Post::CImageModel*>(ui->modelViewer->GetCurrentObject());
	if (img == nullptr)
	{
		QMessageBox::critical(this, "FEBio Studio", "Please select an image data set first.");
		return;
	}

	Post::CMarchingCubes* mc = new Post::CMarchingCubes(img);
	mc->Create();
	img->AddImageRenderer(mc);
	ui->modelViewer->Update();
	ui->modelViewer->Select(mc);
	RedrawGL();
}

void CMainWindow::on_actionIsosurfacePlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	Post::CGLIsoSurfacePlot* pp = new Post::CGLIsoSurfacePlot(glm);
	glm->AddPlot(pp);

	ui->modelViewer->Update();
	ui->modelViewer->Select(pp);
	ui->modelViewer->parentWidget()->raise();
	RedrawGL();
}

void CMainWindow::on_actionSlicePlot_triggered()
{
	Post::CGLModel* glm = GetCurrentModel();
	if (glm == nullptr) return;

	Post::CGLSlicePlot* pp = new Post::CGLSlicePlot(glm);
	glm->AddPlot(pp);

	ui->modelViewer->Update();
	ui->modelViewer->Select(pp);
	ui->modelViewer->parentWidget()->raise();

	RedrawGL();
}

void CMainWindow::on_actionDisplacementMap_triggered()
{
/*	CDocument* doc = GetDocument();
	if (doc == nullptr) return;
	if (doc->IsValid() == false) return;

	CGLModel* pm = doc->GetGLModel();
	if (pm->GetDisplacementMap() == 0)
	{
		if (pm->AddDisplacementMap() == false)
		{
			QMessageBox::warning(this, "PostView", "You need at least one vector field before you can define a displacement map.");
		}
		else
		{
			doc->UpdateFEModel(true);
			ui->modelViewer->Update(true);
		}
	}
	else
	{
		QMessageBox::information(this, "PostView", "This model already has a displacement map.");
	}
*/
}

void CMainWindow::on_actionGraph_triggered()
{
	CPostDoc* postDoc = GetActiveDocument();
	if (postDoc == nullptr) return;

	CGraphWindow* pg = new CModelGraphWindow(this, postDoc);
	AddGraph(pg);

	pg->show();
	pg->raise();
	pg->activateWindow();
	pg->Update();
}

void CMainWindow::on_actionSummary_triggered()
{
	CPostDoc* postDoc = GetActiveDocument();
	if (postDoc == nullptr) return;

	CSummaryWindow* summaryWindow = new CSummaryWindow(this, postDoc);

	summaryWindow->Update(true);
	summaryWindow->show();
	summaryWindow->raise();
	summaryWindow->activateWindow();

	AddGraph(summaryWindow);
}

void CMainWindow::on_actionStats_triggered()
{
	CPostDoc* postDoc = GetActiveDocument();
	if (postDoc == nullptr) return;

	CStatsWindow* statsWindow = new CStatsWindow(this, postDoc);
	statsWindow->Update(true);
	statsWindow->show();
	statsWindow->raise();
	statsWindow->activateWindow();

	AddGraph(statsWindow);
}

void CMainWindow::on_actionIntegrate_triggered()
{
	CPostDoc* postDoc = GetActiveDocument();
	if (postDoc == nullptr) return;

	CIntegrateWindow* integrateWindow = new CIntegrateWindow(this, postDoc);
	integrateWindow->Update(true);
	integrateWindow->show();
	integrateWindow->raise();
	integrateWindow->activateWindow();

	AddGraph(integrateWindow);
}
