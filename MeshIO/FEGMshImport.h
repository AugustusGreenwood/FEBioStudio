#pragma once
#include "FileReader.h"
#include <MeshTools/FEProject.h>

class FEGMshImport : public FEFileImport
{
protected:
	struct ELEMENT
	{
		int ntype;
		int	gid;
		int node[10];
	};

public:
	FEGMshImport();
	bool Load(FEProject& prj, const char* szfile);

protected:
	bool ReadMeshFormat();
	bool ReadPhysicalNames();
	bool ReadNodes();
	bool ReadElements();

protected:
	char	m_szline[256];
	FEModel*	m_pfem;
	FEMesh*		m_pm;
};
