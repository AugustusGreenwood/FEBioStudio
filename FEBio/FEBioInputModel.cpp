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

#include "stdafx.h"
#include "FEBioInputModel.h"
#include <GeomLib/GMeshObject.h>
#include <MeshTools/GDiscreteObject.h>
#include <MeshTools/GModel.h>
#include "FEBioImport.h"
#include <string.h>
#include <stdarg.h>
#include <sstream>

using std::stringstream;

static FEBioFileImport* febImport = nullptr;

void InitLog(FEBioFileImport* im)
{
	febImport = im;
}

void AddLogEntry(const char* sz, ...)
{
	if (febImport == nullptr) return;

	if (sz == 0) return;

	// get a pointer to the argument list
	va_list	args;

	// copy to string
	char szlog[256] = { 0 };
	va_start(args, sz);
	vsprintf(szlog, sz, args);
	va_end(args);

	int l = (int)strlen(szlog);
	if (l == 0) return;

	febImport->AddLogEntry(szlog);
}

//=============================================================================
FEBioMesh::FEBioMesh()
{

}

//-----------------------------------------------------------------------------
void FEBioMesh::UpdateMeshData()
{
	BuildFaceTable();
	BuildEdgeTable();
	BuildElementTable();
}

//-----------------------------------------------------------------------------
void FEBioMesh::BuildEdgeTable()
{
	int nsize = 0, i, j, n, m, l;

	FSMesh& mesh = m_mesh;

	int nodes = mesh.Nodes();
	int edges = mesh.Edges();

	m_nEdge.resize(nodes);
	for (i = 0; i < nodes; ++i) m_nEdge[i] = 0;

	for (i = 0; i < edges; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		n = edge.Nodes();
		for (j = 0; j < n; ++j) m_nEdge[edge.n[j]]++;
		nsize += n;
	}

	m_iEdge.resize(nsize);
	m_pEdge.resize(nodes);

	if (nsize > 0)
	{
		int* pi = &m_iEdge[0];
		for (i = 0; i < nodes; ++i)
		{
			m_pEdge[i] = pi;
			n = m_nEdge[i];
			pi += n;
			m_nEdge[i] = 0;
		}
	}

	for (i = 0; i < edges; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		n = edge.Nodes();
		for (j = 0; j < n; ++j)
		{
			m = edge.n[j];

			l = m_nEdge[m];
			m_pEdge[m][l] = i;
			m_nEdge[m]++;
		}
	}
}


//-----------------------------------------------------------------------------
void FEBioMesh::BuildFaceTable()
{
	int nsize = 0, i, j, n, m, l;

	FSMesh& mesh = m_mesh;

	int nodes = mesh.Nodes();
	int faces = mesh.Faces();

	m_nFace.resize(nodes);
	for (i = 0; i < nodes; ++i) m_nFace[i] = 0;

	for (i = 0; i < faces; ++i)
	{
		FSFace& face = mesh.Face(i);
		n = face.Nodes();
		for (j = 0; j < n; ++j) m_nFace[face.n[j]]++;
		nsize += n;
	}

	m_iFace.resize(nsize);
	m_pFace.resize(nodes);

	if (nsize > 0)
	{
		int* pi = &m_iFace[0];
		for (i = 0; i < nodes; ++i)
		{
			m_pFace[i] = pi;
			n = m_nFace[i];
			pi += n;
			m_nFace[i] = 0;
		}
	}

	for (i = 0; i < faces; ++i)
	{
		FSFace& face = mesh.Face(i);
		n = face.Nodes();
		for (j = 0; j < n; ++j)
		{
			m = face.n[j];

			l = m_nFace[m];
			m_pFace[m][l] = i;
			m_nFace[m]++;
		}
	}
}

void FEBioMesh::BuildElementTable()
{
	FSMesh& mesh = m_mesh;

	// build the element lookup table
	m_minID = m_maxID = -1;
	int NE = mesh.Elements();
	for (int i=0; i<NE; ++i)
	{
		FSElement& el = mesh.Element(i);
		if (el.m_nid != -1)
		{
			if ((m_minID == -1) || (el.m_nid < m_minID)) m_minID = el.m_nid;
			if ((m_maxID == -1) || (el.m_nid > m_maxID)) m_maxID = el.m_nid;
		}
	}

	int nsize = m_maxID - m_minID + 1;
	m_elem.resize(nsize, (FSElement*) 0);

	for (int i = 0; i<NE; ++i)
	{
		FSElement& el = mesh.Element(i);
		int nid = el.m_nid;
		if (nid != -1) m_elem[nid - m_minID] = &el;
	}
}

//-----------------------------------------------------------------------------
FSElement* FEBioMesh::FindElementFromID(int nid)
{
	if (nid == -1) return 0;
	if ((nid < m_minID) || (nid > m_maxID)) return 0;
	return m_elem[nid - m_minID];
}

//-----------------------------------------------------------------------------
int FEBioMesh::FindFace(const int* n, int nn, int noff)
{
	int* pf = m_pFace[n[0] - noff];
	int N = m_nFace[n[0] - noff];

	for (int i = 0; i<N; ++i)
	{
		FSFace& face = m_mesh.Face(pf[i]);
		if (face.Nodes() == nn)
		{
			bool bfound = true;
			for (int j = 0; j < nn; ++j)
				if (face.HasNode(n[j] - noff) == false)
				{
					bfound = false;
					break;
				}

			if (bfound) return pf[i];
		}
	}

	return -1;
}

