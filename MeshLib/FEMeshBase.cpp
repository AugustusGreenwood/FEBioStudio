/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include"FEMeshBase.h"
#include <GeomLib/GObject.h>

//-----------------------------------------------------------------------------
FSMeshBase::FSMeshBase()
{
}

//-----------------------------------------------------------------------------
FSMeshBase::~FSMeshBase()
{
	m_NFL.Clear();
}

//-----------------------------------------------------------------------------
// get the local node positions of a face
void FSMeshBase::FaceNodeLocalPositions(const FEFace& f, vec3d* r) const
{
	int nf = f.Nodes();
	for (int i = 0; i<nf; ++i) r[i] = m_Node[f.n[i]].r;
}

//-----------------------------------------------------------------------------
// Tag all faces
void FSMeshBase::TagAllFaces(int ntag)
{
	const int NF = Faces();
	for (int i = 0; i<NF; ++i) Face(i).m_ntag = ntag;
}

//-----------------------------------------------------------------------------
bool FSMeshBase::IsEdge(int n0, int n1)
{
	int NE = Edges();
	for (int i = 0; i<NE; ++i)
	{
		FEEdge& e = Edge(i);
		if ((e.n[0] == n0) && (e.n[1] == n1)) return true;
		if ((e.n[0] == n1) && (e.n[1] == n0)) return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
FEEdge* FSMeshBase::FindEdge(int n0, int n1)
{
	int NE = Edges();
	for (int i = 0; i<NE; ++i)
	{
		FEEdge& e = Edge(i);
		if ((e.n[0] == n0) && (e.n[1] == n1)) return &e;
		if ((e.n[0] == n1) && (e.n[1] == n0)) return &e;
	}
	return 0;
}

//-----------------------------------------------------------------------------
bool FSMeshBase::IsCreaseEdge(int n0, int n1)
{
	int NE = Edges();
	for (int i = 0; i<NE; ++i)
	{
		FEEdge& e = Edge(i);
		if ((e.n[0] == n0) && (e.n[1] == n1)) return (e.m_gid != -1);
		if ((e.n[0] == n1) && (e.n[1] == n0)) return (e.m_gid != -1);
	}
	return false;
}

//-----------------------------------------------------------------------------
const vector<NodeFaceRef>& FSMeshBase::NodeFaceList(int n) const 
{ 
	return m_NFL.FaceList(n); 
}

//-----------------------------------------------------------------------------
// Remove faces with tag ntag
void FSMeshBase::RemoveFaces(int ntag)
{
	int n = 0;
	for (int i = 0; i<Faces(); ++i)
	{
		FEFace& f1 = Face(i);
		FEFace& f2 = Face(n);

		if (f1.m_ntag != ntag)
		{
			if (i != n) f2 = f1;
			n++;
		}
	}
	m_Face.resize(n);
}

//-----------------------------------------------------------------------------
// Remove edges with tag ntag
void FSMeshBase::RemoveEdges(int ntag)
{
	int n = 0;
	for (int i = 0; i<Edges(); ++i)
	{
		FEEdge& e1 = Edge(i);
		FEEdge& e2 = Edge(n);

		if (e1.m_ntag != ntag)
		{
			if (i != n) e2 = e1;
			n++;
		}
	}
	m_Edge.resize(n);
}

//-----------------------------------------------------------------------------
// This function assignes group ID's to the mesh' faces based on a smoothing
// angle.
//
void FSMeshBase::AutoSmooth(double angleDegrees)
{
	int NF = Faces();

	// smoothing threshold
	double eps = (double)cos(angleDegrees * DEG2RAD);

	// clear face group ID's
	for (int i = 0; i<NF; ++i)
	{
		FEFace* pf = FacePtr(i);
		pf->m_sid = -1;
	}

	// calculate face normals
	for (int i = 0; i<NF; ++i)
	{
		FEFace* pf = FacePtr(i);

		// calculate the face normals
		vec3d& r0 = Node(pf->n[0]).r;
		vec3d& r1 = Node(pf->n[1]).r;
		vec3d& r2 = Node(pf->n[2]).r;

		pf->m_fn = to_vec3f((r1 - r0) ^ (r2 - r0));
		pf->m_fn.Normalize();
	}


	// stack for tracking unprocessed faces
	vector<FEFace*> stack(NF);
	int ns = 0;

	// process all faces
	int nsg = 0;
	for (int i = 0; i<NF; ++i)
	{
		FEFace* pf = FacePtr(i);
		if (pf->m_sid == -1)
		{
			stack[ns++] = pf;
			while (ns > 0)
			{
				// pop a face
				pf = stack[--ns];

				// mark as processed
				pf->m_sid = nsg;

				// loop over neighbors
				int n = pf->Edges();
				for (int j = 0; j<n; ++j)
				{
					FEFace* pf2 = FacePtr(pf->m_nbr[j]);

					// push unprocessed neighbour
					if (pf2 && (pf2->m_sid == -1) && (pf->m_fn*pf2->m_fn >= eps))
					{
						pf2->m_sid = -2;
						stack[ns++] = pf2;
					}
				}
			}
			++nsg;
		}
	}

	// update the normals
	UpdateNormals();
}

//-----------------------------------------------------------------------------
// Calculate normals of the mesh' faces based on smoothing groups
//
void FSMeshBase::UpdateNormals()
{
	int NN = Nodes();
	int NF = Faces();

	// calculate face normals
	for (int i = 0; i<NF; ++i)
	{
		FEFace* pf = FacePtr(i);

		// reset smoothing id
		pf->m_ntag = -1;

		// calculate the face normals
		vec3d& r0 = Node(pf->n[0]).r;
		vec3d& r1 = Node(pf->n[1]).r;
		vec3d& r2 = Node(pf->n[2]).r;

		pf->m_fn = to_vec3f((r1 - r0) ^ (r2 - r0));

		int nf = pf->Nodes();
		for (int j = 0; j<nf; ++j) pf->m_nn[j] = pf->m_fn;
	}

	// buffer for storing node normals
	vector<vec3f> norm(NN, vec3f(0.f, 0.f, 0.f));

	// this array keeps track of all faces in a smoothing group
	vector<FEFace*> F(NF);
	int FC = 0;

	// this array is used as a stack when processing neighbors
	vector<FEFace*> stack(NF);
	int ns = 0;

	// loop over all faces
	int nsg = 0;
	for (int i = 0; i<NF; ++i)
	{
		FEFace* pf = FacePtr(i);
		if (pf->m_ntag == -1)
		{
			// find all connected faces
			stack[ns++] = pf;
			while (ns > 0)
			{
				// pop a face
				pf = stack[--ns];

				// mark as processed
				pf->m_ntag = nsg;
				F[FC++] = pf;

				// add face normal to node normal
				int n = pf->Nodes();
				for (int j = 0; j<n; ++j) norm[pf->n[j]] += pf->m_fn;

				// process neighbors
				n = pf->Edges();
				for (int j = 0; j<n; ++j)
				{
					FEFace* pf2 = FacePtr(pf->m_nbr[j]);
					// push unprocessed neighbor
					if (pf2 && (pf2->m_ntag == -1) && (pf->m_sid == pf2->m_sid))
					{
						pf2->m_ntag = -2;
						stack[ns++] = pf2;
					}
				}
			}

			// assign node normals
			for (int j = 0; j<FC; ++j)
			{
				pf = F[j];
				assert(pf->m_ntag == nsg);
				int nf = pf->Nodes();
				for (int k = 0; k<nf; ++k) pf->m_nn[k] = norm[pf->n[k]];
			}

			// clear normals
			for (int j = 0; j<FC; ++j)
			{
				pf = F[j];
				int nf = pf->Nodes();
				for (int k = 0; k<nf; ++k) norm[pf->n[k]] = vec3f(0.f, 0.f, 0.f);
			}
			++nsg;
			FC = 0;
		}
	}

	// normalize face normals
	FEFace* pf = FacePtr();
	for (int i = 0; i<NF; ++i, ++pf)
	{
		pf->m_fn.Normalize();
		int n = pf->Nodes();
		for (int j = 0; j<n; ++j) pf->m_nn[j].Normalize();
	}
}

//-----------------------------------------------------------------------------
// assign smoothing IDs based on surface partition
void FSMeshBase::SmoothByPartition()
{
	// assign group IDs to smoothing IDs
	for (int i=0; i<Faces(); ++i)
	{
		FEFace& face = Face(i);
		face.m_sid = face.m_gid;
	}	

	// update the normals
	UpdateNormals();
}

//-----------------------------------------------------------------------------
void FSMeshBase::UpdateMesh()
{
	UpdateNormals();
	UpdateBoundingBox();
}

//-----------------------------------------------------------------------------
int FSMeshBase::CountSelectedNodes() const
{
	int N = 0, NN = Nodes();
	for (int i = 0; i < NN; ++i)
	{
		if (Node(i).IsSelected()) N++;
	}
	return N;
}

//-----------------------------------------------------------------------------
int FSMeshBase::CountSelectedEdges() const
{
	int N = 0, NE = Edges();
	for (int i = 0; i < NE; ++i)
	{
		if (Edge(i).IsSelected()) N++;
	}
	return N;
}

//-----------------------------------------------------------------------------
int FSMeshBase::CountSelectedFaces() const
{
	int N = 0, NF = Faces();
	for (int i = 0; i<NF; ++i)
	{
		if (Face(i).IsSelected()) N++;
	}
	return N;
}

//-----------------------------------------------------------------------------
vec3d FSMeshBase::FaceCenter(FEFace& f) const
{
	vec3d r;
	int N = f.Nodes();
	for (int i = 0; i<N; i++) r += m_Node[f.n[i]].r;
	return r / (float)N;
}

//-----------------------------------------------------------------------------
vec3d FSMeshBase::EdgeCenter(FEEdge& e) const
{
	return (m_Node[e.n[0]].r + m_Node[e.n[1]].r)*0.5f;
}

//-----------------------------------------------------------------------------
// area of triangle
double triangle_area(const vec3d& r0, const vec3d& r1, const vec3d& r2)
{
	return ((r1 - r0) ^ (r2 - r0)).Length()*0.5f;
}

//-----------------------------------------------------------------------------
double FSMeshBase::FaceArea(FEFace &f)
{
	const int N = f.Nodes();
	vector<vec3d> nodes(N);
	for (int i = 0; i < N; ++i)
	{
		nodes[i] = m_Node[f.n[i]].r;
	}
	return FaceArea(nodes, N);
}

//-----------------------------------------------------------------------------
void FSMeshBase::ClearFaceSelection()
{
	for (int i = 0; i<Faces(); ++i)
	{
		Face(i).Unselect();
	}
}

//-----------------------------------------------------------------------------
double FSMeshBase::FaceArea(const vector<vec3d>& r, int faceType)
{
	switch (faceType)
	{
	case 3:
	{
		return triangle_area(r[0], r[1], r[2]);
	}
	break;
	case 6:
	{
		double A = 0.0;
		A += triangle_area(r[0], r[3], r[5]);
		A += triangle_area(r[3], r[1], r[4]);
		A += triangle_area(r[2], r[5], r[4]);
		A += triangle_area(r[3], r[4], r[5]);
		return A;
	}
	break;
	case 7:
	{
		return triangle_area(r[0], r[1], r[2]);
	}
	break;
	case 4:
	{
		int i, n;

		// gauss-point data
		const float a = 1.f / (float)sqrt(3.0);
		const int NELN = 4;
		const int NINT = 4;
		static float gr[NINT] = { -a,  a,  a, -a };
		static float gs[NINT] = { -a, -a,  a,  a };
		static float gw[NINT] = { 1,  1,  1,  1 };

		static float H[NINT][NELN] = { 0 };
		static float Gr[NINT][NELN] = { 0 };
		static float Gs[NINT][NELN] = { 0 };
		static bool bfirst = true;

		if (bfirst)
		{

			// calculate shape function values at gauss points
			for (n = 0; n<NINT; ++n)
			{
				H[n][0] = 0.25f*(1 - gr[n])*(1 - gs[n]);
				H[n][1] = 0.25f*(1 + gr[n])*(1 - gs[n]);
				H[n][2] = 0.25f*(1 + gr[n])*(1 + gs[n]);
				H[n][3] = 0.25f*(1 - gr[n])*(1 + gs[n]);
			}

			// calculate local derivatives of shape functions at gauss points
			for (n = 0; n<NINT; ++n)
			{
				Gr[n][0] = -0.25f*(1 - gs[n]);
				Gr[n][1] = 0.25f*(1 - gs[n]);
				Gr[n][2] = 0.25f*(1 + gs[n]);
				Gr[n][3] = -0.25f*(1 + gs[n]);

				Gs[n][0] = -0.25f*(1 - gr[n]);
				Gs[n][1] = -0.25f*(1 + gr[n]);
				Gs[n][2] = 0.25f*(1 + gr[n]);
				Gs[n][3] = 0.25f*(1 - gr[n]);
			}

			bfirst = false;
		}

		float A = 0.f;
		for (n = 0; n<NINT; ++n)
		{
			// calculate jacobian
			vec3d dxr, dxs;
			for (i = 0; i<NELN; ++i)
			{
				dxr.x += Gr[n][i] * r[i].x;
				dxr.y += Gr[n][i] * r[i].y;
				dxr.z += Gr[n][i] * r[i].z;

				dxs.x += Gs[n][i] * r[i].x;
				dxs.y += Gs[n][i] * r[i].y;
				dxs.z += Gs[n][i] * r[i].z;
			}

			double detJ = (dxr ^ dxs).Length();

			A += gw[n] * detJ;
		}

		return A;
	}
	break;
	}

	return 0.0;
}

//-----------------------------------------------------------------------------
void FSMeshBase::FaceNodePosition(const FEFace& f, vec3d* r) const
{
	switch (f.m_type)
	{
	case FE_FACE_TRI10:
		r[9] = m_Node[f.n[9]].r;
	case FE_FACE_QUAD9:
		r[8] = m_Node[f.n[8]].r;
	case FE_FACE_QUAD8:
		r[7] = m_Node[f.n[7]].r;
	case FE_FACE_TRI7:
		r[6] = m_Node[f.n[6]].r;
	case FE_FACE_TRI6:
		r[5] = m_Node[f.n[5]].r;
		r[4] = m_Node[f.n[4]].r;
	case FE_FACE_QUAD4:
		r[3] = m_Node[f.n[3]].r;
	case FE_FACE_TRI3:
		r[2] = m_Node[f.n[2]].r;
		r[1] = m_Node[f.n[1]].r;
		r[0] = m_Node[f.n[0]].r;
		break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void FSMeshBase::FaceNodeNormals(FEFace& f, vec3f* n)
{
	switch (f.m_type)
	{
	case FE_FACE_TRI10:
		n[9] = f.m_nn[9];
	case FE_FACE_QUAD9:
		n[8] = f.m_nn[8];
	case FE_FACE_QUAD8:
		n[7] = f.m_nn[7];
	case FE_FACE_TRI7:
		n[6] = f.m_nn[6];
	case FE_FACE_TRI6:
		n[5] = f.m_nn[5];
		n[4] = f.m_nn[4];
	case FE_FACE_QUAD4:
		n[3] = f.m_nn[3];
	case FE_FACE_TRI3:
		n[2] = f.m_nn[2];
		n[1] = f.m_nn[1];
		n[0] = f.m_nn[0];
		break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void FSMeshBase::FaceNodeTexCoords(FEFace& f, float* t)
{
	for (int i = 0; i<f.Nodes(); ++i) t[i] = f.m_tex[i];
}

//==============================================================================
std::vector<int> MeshTools::GetConnectedFaces(FSMeshBase* pm, int nface, double tolAngleDeg, bool respectPartitions)
{
	vector<int> faceList; 
	faceList.reserve(pm->Faces());

	for (int i = 0; i<pm->Faces(); ++i) pm->Face(i).m_ntag = i;
	std::stack<FEFace*> stack;

	// push the first face to the stack
	FEFace* pf = pm->FacePtr(nface);
	faceList.push_back(nface);
	pf->m_ntag = -1;
	stack.push(pf);

	vec3f Nf = pf->m_fn;
	double wtol = 1.000001*cos(PI*tolAngleDeg / 180.0); // scale factor to address some numerical round-off issue when selecting 180 degrees
	bool bmax = (tolAngleDeg != 0.0);

	int gid = pf->m_gid;

	pm->TagAllNodes(0);
	if (respectPartitions)
	{
		int NE = pm->Edges();
		for (int i = 0; i<NE; ++i)
		{
			FEEdge& e = pm->Edge(i);
			if (e.m_gid != -1)
			{
				pm->Node(e.n[0]).m_ntag = 1;
				pm->Node(e.n[1]).m_ntag = 1;
			}
		}
	}

	// now push the rest
	while (!stack.empty())
	{
		pf = stack.top(); stack.pop();
		int n = pf->Edges();
		for (int i = 0; i<n; ++i)
			if (pf->m_nbr[i] >= 0)
			{
				int n0 = pf->n[i];
				int n1 = pf->n[(i + 1) % n];
				int m0 = pm->Node(n0).m_ntag;
				int m1 = pm->Node(n1).m_ntag;

				FEFace* pf2 = pm->FacePtr(pf->m_nbr[i]);

				bool bpush = true;
				if (pf2->m_ntag < 0) bpush = false;
				else if (pf2->IsVisible() == false) bpush = false;
				else if (bmax && (pf2->m_fn*Nf < wtol)) bpush = false;
				else if (respectPartitions && ((pf2->m_gid != gid) || ((m0 == 1) && (m1 == 1) && pm->IsCreaseEdge(n0, n1)))) bpush = false;

				if (bpush)
				{
					faceList.push_back(pf2->m_ntag);
					pf2->m_ntag = -1;
					stack.push(pf2);
				}
			}
	}

	return faceList;
}

void FSMeshBase::GetNodeNeighbors(int inode, int levels, std::set<int>& nl1)
{
	// add the first node
	nl1.insert(inode);

	// loop over all levels
	vector<int> nl2; nl2.reserve(64);
	for (int k = 0; k <= levels; ++k)
	{
		// reset face marks
		std::set<int>::iterator it;
		for (it = nl1.begin(); it != nl1.end(); ++it)
		{
			// get the node-face list
			const vector<NodeFaceRef>& nfl = NodeFaceList(*it);
			int NF = nfl.size();

			// add the other nodes
			for (int i = 0; i < NF; ++i)
			{
				FEFace& f = Face(nfl[i].fid);
				f.m_ntag = 0;
			}
		}

		// loop over all nodes
		nl2.clear();
		for (it = nl1.begin(); it != nl1.end(); ++it)
		{
			// get the node-face list
			const vector<NodeFaceRef>& nfl = NodeFaceList(*it);
			int NF = nfl.size();

			// add the other nodes
			for (int i = 0; i < NF; ++i)
			{
				FEFace& f = Face(nfl[i].fid);
				if (f.m_ntag == 0)
				{
					int ne = f.Nodes();
					for (int j = 0; j < ne; ++j) if (f.n[j] != *it) nl2.push_back(f.n[j]);
					f.m_ntag = 1;
				}
			}
		}

		// merge sets
		if (!nl2.empty()) nl1.insert(nl2.begin(), nl2.end());
	}
}
