#include "FEFaceEdgeList.h"
#include "FEMeshBase.h"
#include "FEMesh.h"
#include "FESurfaceMesh.h"
#include <assert.h>

FENodeNodeTable::FENodeNodeTable(const FESurfaceMesh& mesh)
{
	// reset node-node table
	int NN = mesh.Nodes();
	NNT.resize(NN);
	for (int i = 0; i<NN; ++i) NNT[i].clear();

	// loop over all faces
	int NF = mesh.Faces();
	for (int i = 0; i<NF; ++i)
	{
		const FEFace& face = mesh.Face(i);
		int nf = face.Nodes();
		for (int j = 0; j<nf; ++j)
		{
			int nj = face.n[j];
			for (int k = 0; k<nf; ++k)
			{
				int nk = face.n[k];
				if (nj != nk) NNT[nj].insert(nk);
			}
		}
	}
}

FENodeNodeTable::FENodeNodeTable(const FEMesh& mesh, bool surfOnly)
{
	// reset node-node table
	int NN = mesh.Nodes();
	NNT.resize(NN);
	for (int i = 0; i<NN; ++i) NNT[i].clear();

	if (surfOnly)
	{
		// loop over all faces
		int NF = mesh.Faces();
		for (int i = 0; i<NF; ++i)
		{
			const FEFace& f = mesh.Face(i);
			int nf = f.Nodes();
			for (int j = 0; j<nf; ++j)
			{
				int nj = f.n[j];
				for (int k = 0; k<nf; ++k)
				{
					int nk = f.n[k];
					if (nj != nk) NNT[nj].insert(nk);
				}
			}
		}
	}
	else if (mesh.IsType(FE_HEX8))
	{
		const int EHEX[12][2] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 } };
		int NE = mesh.Elements();
		for (int i = 0; i<NE; ++i)
		{
			const FEElement& e = mesh.Element(i);
			for (int j = 0; j<12; ++j)
			{
				int n0 = e.m_node[EHEX[j][0]];
				int n1 = e.m_node[EHEX[j][1]];
				NNT[n0].insert(n1);
				NNT[n1].insert(n0);
			}
		}
	}
	else
	{
		// loop over all elements
		int NE = mesh.Elements();
		for (int i = 0; i<NE; ++i)
		{
			const FEElement& e = mesh.Element(i);
			int ne = e.Nodes();
			for (int j = 0; j<ne; ++j)
			{
				int nj = e.m_node[j];
				for (int k = 0; k<ne; ++k)
				{
					int nk = e.m_node[k];
					if (nj != nk) NNT[nj].insert(nk);
				}
			}
		}
	}
}

FEEdgeList::FEEdgeList(const FEMesh& mesh, bool surfOnly)
{
	ET.clear();

	// build the node-node table
	FENodeNodeTable NNT(mesh, surfOnly);

	// add all the edges
	int NN = mesh.Nodes();
	for (int i = 0; i<NN; ++i)
	{
		pair<int, int> edge;
		set<int>& NL = NNT[i];
		if (NL.empty() == false)
		{
			set<int>::iterator it;
			for (it = NL.begin(); it != NL.end(); ++it)
			{
				int m = *it;
				if (m > i)
				{
					edge.first = i;
					edge.second = m;
					ET.push_back(edge);
				}
			}
		}
	}
}

FEEdgeList::FEEdgeList(const FESurfaceMesh& mesh)
{
	ET.clear();

	// add all the edges
	for (int i = 0; i<mesh.Edges(); ++i)
	{
		const FEEdge& e = mesh.Edge(i);
		pair<int, int> edge;
		edge.first = e.n[0];
		edge.second = e.n[1];
		ET.push_back(edge);
	}
}

FEFaceTable::FEFaceTable(const FEMesh& mesh)
{
	int NE = mesh.Elements();
	vector<int> tag(NE, 0);
	for (int i = 0; i<NE; ++i) tag[i] = i;

	for (int i = 0; i<mesh.Elements(); ++i)
	{
		const FEElement& ei = mesh.Element(i);
		int nf = ei.Faces();
		for (int j = 0; j<nf; ++j)
		{
			if ((ei.m_nbr[j] < 0) || (tag[ ei.m_nbr[j] ] < tag[i]))
			{
				FEFace f = ei.GetFace(j);
				FT.push_back(f);
			}
		}
	}
}