//-----------------------------------------------------------------------------
int FEBioMesh::FindEdge(const int* n, int nn, int noff)
{
	int* pf = m_pEdge[n[0] - noff];
	int N = m_nEdge[n[0] - noff];

	for (int i = 0; i < N; ++i)
	{
		FSEdge& edge = m_mesh.Edge(pf[i]);
		if (edge.Nodes() == nn)
		{
			bool bfound = true;
			for (int j = 0; j < nn; ++j)
				if (edge.HasNode(n[j] - noff) == false)
				{
					bfound = false;
					break;
				}

			if (bfound) return pf[i];
		}
	}

	return -1;
}


//-----------------------------------------------------------------------------
int FEBioMesh::FindFace(const vector<int>& n, int noff)
{
	return FindFace(&n[0], (int)n.size(), noff);
}

//-----------------------------------------------------------------------------
int FEBioMesh::FindEdge(const vector<int>& n, int noff)
{
	return FindEdge(&n[0], (int)n.size(), noff);
}

//=============================================================================
FEBioInputModel::NodeSet::NodeSet()
{
}

FEBioInputModel::NodeSet::NodeSet(const NodeSet& nodeSet)
{
	m_name = nodeSet.m_name;
	m_node = nodeSet.m_node;
}

FEBioInputModel::NodeSet::NodeSet(const std::string& name, const vector<int>& node)
{
	m_name = name;
	m_node = node;
}

void FEBioInputModel::NodeSet::operator = (const NodeSet& nodeSet)
{
	m_name = nodeSet.m_name;
	m_node = nodeSet.m_node;
}

//=============================================================================
FEBioInputModel::Surface::Surface()
{
	m_refs = 0;
}

FEBioInputModel::Surface::Surface(const FEBioInputModel::Surface& s)
{
	m_name = s.m_name;
	m_face = s.m_face;
	m_refs = s.m_refs;
}

void FEBioInputModel::Surface::operator = (const FEBioInputModel::Surface& s)
{
	m_name = s.m_name;
	m_face = s.m_face;
	m_refs = s.m_refs;
}

//=============================================================================
FEBioInputModel::ElementSet::ElementSet()
{
}

FEBioInputModel::ElementSet::ElementSet(const string& name, const vector<int>& elem)
{
	m_name = name;
	m_elem = elem;
}

FEBioInputModel::ElementSet::ElementSet(const FEBioInputModel::ElementSet& s)
{
	m_name = s.m_name;
	m_elem = s.m_elem;
}

void FEBioInputModel::ElementSet::operator = (const FEBioInputModel::ElementSet& s)
{
	m_name = s.m_name;
	m_elem = s.m_elem;
}

//=============================================================================
FEBioInputModel::Part::Part()
{
	
}

void FEBioInputModel::Part::SetName(const char* szname)
{
	m_name = szname;
}

const char* FEBioInputModel::Part::GetName()
{
	return m_name.c_str();
}

FEBioInputModel::Part::~Part()
{
	
}

FEBioInputModel::Domain* FEBioInputModel::Part::AddDomain(const string& name, int matID)
{
	m_dom.push_back(Domain(this, name, matID));
	return &m_dom[m_dom.size() - 1];
}

FEBioInputModel::NodeSet* FEBioInputModel::Part::FindNodeSet(const std::string& name)
{
	for (int i = 0; i<(int)m_nset.size(); ++i)
	{
		NodeSet* ps = &m_nset[i];
		if (ps->name() == name) return ps;
	}
	return 0;
}

FEBioInputModel::EdgeSet* FEBioInputModel::Part::FindEdgeSet(const std::string& name)
{
	for (int i = 0; i < (int)m_edge.size(); ++i)
	{
		EdgeSet* ps = &m_edge[i];
		if (ps->name() == name) return ps;
	}
	return 0;
}

FEBioInputModel::Surface* FEBioInputModel::Part::FindSurface(const std::string& name)
{
	for (int i = 0; i<(int)m_surf.size(); ++i)
	{
		Surface* ps = &m_surf[i];
		if (ps->name() == name) return ps;
	}
	return 0;
}

int FEBioInputModel::Part::FindSurfaceIndex(const std::string& name)
{
	for (int i = 0; i<(int)m_surf.size(); ++i)
	{
		Surface* ps = &m_surf[i];
		if (ps->name() == name) return i;
	}
	return -1;
}

FEBioInputModel::ElementSet* FEBioInputModel::Part::FindElementSet(const std::string& name)
{
	for (int i = 0; i<(int)m_eset.size(); ++i)
	{
		ElementSet* ps = &m_eset[i];
		if (ps->name() == name) return ps;
	}
	return 0;
}

int FEBioInputModel::Part::FindElementSetIndex(const std::string& name)
{
	for (int i = 0; i<(int)m_eset.size(); ++i)
	{
		ElementSet* ps = &m_eset[i];
		if (ps->name() == name) return i;
	}
	return -1;
}


