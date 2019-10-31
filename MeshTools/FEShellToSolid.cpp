#include "stdafx.h"
#include "FEShellToSolid.h"

FEShellToSolid::FEShellToSolid() : FEModifier("Shell To Solid")
{
}

FEMesh* FEShellToSolid::Apply(FEMesh* pm)
{
	// Make sure that we have a triangle or quad mesh
	if ((pm->IsType(FE_TRI3)==false)&&(pm->IsType(FE_QUAD4)==false)) return 0;

	// we need to find the nodes that are to be duplicated
	int NN = pm->Nodes();
	vector<int> tag(NN, 0);

	int NE = pm->Elements();
	for (int i=0; i<NE; ++i)
	{
		FEElement& el = pm->Element(i);
		if (el.IsSelected())
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j) tag[el.m_node[j]] += 1;
		}
	}

	// count the nodes
	int nn = 0;
	for (int i=0; i<NN; ++i) if (tag[i] != 0) nn++;

	// make sure we have something
	assert(nn);
	if (nn==0) return 0;

	// create the node normals
	vector<vec3d> normals(NN, vec3d(0,0,0));
	int NF = pm->Faces();
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = pm->Face(i);
		if (pm->Element(f.m_elem[0].eid).IsSelected())
		{
			int nf = f.Nodes();
			for (int j=0; j<nf; ++j)
			{
				if (tag[f.n[j]] != 0) normals[f.n[j]] += f.m_nn[j];
			}
		}
	}

	// normalize
	for (int i=0; i<NN; ++i) normals[i].Normalize();

	// calculate average nodal shell thickness
	vector<double> h(NN, 0.0);
	for (int i=0; i<NE; ++i)
	{
		FEElement& el = pm->Element(i);
		if (el.IsSelected())
		{
			int ne = el.Nodes();
			for (int j=0; j<ne; ++j)
				if (tag[el.m_node[j]] != 0) h[el.m_node[j]] += el.m_h[j];
		}
	}
	for (int i=0; i<NN; ++i)
		if (tag[i] != 0) h[i] /= (double) tag[i];

	// create a new mesh
	FEMesh* pnew = new FEMesh(*pm);
	pnew->Create(NN + nn, 0);

	// position the new nodes
	nn = NN;
	for (int i=0; i<NN; ++i) if (tag[i] != 0) tag[i] = nn++; else tag[i] = -1;

	for (int i=0; i<NN; ++i)
	{
		if (tag[i] >= 0)
		{
			FENode& nd = pnew->Node(tag[i]);
			FENode& ns = pm->Node(i);
			nd.r = ns.r + normals[i]*h[i];
		}
	}

	// upgrade elements
	for (int i=0; i<NE; ++i)
	{
		FEElement& el = pnew->Element(i);
		if (el.IsSelected())
		{
			if (el.Type() == FE_QUAD4)
			{
				el.SetType(FE_HEX8);
				el.m_node[4] = tag[el.m_node[0]];
				el.m_node[5] = tag[el.m_node[1]];
				el.m_node[6] = tag[el.m_node[2]];
				el.m_node[7] = tag[el.m_node[3]];
			}
			else if (el.Type() == FE_TRI3)
			{
				el.SetType(FE_PENTA6);
				el.m_node[3] = tag[el.m_node[0]];
				el.m_node[4] = tag[el.m_node[1]];
				el.m_node[5] = tag[el.m_node[2]];
			}
		}
	}

	pnew->RebuildMesh();

	return pnew;
}
