#include "stdafx.h"
#include "FEBioJob.h"
#include "PostDoc.h"
#include "Document.h"
#include <sstream>
#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <FSCore/FSDir.h>
#include <QtCore/QThread>
#ifdef HAS_SSH
#include "SSHHandler.h"
#endif

//-----------------------------------------------------------------------------
int CFEBioJob::m_count = 0;

CFEBioJob::CFEBioJob(CDocument* doc) : m_doc(doc)
{
	m_count++;
	std::stringstream ss;
	ss << "job" << m_count;
	SetName(ss.str());

	m_status = NONE;

	m_postDoc = nullptr;
}

CFEBioJob::~CFEBioJob()
{
	delete m_postDoc;
#ifdef HAS_SSH
	delete m_sshHandler;
#endif
}

CFEBioJob::CFEBioJob(CDocument* doc, const std::string& jobName, const std::string& workingDirectory, CLaunchConfig launchConfig)
	: m_doc(doc), m_launchConfig(launchConfig)
{
	// set the job's name
	SetName(jobName);

	string dir = workingDirectory;
	char ch = dir.back();
	if (!((ch == '/') || (ch == '\\')))
	{
#ifdef WIN32
		dir += "\\";
#else
		dir += "/";
#endif
	}

	// build the feb file name
	m_fileName = dir + jobName + ".feb";

	m_status = NONE;

	// set default plot file name
	m_plotFile = m_fileName;
	size_t pos = m_plotFile.rfind(".");
	if (pos != std::string::npos)
	{
		// remove extension
		m_plotFile.erase(pos + 1);
	}

	// add the xplt extension
	m_plotFile += "xplt";

	m_postDoc = nullptr;

#ifdef HAS_SSH
	if(launchConfig.type == LOCAL)
	{
		m_sshHandler = nullptr;
	}
	else
	{
		m_sshHandler = new CSSHHandler(this);
	}
#endif

}

void CFEBioJob::UpdateWorkingDirectory(const std::string& dir)
{
	QString dirName = QString::fromStdString(dir);

	// update feb file name
	QString febName = QFileInfo(QString::fromStdString(m_fileName)).fileName();
#ifdef WIN32
	m_fileName = (dirName + "\\" + febName).toStdString();
#else
	m_fileName = (dirName + "/" + febName).toStdString();
#endif

	// update xplt file name
	QString xpltName = QFileInfo(QString::fromStdString(m_plotFile)).fileName();

#ifdef WIN32
	m_plotFile = (dirName + "\\" + xpltName).toStdString();
#else
	m_plotFile = (dirName + "/" + xpltName).toStdString();
#endif

	// update log file name
	QString logName = QFileInfo(QString::fromStdString(m_logFile)).fileName();
#ifdef WIN32
	m_logFile = (dirName + "\\" + logName).toStdString();
#else
	m_logFile = (dirName + "/" + logName).toStdString();
#endif
}

CLaunchConfig* CFEBioJob::GetLaunchConfig()
{
	return &m_launchConfig;
}

void CFEBioJob::UpdateLaunchConfig(CLaunchConfig launchConfig)
{
	CLaunchConfig oldConfig = m_launchConfig;

	m_launchConfig = launchConfig;

#ifdef HAS_SSH
	if(launchConfig.type == LOCAL)
	{
		if(m_sshHandler)
		{
			delete m_sshHandler;
		}

		m_sshHandler = nullptr;
	}
	else
	{
		if(m_sshHandler)
		{
			m_sshHandler->Update(oldConfig);
		}
		else
		{
			m_sshHandler = new CSSHHandler(this);
		}
	}
#endif

}

#ifdef HAS_SSH
CSSHHandler* CFEBioJob::GetSSHHandler()
{
	return m_sshHandler;
}

void CFEBioJob::GetRemoteJobFiles()
{
	m_sshHandler->GetJobFiles();
}

void CFEBioJob::StartRemoteJob()
{
//	QThread* thread = new QThread;
//
//	m_sshHandler->moveToThread(thread);
//
//	connect(m_sshHandler, SIGNAL, (error(QString)), this, SLOT (errorString(QString)));
//	connect(thread, SIGNAL (started()), m_sshHandler, SLOT (process()));
//	connect(m_sshHandler, SIGNAL (finished()), thread, SLOT (quit()));
//	connect(m_sshHandler, SIGNAL (finished()), m_sshHandler, SLOT (deleteLater()));
//	connect(thread, SIGNAL (finished()), thread, SLOT (deleteLater()));
//	thread->start();


//	m_sshHandler->StartRemoteJob();
}

#endif

void CFEBioJob::SetStatus(JOB_STATUS status)
{
	m_status = status;
}

int CFEBioJob::GetStatus()
{
	return m_status;
}

void CFEBioJob::SetFileName(const std::string& fileName)
{
	m_fileName = fileName;
}

std::string CFEBioJob::GetFileName() const
{
	return m_fileName;
}

void CFEBioJob::SetPlotFileName(const std::string& plotFile)
{
	m_plotFile = plotFile;
}

std::string CFEBioJob::GetPlotFileName() const
{
	return m_plotFile;
}

bool CFEBioJob::HasPostDoc()
{
	return (m_postDoc != nullptr);
}

CPostDoc* CFEBioJob::GetPostDoc()
{
	return m_postDoc;
}

size_t CFEBioJob::RemoveChild(FSObject* po)
{
	assert(m_postDoc == po);
	m_postDoc = nullptr;
	return 0;
}

bool CFEBioJob::OpenPlotFile()
{
	string plotFile = FSDir::toAbsolutePath(m_plotFile);

	if (m_postDoc == nullptr)
	{
		m_postDoc = new CPostDoc;
		m_postDoc->SetParent(this);
		m_postDoc->SetName(GetName());
	}

	if (m_postDoc->LoadPlotfile(plotFile) == false)
	{
		return false;
	}

	return true;
}

void CFEBioJob::Save(OArchive& ar)
{
	ar.WriteChunk(CID_FEOBJ_NAME, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());
	ar.WriteChunk(CID_FEBIOJOB_FILENAME, m_fileName);
	ar.WriteChunk(CID_FEBIOJOB_PLOTFILE, m_plotFile);

	ar.BeginChunk(CID_FEBIOJOB_LCONFIG);
	m_launchConfig.Save(ar);
	ar.EndChunk();

}

void CFEBioJob::Load(IArchive& ar)
{
	// TODO: get relative file name
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEOBJ_NAME: { string name; ar.read(name); SetName(name); } break;
		case CID_FEOBJ_INFO: { string info; ar.read(info); SetInfo(info); } break;
		case CID_FEBIOJOB_FILENAME: ar.read(m_fileName); break;
		case CID_FEBIOJOB_PLOTFILE: ar.read(m_plotFile); break;
		case CID_FEBIOJOB_LCONFIG: {CLaunchConfig lConfig; lConfig.Load(ar); m_sshHandler = nullptr; UpdateLaunchConfig(lConfig); } break;
		}
		ar.CloseChunk();
	}
}
