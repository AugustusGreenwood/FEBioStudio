#include "GMeshObject.h"
#include <MeshLib/FESurfaceMesh.h>
#include <list>
#include <stack>
using namespace std;

//-----------------------------------------------------------------------------
// Constructor for creating a GMeshObject from a naked mesh. 
GMeshObject::GMeshObject(FEMesh* pm) : GObject(GMESH_OBJECT)
{
	// update the object
	if (pm)
	{
		SetFEMesh(pm);

		// It set this to false, otherwise cloning would loose partitioning
		if (pm->Nodes()) Update(false);
	}
}

//-----------------------------------------------------------------------------
// Constructor for creating a GMeshObject from a naked mesh. 
GMeshObject::GMeshObject(FESurfaceMesh* pm) : GObject(GMESH_OBJECT)
{
	// update the object
	if (pm)
	{
		SetFEMesh(ConvertSurfaceToMesh(pm));
		if (pm->Nodes()) Update();
	}
}

//-----------------------------------------------------------------------------
// This function creates a new GMeshObject from an existing GObject
// The new GMeshObject will have the same ID as the existing GObject
// This is used in the CCmdConvertObject command that converts a GPrimitve
// a GMeshObject
GMeshObject::GMeshObject(GObject* po) : GObject(GMESH_OBJECT)
{
	// copy to old object's ID
	SetID(po->GetID());

	// creating a new object has increased the object counter
	// so we need to decrease it again
	GItem_T<GBaseObject>::DecreaseCounter();

	// next, we copy the geometry info
	// --- Nodes ---
	int NN = po->Nodes();
	m_Node.reserve(NN);
	for (int i=0; i<NN; ++i)
	{
		GNode* n = new GNode(this);
		GNode& no = *po->Node(i);
		n->LocalPosition() = no.LocalPosition();
		n->SetID(no.GetID());
		n->SetLocalID(i);
		n->SetType(no.Type());
		assert(n->GetLocalID() == no.GetLocalID());
		n->SetName(no.GetName());
		m_Node.push_back(n);
	}

	// --- Edges ---
	int NE = po->Edges();
	m_Edge.reserve(NE);
	for (int i=0; i<NE; ++i)
	{
		GEdge* e = new GEdge(this);
		GEdge& eo = *po->Edge(i);
		e->m_node[0] = eo.m_node[0];
		e->m_node[1] = eo.m_node[1];
		e->SetID(eo.GetID());
		e->SetLocalID(i);
		e->SetName(eo.GetName());
		assert(e->GetLocalID() == eo.GetLocalID());
		m_Edge.push_back(e);
	}

	// --- Faces ---
	int NF = po->Faces();
	m_Face.reserve(NF);
	for (int i = 0; i<NF; ++i)
	{
		GFace* f = new GFace(this);
		GFace& fo = *po->Face(i);
		f->m_node = fo.m_node;
		f->m_nPID[0] = fo.m_nPID[0];
		f->m_nPID[1] = fo.m_nPID[1];
		f->SetID(fo.GetID());
		f->SetLocalID(i);
		f->SetName(fo.GetName());
		assert(f->GetLocalID() == fo.GetLocalID());
		m_Face.push_back(f);
	}

	// --- Parts ---
	int NP = po->Parts();
	m_Part.reserve(NP);
	for (int i = 0; i<NP; ++i)
	{
		GPart* g = new GPart(this);
		GPart& go = *po->Part(i);
		g->SetMaterialID(go.GetMaterialID());
		g->SetID(go.GetID());
		g->SetLocalID(i);
		g->SetName(go.GetName());
		assert(g->GetLocalID() == go.GetLocalID());
		m_Part.push_back(g);
	}

	// copy the mesh from the original object
	FEMesh* pm = new FEMesh(*po->GetFEMesh());
	SetFEMesh(pm);

	// rebuild the GMesh
	BuildGMesh();
}

//-----------------------------------------------------------------------------
// This function updates the geometry information and takes a boolean parameter
// (default = true) to indicate whether the mesh has to be repartitioned. This
// function will also rebuild the GMesh for rendering.
bool GMeshObject::Update(bool b)
{
	UpdateParts();
	UpdateSurfaces();
	UpdateNodes();
	UpdateEdges();

	BuildGMesh();

	return true;
}

