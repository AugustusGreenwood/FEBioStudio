// FEHollowSphere.cpp: implementation of the FEHollowSphere class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FEHollowSphere.h"
#include <GeomLib/GPrimitive.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FEHollowSphere::FEHollowSphere(GHollowSphere* po)
{
	m_pobj = po;
	m_nd = 6;
	m_ns = 1;
	m_gr = 1;
	m_br = false;

	AddIntParam(m_nd, "nd", "Divisions");
	AddIntParam(m_ns, "ns", "Segments");

	AddDoubleParam(m_gr, "gr", "R-bias");
	AddBoolParam(m_br, "br", "R-mirrored bias");
}

double gain2(double x, double r, double n)
{
	if ((r == 1) || (n == 0)) return x;
	return (pow(r, n*x) - 1.0)/(pow(r, n) - 1.0);
}

FEMesh* FEHollowSphere::BuildMesh()
{
	assert(m_pobj);

	// get object parameters
	ParamBlock& param = m_pobj->GetParamBlock();
	double R0 = param.GetFloatValue(GHollowSphere::RIN);
	double R1 = param.GetFloatValue(GHollowSphere::ROUT);

	// get mesh parameters
	m_nd = GetIntValue(NDIV);
	m_ns = GetIntValue(NSEG);
	m_gr = GetFloatValue(GR);
	m_br = GetBoolValue(GR2);

	// get parameters
	int nd = m_nd;
	int ns = m_ns;

	// check parameters
	if (nd < 1) nd   = 1;
	if (ns < 1) ns   = 1;

	double d0 = R0/sqrt(2.0);
	double d1 = R1/sqrt(2.0);

	// create the MB nodes
	m_MBNode.resize(52);
	m_MBNode[ 0].m_r = vec3d(-d0,-d0,-d0);
	m_MBNode[ 1].m_r = vec3d(  0,-d0,-d0);
	m_MBNode[ 2].m_r = vec3d( d0,-d0,-d0);
	m_MBNode[ 3].m_r = vec3d(-d0,  0,-d0);
	m_MBNode[ 4].m_r = vec3d(  0,  0,-d0);
	m_MBNode[ 5].m_r = vec3d( d0,  0,-d0);
	m_MBNode[ 6].m_r = vec3d(-d0, d0,-d0);
	m_MBNode[ 7].m_r = vec3d(  0, d0,-d0);
	m_MBNode[ 8].m_r = vec3d( d0, d0,-d0);
	m_MBNode[ 9].m_r = vec3d(-d0,-d0,  0);
	m_MBNode[10].m_r = vec3d(  0,-d0,  0);
	m_MBNode[11].m_r = vec3d( d0,-d0,  0);
	m_MBNode[12].m_r = vec3d(-d0,  0,  0);
	m_MBNode[13].m_r = vec3d( d0,  0,  0);
	m_MBNode[14].m_r = vec3d(-d0, d0,  0);
	m_MBNode[15].m_r = vec3d(  0, d0,  0);
	m_MBNode[16].m_r = vec3d( d0, d0,  0);
	m_MBNode[17].m_r = vec3d(-d0,-d0, d0);
	m_MBNode[18].m_r = vec3d(  0,-d0, d0);
	m_MBNode[19].m_r = vec3d( d0,-d0, d0);
	m_MBNode[20].m_r = vec3d(-d0,  0, d0);
	m_MBNode[21].m_r = vec3d(  0,  0, d0);
	m_MBNode[22].m_r = vec3d( d0,  0, d0);
	m_MBNode[23].m_r = vec3d(-d0, d0, d0);
	m_MBNode[24].m_r = vec3d(  0, d0, d0);
	m_MBNode[25].m_r = vec3d( d0, d0, d0);

	m_MBNode[26].m_r = vec3d(-d1,-d1,-d1);
	m_MBNode[27].m_r = vec3d(  0,-d1,-d1);
	m_MBNode[28].m_r = vec3d( d1,-d1,-d1);
	m_MBNode[29].m_r = vec3d(-d1,  0,-d1);
	m_MBNode[30].m_r = vec3d(  0,  0,-d1);
	m_MBNode[31].m_r = vec3d( d1,  0,-d1);
	m_MBNode[32].m_r = vec3d(-d1, d1,-d1);
	m_MBNode[33].m_r = vec3d(  0, d1,-d1);
	m_MBNode[34].m_r = vec3d( d1, d1,-d1);

	m_MBNode[35].m_r = vec3d(-d1, -d1, 0);
	m_MBNode[36].m_r = vec3d(  0, -d1, 0);
	m_MBNode[37].m_r = vec3d( d1, -d1, 0);
	m_MBNode[38].m_r = vec3d(-d1,   0, 0);
	m_MBNode[39].m_r = vec3d( d1,   0, 0);
	m_MBNode[40].m_r = vec3d(-d1,  d1, 0);
	m_MBNode[41].m_r = vec3d(  0,  d1, 0);
	m_MBNode[42].m_r = vec3d( d1,  d1, 0);

	m_MBNode[43].m_r = vec3d(-d1,-d1, d1);
	m_MBNode[44].m_r = vec3d(  0,-d1, d1);
	m_MBNode[45].m_r = vec3d( d1,-d1, d1);
	m_MBNode[46].m_r = vec3d(-d1,  0, d1);
	m_MBNode[47].m_r = vec3d(  0,  0, d1);
	m_MBNode[48].m_r = vec3d( d1,  0, d1);
	m_MBNode[49].m_r = vec3d(-d1, d1, d1);
	m_MBNode[50].m_r = vec3d(  0, d1, d1);
	m_MBNode[51].m_r = vec3d( d1, d1, d1);

	// create the MB block
	m_MBlock.resize(24);

	int MB[24][8] = {
		{ 0, 26, 27,  1,  9, 35, 36, 10},	// 0 ---
		{ 1, 27, 28,  2, 10, 36, 37, 11},	// 1
		{ 9, 35, 36, 10, 17, 43, 44, 18},	// 2 
		{10, 36, 37, 11, 18, 44, 45, 19},	// 3
		{ 2, 28, 31,  5, 11, 37, 39, 13},	// 4 ---
		{ 5, 31, 34,  8, 13, 39, 42, 16},	// 5
		{11, 37, 39, 13, 19, 45, 48, 22},	// 6
		{13, 39, 42, 16, 22, 48, 51, 25},	// 7
		{ 8, 34, 33,  7, 16, 42, 41, 15},	// 8 ---
		{ 7, 33, 32,  6, 15, 41, 40, 14},	// 9
		{16, 42, 41, 15, 25, 51, 50, 24},	// 10
		{15, 41, 40, 14, 24, 50, 49, 23},	// 11
		{ 6, 32, 29,  3, 14, 40, 38, 12},	// 12 ---
		{ 3, 29, 26,  0, 12, 38, 35,  9},	// 13
		{14, 40, 38, 12, 23, 49, 46, 20},	// 14
		{12, 38, 35,  9, 20, 46, 43, 17},	// 15
		{ 0, 26, 29,  3,  1, 27, 30,  4},	// 16 ---
		{ 1, 27, 30,  4,  2, 28, 31,  5},	// 17
		{ 3, 29, 32,  6,  4, 30, 33,  7},	// 18
		{ 4, 30, 33,  7,  5, 31, 34,  8},	// 19
		{17, 43, 44, 18, 20, 46, 47, 21},	// 20 ---
		{18, 44, 45, 19, 21, 47, 48, 22},	// 21
		{20, 46, 47, 21, 23, 49, 50, 24},	// 22
		{21, 47, 48, 22, 24, 50, 51, 25}	// 23
	};

	int i, *n;
	for (i=0; i<24; ++i)
	{
		n = MB[i];
		m_MBlock[i].SetNodes(n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7]);
		m_MBlock[i].SetSizes(ns,nd,nd);
		m_MBlock[i].SetID(0);
	}

	// update the MB data
	UpdateMB();

	// Face ID's
	GetBlockFace( 0, 1).SetID(2);
	GetBlockFace( 1, 1).SetID(3);
	GetBlockFace( 2, 1).SetID(6);
	GetBlockFace( 3, 1).SetID(7);

	GetBlockFace( 4, 1).SetID(3);
	GetBlockFace( 5, 1).SetID(0);
	GetBlockFace( 6, 1).SetID(7);
	GetBlockFace( 7, 1).SetID(4);

	GetBlockFace( 8, 1).SetID(0);
	GetBlockFace( 9, 1).SetID(1);
	GetBlockFace(10, 1).SetID(4);
	GetBlockFace(11, 1).SetID(5);

	GetBlockFace(12, 1).SetID(1);
	GetBlockFace(13, 1).SetID(2);
	GetBlockFace(14, 1).SetID(5);
	GetBlockFace(15, 1).SetID(6);

	GetBlockFace(16, 1).SetID(2);
	GetBlockFace(17, 1).SetID(3);
	GetBlockFace(18, 1).SetID(1);
	GetBlockFace(19, 1).SetID(0);

	GetBlockFace(20, 1).SetID(6);
	GetBlockFace(21, 1).SetID(7);
	GetBlockFace(22, 1).SetID(5);
	GetBlockFace(23, 1).SetID(4);

	GetBlockFace( 0, 3).SetID(10);
	GetBlockFace( 1, 3).SetID(11);
	GetBlockFace( 2, 3).SetID(14);
	GetBlockFace( 3, 3).SetID(15);

	GetBlockFace( 4, 3).SetID(11);
	GetBlockFace( 5, 3).SetID(8);
	GetBlockFace( 6, 3).SetID(15);
	GetBlockFace( 7, 3).SetID(12);

	GetBlockFace( 8, 3).SetID(8);
	GetBlockFace( 9, 3).SetID(9);
	GetBlockFace(10, 3).SetID(12);
	GetBlockFace(11, 3).SetID(13);

	GetBlockFace(12, 3).SetID(9);
	GetBlockFace(13, 3).SetID(10);
	GetBlockFace(14, 3).SetID(13);
	GetBlockFace(15, 3).SetID(14);

	GetBlockFace(16, 3).SetID(10);
	GetBlockFace(17, 3).SetID(11);
	GetBlockFace(18, 3).SetID(9);
	GetBlockFace(19, 3).SetID(8);

	GetBlockFace(20, 3).SetID(14);
	GetBlockFace(21, 3).SetID(15);
	GetBlockFace(22, 3).SetID(13);
	GetBlockFace(23, 3).SetID(12);

	// Edge ID's
	GetFaceEdge(GetBlockFace( 7,1),0).SetID(0);
	GetFaceEdge(GetBlockFace(10,1),0).SetID(0);
	GetFaceEdge(GetBlockFace(11,1),0).SetID(1);
	GetFaceEdge(GetBlockFace(14,1),0).SetID(1);
	GetFaceEdge(GetBlockFace(15,1),0).SetID(2);
	GetFaceEdge(GetBlockFace( 2,1),0).SetID(2);
	GetFaceEdge(GetBlockFace( 3,1),0).SetID(3);
	GetFaceEdge(GetBlockFace( 6,1),0).SetID(3);

	GetFaceEdge(GetBlockFace( 7,1),3).SetID(4);
	GetFaceEdge(GetBlockFace(23,1),0).SetID(4);
	GetFaceEdge(GetBlockFace(11,1),3).SetID(5);
	GetFaceEdge(GetBlockFace(23,1),3).SetID(5);
	GetFaceEdge(GetBlockFace(14,1),1).SetID(6);
	GetFaceEdge(GetBlockFace(20,1),2).SetID(6);
	GetFaceEdge(GetBlockFace( 3,1),3).SetID(7);
	GetFaceEdge(GetBlockFace(20,1),1).SetID(7);

	GetFaceEdge(GetBlockFace( 4,1),1).SetID(8);
	GetFaceEdge(GetBlockFace(19,1),3).SetID(8);
	GetFaceEdge(GetBlockFace( 8,1),1).SetID(9);
	GetFaceEdge(GetBlockFace(19,1),0).SetID(9);
	GetFaceEdge(GetBlockFace(12,1),1).SetID(10);
	GetFaceEdge(GetBlockFace(16,1),1).SetID(10);
	GetFaceEdge(GetBlockFace( 0,1),1).SetID(11);
	GetFaceEdge(GetBlockFace(16,1),2).SetID(11);


	GetFaceEdge(GetBlockFace( 7,3),0).SetID(12);
	GetFaceEdge(GetBlockFace(10,3),0).SetID(12);
	GetFaceEdge(GetBlockFace(11,3),0).SetID(13);
	GetFaceEdge(GetBlockFace(14,3),0).SetID(13);
	GetFaceEdge(GetBlockFace(15,3),0).SetID(14);
	GetFaceEdge(GetBlockFace( 2,3),0).SetID(14);
	GetFaceEdge(GetBlockFace( 3,3),0).SetID(15);
	GetFaceEdge(GetBlockFace( 6,3),0).SetID(15);

	GetFaceEdge(GetBlockFace( 7,3),1).SetID(16);
	GetFaceEdge(GetBlockFace(23,3),0).SetID(16);
	GetFaceEdge(GetBlockFace(11,3),1).SetID(17);
	GetFaceEdge(GetBlockFace(23,3),1).SetID(17);
	GetFaceEdge(GetBlockFace(14,3),3).SetID(18);
	GetFaceEdge(GetBlockFace(20,3),2).SetID(18);
	GetFaceEdge(GetBlockFace( 3,3),1).SetID(19);
	GetFaceEdge(GetBlockFace(20,3),3).SetID(19);

	GetFaceEdge(GetBlockFace( 4,3),3).SetID(20);
	GetFaceEdge(GetBlockFace(19,3),1).SetID(20);
	GetFaceEdge(GetBlockFace( 8,3),3).SetID(21);
	GetFaceEdge(GetBlockFace(19,3),0).SetID(21);
	GetFaceEdge(GetBlockFace(12,3),3).SetID(22);
	GetFaceEdge(GetBlockFace(16,3),3).SetID(22);
	GetFaceEdge(GetBlockFace( 0,3),3).SetID(23);
	GetFaceEdge(GetBlockFace(16,3),2).SetID(23);

	// Node ID's
	m_MBNode[39].SetID(0);
	m_MBNode[41].SetID(1);
	m_MBNode[38].SetID(2);
	m_MBNode[36].SetID(3);
	m_MBNode[30].SetID(4);
	m_MBNode[47].SetID(5);
	m_MBNode[13].SetID(6);
	m_MBNode[15].SetID(7);
	m_MBNode[12].SetID(8);
	m_MBNode[10].SetID(9);
	m_MBNode[4].SetID(10);
	m_MBNode[21].SetID(11);

	// create the MB
	FEMesh* pm = FEMultiBlockMesh::BuildMesh();

	// project the nodes onto a sphere
	double x, y, z, d, w;
	vec3d p, r0, r1;
	for (int i=0; i<pm->Nodes(); ++i)
	{
		vec3d& r = pm->Node(i).r;
		x = fabs(r.x);
		y = fabs(r.y);
		z = fabs(r.z);
		d = fmax(fmax(x,y),fmax(y,z));
		w = (d - d0)/(d1 - d0);
		if (w>1) w = 1;
		if (w<0) w = 0;

		if (m_br)
		{
			if (w <= 0.5)
				w = 0.5*gain2(2*w, m_gr, m_ns);
			else
				w = 1 - 0.5*gain2(2 - 2*w, m_gr, m_ns);
		}
		else w = gain2(w, m_gr, m_ns); 

		p = r;
		p.Normalize();
		r0 = p*R0;
		r1 = p*R1;
		r = r0*(1-w) + r1*w;
	}

	// update the mesh
	pm->Update();

	// the Multi-block mesher will assign a different smoothing ID
	// to each face, but we don't want that here. 
	// For now, we autosmooth the mesh although we should think of a 
	// better way
	pm->AutoSmooth(60);

	return pm;
}

