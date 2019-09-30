// FEModifier.cpp: implementation of the FEModifier class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEModifier.h"
#include "FENNQuery.h"
#include <MeshLib/FENodeNodeList.h>
#include <MeshLib/FENodeElementList.h>
#include "FELinearToQuadratic.h"
#include "FESplitModifier.h"
#include <GeomLib/GObject.h>
#include <stdarg.h>

std::string FEModifier::m_error;

bool FEModifier::SetError(const char* szerr, ...)
{
	// get a pointer to the argument list
	va_list	args;

	// copy to string
	char sz[256] = {0};
	va_start(args, szerr);
	vsprintf(sz, szerr, args);
	va_end(args);

	m_error = std::string(sz);
	return false;
}

std::string FEModifier::GetErrorString() 
{ 
	return m_error; 
}
	
//=============================================================================
// FEPartitionSelection
//-----------------------------------------------------------------------------

FEPartitionSelection::FEPartitionSelection() : FEModifier("Partition")
{
	
}

FEMesh* FEPartitionSelection::Apply(FEMesh* pm)
{
	FEMesh* newMesh = new FEMesh(*pm);

	newMesh->PartitionElementSelection();

	return newMesh;
}

FEMesh* FEPartitionSelection::Apply(FEGroup* pg)
{
	FEMesh* oldMesh = pg->GetMesh();
	if (oldMesh == 0) return 0;

	FEMesh* newMesh = new FEMesh(*oldMesh);

	FESurface* s = dynamic_cast<FESurface*>(pg);
	if (s)
	{
		newMesh->PartitionFaceSelection();
	}

	FEPart* p = dynamic_cast<FEPart*>(pg);
	if (p)
	{
		newMesh->PartitionElementSelection();
	}

	FENodeSet* n = dynamic_cast<FENodeSet*>(pg);
	if (n)
	{
		newMesh->PartitionNodeSet(n);
	}

	FEEdgeSet* e = dynamic_cast<FEEdgeSet*>(pg);
	if (e)
	{
		newMesh->PartitionEdgeSelection();
	}

	return newMesh;
}

//=============================================================================
// FERemoveDuplicateElements
//-----------------------------------------------------------------------------

FEMesh* FERemoveDuplicateElements::Apply(FEMesh* pm)
{
	int i, j, k;

	FEMesh* pnm = new FEMesh(*pm);
	FEMesh& m = *pnm;

	FENodeElementList NEL; NEL.Build(&m);
	int NE = m.Elements();
	int NN = m.Nodes();
	for (i=0; i<NE; ++i) m.Element(i).m_ntag = i;

	// tag all duplicate elements
	for (i=0; i<NN; ++i)
	{
		int ne = NEL.Valence(i);
		for (j=0; j<ne; ++j)
		{
			FEElement_& ej = *NEL.Element(i, j);
			if (ej.m_ntag != -1)
			{
				for (k=j+1; k<ne; ++k)
				{
					FEElement_& ek = *NEL.Element(i, k);
					if ((ek.m_ntag!=-1) && (ej.is_equal(ek))) 
					{
						ej.m_ntag = -1;
						ek.m_ntag = -1;
					}
				}
			}
		}
	}

	// delete tagged elements
	m.DeleteTaggedElements(-1);

	return pnm;
}

//////////////////////////////////////////////////////////////////////
// FEFlattenFaces
//////////////////////////////////////////////////////////////////////