FEBioInputModel::DiscreteSet* FEBioInputModel::Part::FindDiscreteSet(const std::string& name)
{
	for (int i = 0; i<(int)m_des.size(); ++i)
	{
		DiscreteSet* ps = &m_des[i];
		if (ps->name() == name) return ps;
	}
	return 0;
}

FEBioInputModel::SurfacePair* FEBioInputModel::Part::FindSurfacePair(const std::string& name)
{
	for (int i = 0; i<(int)m_spr.size(); ++i)
	{
		SurfacePair* ps = &m_spr[i];
		if (ps->name() == name) return ps;
	}
	return 0;
}

FEBioInputModel::Domain* FEBioInputModel::Part::FindDomain(const std::string& name)
{
	for (int i = 0; i<(int)m_dom.size(); ++i)
	{
		Domain* pd = &m_dom[i];
		if (pd->name() == name) return pd;
	}
	return 0;
}

void FEBioInputModel::Part::Update()
{
	if (GetFEMesh() == 0) return;

	FSMesh& mesh = *GetFEMesh();

	// build node-index lookup table
	int noff = -1, maxID = 0;
	int NN = mesh.Nodes();
	for (int i = 0; i<NN; ++i)
	{
		int nid = mesh.Node(i).m_ntag;
		assert(nid > 0);
		if ((noff < 0) || (nid < noff)) noff = nid;
		if (nid > maxID) maxID = nid;
	}
	vector<int> NLT(maxID - noff + 1, -1);
	for (int i = 0; i<NN; ++i)
	{
		int nid = mesh.Node(i).m_ntag - noff;
		NLT[nid] = i;
	}

	int NE = mesh.Elements();
	for (int i=0; i<NE; ++i)
	{
		FSElement& el = mesh.Element(i);
		int ne = el.Nodes();
		for (int j=0; j<ne; ++j) el.m_node[j] = NLT[ el.m_node[j] - noff ];
	}
}


//=============================================================================
FEBioInputModel::PartInstance::PartInstance(FEBioInputModel::Part* part) : m_part(part), m_po(nullptr)
{
	m_scl = vec3d(1,1,1);
}

FEBioInputModel::PartInstance::~PartInstance()
{
	if (m_po) delete m_po; m_po = nullptr;
}

FSMesh* FEBioInputModel::PartInstance::GetMesh() { return m_po->GetFEMesh(); }

FSNodeSet* FEBioInputModel::PartInstance::BuildFENodeSet(const char* szname)
{
	NodeSet* nodeSet = m_part->FindNodeSet(szname);
	if (nodeSet == 0) return 0;

	// create the node set
	FSNodeSet* pns = new FSNodeSet(m_po, nodeSet->nodeList());

	// copy the name
	std::string name = nodeSet->name();
	pns->SetName(name.c_str());

	// all done
	return pns;
}

FSSurface* FEBioInputModel::PartInstance::BuildFESurface(FEBioInputModel::Surface& surf)
{
	// create face list
	vector<int> faceList;
	int NF = surf.faces();
	for (int i = 0; i<NF; ++i)
	{
		const vector<int>& face = surf.face(i);
		int faceID = m_part->m_mesh.FindFace(face);
		if (faceID >= 0) faceList.push_back(faceID);
		else
		{
			stringstream ss;
			ss << "Cannot find facet: ";
			for (int j = 0; j < face.size(); ++j)
			{
				ss << face[j];
				if (j != face.size() - 1) ss << ",";
			}
			string s = ss.str();
			AddLogEntry(s.c_str());
		}
	}

	// create the surface
	FSSurface* ps = new FSSurface(m_po, faceList);

	// copy the name
	std::string name = surf.name();
	ps->SetName(name.c_str());

	// increment surface reference counter
	surf.m_refs++;

	// all done
	return ps;
}

FSNodeSet* FEBioInputModel::PartInstance::BuildFENodeSet(const FEBioInputModel::NodeSet& nset)
{
	// create the surface
	FSNodeSet* pns = new FSNodeSet(m_po, nset.nodeList());

	// copy the name
	std::string name = nset.name();
	pns->SetName(name.c_str());

	// all done
	return pns;
}

FSEdgeSet* FEBioInputModel::PartInstance::BuildFEEdgeSet(FEBioInputModel::EdgeSet& edge)
{
	// create face list
	vector<int> edgeList;
	int NE = edge.edges();
	for (int i = 0; i < NE; ++i)
	{
		const vector<int>& el = edge.edge(i);
		int edgeID = m_part->m_mesh.FindEdge(el);
		if (edgeID >= 0) edgeList.push_back(edgeID);
		else
		{
			stringstream ss;
			ss << "Cannot find edge: ";
			for (int j = 0; j < el.size(); ++j)
			{
				ss << el[j];
				if (j != el.size() - 1) ss << ",";
			}
			string s = ss.str();
			AddLogEntry(s.c_str());
		}
	}

	// create the surface
	FSEdgeSet* ps = new FSEdgeSet(m_po, edgeList);

	// copy the name
	std::string name = edge.name();
	ps->SetName(name.c_str());

	// increment surface reference counter
	edge.m_refs++;

	// all done
	return ps;
}