FEFaceEdgeList::FEFaceEdgeList(const FEMeshBase& mesh, const FEEdgeList& ET)
{
	// build a node-edge table
	int NN = mesh.Nodes();
	vector< vector<int> > NET;
	NET.resize(NN);
	for (int i = ET.size() - 1; i >= 0; --i)
	{
		const pair<int, int>& et = ET[i];
		NET[et.first ].push_back(i);
		NET[et.second].push_back(i);
	}

	// loop over all faces
	int NF = mesh.Faces();
	FET.resize(NF);
	for (int i = 0; i<NF; ++i)
	{
		const FEFace& face = mesh.Face(i);
		int ne = face.Edges();
		vector<int>& FETi = FET[i];
		FETi.resize(ne, -1);
		for (int j = 0; j<ne; ++j)
		{
			int n0 = face.n[j];
			int n1 = face.n[(j+1)%ne];
			if (n1 < n0) { int nt = n1; n1 = n0; n0 = nt; }

			vector<int> NETj = NET[n0];
			for (int l = 0; l<(int)NETj.size(); ++l)
			{
				int m0 = ET[ NETj[l] ].first;
				int m1 = ET[ NETj[l] ].second;
				if (((n0 == m0) && (n1 == m1)) || ((n0 == m1) && (n1 == m0)))
				{
					FETi[j] = NETj[l];
					break;
				}
			}

			assert(FETi[j] != -1);
		}
	}
}

