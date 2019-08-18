#include "stdafx.h"
#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <PostLib/ImgAnimation.h>
#include <PostLib/AVIAnimation.h>
#include <PostLib/MPEGAnimation.h>

void CMainWindow::on_actionRecordNew_triggered()
{
#ifdef WIN32
	int noff = 1;
	QStringList filters;
	filters << "Windows AVI files (*.avi)"
		<< "Bitmap files (*.bmp)"
		<< "JPG files (*.jpg)"
		<< "PNG files (*.png)"
		<< "MPG files (*.mpg)";
#else
	int noff = 0;
	QStringList filters;
	filters << "Bitmap files (*.bmp)"
		<< "JPG files (*.jpg)"
		<< "Tiff files (*.tiff)"
		<< "MPG files (*.mpg)";
#endif

	QFileDialog dlg(this, "Save");
	dlg.setNameFilters(filters);
	dlg.setFileMode(QFileDialog::AnyFile);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	if (dlg.exec())
	{
		QString fileName = dlg.selectedFiles().first();
		string sfile = fileName.toStdString();
		char szfilename[512] = { 0 };
		sprintf(szfilename, "%s", sfile.c_str());
		int l = (int)sfile.length();
		char* ch = strrchr(szfilename, '.');

		int nfilter = filters.indexOf(dlg.selectedNameFilter());

		bool bret = false;
		CAnimation* panim = 0;
#ifdef WIN32
		if (nfilter == 0)
		{
			panim = new CAVIAnimation;
			if (ch == 0) sprintf(szfilename + l, ".avi");
			bret = ui->glview->NewAnimation(szfilename, panim, GL_BGR_EXT);
		}
		else if (nfilter == noff)
#else
		if (nfilter == noff)
#endif
		{
			panim = new CBmpAnimation;
			if (ch == 0) sprintf(szfilename + l, ".bmp");
			bret = ui->glview->NewAnimation(szfilename, panim);
		}
		else if (nfilter == noff + 1)
		{
			panim = new CJpgAnimation;
			if (ch == 0) sprintf(szfilename + l, ".jpg");
			bret = ui->glview->NewAnimation(szfilename, panim);
		}
		else if (nfilter == noff + 2)
		{
			panim = new CPNGAnimation;
			if (ch == 0) sprintf(szfilename + l, ".png");
			bret = ui->glview->NewAnimation(szfilename, panim);
		}
		else if (nfilter == noff + 3)
		{
#ifdef FFMPEG
			panim = new CMPEGAnimation;
			if (ch == 0) sprintf(szfilename + l, ".mpg");
			bret = ui->glview->NewAnimation(szfilename, panim);
#else
			QMessageBox::critical(this, "PostView2", "This video format is not supported in this version");
#endif
		}

		if (bret)
		{
			ui->m_old_title = windowTitle();
			setWindowTitle(ui->m_old_title + "   (RECORDING PAUSED)");
		}
		else bret = QMessageBox::critical(this, "PostView", "Failed creating animation stream.");

		RedrawGL();
	}
}

void CMainWindow::on_actionRecordStart_triggered()
{
	if (ui->glview->HasRecording())
	{
		if (ui->m_old_title.isEmpty()) ui->m_old_title = windowTitle();

		setWindowTitle(ui->m_old_title + "   (RECORDING)");

		ui->glview->StartAnimation();
	}
	else QMessageBox::information(this, "PostView", "You need to create a new video file before you can start recording");
}

void CMainWindow::on_actionRecordPause_triggered()
{
	if (ui->glview->HasRecording())
	{
		if (ui->glview->AnimationMode() == ANIM_RECORDING)
		{
			ui->glview->PauseAnimation();
			setWindowTitle(ui->m_old_title + "   (RECORDING PAUSED)");
		}
	}
	else QMessageBox::information(this, "PostView", "You need to create a new video file first.");
}

void CMainWindow::on_actionRecordStop_triggered()
{
	if (ui->glview->HasRecording())
	{
		if (ui->glview->AnimationMode() != ANIM_STOPPED)
		{
			ui->glview->StopAnimation();
			setWindowTitle(ui->m_old_title);
		}

		ui->m_old_title.clear();
	}
	else QMessageBox::information(this, "PostView", "You need to create a new video file first.");
}