FEMesh* FEFlattenFaces::Apply(FEMesh *pm)
{
	int i, j, ntag;

	// create a new mesh
	FEMesh* pnm = new FEMesh(*pm);
	FEMesh& m = *pnm;

	// tag all nodes
	for (i=0; i<m.Nodes(); ++i) m.Node(i).m_ntag = 0;

	for (i=0; i<m.Faces(); ++i)
	{
		FEFace& f = m.Face(i);
		if (f.IsSelected())
		{
			for (j=0; j<f.Nodes(); ++j) m.Node(f.n[j]).m_ntag = 1;
		}
	}

	// calculate the average normal
	vec3d na;
	if (!m_bun)
	{
		for (i=0; i<m.Faces(); ++i)
		{
			FEFace& f = m.Face(i);
			if (f.IsSelected()) na += f.m_fn;
		}
	}
	else na = m_na;

	// make sure our vector is normal
	na.Normalize();

	BOX box = m.GetBoundingBox();
	double R = box.GetMaxExtent();

	// find the lowest point
	vec3d p = box.Center() + na*R*2;
	double d0 = na*p, d;
	for (i=0, ntag = 0; i<m.Nodes(); ++i)
	{
		FENode& n = m.Node(i);
		if (n.m_ntag)
		{
			d = na*n.r;
			if (d < d0) 
			{
				p = n.r;
				d0 = d;
			}
			++ntag;
		}
	}

	vector<int> tag(ntag);
	vector<double>	wgt(m.Nodes());
	for (i=0, ntag = 0; i<m.Nodes(); ++i)
	{
		if (m.Node(i).m_ntag) tag[ntag++] = i;
	}

	// find the distance to project for the tagged nodes
	for (i=0; i<m.Nodes(); ++i)
	{
		FENode& n = m.Node(i);
		if (n.m_ntag) wgt[i] = na*(n.r - p);
	}

	// find the distance to project for the non-tagged nodes
	for (i=0; i<m.Nodes(); ++i)
	{
		FENode& n = m.Node(i);
		if (n.m_ntag == 0)
		{
			// find the closest tagged node
			vec3d& r0 = n.r;
			double dmin = 1e300;
			int jmin = -1;
			for (j=0; j<ntag; ++j)
			{
				vec3d& r1 = m.Node(tag[j]).r;
				d = (r1-r0)*(r1-r0);
				if (d < dmin) { dmin = d; jmin = tag[j]; }
			}

			assert(jmin != -1);

			double f = 1 - sqrt(dmin)/m_rad;

			wgt[i] = wgt[jmin]*f;
			if (wgt[i] < 0) wgt[i] = 0;
		}
	}

	// project all nodes
	for (i=0; i<m.Nodes(); ++i)
	{
		FENode& node = m.Node(i);
		if (wgt[i] > 0.0) node.r -= na*wgt[i];
	}

	// update geometry
	m.UpdateNormals();
	m.UpdateBox();

	return pnm;
}

//=============================================================================
// FEAlignNodes
//=============================================================================

FEAlignNodes::FEAlignNodes() : FEModifier("Align")
{
	AddChoiceParam(0, "align", "align")->SetEnumNames("+X\0-X\0+Y\0-Y\0+Z\0-Z\0");
}

FEMesh* FEAlignNodes::Apply(FEMesh* pm)
{
	int nalign = GetIntValue(0);

	FEMesh* pnm = new FEMesh(*pm);

	vec3d rc;
	int iref = -1;
	for (int i=0; i<pnm->Nodes(); ++i)
	{
		FENode& node = pnm->Node(i);
		vec3d ri = node.pos();
		if (node.IsSelected())
		{
			if (iref == -1) 
			{
				iref = i;
				rc = ri;
			}
			else
			{
				switch (nalign)
				{
				case 0: if (ri.x > rc.x) rc.x = ri.x; break;
				case 1: if (ri.x < rc.x) rc.x = ri.x; break;
				case 2: if (ri.y > rc.y) rc.y = ri.y; break;
				case 3: if (ri.y < rc.y) rc.y = ri.y; break;
				case 4: if (ri.z > rc.z) rc.z = ri.z; break;
				case 5: if (ri.z < rc.z) rc.z = ri.z; break;
				}
			}
		}
	}

	if (iref == -1) { delete pnm; return 0; }

	for (int i = 0; i<pnm->Nodes(); ++i)
	{
		FENode& node = pnm->Node(i);
		if (node.IsSelected())
		{
			vec3d ri = node.pos();

			switch (nalign)
			{
			case 0:
			case 1: ri.x = rc.x; break;
			case 2:
			case 3: ri.y = rc.y; break;
			case 4:
			case 5: ri.z = rc.z; break;
			}

			node.pos(ri);
		}
	}

	return pnm;
}


//////////////////////////////////////////////////////////////////////
// FESetShellThickness
//////////////////////////////////////////////////////////////////////

FESetShellThickness::FESetShellThickness() : FEModifier("Set shell thickness")
{
	AddDoubleParam(0, "h", "h");
}