//-----------------------------------------------------------------------------
// It is assumed that the group ID's have already been assigned to the elements.
// This was either done by the auto-mesher or by copying the element data from
// an existing mesh (when converting an object to a GMeshObject).
void GMeshObject::UpdateParts()
{
	// get the mesh
	FEMesh& m = *m_pmesh;

	// count how many parts there are
	int nparts = 0;
	for (int i=0; i<m.Elements(); ++i)
	{
		FEElement& el = m.Element(i);
		if (el.m_gid+1 > nparts) nparts = el.m_gid+1;
	}

	// create the GParts
	// We only create needed parts
	if (nparts < (int) m_Part.size()) ResizeParts(nparts);
	else if (nparts > (int) m_Part.size())
	{
		for (int i = (int)m_Part.size(); i<nparts; ++i) AddPart();
	}
}

//-----------------------------------------------------------------------------
// The surfaces are defined by face connectivity

void GMeshObject::UpdateSurfaces()
{
	// get the mesh
	FEMesh& m = *m_pmesh;
	int NF = m.Faces();

	// find the number of surfaces we have
	int ng = -1;
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = m.Face(i);
		if (f.m_gid > ng) ng = f.m_gid;
	}
	++ng;

	// create the Surfaces (if necessary)
	if (ng < (int) m_Face.size()) ResizeSurfaces(ng);
	else if (ng > (int) m_Face.size())
	{
		for (int i=(int)m_Face.size(); i<ng; ++i)
		{
			GFace* s = new GFace(this);
			AddSurface(s);
		}
	}

	// reset part ID's for all surfaces
	for (int i = 0; i<(int)m_Face.size(); ++i) { m_Face[i]->m_nPID[0] = m_Face[i]->m_nPID[1] = -1; }

	// assign part ID's
	FEElement* pe;
	for (int i=0; i<m.Faces(); ++i)
	{
		// get the face
		FEFace& f = m.Face(i);

		// get the two part IDs
		int pid0 = -1, pid1 = -1;

		// make sure the first element is nonzero
		pe = m.ElementPtr(f.m_elem[0]);
		if (pe == 0) throw GObjectException(this, "GMeshObject::UpdateSurfaces\n(pe == NULL)");
		if (pe) pid0 = pe->m_gid;

		// get the second element (which can be zero)
		pe = m.ElementPtr(f.m_elem[1]);
		if (pe) pid1 = pe->m_gid;

		// assign the part ID's
		int* pid = m_Face[f.m_gid]->m_nPID;
		if (pid[0] == -1) pid[0] = pid0;
		if (pid[1] == -1) pid[1] = pid1;

		// make sure the part IDs match
		if ((pid[0] != pid0) && (pid[0] != pid1))
		{
			throw GObjectException(this, "GMeshObject::UpdateSurfaces\nID's don't match");
		}
		// make sure the part IDs match
		if ((pid[1] != pid0) && (pid[1] != pid1))
		{
			throw GObjectException(this, "GMeshObject::UpdateSurfaces\nID's don't match");
		}
	}
}

//-----------------------------------------------------------------------------

