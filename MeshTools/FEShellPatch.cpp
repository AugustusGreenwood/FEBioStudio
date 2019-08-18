// FEShellPatch.cpp: implementation of the FEShellPatch class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEShellPatch.h"
#include <GeomLib/GPrimitive.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEShellPatch::FEShellPatch(GPatch* po)
{
	m_pobj = po;

	m_t = 0.01;
	m_nx = m_ny = 10;

	AddDoubleParam(m_t, "t", "Thickness");
	AddIntParam(m_nx, "nx", "Nx");
	AddIntParam(m_ny, "ny", "Ny");
}

FEMesh* FEShellPatch::BuildMesh()
{
	// get the object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double w = param.GetFloatValue(GPatch::W);
	double h = param.GetFloatValue(GPatch::H);

	// get parameters
	m_t = GetFloatValue(T);
	double t = m_t;
	m_nx = GetIntValue(NX);
	m_ny = GetIntValue(NY);
	int nx = m_nx;
	int ny = m_ny;

	// check parameters
	if (nx < 1) nx = 1;
	if (ny < 1) ny = 1;

	int nodes = (nx+1)*(ny+1);
	int elems = nx*ny;

	// allocate storage
	FEMesh* pm = new FEMesh();
	pm->Create(nodes, elems);
	m_nx = nx;
	m_ny = ny;

	// position the nodes
	int i, j;
	double x, y;
	double fx, fy;
	FENode* pn = pm->NodePtr();
	for (i=0; i<=nx; i++)
	{
		fx = (double) i/(double) nx;
		x = -w/2 + fx*w;
		for (j=0; j<=ny; j++, pn++)
		{
			fy = (double) j/(double) ny;
			y = -h/2 + fy*h;

			pn->r = vec3d(x, y, 0);
		}
	}

	pm->Node(NodeIndex( 0, 0)).m_gid = 0;
	pm->Node(NodeIndex(nx, 0)).m_gid = 1;
	pm->Node(NodeIndex(nx,ny)).m_gid = 2;
	pm->Node(NodeIndex( 0,ny)).m_gid = 3;

	// create the connectivity
	FEElement* pe = pm->ElementPtr();
	for (i=0; i<nx; i++)
		for (j=0; j<ny; j++, pe++)
		{
			pe->SetType(FE_QUAD4);
			pe->m_gid = 0;
			int* n = pe->m_node;
			n[0] = NodeIndex(i  ,j  );
			n[1] = NodeIndex(i+1,j  );
			n[2] = NodeIndex(i+1,j+1);
			n[3] = NodeIndex(i  ,j+1);
		}
	
	// assign thickness to shells
	pe = pm->ElementPtr();
	for (i=0; i<elems; ++i, ++pe)
	{
		pe->m_h[0] = t;
		pe->m_h[1] = t;
		pe->m_h[2] = t;
		pe->m_h[3] = t;
	}

	BuildFaces(pm);
	BuildEdges(pm);

	pm->Update();

	return pm;
}

void FEShellPatch::BuildFaces(FEMesh* pm)
{
	int i, j;
	// count faces
	int nfaces = m_nx*m_ny;
	pm->Create(0,0,nfaces);
	FEFace* pf = pm->FacePtr();
	for (i=0; i<m_nx; ++i)
	{
		for (j=0; j<m_ny; ++j, ++pf)
		{
			FEFace& f = *pf;
			f.m_gid = 0;
			f.SetType(FE_FACE_QUAD4);
			f.n[0] = NodeIndex(i, j);
			f.n[1] = NodeIndex(i+1, j);
			f.n[2] = NodeIndex(i+1, j+1);
			f.n[3] = NodeIndex(i, j+1);
		}
	}
}

void FEShellPatch::BuildEdges(FEMesh* pm)
{
	int i;
	int nedges = 2*(m_nx+m_ny);
	pm->Create(0,0,0,nedges);
	FEEdge* pe = pm->EdgePtr();
	for (i=0; i<m_nx; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 0; pe->n[0] = NodeIndex(i, 0); pe->n[1] = NodeIndex(i+1, 0); }
	for (i=0; i<m_ny; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 1; pe->n[0] = NodeIndex(m_nx, i); pe->n[1] = NodeIndex(m_nx, i+1); }
	for (i=0; i<m_nx; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 2; pe->n[0] = NodeIndex(m_nx-i, m_ny); pe->n[1] = NodeIndex(m_nx-i-1, m_ny); }
	for (i=0; i<m_ny; ++i, ++pe) { pe->SetType(FE_EDGE2); pe->m_gid = 3; pe->n[0] = NodeIndex(0, m_ny-i); pe->n[1] = NodeIndex(0, m_ny-i-1); }
}