FEMesh* FESetShellThickness::Apply(FEMesh *pm)
{
	FEMesh* pnm = new FEMesh(*pm);

	double thick = GetFloatValue(0);
	double percent = 0;	// TODO: Add a parameter for this

	if (thick != 0 )
	{
		for (int i=0; i<pnm->Elements(); ++i)
		{
			FEElement& el = pnm->Element(i);
			if (el.IsSelected())
			{
				double* h = el.m_h;
                for (int j=0; j<el.Nodes(); ++j) h[j] = thick;
			}
		}
	}
	else if (percent != 0)
	{
		for (int i=0; i<pnm->Elements(); ++i)
		{
			FEElement& el = pnm->Element(i);
			if (el.IsSelected())
			{
				double* h = el.m_h;
                double H = h[0] * percent;
				for (int j=0; j<el.Nodes(); ++j) h[j] = H;
			}
		}
	}

	return pnm;
}

//------------------------------------------------------------------------
// FESetFiberOrientation
//------------------------------------------------------------------------

FESetFiberOrientation::FESetFiberOrientation() : FEModifier("Set fiber orientation")
{
	AddChoiceParam(0, "generator")->SetEnumNames("vector\0node numbering\0");
	AddVecParam(vec3d(1,0,0), "vector");
	AddIntParam(0, "n0");
	AddIntParam(1, "n1");
}

FEMesh* FESetFiberOrientation::Apply(FEMesh *pm)
{
	FEMesh* pnm = new FEMesh(*pm);

	int ngen = GetIntValue(0);
	switch (ngen)
	{
	case 0: SetFiberVector(pnm); break;
	case 1: SetFiberNodes (pnm); break;
	default:
		delete pnm;
		assert(false);
		return nullptr;
	}

	return pnm;
}

void FESetFiberOrientation::SetFiberVector(FEMesh *pm)
{
	vec3d r = GetVecValue(1);
	r.Normalize();

	int nsel = 0;
	for (int i=0; i<pm->Elements(); ++i)
		if (pm->Element(i).IsSelected()) nsel++;

	for (int i=0; i<pm->Elements(); ++i)
	{
		FEElement& el = pm->Element(i);
		if (el.IsSelected() || (nsel==0))
			el.m_fiber = r;
	}
}

void FESetFiberOrientation::SetFiberNodes(FEMesh *pm)
{
	int nsel = 0;
	for (int i = 0; i<pm->Elements(); ++i)
		if (pm->Element(i).IsSelected()) nsel++;

	vec3d r1, r2, n;
	int node0 = GetIntValue(2);
	int node1 = GetIntValue(3);
	for (int i = 0; i<pm->Elements(); ++i)
	{
		FEElement& el = pm->Element(i);
		if (el.IsSelected() || (nsel == 0))
		{
			r1 = pm->Node(el.m_node[ node0 ]).r;
			r2 = pm->Node(el.m_node[ node1 ]).r;
			n = r2 - r1;
			n.Normalize();
			el.m_fiber = n;
		}
	}
}

//------------------------------------------------------------------------
// FESetAxesOrientation
//------------------------------------------------------------------------

FESetAxesOrientation::FESetAxesOrientation() : FEModifier("Set axes orientation")
{
	AddChoiceParam(0, "generator")->SetEnumNames("vector\0node numbering\0");
	AddVecParam(vec3d(1, 0, 0), "a");
	AddVecParam(vec3d(0, 1, 0), "d");
	AddIntParam(0, "n0");
	AddIntParam(1, "n1");
	AddIntParam(2, "n2");
}

FEMesh* FESetAxesOrientation::Apply(FEMesh *pm)
{
	FEMesh* pnm = new FEMesh(*pm);

	pnm->TagAllElements(-1);
	int nsel = 0;
	for (int i=0; i<pnm->Elements(); ++i)
	{
		FEElement& el = pnm->Element(i);
		if (el.IsSelected())
		{
			el.m_ntag = 1;
			nsel++;
		}
	}
	if (nsel == 0) pnm->TagAllElements(1);

	int ngen = GetIntValue(0);
	bool bret = false;
	switch (ngen)
	{
		case 0: bret = SetAxesVectors(pnm); break;
		case 1: bret = SetAxesNodes(pnm); break;
//		case 2: SetAxesCopy  (pnm); break;
		default:
			assert(false);
	}

	if (bret == false)
	{	
		delete pnm;
		pnm = 0;
	}
	
	return pnm;
}