bool check_winding(const vector<int>& nodeList, const FSFace& face)
{
	int nf = 0;
	if (face.Shape() == FE_FACE_TRI ) nf = 3;
	if (face.Shape() == FE_FACE_QUAD) nf = 4;
	if (nf == 0) return false;
	if (nodeList.size() < nf) return false;

	int n0 = nodeList[0];
	for (int j = 0; j < nf; ++j)
	{
		if (n0 == face.n[j])
		{
			for (int i = 1; i < nf; ++i)
			{
				int ni = (j + i) % nf;
				if (nodeList[i] != face.n[ni]) return false;
			}

			return true;
		}
	}
	return false;
}

FSEdgeSet* FEBioInputModel::PartInstance::BuildFEEdgeSet(const char* szname)
{
	EdgeSet* edge = m_part->FindEdgeSet(szname);
	if (edge == 0) return 0;

	FSEdgeSet* feedge = BuildFEEdgeSet(*edge);

	// increase ref counter on surface
	edge->m_refs++;

	return feedge;
}

FSSurface* FEBioInputModel::PartInstance::BuildFESurface(const char* szname)
{
	Surface* surface = m_part->FindSurface(szname);
	if (surface == 0) return 0;

	FSSurface* fesurf = BuildFESurface(*surface);

	// increase ref counter on surface
	surface->m_refs++;

	return fesurf;
}

FSSurface* FEBioInputModel::PartInstance::BuildFESurface(const FEBioInputModel::Surface& surface)
{
	const char* szname = surface.name().c_str();

	const int maxIssues = 10;
	int issuesFound = 0;
	stringstream serr;
	serr << "Building surface \"" << szname << "\":\n";

	// create face list
	vector<int> faceList;
	int NF = surface.faces();
	for (int i = 0; i < NF; ++i)
	{
		const vector<int>& face = surface.face(i);
		int faceID = m_part->m_mesh.FindFace(face);
		if (faceID >= 0)
		{
			// check winding
			FSFace& meshFace = m_part->m_mesh->Face(faceID);
			bool winding = check_winding(face, meshFace);
			if (winding == false)
			{
				if (issuesFound < maxIssues)
				{
					serr << "\tfacet has incorrect winding: ";
					for (int j = 0; j < face.size(); ++j)
					{
						serr << face[j] + 1;
						if (j != face.size() - 1) serr << ",";
					}
					serr << "\n";
				}
				issuesFound++;
			}

			// add it to the list
			faceList.push_back(faceID);
		}
		else
		{
			if (issuesFound < maxIssues)
			{
				serr << "\tCannot find facet: ";
				for (int j = 0; j < face.size(); ++j)
				{
					serr << face[j] + 1;
					if (j != face.size() - 1) serr << ",";
				}
				serr << "\n";
			}
			issuesFound++;
		}
	}

	if (issuesFound)
	{
		if (issuesFound > maxIssues)
		{
			serr << "\t(issue count exceeded limit)\n";
			serr << "\t" << issuesFound << " issues found when building surface.\n";
		}
		string s = serr.str();
		febImport->AddLogEntry(s.c_str());
	}

	// create the surface
	FSSurface* ps = new FSSurface(m_po, faceList);

	// copy the name
	std::string name = surface.name();
	ps->SetName(surface.name());

	// all done
	return ps;
}

FSPart* FEBioInputModel::PartInstance::BuildFEPart(const char* szname)
{
	vector<int> elemList;

	ElementSet* set = m_part->FindElementSet(szname);
	if (set)
	{
		// get the element list
		elemList = set->elemList();

		// these are element IDs. we need to convert them to indices
		// TODO: implement this!
		for (size_t i = 0; i < elemList.size(); ++i) elemList[i] -= 1;
	}
	else
	{
		// could be a domain?
		Domain* dom = m_part->FindDomain(szname);

		// get the element list
		elemList = dom->GetElementIDList();
	}

	if (elemList.empty()) return nullptr;

	// create the part
	FSPart* pg = new FSPart(m_po, elemList);

	// copy the name
	pg->SetName(szname);

	// all done
	return pg;
}


//=============================================================================
FEBioInputModel::Domain::Domain(Part* part) : m_part(part)
{
	m_form = nullptr;
	m_type = UNKNOWN;
	m_defaultShellThickness = 0.0;
}

FEBioInputModel::Domain::Domain(Part* part, const std::string& name, int matID) : m_part(part)
{
	m_name = name;
	m_matID = matID;
	m_form = nullptr;
	m_type = UNKNOWN;
	m_defaultShellThickness = 0.0;
}

FEBioInputModel::Domain::Domain(const Domain& part)
{
	m_part = part.m_part;
	m_name = part.m_name;
	m_matID = part.m_matID;
	m_elem = part.m_elem;
	m_type = part.m_type;
	m_form = part.m_form; // TODO: Need to copy!
	m_defaultShellThickness = part.m_defaultShellThickness;
}

void FEBioInputModel::Domain::operator = (const Domain& part)
{
	m_part = part.m_part;
	m_name = part.m_name;
	m_matID = part.m_matID;
	m_elem = part.m_elem;
	m_type = part.m_type;
}

//=============================================================================
FEBioInputModel::DiscreteSet::DiscreteSet()
{
	m_part = 0;
}