void GMeshObject::UpdateEdges()
{
	// get the mesh
	FEMesh& m = *m_pmesh;
	int NE = m.Edges();

	int ng = -1;
	for (int i=0; i<NE; ++i)
	{
		FEEdge& e = m.Edge(i);
		if (e.m_gid > ng) ng = e.m_gid;
	}
	++ng;

	// create the Edge
	if (ng < (int) m_Edge.size()) ResizeCurves(ng);
	else if (ng > (int) m_Edge.size())
	{
		for (int i=(int)m_Edge.size(); i<ng; ++i) 
		{
			GEdge* e = new GEdge(this);
			e->m_node[0] = -1;
			e->m_node[1] = -1;
			AddEdge(e);
		}
	}

	// reset edge nodes
	for (int i=0; i<ng; ++i)
	{
		GEdge& e = *m_Edge[i];
		e.m_node[0] = -1;
		e.m_node[1] = -1;
	}

	// set the nodes for the GEdge
	const int NN = Nodes();
	if (NN > 0)
	{
		for (int i=0; i<NE; ++i)
		{
			FEEdge& e = m.Edge(i);
			for (int j=0; j<2; ++j)
			{
				if (e.m_nbr[j] == -1)
				{
					GEdge& ge = *m_Edge[e.m_gid];
					if (ge.m_node[0] == -1)
					{
						FENode& nj = m.Node(e.n[j]);
						if ((nj.m_gid < 0) || (nj.m_gid >= NN)) 
						{
							throw GObjectException(this, "GMeshObject::UpdateEdges\n(invalid node ID)");
						}
						ge.m_node[0] = m_Node[nj.m_gid]->GetLocalID();
					}
					else if (ge.m_node[1] == -1)
					{
						FENode& nj = m.Node(e.n[j]);
						if ((nj.m_gid < 0) || (nj.m_gid >= NN)) 
						{
							throw GObjectException(this, "GMeshObject::UpdateEdges\n(invalid node ID)");
						}
						ge.m_node[1] = m_Node[nj.m_gid]->GetLocalID();
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------

void GMeshObject::UpdateNodes()
{
	// get the mesh
	FEMesh& m = *m_pmesh;

	// first, we need to figure out which nodes are no longer being used
	int NN = Nodes();
	vector<int> tag; tag.assign(NN, -1);
	for (int i=0; i<m.Nodes(); ++i)
	{
		FENode& n = m.Node(i);
		if ((n.m_gid >= 0) && (n.m_gid < NN)) tag[n.m_gid] = i;
	}

	// remove the nodes that are no longer used
	int n = 0;
	for (int i=0; i<NN; ++i)
	{
		if (tag[i] != -1)
		{
			if (n != i)
			{
				m_Node[n] = m_Node[i];
				m.Node(tag[i]).m_gid = n;
			}
			n++;
		}
	}
	if (n != NN) 
	{
		ResizeNodes(n);
		// reset local ID's
		for (int i = 0; i<(int)m_Node.size(); ++i) m_Node[i]->SetLocalID(i);
	}

	// now find the largest gid
	int nn = -1, gid;
	for (int i=0; i<m.Nodes(); ++i)
	{
		gid = m.Node(i).m_gid;
		if (gid > nn) nn = gid;
	}
	++nn;
	assert(nn >= n);

	if (nn > (int) m_Node.size())
	{
		for (int i=(int)m_Node.size(); i<nn; ++i) 
		{
			GNode* n = new GNode(this);
			n->SetType(NODE_VERTEX);
			GObject::AddNode(n);
		}
	}

	for (int i=0; i<m.Nodes(); ++i)
	{
		FENode& node = m.Node(i);
		if (node.m_gid >= 0) m_Node[node.m_gid]->LocalPosition() = node.r;
	}
}

//-----------------------------------------------------------------------------
// This function converts an FE node to a GNode and return the ID
// of the GNode
//
int GMeshObject::MakeGNode(int n)
{
	// get the mesh
	FEMesh& m = *m_pmesh;
	FENode& fen = m.Node(n);

	if (fen.m_gid == -1)
	{
		// create a new node
		GNode* node = new GNode(this);
		node->SetID(GNode::CreateUniqueID());
		int N = (int)m_Node.size();
		node->SetLocalID(N);
		node->LocalPosition() = fen.r;
		m_Node.push_back(node);

		fen.m_gid = N;
		return node->GetID();
	}
	else
	{
		return m_Node[fen.m_gid]->GetID();
	}
}

//-----------------------------------------------------------------------------
// This function adds a node to the GMeshObject and to the FEMesh

void GMeshObject::AddNode(vec3d r)
{
	FEMesh& m = *m_pmesh;
	FENode n;
	n.m_bext = true;
	n.m_gid = (int)m_Node.size();

	// convert from global to local
	r = Transform().GlobalToLocal(r);

	// add the node to the mesh
	n.r = r;
	m.AddNode(n);

	// create a geometry node for this
	GNode* gn = new GNode(this);
	gn->SetID(GNode::CreateUniqueID());
	gn->SetLocalID((int)m_Node.size());
	gn->LocalPosition() = r;
	m_Node.push_back(gn);

	m.UpdateBox();
	BuildGMesh();
}

//-----------------------------------------------------------------------------

FEMesh* GMeshObject::BuildMesh()
{
	// the mesh is already built so we don't have to rebuilt it
	return m_pmesh;
}

//-----------------------------------------------------------------------------
void GMeshObject::BuildGMesh()
{
	// allocate new GL mesh
	if (m_pGMesh == 0) delete m_pGMesh;
	m_pGMesh = new GLMesh();

	// we'll extract the data from the FE mesh
	FEMesh* pm = GetFEMesh();

	// clear tags on all nodes
	int NN = pm->Nodes();
	for (int i = 0; i<NN; ++i) pm->Node(i).m_ntag = 1;

	// Identify the isolated vertices since we want the bounding box to include those as well
	int NE = pm->Elements();
	for (int i=0; i<NE; ++i)
	{
		FEElement& el = pm->Element(i);
		int ne = el.Nodes();
		for (int j=0; j<ne; ++j)
		{
			pm->Node(el.m_node[j]).m_ntag = -1;
		}
	}

	// count all faces and tag nodes
	for (int i=0; i<pm->Faces(); ++i)
	{
		FEFace& f = pm->Face(i);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j) pm->Node(f.n[j]).m_ntag = 1;
	}

	// create nodes
	for (int i=0; i<NN; ++i)
	{
		FENode& node = pm->Node(i);
		if (node.m_ntag == 1) node.m_ntag = m_pGMesh->AddNode(node.r, node.m_gid);
	}

	// create edges
	int n[FEFace::MAX_NODES];
	for (int i=0; i<pm->Edges(); ++i)
	{
		FEEdge& es = pm->Edge(i);
		n[0] = pm->Node(es.n[0]).m_ntag; assert(n[0] >= 0);
		n[1] = pm->Node(es.n[1]).m_ntag; assert(n[1] >= 0);
		if (es.n[2] != -1) { n[2] = pm->Node(es.n[2]).m_ntag; assert(n[2] >= 0); }
		if (es.n[3] != -1) { n[3] = pm->Node(es.n[3]).m_ntag; assert(n[3] >= 0); }
		m_pGMesh->AddEdge(n, es.Nodes(), es.m_gid);
	}

	// create face data
	for (int i=0; i<pm->Faces(); ++i)
	{
		FEFace& fs = pm->Face(i);
		int nf = fs.Nodes();
		for (int j=0; j<nf; ++j) n[j] = pm->Node(fs.n[j]).m_ntag;
		m_pGMesh->AddFace(n, nf, fs.m_gid, fs.m_sid, fs.IsExternal());
	}

	m_pGMesh->Update();
}

//-----------------------------------------------------------------------------
// Create a clone of this object
GObject* GMeshObject::Clone()
{
	// create a copy of our mesh
	FEMesh* pm = new FEMesh(*m_pmesh);

	// create a new GMeshObject from this mesh
	GMeshObject* po = new GMeshObject(pm);

	// copy transform
	po->CopyTransform(this);

	// copy color
	po->SetColor(GetColor());

	//return the object
	return po;
}

//-----------------------------------------------------------------------------
// Save data to file
void GMeshObject::Save(OArchive &ar)
{
	// save the name
	ar.WriteChunk(CID_OBJ_NAME, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());

	// save the transform stuff
	ar.BeginChunk(CID_OBJ_HEADER);
	{
		int nid = GetID();
		ar.WriteChunk(CID_OBJ_ID, nid);
		ar.WriteChunk(CID_OBJ_POS, Transform().GetPosition());
		ar.WriteChunk(CID_OBJ_ROT, Transform().GetRotation());
		ar.WriteChunk(CID_OBJ_SCALE, Transform().GetScale());
		ar.WriteChunk(CID_OBJ_COLOR, GetColor());

		int nparts = Parts();
		int nfaces = Faces();
		int nedges = Edges();
		int nnodes = Nodes();

		ar.WriteChunk(CID_OBJ_PARTS, nparts);
		ar.WriteChunk(CID_OBJ_FACES, nfaces);
		ar.WriteChunk(CID_OBJ_EDGES, nedges);
		ar.WriteChunk(CID_OBJ_NODES, nnodes);
	}
	ar.EndChunk();

	// save the parameters
	if (Parameters() > 0)
	{
		ar.BeginChunk(CID_OBJ_PARAMS);
		{
			ParamContainer::Save(ar);
		}
		ar.EndChunk();
	}

	// save the parts
	ar.BeginChunk(CID_OBJ_PART_SECTION);
	{
		for (int i=0; i<Parts(); ++i)
		{
			ar.BeginChunk(CID_OBJ_PART);
			{
				GPart& p = *Part(i);
				int nid = p.GetID();
				int mid = p.GetMaterialID();
				ar.WriteChunk(CID_OBJ_PART_ID, nid);
				ar.WriteChunk(CID_OBJ_PART_MAT, mid);
				ar.WriteChunk(CID_OBJ_PART_NAME, p.GetName());
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the surfaces
	ar.BeginChunk(CID_OBJ_FACE_SECTION);
	{
		for (int i=0; i<Faces(); ++i)
		{
			ar.BeginChunk(CID_OBJ_FACE);
			{
				GFace& f = *Face(i);
				int nid = f.GetID();
				ar.WriteChunk(CID_OBJ_FACE_ID, nid);
				ar.WriteChunk(CID_OBJ_FACE_NAME, f.GetName());
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the edges
	ar.BeginChunk(CID_OBJ_EDGE_SECTION);
	{
		for (int i=0; i<Edges(); ++i)
		{
			ar.BeginChunk(CID_OBJ_EDGE);
			{
				GEdge& e = *Edge(i);
				int nid = e.GetID();
				ar.WriteChunk(CID_OBJ_EDGE_ID, nid);
				ar.WriteChunk(CID_OBJ_EDGE_NAME, e.GetName());
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the nodes
	// note that it is possible that an object doesn't have any nodes
	// for instance, a shell disc
	if (Nodes()>0)
	{
		ar.BeginChunk(CID_OBJ_NODE_SECTION);
		{
			for (int i=0; i<Nodes(); ++i)
			{	
				ar.BeginChunk(CID_OBJ_NODE);
				{
					GNode& v = *Node(i);
					int nid = v.GetID();
					ar.WriteChunk(CID_OBJ_NODE_ID, nid);
					ar.WriteChunk(CID_OBJ_NODE_POS, v.LocalPosition());
					ar.WriteChunk(CID_OBJ_NODE_NAME, v.GetName());
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the mesh
	if (m_pmesh)
	{
		ar.BeginChunk(CID_MESH);
		{
			m_pmesh->Save(ar);
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
// Load data from file
void GMeshObject::Load(IArchive& ar)
{
	TRACE("GMeshObject::Load");

	int nparts = -1, nfaces = -1, nedges = -1, nnodes = -1;

	while (IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		// object name
		case CID_OBJ_NAME: 
			{
				string name;
				ar.read(name);
				SetName(name);
			}
			break;
		case CID_FEOBJ_INFO:
		{
			string info;
			ar.read(info);
			SetInfo(info);
		}
		break;
		// header
		case CID_OBJ_HEADER:
			{
				vec3d pos, scl;
				quatd rot;
				GLColor col;
				while (IO_OK == ar.OpenChunk())
				{
					int nid = ar.GetChunkID();
					int oid;
					switch (nid)
					{
					case CID_OBJ_ID: ar.read(oid); SetID(oid); break;
					case CID_OBJ_POS: ar.read(pos); break;
					case CID_OBJ_ROT: ar.read(rot); break;
					case CID_OBJ_SCALE: ar.read(scl); break;
					case CID_OBJ_COLOR: ar.read(col); break;
					case CID_OBJ_PARTS: ar.read(nparts); break;
					case CID_OBJ_FACES: ar.read(nfaces); break;
					case CID_OBJ_EDGES: ar.read(nedges); break;
					case CID_OBJ_NODES: ar.read(nnodes); break;
					}
					ar.CloseChunk();
				}

				SetColor(col);

				GTransform& transform = Transform();
				transform.SetPosition(pos);
				transform.SetRotation(rot);
				transform.SetScale(scl);
			}
			break;
		// object parameters
		case CID_OBJ_PARAMS:
			ParamContainer::Load(ar);
			break;
		// object parts
		case CID_OBJ_PART_SECTION:
			{
				assert(nparts > 0);
				m_Part.reserve(nparts);
				int n = 0;
				while (IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_PART) throw ReadError("error parsing CID_OBJ_PART_SECTION (GMeshObject::Load)");

					GPart* p = new GPart(this);
					while (IO_OK == ar.OpenChunk())
					{
						int nid, mid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_PART_ID: ar.read(nid); p->SetID(nid); break;
						case CID_OBJ_PART_MAT: ar.read(mid); p->SetMaterialID(mid); break;
						case CID_OBJ_PART_NAME:
							{
								char szname[256]={0};
								ar.read(szname);
								p->SetName(szname);
							}
							break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					p->SetLocalID(n++);

					m_Part.push_back(p);
				}
				assert((int) m_Part.size() == nparts);
			}
			break;
		// object surfaces
		case CID_OBJ_FACE_SECTION:
			{
				assert(nfaces > 0);
				m_Face.reserve(nfaces);
				int n = 0;
				while (IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_FACE) throw ReadError("error parsing CID_OBJ_FACE_SECTION (GMeshObject::Load)");

					GFace* f = new GFace(this);
					while (IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_FACE_ID: ar.read(nid); f->SetID(nid); break;
						case CID_OBJ_FACE_NAME:
							{
								char szname[256]={0};
								ar.read(szname);
								f->SetName(szname);
							}
							break;					
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					f->SetLocalID(n++);

					m_Face.push_back(f);
				}
				assert((int) m_Face.size() == nfaces);
			}
			break;
		// object edges
		case CID_OBJ_EDGE_SECTION:
			{
				m_Edge.clear();
				if (nedges > 0) m_Edge.reserve(nedges);
				int n = 0;
				while (IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_EDGE) throw ReadError("error parsing CID_OBJ_EDGE_SECTION (GMeshObject::Load)");

					GEdge* e = new GEdge(this);
					while (IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_EDGE_ID: ar.read(nid); e->SetID(nid); break;
						case CID_OBJ_EDGE_NAME:
							{
								char szname[256]={0};
								ar.read(szname);
								e->SetName(szname);
							}
							break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					e->SetLocalID(n++);

					m_Edge.push_back(e);
				}
				assert((int) m_Edge.size() == nedges);
			}
			break;
		// object nodes
		case CID_OBJ_NODE_SECTION:
			{
				m_Node.clear();
				if (nnodes > 0)
				{
					m_Node.reserve(nnodes);
					int m = 0;
					while (IO_OK == ar.OpenChunk())
					{
						if (ar.GetChunkID() != CID_OBJ_NODE) throw ReadError("error parsing CID_OBJ_NODE_SECTION (GMeshObject::Load)");

						GNode* n = new GNode(this);
						while (IO_OK == ar.OpenChunk())
						{
							int nid;
							switch (ar.GetChunkID())
							{
							case CID_OBJ_NODE_ID: ar.read(nid); n->SetID(nid); break;
							case CID_OBJ_NODE_POS: ar.read(n->LocalPosition()); break;
							case CID_OBJ_NODE_NAME:
								{
									char szname[256]={0};
									ar.read(szname);
									n->SetName(szname);
								}
								break;		
							}
							ar.CloseChunk();
						}
						ar.CloseChunk();

						n->SetLocalID(m++);

						m_Node.push_back(n);
					}
					assert((int) m_Node.size() == nnodes);
				}
			}
			break;
		// the mesh object
		case CID_MESH:
			if (m_pmesh) delete m_pmesh;
			SetFEMesh(new FEMesh);
			m_pmesh->Load(ar);
			break;
		}
		ar.CloseChunk();
	}

	Update(false);
}

//-----------------------------------------------------------------------------
void GMeshObject::Attach(GObject* po, bool bweld, double tol)
{
	// Add the nodes
	int NN0 = Nodes();
	int NN = po->Nodes();
	for (int i=0; i<NN; ++i)
	{
		GNode* n = new GNode(this);
		GNode& no = *po->Node(i);
		n->LocalPosition() = Transform().GlobalToLocal(po->Transform().LocalToGlobal(no.LocalPosition()));
		n->SetID(no.GetID());
		n->SetLocalID(i + NN0);
		n->SetType(no.Type());
		n->SetName(no.GetName());
		m_Node.push_back(n);
	}

	// --- Edges ---
	int NE0 = Edges();
	int NE = po->Edges();
	for (int i=0; i<NE; ++i)
	{
		GEdge* e = new GEdge(this);
		GEdge& eo = *po->Edge(i);
		e->m_node[0] = eo.m_node[0] + NN0;
		e->m_node[1] = eo.m_node[1] + NN0;
		e->m_cnode = (eo.m_cnode >= 0 ? eo.m_cnode + NN0 : -1);
		e->SetID(eo.GetID());
		e->SetLocalID(i + NE0);
		e->SetName(eo.GetName());
		m_Edge.push_back(e);
	}

	// --- Parts ---
	int NP0 = Parts();
	int NP = po->Parts();
	for (int i=0; i<NP; ++i)
	{
		GPart* g = new GPart(this);
		GPart& go = *po->Part(i);
		g->SetMaterialID(go.GetMaterialID());
		g->SetID(go.GetID());
		g->SetLocalID(i + NP0);
		g->SetName(go.GetName());
		m_Part.push_back(g);
	}

	// --- Faces ---
	int NF0 = Faces();
	int NF = po->Faces();
	for (int i=0; i<NF; ++i)
	{
		GFace* f = new GFace(this);
		GFace& fo = *po->Face(i);

		f->m_node = fo.m_node;
		for (int j=0; j<(int)fo.m_node.size(); ++j)
		{
			f->m_node[j] = fo.m_node[j] + NN0;
		}

		f->m_nPID[0] = (fo.m_nPID[0] >= 0 ? fo.m_nPID[0] + NP0 : -1);
		f->m_nPID[1] = (fo.m_nPID[1] >= 0 ? fo.m_nPID[1] + NP0 : -1);
		f->SetID(fo.GetID());
		f->SetLocalID(i + NF0);
		f->SetName(fo.GetName());
		m_Face.push_back(f);
	}

	// attach to the new mesh
	FEMesh* pm = po->GetFEMesh();
	if (bweld)
	{
		m_pmesh->AttachAndWeld(*pm, tol);
		Update(false);
	}
	else
	{
		m_pmesh->Attach(*pm);
	}

	m_pmesh->UpdateBox();
	m_pmesh->UpdateNormals();

	BuildGMesh();
}

void GMeshObject::DeletePart(GPart* pg)
{
	// make sure this is a part of this object
	if (pg->Object() != this)
	{
		assert(false);
		return;
	}

	// find the part
	int npart = -1;
	for (int i=0; i<Parts(); ++i)
	{
		if (m_Part[i] == pg)
		{
			npart = i;
			break;
		}
	}
	assert(npart != -1);
	if (npart == -1) return;

	// remove this part from the list
	m_Part.erase(m_Part.begin() + npart);
	for (int n=0; n<m_Part.size(); ++n)
	{
		m_Part[n]->SetLocalID(n);
	}
	delete pg;

	// delete all the elements 
	FEMesh* pm = GetFEMesh(); assert(pm);
	if (pm == 0) return;

	pm->TagAllElements(0);
	int NE = pm->Elements();
	for (int i=0; i<NE; ++i)
	{
		FEElement& el = pm->Element(i);
		if (el.m_gid == npart) el.m_ntag = 1;
	}
	pm->DeleteTaggedElements(1);

	// update the rest
	Update();
}

// detach an element selection
GMeshObject* GMeshObject::DetachSelection()
{
	FEMesh* pm = GetFEMesh();

	FEMesh* newMesh = pm->DetachSelectedMesh();
	Update(true);

	// create a new object for this mesh
	GMeshObject* newObject = new GMeshObject(newMesh);

	return newObject;
}

GMeshObject* ExtractSelection(GObject* po)
{
	FEMesh* pm = po->GetFEMesh();

	FEMesh* newMesh = pm->ExtractFaces(true);

	// create a new object for this mesh
	GMeshObject* newObject = new GMeshObject(newMesh);
	newObject->CopyTransform(po);

	return newObject;
}