bool FESetAxesOrientation::SetAxesVectors(FEMesh *pm)
{
	vec3d a = GetVecValue(1);
	vec3d d = GetVecValue(2);

	vec3d c = a^d;
	vec3d b = c^a;
	a.Normalize();
	b.Normalize();
	c.Normalize();
	for (int i=0; i<pm->Elements(); ++i)
	{
		FEElement& el = pm->Element(i);
		if (el.m_ntag == 1)
		{
			mat3d& m = el.m_Q;
			m.zero();
			m[0][0] = a.x; m[0][1] = b.x; m[0][2] = c.x;
			m[1][0] = a.y; m[1][1] = b.y; m[1][2] = c.y;
			m[2][0] = a.z; m[2][1] = b.z; m[2][2] = c.z;
			el.m_Qactive = true;
		}
	}

	return true;
}

bool FESetAxesOrientation::SetAxesNodes(FEMesh *pm)
{
	int node[3] = {0, 1, 2};
	node[0] = GetIntValue(3);
	node[1] = GetIntValue(4);
	node[2] = GetIntValue(5);

	vec3d r1, r2, r3, a, b, c, d;
	for (int i=0; i<pm->Elements(); ++i)
	{
		FEElement& el = pm->Element(i);
		if (el.m_ntag == 1)
		{
			int neln = el.Nodes();
			if ((node[0] < 0) || (node[0] >= neln)) return false;
			if ((node[1] < 0) || (node[1] >= neln)) return false;
			if ((node[2] < 0) || (node[2] >= neln)) return false;

			r1 = pm->Node(el.m_node[ node[0] ]).r;
			r2 = pm->Node(el.m_node[ node[1] ]).r;
			r3 = pm->Node(el.m_node[ node[2] ]).r;
			a = r2 - r1;
			d = r3 - r1;
			c = a^d;
			b = c^a;
			a.Normalize();
			b.Normalize();
			c.Normalize();
			mat3d& m = el.m_Q;
			m.zero();
			m[0][0] = a.x; m[0][1] = b.x; m[0][2] = c.x;
			m[1][0] = a.y; m[1][1] = b.y; m[1][2] = c.y;
			m[2][0] = a.z; m[2][1] = b.z; m[2][2] = c.z;
			el.m_Qactive = true;
		}
	}

	return true;
}

bool FESetAxesOrientation::SetAxesCopy(FEMesh *pm)
{
/*	assert(m_pms);
	
	int i, j, n;
	
	// create the array of source data
	int NY = m_pms->Elements();
	vector<vec3d> Y(NY);
	for (i=0; i<NY; ++i)
	{
		FEElement& el = m_pms->Element(i);
		n = el.Nodes();
		vec3d c(0,0,0);
		for (j=0; j<n; ++j) c += m_pms->Node(el.m_node[j]).r;
		c /= n;
		Y[i] = c;
	}
	
	// set up the nearest node search
	FENNQuery q(&Y);
	q.Init();
	
	// do the mapping
	int N = pm->Elements();
	for (i=0; i<N; ++i)
	{
		FEElement& el = pm->Element(i);
		n = el.Nodes();
		vec3d c(0,0,0);
		for (j=0; j<n; ++j) c += pm->Node(el.m_node[j]).r;
		c /= n;
		n = q.Find(c);
		
		FEElement& els = m_pms->Element(n);
		el.m_Q = els.m_Q;
		el.m_Qactive = els.m_Qactive;
		
		// if the element is a shell, we project the fiber on the shell
		if (el.IsShell())
		{
			vec3d x[4];
			n = el.Nodes();
			for (j=0; j<n; ++j) x[j] = pm->Node(el.m_node[j]).r;
			vec3d e1, e2;
			if (n==4)
			{
				e1 = -x[0] + x[1] + x[2] - x[3];
				e2 = -x[0] - x[1] + x[2] + x[3];
			}
			else
			{
				e1 = x[1] - x[0];
				e2 = x[2] - x[0];
			}
			vec3d f = e1^e2;
			f.Normalize();
			
			el.m_fiber -= f*(f*el.m_fiber);
		}
	}
*/
	return true;
}

//=============================================================================
// FEMirrorMesh
//-----------------------------------------------------------------------------

FEMirrorMesh::FEMirrorMesh() : FEModifier("Mirror") 
{ 
	AddIntParam(0, "plane", "Mirror plane")->SetEnumNames("X-plane\0Y-plane\0Z-plane\0");
	AddVecParam(vec3d(0,0,0), "center", "Center");
}