FEBioInputModel::DiscreteSet::DiscreteSet(const FEBioInputModel::DiscreteSet& s)
{
	m_name = s.m_name;
	m_elem = s.m_elem;
	m_part = s.m_part;
}

//-----------------------------------------------------------------------------
void FEBioInputModel::DiscreteSet::operator = (const FEBioInputModel::DiscreteSet& s)
{
	m_name = s.m_name;
	m_elem = s.m_elem;
	m_part = s.m_part;
}

//-----------------------------------------------------------------------------
void FEBioInputModel::DiscreteSet::SetName(const char* sz)
{
	m_name = sz;
}

//-----------------------------------------------------------------------------
void FEBioInputModel::DiscreteSet::Add(int n0, int n1)
{
	DISCRETE_ELEMENT el = { n0, n1 };
	m_elem.push_back(el);
}


//=============================================================================
FEBioInputModel::SurfacePair::SurfacePair() : m_part(0)
{
}

FEBioInputModel::SurfacePair::SurfacePair(const std::string& name, int masterID, int slaveID) : m_part(0)
{
	m_name = name;
	m_masterID = masterID;
	m_slaveID = slaveID;
}

FEBioInputModel::SurfacePair::SurfacePair(const SurfacePair& sp)
{
	m_part = sp.m_part;
	m_name = sp.m_name;
	m_masterID = sp.m_masterID;
	m_slaveID  = sp.m_slaveID;
}

void FEBioInputModel::SurfacePair::operator = (const SurfacePair& sp)
{
	m_part = sp.m_part;
	m_name = sp.m_name;
	m_masterID = sp.m_masterID;
	m_slaveID = sp.m_slaveID;
}

//=============================================================================
FEBioInputModel::PlotVariable::PlotVariable()
{
	m_type = DOMAIN_MESH;
}

FEBioInputModel::PlotVariable::PlotVariable(const std::string& name) : m_name(name)
{
	m_type = DOMAIN_MESH;
}

FEBioInputModel::PlotVariable::PlotVariable(const std::string& name, const std::string& domain, DOMAIN_TYPE domType) : m_name(name), m_domain(domain), m_type(domType)
{
}

FEBioInputModel::PlotVariable::PlotVariable(const FEBioInputModel::PlotVariable& s)
{
	m_name = s.m_name;
    m_domain = s.m_domain;
	m_type = s.m_type;
}

DOMAIN_TYPE FEBioInputModel::PlotVariable::domainType()
{
	return m_type;
}

//-----------------------------------------------------------------------------
void FEBioInputModel::PlotVariable::operator = (const FEBioInputModel::PlotVariable& s)
{
	m_name = s.m_name;
	m_domain = s.m_domain;
}

//=============================================================================
FEBioInputModel::LogVariable::LogVariable()
{
	m_type = -1;
	m_groupID = -1;
}

FEBioInputModel::LogVariable::LogVariable(const LogVariable& log)
{
	m_type = log.m_type;
	m_data = log.m_data;
	m_groupID = log.m_groupID;
	m_file = log.m_file;
}

void FEBioInputModel::LogVariable::operator=(const LogVariable& log)
{
	m_type = log.m_type;
	m_data = log.m_data;
	m_groupID = log.m_groupID;
	m_file = log.m_file;
}

FEBioInputModel::LogVariable::LogVariable(int ntype, const std::string& data)
{
	m_type = ntype;
	m_data = data;
	m_groupID = -1;
}

//=============================================================================
FEBioInputModel::FEBioInputModel(FSModel& fem) : m_fem(fem)
{
	m_shellNodalNormals = true;
}

FEBioInputModel::~FEBioInputModel()
{
	for (int i = 0; i<Parts(); ++i) delete m_part[i];
	m_part.clear();

	for (int i = 0; i<Instances(); ++i) delete m_Inst[i];
	m_Inst.clear();
}

void FEBioInputModel::AddParamCurve(LoadCurve* plc, int lc)
{
	PARAM_CURVE pc = { 0, plc, lc };
	m_PC.push_back(pc);
}

void FEBioInputModel::AddParamCurve(Param* p, int lc)
{
	PARAM_CURVE pc = {p, 0, lc};
	m_PC.push_back(pc);
}

FEBioInputModel::Part* FEBioInputModel::AddPart(const char* szname)
{
	Part* newPart = new Part;
	newPart->SetName(szname);
	m_part.push_back(newPart);
	return newPart;
}

FEBioInputModel::Part* FEBioInputModel::FindPart(const char* szname)
{
	for (int i=0; i<Parts(); ++i)
	{
		Part& part =  GetPart(i);
		if (strcmp(part.GetName(), szname) == 0) return &part;
	}
	return 0;
}