/*
void FEHollowSphere::Create()
{
	int i, j, k, nn, n;

	// get parameters
	double R0 = m_Param.GetFloatValue(RIN);
	double R1 = m_Param.GetFloatValue(ROUT);
	int nd = m_Param.GetIntValue(NDIV);
	int ns = m_Param.GetIntValue(NSEG);

	// check parameters
	if (nd   < 1) nd   = 1;
	if (ns   < 1) ns   = 1;

	// count nodes and elements
	int N = nd;
	int M = ns;
	int nodes = (M+1)*(6*(N+1)*(N-1) + 8);
	int elems = 6*N*N*M;

	// allocate storage
	if ((m_nd != nd) || (m_ns != ns))
	{
		FEMesh::Create(nodes, elems);
		m_nd = nd;
		m_ns = ns;
		for (i=0; i<elems; ++i) Element(i).m_pmat = 0;
	}

	// create the reference array
	int nsize = (N+1)*(N+1)*(N+1);
	int *ref = new int[nsize];

	for (i=0; i<nsize; ++i) ref[i] = -1;

	// create the first layer of nodes
	FENode* pn = NodePtr();
	n = nn = 0;
	double g = 0.45f;
	double* fg = new double[N+1];
	for (i=0; i<=N; ++i) fg[i] = gain(g, (double) i / (double) N);

	for (i=0; i<=N; ++i)
		for (j=0; j<=N; ++j)
			for (k=0; k<=N; ++k, ++n)
			{
				if ((i==0||i==N)||(j==0||j==N)||(k==0||k==N))
				{
					ref[n] = nn;

					vec3d& r = pn->r;

					r.x = -1.f + 2.f*fg[i];
					r.y = -1.f + 2.f*fg[j];
					r.z = -1.f + 2.f*fg[k];

					r.Normalize();
					r *= R0;

					++pn;
					++nn;
				}
			}

	delete [] fg;

	// create the other nodes
	double R;
	for (k=1; k<=M; ++k)
	{
		R = 1 + k*(R1/R0 - 1)/M;
		for (i=0; i<nn; ++i)
		{
			vec3d& r = pn->r;
			r = Node(i).r;
			r *= (double) R;
			++pn;
		}
	}

	// create the first layer of elements
	FEElement* pe = ElementPtr();

	//-x
	i = 0;
	for (j=0; j<N; ++j)
		for (k=0; k<N; ++k)
		{
			int* en = pe->m_node;
			pe->m_ntype = FE_HEX8;
			pe->m_gid = 0;

			en[0] = ref[i*(N+1)*(N+1)+(j+1)*(N+1)+k];
			en[1] = ref[i*(N+1)*(N+1)+j*(N+1)+k];
			en[2] = ref[i*(N+1)*(N+1)+j*(N+1)+k+1];
			en[3] = ref[i*(N+1)*(N+1)+(j+1)*(N+1)+k+1];

			++pe;
		}

	//+x
	i = N;
	for (j=0; j<N; ++j)
		for (k=0; k<N; ++k)
		{
			int* en = pe->m_node;
			pe->m_ntype = FE_HEX8;
			pe->m_gid = 1;

			en[0] = ref[i*(N+1)*(N+1)+j*(N+1)+k];
			en[1] = ref[i*(N+1)*(N+1)+(j+1)*(N+1)+k];
			en[2] = ref[i*(N+1)*(N+1)+(j+1)*(N+1)+k+1];
			en[3] = ref[i*(N+1)*(N+1)+j*(N+1)+k+1];

			++pe;
		}

	//-y
	j = 0;
	for (i=0; i<N; ++i)
		for (k=0; k<N; ++k)
		{
			int* en = pe->m_node;
			pe->m_ntype = FE_HEX8;
			pe->m_gid = 2;

			en[0] = ref[i*(N+1)*(N+1)+j*(N+1)+k];
			en[1] = ref[(i+1)*(N+1)*(N+1)+j*(N+1)+k];
			en[2] = ref[(i+1)*(N+1)*(N+1)+j*(N+1)+k+1];
			en[3] = ref[i*(N+1)*(N+1)+j*(N+1)+k+1];

			++pe;
		}

	//+y
	j = N;
	for (i=0; i<N; ++i)
		for (k=0; k<N; ++k)
		{
			int* en = pe->m_node;
			pe->m_ntype = FE_HEX8;
			pe->m_gid = 3;

			en[0] = ref[(i+1)*(N+1)*(N+1)+j*(N+1)+k];
			en[1] = ref[i*(N+1)*(N+1)+j*(N+1)+k];
			en[2] = ref[i*(N+1)*(N+1)+j*(N+1)+k+1];
			en[3] = ref[(i+1)*(N+1)*(N+1)+j*(N+1)+k+1];

			++pe;
		}

	//-z
	k = 0;
	for (i=0; i<N; ++i)
		for (j=0; j<N; ++j)
		{
			int* en = pe->m_node;
			pe->m_ntype = FE_HEX8;
			pe->m_gid = 4;

			en[0] = ref[i*(N+1)*(N+1)+j*(N+1)+k];
			en[1] = ref[i*(N+1)*(N+1)+(j+1)*(N+1)+k];
			en[2] = ref[(i+1)*(N+1)*(N+1)+(j+1)*(N+1)+k];
			en[3] = ref[(i+1)*(N+1)*(N+1)+j*(N+1)+k];

			++pe;
		}

	//+z
	k = N;
	for (i=0; i<N; ++i)
		for (j=0; j<N; ++j)
		{
			int* en = pe->m_node;
			pe->m_ntype = FE_HEX8;
			pe->m_gid = 5;

			en[0] = ref[i*(N+1)*(N+1)+j*(N+1)+k];
			en[1] = ref[(i+1)*(N+1)*(N+1)+j*(N+1)+k];
			en[2] = ref[(i+1)*(N+1)*(N+1)+(j+1)*(N+1)+k];
			en[3] = ref[i*(N+1)*(N+1)+(j+1)*(N+1)+k];

			++pe;
		}

	pe = ElementPtr();
	for (i=0; i<6*N*N; ++i, ++pe)
	{
		int* en = pe->m_node;
		pe->m_ntype = FE_HEX8;

		en[4] = en[0]+nn;
		en[5] = en[1]+nn;
		en[6] = en[2]+nn;
		en[7] = en[3]+nn;
	}

	// create the other elements
	int ne = 6*N*N;
	for (k=1; k<M; ++k)
	{
		for (i=0; i<ne; ++i)
		{
			int* en = pe->m_node;
			int* enp = Element((k-1)*ne + i).m_node;
			pe->m_gid = Element((k-1)*ne+i).m_gid;

			en[0] = enp[4];
			en[1] = enp[5];
			en[2] = enp[6];
			en[3] = enp[7];

			en[4] = en[0]+nn;
			en[5] = en[1]+nn;
			en[6] = en[2]+nn;
			en[7] = en[3]+nn;
			pe->m_ntype = FE_HEX8;

			++pe;
		}
	}

	// cleanup
	delete [] ref;

	Update();
}
*/