FEMesh* FEMirrorMesh::Apply(FEMesh *pm)
{
	int nplane = GetIntValue(0);
	vec3d rc = GetVecValue(1);

	FEMesh* pmn = new FEMesh(*pm);

	// mirror the nodes
	for (int i=0; i<pmn->Nodes(); ++i)
	{
		FENode& n = pmn->Node(i);
		vec3d r = n.r - rc;
		switch (nplane)
		{
		case 0: r.x = -r.x; break;
		case 1: r.y = -r.y; break;
		case 2: r.z = -r.z; break;
		}
		n.r = r + rc;
	}

	// invert elements
	pmn->InvertSelectedElements();

	return pmn;
}

//=============================================================================
// FEQuad2Tri
//-----------------------------------------------------------------------------

FEMesh* FEQuad2Tri::Apply(FEMesh *pm)
{
	int i;
	int NN = pm->Nodes();
	int NE0 = pm->Elements();
	int NF0 = pm->Faces();
	int NC = pm->Edges();

	// count number of elements
	int NE1 = 0;
	for (i=0; i<NE0; ++i)
	{
		if (pm->Element(i).IsType(FE_QUAD4)) NE1 += 2; else NE1++;
	}

	// count number of faces
	int NF1 = 0;
	for (i=0; i<NF0; ++i)
	{
		if (pm->Face(i).Nodes() == 4) NF1 += 2; else NF1++;
	}

	// create a new mesh
	FEMesh* pnew = new FEMesh;
	pnew->Create(NN, NE1, NF1, NC);

	// copy nodes
	for (i=0; i<NN; ++i) pnew->Node(i) = pm->Node(i);

	// copy edges
	for (i=0; i<NC; ++i) pnew->Edge(i) = pm->Edge(i);

	// copy elements
	NE1 = 0;
	for (i=0; i<NE0; ++i)
	{
		FEElement& e0 = pm->Element(i);
		if (e0.IsType(FE_QUAD4))
		{
			FEElement& e1 = pnew->Element(NE1++);
			FEElement& e2 = pnew->Element(NE1++);

			e1 = e0;
			e2 = e0;

			e1.SetType(FE_TRI3);
			e2.SetType(FE_TRI3);
			e1.m_node[0] = e0.m_node[0];
			e1.m_node[1] = e0.m_node[1];
			e1.m_node[2] = e0.m_node[2];
			e1.m_h[0] = e0.m_h[0];
			e1.m_h[1] = e0.m_h[1];
			e1.m_h[2] = e0.m_h[2];

			e2.m_node[0] = e0.m_node[2];
			e2.m_node[1] = e0.m_node[3];
			e2.m_node[2] = e0.m_node[0];
			e2.m_h[0] = e0.m_h[2];
			e2.m_h[1] = e0.m_h[3];
			e2.m_h[2] = e0.m_h[0];
		}
		else
		{
			FEElement& e1 = pnew->Element(NE1++);
			e1 = e0;
		}
	}

	// copy faces
	NF1 = 0;
	for (i=0; i<NF0; ++i)
	{
		FEFace& f0 = pm->Face(i);
		if (f0.Nodes() == 4)
		{
			FEFace& f1 = pnew->Face(NF1++);
			FEFace& f2 = pnew->Face(NF1++);

			f1 = f0;
			f2 = f0;

			f1.SetType(FE_FACE_TRI3);
			f2.SetType(FE_FACE_TRI3);

			f1.n[0] = f0.n[0];
			f1.n[1] = f0.n[1];
			f1.n[2] = f0.n[2];

			f2.n[0] = f0.n[2];
			f2.n[1] = f0.n[3];
			f2.n[2] = f0.n[0];
		}
		else
		{
			FEFace& f1 = pnew->Face(NF1++);
			f1 = f0;
		}
	}

	pnew->Update();

	return pnew;
}

//=============================================================================
// FERefineMesh
//-----------------------------------------------------------------------------

FERefineMesh::FERefineMesh() : FEModifier("Refine")
{
	AddIntParam(1, "iterations:", "iterations");
	AddBoolParam(false, "Smooth surface");
}