// Call this function after all geometry was processed.
void FEBioInputModel::UpdateGeometry()
{
	// update all the part mesh data
	for (int i=0; i<Parts(); ++i)
	{
		Part& part = GetPart(i);
		FSMesh* mesh = part.GetFEMesh();
		mesh->RebuildMesh();
	}

	// let's also create a new object for each instance
	int NP = Instances();
	for (int i = 0; i<NP; ++i)
	{
		PartInstance& instance = *GetInstance(i);
		Part* part = instance.GetPart();

		GMeshObject* po = instance.GetGObject();
		assert(po == nullptr);

		// clone the mesh
		FSMesh* newMesh = new FSMesh(*part->GetFEMesh());

		// create a new mesh object
		po = new GMeshObject(newMesh);
		instance.SetGObject(po);
		po->SetName(instance.GetName());

		// don't forget to update
		po->Update();

		// apply transform
		po->GetTransform().SetScale(instance.m_scl);
		po->GetTransform().SetPosition(instance.m_pos);
		po->GetTransform().SetRotation(instance.m_rot);

		// update the mesh index data
		part->GetFEBioMesh().UpdateMeshData();

		// assign materials to parts
		assert(part->Domains() == po->Parts());
		for (int i=0; i<po->Parts(); ++i)
		{
			GPart& gpart = *po->Part(i);
			Domain& elSet = part->GetDomain(i);

			GMaterial* mat = GetMaterial(elSet.MatID());
			if (mat) gpart.SetMaterialID(mat->GetID());

			std::string name = elSet.name();
			gpart.SetName(name.c_str());

			FEElementFormulation* eform = elSet.GetElementFormulation();
			FESolidFormulation* solidForm = dynamic_cast<FESolidFormulation*>(eform);
			if (solidForm || (elSet.Type() == Domain::SOLID))
			{
				GSolidSection* solidSection = new GSolidSection(&gpart);
				gpart.SetSection(solidSection);

				solidSection->SetElementFormulation(solidForm);
			}

			FEShellFormulation* shellForm = dynamic_cast<FEShellFormulation*>(eform);
			if (shellForm || (elSet.Type() == Domain::SHELL))
			{
				GShellSection* shellSection = new GShellSection(&gpart);
				gpart.SetSection(shellSection);

				if (shellForm)
					shellSection->SetElementFormulation(shellForm);
				else
					shellSection->SetShellThickness(elSet.GetDefaultShellThickness());
			}

			FEBeamFormulation* beamForm = dynamic_cast<FEBeamFormulation*>(eform);
			if (beamForm || (elSet.Type() == Domain::BEAM))
			{
				GBeamSection* beamSection = new GBeamSection(&gpart);
				gpart.SetSection(beamSection);

				beamSection->SetElementFormulation(beamForm);
			}
        }
	}
}

void FEBioInputModel::CopyMeshSelections()
{
	int NP = Instances();
	for (int i = 0; i < NP; ++i)
	{
		PartInstance& instance = *GetInstance(i);
		Part* part = instance.GetPart();

		GMeshObject* po = instance.GetGObject();
		assert(po);

		// create the nodesets
		for (int j = 0; j < part->NodeSets(); ++j)
		{
			NodeSet& ns = part->GetNodeSet(j);

			// create the node set
			FSNodeSet* pns = new FSNodeSet(po, ns.nodeList());

			// copy the name
			pns->SetName(ns.name());

			po->AddFENodeSet(pns);
		}

		// create the surfaces
		for (int j = 0; j < part->Surfaces(); ++j)
		{
			Surface& s = part->GetSurface(j);

			// create the FESurface
			FSSurface* ps = instance.BuildFESurface(s);

			po->AddFESurface(ps);
		}

		// create the element sets
		for (int j = 0; j < part->ElementSets(); ++j)
		{
			ElementSet& es = part->GetElementSet(j);

			// get the element list
			vector<int> elemList = es.elemList();

			// these are element IDs. we need to convert them to indices
			// TODO: implement this!
			for (size_t i = 0; i < elemList.size(); ++i) elemList[i] -= 1;

			// create the part
			FSPart* pg = new FSPart(po, elemList);

			// copy the name
			pg->SetName(es.name());

			po->AddFEPart(pg);
		}
	}
}


FEBioInputModel::PartInstance* FEBioInputModel::FindInstance(const char* szname)
{
	int N = Instances();
	for (int i=0; i<N; ++i)
	{
		PartInstance* instance = GetInstance(i);
		if (strcmp(instance->GetName(), szname) == 0) return instance;
	}
	return 0;
}

FEItemListBuilder* FEBioInputModel::BuildItemList(const char* szname)
{
	if ((szname == 0) || (strlen(szname) == 0)) return nullptr;

	if (szname[0] == '@')
	{
		const char* ch = strchr(szname, ':');
		if (ch == 0) return nullptr;
		int n = ch - szname;
		if (strncmp(szname, "@surface", n) == 0)
		{
			return BuildFESurface(szname + n + 1);
		}
		if (strncmp(szname, "@edge", n) == 0)
		{
			return BuildFEEdgeSet(szname + n + 1);
		}
		else if (strncmp(szname, "@elem_set", n) == 0)
		{
			return BuildFEPart(szname + n + 1);
		}
		else return nullptr;
	}
	else return BuildFENodeSet(szname);
}