//-----------------------------------------------------------------------------
// TODO: This assumes TET4 or HEX8 elements
FEElementEdgeList::FEElementEdgeList(const FEMesh& mesh, const FEEdgeList& ET)
{
	const int ETET[6][2] = { { 0, 1 }, { 1, 2 }, { 2, 0 }, { 0, 3 }, { 1, 3 }, { 2, 3 } };
	const int EHEX[12][2] = { {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 7}, {7, 4}, {0, 4}, {1, 5}, {2, 6}, {3, 7} };

	int NN = mesh.Nodes();
	vector<pair<int, int> > NI;
	NI.resize(NN);
	for (int i = 0; i<NN; ++i) NI[i].second = 0;
	for (int i = ET.size() - 1; i >= 0; --i)
	{
		const pair<int, int>& et = ET[i];
		NI[et.first].first = i;
		NI[et.first].second++;
	}

	int NE = mesh.Elements();
	EET.resize(NE);
	for (int i = 0; i<NE; ++i)
	{
		const FEElement& el = mesh.Element(i);
		vector<int>& EETi = EET[i];
		if ((el.GetType() == FE_TET4)|| (el.GetType() == FE_TET5))
		{
			EETi.resize(6);
			for (int j = 0; j<6; ++j)
			{
				int n0 = el.m_node[ETET[j][0]];
				int n1 = el.m_node[ETET[j][1]];

				if (n1 < n0) { int nt = n1; n1 = n0; n0 = nt; }

				int l0 = NI[n0].first;
				int ln = NI[n0].second;
				for (int l = 0; l<ln; ++l)
				{
					assert(ET[l0 + l].first == n0);
					if (ET[l0 + l].second == n1)
					{
						EETi[j] = l0 + l;
						break;
					}
				}
			}
		}
		else if (el.GetType() == FE_HEX8)
		{
			EETi.resize(12);
			for (int j = 0; j<12; ++j)
			{
				int n0 = el.m_node[EHEX[j][0]];
				int n1 = el.m_node[EHEX[j][1]];

				if (n1 < n0) { int nt = n1; n1 = n0; n0 = nt; }

				int l0 = NI[n0].first;
				int ln = NI[n0].second;
				for (int l = 0; l<ln; ++l)
				{
					assert(ET[l0 + l].first == n0);
					if (ET[l0 + l].second == n1)
					{
						EETi[j] = l0 + l;
						break;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// only works with tet4 or hex8 meshes
FEElementFaceList::FEElementFaceList(const FEMesh& mesh, const FEFaceTable& FT)
{
	// build a node face table for FT to facilitate searching
	vector<vector<int> > NFT; NFT.resize(mesh.Nodes());
	for (int i = 0; i<(int)FT.size(); ++i)
	{
		const FEFace& f = FT[i];
		assert((f.Type() == FE_FACE_TRI3) || (f.Type() == FE_FACE_QUAD4));
		NFT[f.n[0]].push_back(i);
		NFT[f.n[1]].push_back(i);
		NFT[f.n[2]].push_back(i);
		if ((f.Type() == FE_FACE_QUAD4)) NFT[f.n[3]].push_back(i);
	}

	EFT.resize(mesh.Elements());
	for (int i = 0; i<mesh.Elements(); ++i)
	{
		const FEElement& ei = mesh.Element(i);
		vector<int>& EFTi = EFT[i];

		int nf = ei.Faces();
		EFTi.resize(nf);
		for (int j = 0; j<nf; ++j)
		{
			FEFace fj = ei.GetFace(j);
			EFTi[j] = -1;
			vector<int>& nfi = NFT[fj.n[0]];
			for (int k = 0; k<(int)nfi.size(); ++k)
			{
				const FEFace& fk = FT[nfi[k]];
				if (fj == fk)
				{
					EFTi[j] = nfi[k];
					break;
				}
			}
			assert(EFTi[j] != -1);
		}
	}
}

FEFaceFaceList::FEFaceFaceList(const FEMesh& mesh, const FEFaceTable& FT)
{
	// build a node face table for FT to facilitate searching
	vector<vector<int> > NFT; NFT.resize(mesh.Nodes());
	for (int i = 0; i<(int)FT.size(); ++i)
	{
		const FEFace& f = FT[i];
		assert(f.Type() == FE_FACE_TRI3);
		NFT[f.n[0]].push_back(i);
		NFT[f.n[1]].push_back(i);
		NFT[f.n[2]].push_back(i);
	}

	FFT.resize(mesh.Faces());
	for (int i = 0; i<mesh.Faces(); ++i)
	{
		const FEFace& fi = mesh.Face(i);
		vector<int>& nfi = NFT[fi.n[0]];
		FFT[i] = -1;
		for (int k = 0; k<(int)nfi.size(); ++k)
		{
			const FEFace& fk = FT[nfi[k]];
			if (fi == fk)
			{
				FFT[i] = nfi[k];
				break;
			}
		}
		assert(FFT[i] != -1);
	}
}

FEEdgeEdgeList::FEEdgeEdgeList(const FEMesh& mesh, const FEEdgeList& ET)
{
	// build a node-edge table for ET to facilitate searching
	vector<vector<int> > NET; NET.resize(mesh.Nodes());
	for (int i = 0; i<(int)ET.size(); ++i)
	{
		const pair<int, int>& edge = ET[i];
		NET[edge.first].push_back(i);
		NET[edge.second].push_back(i);
	}

	EET.resize(mesh.Edges());
	for (int i = 0; i<mesh.Edges(); ++i)
	{
		const FEEdge& ei = mesh.Edge(i);
		vector<int>& nei = NET[ei.n[0]];
		EET[i] = -1;
		for (int k = 0; k<(int)nei.size(); ++k)
		{
			const pair<int, int>& ek = ET[nei[k]];
			if (((ei.n[0] == ek.first) && (ei.n[1] == ek.second)) ||
				((ei.n[1] == ek.first) && (ei.n[0] == ek.second)))
			{
				EET[i] = nei[k];
				break;
			}
		}
		assert(EET[i] != -1);
	}
}

FEEdgeFaceList::FEEdgeFaceList(const FEMesh& mesh)
{
	// build the edge list (surface only)
	FEEdgeList EL(mesh, true);

	// build the face-edge list
	FEFaceEdgeList FEL(mesh, EL);

	// build the edge face list
	int NE = EL.size();
	EFL.resize(NE);

	int NF = mesh.Faces();
	for (int i=0; i<NF; ++i)
	{
		const FEFace& face = mesh.Face(i);

		int ne = FEL[i].size();
		for (int j=0; j<ne; ++j)
		{
			int iedge = FEL[i][j];

			EFL[iedge].push_back(i);
		}
	}
}

FEEdgeFaceList::FEEdgeFaceList(const FESurfaceMesh& mesh)
{
	// build the edge list
	FEEdgeList EL(mesh);

	// build the face-edge list
	FEFaceEdgeList FEL(mesh, EL);

	// build the edge face list
	int NE = EL.size();
	EFL.resize(NE);

	int NF = mesh.Faces();
	for (int i = 0; i<NF; ++i)
	{
		const FEFace& face = mesh.Face(i);

		int ne = FEL[i].size();
		for (int j = 0; j<ne; ++j)
		{
			int iedge = FEL[i][j];

			EFL[iedge].push_back(i);
		}
	}
}