FEMesh* FERefineMesh::Apply(FEMesh* pm)
{
	// get parameters
	int niter = GetIntValue(0);
	bool bsmooth = GetBoolValue(1);
	if (niter < 1) return 0;

	// select a modifier
	FEModifier* mod = 0;
	if (pm->IsType(FE_QUAD4))
	{
		mod = new FEQuadSplitModifier;
	}
	if (pm->IsType(FE_TRI3))
	{
		mod = new FETriSplitModifier;
	}
	if (pm->IsType(FE_TET4))
	{
		mod = new FETetSplitModifier;
	}
	if (pm->IsType(FE_HEX8))
	{
		FEHexSplitModifier* hexmod = new FEHexSplitModifier;
		hexmod->DoSurfaceSmoothing(bsmooth);
		mod = hexmod;
	}
	if (mod == 0) return 0;

	// apply the refine modifier
	FEMesh* pmold = pm;
	for (int i=0; i<niter; ++i)
	{
		FEMesh* pmnew = mod->Apply(pmold);
		if (i != 0) delete pmold;
		pmold = pmnew;
	}

	delete mod;

	return pmold;
}

//=============================================================================
// FEConvertMesh
//-----------------------------------------------------------------------------

FEConvertMesh::FEConvertMesh() : FEModifier("Convert")
{
	const char* szops = \
		"Quad4 to Tri3\0"\
		"Hex8 to Tet4\0"\
		"Tet4 to Tet5\0"\
		"Tet4 to Tet10\0"\
		"Tet4 to Tet15\0"\
		"Tet4 to Tet20\0"\
		"Tet5 to Tet4\0"\
		"Tet10 to Tet4\0"\
		"Tet15 to Tet4\0"\
		"Hex8 to Hex20\0"\
		"Hex20 to Hex8\0"\
		"Quad4 to Quad8\0"\
		"Quad8 to Quad4\0"\
		"Tri3 to Tri6\0"\
		"Tri6 to Tri 3\0"\
		"Linear to Quadratic\0"\
		"Quadratic to Linear\0\0";

	AddIntParam(0, "convert", "Convert")->SetEnumNames(szops);
	AddBoolParam(false, "smooth", "smooth quadratic elements");
}

FEMesh* FEConvertMesh::Apply(FEMesh* pm)
{
	int nsel = GetIntValue(0);
	bool bsmooth = GetBoolValue(1);

	FEModifier* pmod = 0;
	switch (nsel)
	{
	case 0: pmod = new FEQuad2Tri; break;
	case 1: pmod = new FEHex2Tet; break;
	case 2: pmod = new FETet4ToTet5; break;
	case 3:
		{
			  FETet4ToTet10* pfe = new FETet4ToTet10;
			  pfe->SetSmoothing(bsmooth);
			  pmod = pfe;
		}
		break;
	case 4:
		{
			  FETet4ToTet15* pfe = new FETet4ToTet15;
			  pfe->SetSmoothing(bsmooth);
			  pmod = pfe;
		}
		break;
	case 5: pmod = new FETet4ToTet20; break;
	case 6: pmod = new FETet5ToTet4; break;
	case 7: pmod = new FETet10ToTet4; break;
	case 8: pmod = new FETet15ToTet4; break;
	case 9:
		{
			  FEHex8ToHex20* pfe = new FEHex8ToHex20;
			  pfe->SetSmoothing(bsmooth);
			  pmod = pfe;
		}
		break;
	case 10: pmod = new FEHex20ToHex8; break;
	case 11:
		{
			  FEQuad4ToQuad8* pfe = new FEQuad4ToQuad8;
			  pfe->SetSmoothing(bsmooth);
			  pmod = pfe;
		}
		break;
	case 12: pmod = new FEQuad8ToQuad4; break;
	case 13:
		{
			FETri3ToTri6* pfe = new FETri3ToTri6;
			pfe->SetSmoothing(bsmooth);
			pmod = pfe;
		}
		break;
	case 14: pmod = new FETri6ToTri3; break;
	case 15: pmod = new FELinearToQuadratic; break;
	case 16: pmod = new FEQuadraticToLinear; break;
	default:
		FEModifier::SetError("Unknown converter selected");
		assert(false);
		return 0;
	}

	if (pmod)
	{
		FEMesh* newMesh = pmod->Apply(pm);
		delete pmod;
		return newMesh;
	}
	else return 0;
}

//=============================================================================
// FEAddNode
//-----------------------------------------------------------------------------


FEAddNode::FEAddNode() : FEModifier("Add Node")
{
	AddVecParam(vec3d(0,0,0), "position", "position");
}