FSNodeSet* FEBioInputModel::BuildFENodeSet(const char* szname)
{
	// see if there is a dot
	const char* ch = 0;
	if (szname) ch = strchr(szname, '.');
	if (ch)
	{
		// if there is, we assume the format is "instance_name.node_set"
		int l = strlen(szname);
		int l1 = ch - szname;
		int l2 = l - l1 - 1;
		char* szpart = new char[l1 + 1];
		char* szset  = new char[l2 + 1];

		if (l1>0) strncpy(szpart, szname, l1); szpart[l1] = 0;
		if (l2>0) strncpy(szset , ch + 1, l2); szset [l2] = 0;

		// find the instance with this name
		PartInstance* part = FindInstance(szpart);

		FSNodeSet* set = 0;
		if (part) set = part->BuildFENodeSet(szset);

		delete [] szset;
		delete [] szpart;

		return set;
	}
	else
	{
		// if not, we assume that there is only one instance
		if (Instances() != 1) return 0;

		PartInstance* part = GetInstance(0);
		return part->BuildFENodeSet(szname);
	}
}

FEBioInputModel::Surface* FEBioInputModel::FindSurface(const char* szname)
{
	// the surface that will be returned
	FEBioInputModel::Surface* surf = 0;

	// see if there is a dot
	const char* ch = strchr(szname, '.');
	if (ch)
	{
		// if there is, we assume the format is "instance_name.surface_name"
		int l = strlen(szname);
		int l1 = ch - szname;
		int l2 = l - l1 - 1;
		char* szpart = new char[l1 + 1];
		char* szset = new char[l2 + 1];

		if (l1>0) strncpy(szpart, szname, l1); szpart[l1] = 0;
		if (l2>0) strncpy(szset, ch + 1, l2); szset[l2] = 0;

		// find the instance with this name
		PartInstance* part = FindInstance(szpart);
		if (part)
		{
			surf = part->GetPart()->FindSurface(szset);
		}

		delete[] szset;
		delete[] szpart;
	}
	else
	{
		// if not, we assume that there is only one instance
		if (Instances() != 1) return 0;

		PartInstance* part = GetInstance(0);
		surf = part->GetPart()->FindSurface(szname);
	}

	return surf;
}

bool FEBioInputModel::BuildDiscreteSet(GDiscreteElementSet& set, const char* szname)
{
	// find the set
	FEBioInputModel::PartInstance* part = 0;
	FEBioInputModel::DiscreteSet* dset = 0;

	// see if there is a dot
	const char* ch = strchr(szname, '.');
	if (ch)
	{
		// if there is, we assume the format is "instance_name.discrete_set_name"
		int l = strlen(szname);
		int l1 = ch - szname;
		int l2 = l - l1 - 1;
		char* szpart = new char[l1 + 1];
		char* szset = new char[l2 + 1];

		if (l1>0) strncpy(szpart, szname, l1); szpart[l1] = 0;
		if (l2>0) strncpy(szset, ch + 1, l2); szset[l2] = 0;

		// find the instance with this name
		part = FindInstance(szpart);
		if (part)
		{
			dset = part->GetPart()->FindDiscreteSet(szset);
		}

		delete[] szset;
		delete[] szpart;
	}
	else
	{
		// if not, we assume that there is only one instance
		if (Instances() != 1) return false;

		part = GetInstance(0);
		dset = part->GetPart()->FindDiscreteSet(szname);
	}

	if (part == 0) { assert(false); return false; }
	if (dset == 0) { assert(false); return false; }

	GMeshObject* po = part->GetGObject();
	if (po && po->GetFEMesh())
	{
		FSMesh& mesh = *po->GetFEMesh();
		int NN = mesh.Nodes();
		int ns = dset->size();
		for (int i = 0; i<ns; ++i)
		{
			const FEBioInputModel::DiscreteSet::DISCRETE_ELEMENT& el = dset->Element(i);

			int n0 = el.n0;
			int n1 = el.n1;
			if ((n0 >= 0) && (n0 < NN) && (n1 >= 0) && (n1 < NN))
			{
				n0 = po->MakeGNode(n0);
				n1 = po->MakeGNode(n1);

				GNode* pn0 = po->FindNode(n0); assert(pn0);
				GNode* pn1 = po->FindNode(n1); assert(pn1);
				set.AddElement(pn0, pn1);
			}
			else
			{
				assert(false);
			}
		}
	}

	return true;
}

FEBioInputModel::Domain* FEBioInputModel::FindDomain(const char* szname)
{
	// the domain that will be returned
	FEBioInputModel::Domain* dom = 0;

	// see if there is a dot
	const char* ch = strchr(szname, '.');
	if (ch)
	{
		// if there is, we assume the format is "instance_name.domain_name"
		int l = strlen(szname);
		int l1 = ch - szname;
		int l2 = l - l1 - 1;
		char* szpart = new char[l1 + 1];
		char* szset = new char[l2 + 1];

		if (l1>0) strncpy(szpart, szname, l1); szpart[l1] = 0;
		if (l2>0) strncpy(szset, ch + 1, l2); szset[l2] = 0;

		// find the instance with this name
		PartInstance* part = FindInstance(szpart);
		if (part)
		{
			dom = part->GetPart()->FindDomain(szset);
		}

		delete[] szset;
		delete[] szpart;
	}
	else
	{
		// if not, we assume that there is only one instance
		if (Instances() != 1) return 0;

		PartInstance* part = GetInstance(0);
		dom = part->GetPart()->FindDomain(szname);
	}

	return dom;
}