FEMesh* FEAddNode::Apply(FEMesh* pm)
{
	FEMesh* newMesh = new FEMesh(*pm);

	vec3d r = GetVecValue(0);
	GObject* po = pm->GetGObject();
	if (po) r = po->Transform().GlobalToLocal(r);

	FENode node;
	node.SetExterior(true);
	node.r = r;

	// let's partition the node
	int ng = newMesh->CountNodePartitions();
	node.m_gid = ng;

	newMesh->AddNode(node);

	newMesh->UpdateBox();

	return newMesh;
}

//=============================================================================
// FEInvertElements
//-----------------------------------------------------------------------------


FEInvertMesh::FEInvertMesh() : FEModifier("Invert Mesh")
{
	AddBoolParam(true, "Invert elements", "Invert elements");
	AddBoolParam(true, "Invert faces", "Invert faces");
}

FEMesh* FEInvertMesh::Apply(FEMesh* pm)
{
	bool invertElems = GetBoolValue(0);
	bool invertFaces = GetBoolValue(1);

	FEMesh* newMesh = new FEMesh(*pm);

	if (invertElems)
	{
		newMesh->InvertSelectedElements();
	}
	else if (invertFaces)
	{
		newMesh->InvertSelectedFaces();
	}
	else
	{
		delete newMesh;
		return nullptr;
	}

	return newMesh;
}

//=============================================================================
// FEDetachElements
//-----------------------------------------------------------------------------

FEDetachElements::FEDetachElements() : FEModifier("Detach Elements")
{
	AddBoolParam(true, "Repartition selection");
}

FEMesh* FEDetachElements::Apply(FEMesh* pm)
{
	// Figure out which nodes need to be duplicated
	pm->TagAllNodes(-1);
	int NE = pm->Elements();
	for (int i=0; i<NE; ++i)
	{
		FEElement& el = pm->Element(i);
		if (el.IsSelected())
		{
			// solid element
			int nf = el.Faces();
			for (int j=0; j<nf; ++j)
			{
				int nbr = el.m_nbr[j];
				if (nbr >= 0)
				{
					FEElement& elj = pm->Element(nbr);
					if (elj.IsSelected() == false)
					{
						FEFace f = el.GetFace(j);
						int nf = f.Nodes();
						for (int k=0; k<nf; ++k) pm->Node(f.n[k]).m_ntag = 1;
					}
				}
			}

			// shell element
			int nc = el.Edges();
			for (int j=0; j<nc; ++j)
			{
				int nbr = el.m_nbr[j];
				if (nbr >= 0)
				{
					FEElement& elj = pm->Element(nbr);
					if (elj.IsSelected() == false)
					{
						FEEdge e = el.GetEdge(j);
						int ne = e.Nodes();
						for (int k = 0; k<ne; ++k) pm->Node(e.n[k]).m_ntag = 1;
					}
				}
			}
		}
	}

	// count the new nodes
	int NN = pm->Nodes();
	int nn1 = NN;
	for (int i=0; i<NN; ++i)
	{
		FENode& node = pm->Node(i);
		if (node.m_ntag == 1) 
		{
			node.m_ntag = nn1++;
		}
	}

	// create a new mesh
	FEMesh* newMesh = new FEMesh;
	newMesh->Create(nn1, NE);

	// create nodes
	for (int i=0; i<NN; ++i) 
	{
		FENode& si = pm->Node(i);
		FENode& di = newMesh->Node(i);
		di = si;
		if (si.m_ntag != -1)
		{
			FENode& di2 = newMesh->Node(si.m_ntag);
			di2 = si;
		}
	}

	// create elements
	for (int i=0; i<NE; ++i)
	{
		FEElement& si = pm->Element(i);
		FEElement& di = newMesh->Element(i);
		di = si;

		if (si.IsSelected())
		{
			for (int j=0; j<si.Nodes(); ++j)
			{
				FENode& nj = pm->Node(si.m_node[j]);
				if (nj.m_ntag != -1)
				{
					di.m_node[j] = nj.m_ntag;
				}
			}
		}
	}

	// see if we need to repartition
	bool repart = GetBoolValue(0);
	if (repart)
	{
		int ng = pm->CountElementPartitions();
		for (int i=0; i<NE; ++i)
		{
			FEElement& si = pm->Element(i);
			FEElement& di = newMesh->Element(i);

			if (si.IsSelected())
			{
				di.m_gid += ng;
			}
		}
		newMesh->UpdateElementPartitions();
	}

	// update the mesh
	newMesh->RebuildMesh();
	
	return newMesh;		
}