FEBioInputModel::ElementSet* FEBioInputModel::FindElementSet(const char* szname)
{
	if (Instances() != 1) return nullptr;

	PartInstance* part = GetInstance(0);
	ElementSet* pg = part->GetPart()->FindElementSet(szname);

	return pg;
}

FSEdgeSet* FEBioInputModel::BuildFEEdgeSet(const char* szname)
{
	// see if there is a dot
	const char* ch = strchr(szname, '.');
	if (ch)
	{
		// if there is, we assume the format is "instance_name.edge_name"
		int l = strlen(szname);
		int l1 = ch - szname;
		int l2 = l - l1 - 1;
		char* szpart = new char[l1 + 1];
		char* szset = new char[l2 + 1];

		if (l1 > 0) strncpy(szpart, szname, l1); szpart[l1] = 0;
		if (l2 > 0) strncpy(szset, ch + 1, l2); szset[l2] = 0;

		// find the instance with this name
		PartInstance* part = FindInstance(szpart);

		FSEdgeSet* edge = 0;
		if (part) edge = part->BuildFEEdgeSet(szset);

		delete[] szset;
		delete[] szpart;

		return edge;
	}
	else
	{
		// if not, we assume that there is only one instance
		if (Instances() != 1) return 0;

		PartInstance* part = GetInstance(0);
		return part->BuildFEEdgeSet(szname);
	}
}

FSSurface* FEBioInputModel::BuildFESurface(const char* szname)
{
	// see if there is a dot
	const char* ch = strchr(szname, '.');
	if (ch)
	{
		// if there is, we assume the format is "instance_name.surface_name"
		int l = strlen(szname);
		int l1 = ch - szname;
		int l2 = l - l1 - 1;
		char* szpart = new char[l1 + 1];
		char* szset = new char[l2 + 1];

		if (l1>0) strncpy(szpart, szname, l1); szpart[l1] = 0;
		if (l2>0) strncpy(szset, ch + 1, l2); szset[l2] = 0;

		// find the instance with this name
		PartInstance* part = FindInstance(szpart);

		FSSurface* surf = 0;
		if (part) surf = part->BuildFESurface(szset);

		delete[] szset;
		delete[] szpart;

		return surf;
	}
	else
	{
		// if not, we assume that there is only one instance
		if (Instances() != 1) return 0;

		PartInstance* part = GetInstance(0);
		return part->BuildFESurface(szname);
	}
}

FSPart* FEBioInputModel::BuildFEPart(const char* szname)
{
	// see if there is a dot
	const char* ch = strchr(szname, '.');
	if (ch)
	{
		// if there is, we assume the format is "instance_name.surface_name"
		int l = strlen(szname);
		int l1 = ch - szname;
		int l2 = l - l1 - 1;
		char* szpart = new char[l1 + 1];
		char* szset = new char[l2 + 1];

		if (l1>0) strncpy(szpart, szname, l1); szpart[l1] = 0;
		if (l2>0) strncpy(szset, ch + 1, l2); szset[l2] = 0;

		// find the instance with this name
		PartInstance* part = FindInstance(szpart);

		FSPart* pg = 0;
		if (part) pg = part->BuildFEPart(szset);

		delete[] szset;
		delete[] szpart;

		return pg;
	}
	else
	{
		// if not, we assume that there is only one instance
		if (Instances() != 1) return 0;

		PartInstance* part = GetInstance(0);
		return part->BuildFEPart(szname);
	}
}

FSPart* FEBioInputModel::BuildFEPart(FEBioInputModel::Domain* dom)
{
	PartInstance* part = GetInstance(0);

	FSPart* pg = new FSPart(part->GetGObject(), dom->GetElementIDList());

	pg->SetName(dom->name());

	return pg;
}

GPart* FEBioInputModel::FindGPart(const char* szname)
{
	GMeshObject* po = GetInstance(0)->GetGObject();
	if (po == nullptr) return nullptr;
	return po->FindPartFromName(szname);
}

FEBioInputModel::SurfacePair* FEBioInputModel::FindSurfacePair(const char* szname)
{
	// see if there is a dot
	const char* ch = strchr(szname, '.');
	if (ch)
	{
		// if there is, we assume the format is "instance_name.surface_name"
		int l = strlen(szname);
		int l1 = ch - szname;
		int l2 = l - l1 - 1;
		char* szpart = new char[l1 + 1];
		char* szset = new char[l2 + 1];

		if (l1>0) strncpy(szpart, szname, l1); szpart[l1] = 0;
		if (l2>0) strncpy(szset, ch + 1, l2); szset[l2] = 0;

		// find the instance with this name
		PartInstance* instance = FindInstance(szpart);

		SurfacePair* surf = 0;
		if (instance) surf = instance->GetPart()->FindSurfacePair(szset);

		delete[] szset;
		delete[] szpart;

		return surf;
	}
	else
	{
		// if not, we assume that there is only one instance
		if (Instances() != 1) return 0;

		Part* part = &GetPart(0);
		return part->FindSurfacePair(szname);
	}
}

int FEBioInputModel::GetMaterialIndex(const char* szmat)
{
	for (int i=0; i<Materials(); ++i)
	{
		GMaterial* mat = GetMaterial(i);
		string name = mat->GetName();
		if (name == szmat) return i;
	}
	return -1;
}
