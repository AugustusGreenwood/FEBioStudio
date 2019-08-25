#include "GSurfaceMeshObject.h"
#include <MeshTools/FETetGenMesher.h>
#include <MeshTools/FEModifier.h>
#include <MeshLib/FECurveMesh.h>
#include <MeshTools/GLMesh.h>

GSurfaceMeshObject::GSurfaceMeshObject(FESurfaceMesh* pm) : GObject(GSURFACEMESH_OBJECT), m_surfmesh(pm)
{
	SetFEMesher(new FETetGenMesher(this));
	if (m_surfmesh)
	{
		m_surfmesh->SetGObject(this);
		Update();
	}
}

GSurfaceMeshObject::GSurfaceMeshObject(GObject* po) : GObject(GSURFACEMESH_OBJECT)
{
	SetFEMesher(new FETetGenMesher(this));

	// copy to old object's ID
	SetID(po->GetID());

	// creating a new object has increased the object counter
	// so we need to decrease it again
	GItem_T<GBaseObject>::DecreaseCounter();

	// next, we copy the geometry info
	// --- Nodes ---
	int NN = po->Nodes();
	m_Node.reserve(NN);
	for (int i = 0; i<NN; ++i)
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
	for (int i = 0; i<NE; ++i)
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

	// create the surface mesh
	m_surfmesh = new FESurfaceMesh;
	m_surfmesh->SetGObject(this);

	// copy the surface mesh from the original object's mesh
	FEMeshBase* pm = po->GetEditableMesh(); assert(pm);
	NN = pm->Nodes();
	for (int i=0;i <NN; ++i) pm->Node(i).m_ntag = -1;
	NF = pm->Faces();
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = pm->Face(i);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j) pm->Node(f.n[j]).m_ntag = 1;
	}

	int nodes = 0;
	for (int i=0; i<NN; ++i)
	{
		FENode& node = pm->Node(i);
		if (node.m_ntag == 1) node.m_ntag = nodes++;
	}

	// create new mesh
	m_surfmesh->Create(nodes, 0, NF);

	// copy nodes
	for (int i=0; i<NN; ++i)
	{
		FENode& node = pm->Node(i);
		if (node.m_ntag >= 0)
		{
			FENode& snode = m_surfmesh->Node(node.m_ntag);
			snode = node;
		}
	}
	m_surfmesh->UpdateNodePartitions();

	// copy faces
	for (int i=0; i<NF; ++i)
	{
		FEFace& f = m_surfmesh->Face(i);
		f = pm->Face(i);
		int nf = f.Nodes();
		for (int j=0; j<nf; ++j) f.n[j] = pm->Node(f.n[j]).m_ntag;
	}
	m_surfmesh->UpdateFacePartitions();
	m_surfmesh->UpdateFaceNeighbors();
	m_surfmesh->UpdateNormals();
	m_surfmesh->UpdateBox();

	// copy edges
	m_surfmesh->BuildEdges();
	for (int i=0; i<pm->Edges(); ++i)
	{
		FEEdge& src = pm->Edge(i);
		int n0 = pm->Node(src.n[0]).m_ntag;
		int n1 = pm->Node(src.n[1]).m_ntag;
		FEEdge* pe = m_surfmesh->FindEdge(n0, n1);
		assert(pe);
		if (pe)
		{
			pe->m_gid = src.m_gid;
		}
	}
	m_surfmesh->UpdateEdgePartitions();
	m_surfmesh->UpdateEdgeNeighbors();

	// update the object
	Update();
}

FEMesh* GSurfaceMeshObject::BuildMesh()
{
	// make sure that the surface is triangular and is closed
	int NF = m_surfmesh->Faces();
	for (int i = 0; i<NF; ++i)
	{
		FEFace& face = m_surfmesh->Face(i);

		// Only triangles, quads are accepted
		int nf = face.Nodes();
		if ((nf != 3) && (nf != 4)) return 0;

		// mesh has to to be closed
		for (int j=0; j<nf; ++j)
			if (face.m_nbr[j] == -1) return 0;
	}

	// get the tetgen mesher
	FETetGenMesher* mesher = static_cast<FETetGenMesher*>(GetMesher());

	// keep a pointer to the old mesh
	FEMesh* pold = GetFEMesh();

	// create a new mesh
	FEMesh* pmesh = mesher->CreateMesh(m_surfmesh);
	SetFEMesh(pmesh);

	// now it is safe to delete the old mesh
	if (pold) delete pold;

	return pmesh;
}

void GSurfaceMeshObject::Update()
{
	// create one part
	if (Parts() == 0) AddPart();

	// Add surfaces
	UpdateSurfaces();

	// update nodes
	UpdateNodes();

	// update edges
	UpdateEdges();

	BuildGMesh();
}

//-----------------------------------------------------------------------------
void GSurfaceMeshObject::UpdateNodes()
{
	// get the mesh
	FESurfaceMesh& m = *GetSurfaceMesh();

	// count the node partitions
	int ng = m.CountNodePartitions();

	// create the Nodes
	if (ng < (int) m_Node.size()) ResizeNodes(ng);
	else if (ng >(int) m_Node.size())
	{
		for (int i= (int)m_Node.size(); i<ng; ++i)
		{
			GNode* node = new GNode(this);
			node->SetType(NODE_VERTEX);
			AddNode(node);
		}
	}

	// reset nodes
	int NN = m.Nodes();
	for (int i = 0; i<NN; ++i)
	{
		FENode& node = m.Node(i);
		if (node.m_gid >= 0)
		{
			GNode& n = *m_Node[node.m_gid];
			n.LocalPosition() = node.r;
		}
	}
}

//-----------------------------------------------------------------------------
void GSurfaceMeshObject::UpdateEdges()
{
	// get the mesh
	FESurfaceMesh& m = *GetSurfaceMesh();
	int NE = m.Edges();

	int ng = m.CountEdgePartitions();
	for (int i = 0; i<NE; ++i)

	// create the Edge
	if (ng < (int)m_Edge.size()) ResizeCurves(ng);
	else if (ng >(int) m_Edge.size())
	{
		for (int i = (int)m_Edge.size(); i<ng; ++i)
		{
			GEdge* e = new GEdge(this);
			e->m_node[0] = -1;
			e->m_node[1] = -1;
			AddEdge(e);
		}
	}

	// reset edge nodes
	for (int i = 0; i<ng; ++i)
	{
		GEdge& e = *m_Edge[i];
		e.m_node[0] = -1;
		e.m_node[1] = -1;
	}

	// set the nodes for the GEdge
	const int NN = Nodes();
	for (int i = 0; i<NE; ++i)
	{
		FEEdge& e = m.Edge(i);
		if (e.m_gid >= 0)
		{
			for (int j = 0; j<2; ++j)
			{
				if (e.m_nbr[j] == -1)
				{
					GEdge& ge = *m_Edge[e.m_gid];
					if (ge.m_node[0] == -1)
					{
						FENode& nj = m.Node(e.n[j]);
						if ((nj.m_gid >= 0) && (nj.m_gid < NN))
						{
							ge.m_node[0] = m_Node[nj.m_gid]->GetLocalID();
						}
						else assert(false);
						
					}
					else if (ge.m_node[1] == -1)
					{
						FENode& nj = m.Node(e.n[j]);
						if ((nj.m_gid >= 0) && (nj.m_gid < NN))
						{
							ge.m_node[1] = m_Node[nj.m_gid]->GetLocalID();
						}
						else assert(false);						
					}
				}
			}
		}
	}
}

void GSurfaceMeshObject::UpdateSurfaces()
{
	FESurfaceMesh& m = *GetSurfaceMesh();
	int NF = m.Faces();

	int ng = m.CountFacePartitions();
	if (ng > Faces())
	{
		for (int i=Faces(); i<ng; ++i)
		{
			GFace* face = new GFace(this);
			face->m_nPID[0] = 0;
			AddFace(face);
		}
	}
	else if (ng < Faces()) ResizeSurfaces(ng);
}


void GSurfaceMeshObject::BuildGMesh()
{
	// allocate new GL mesh
	GLMesh* gmesh = new GLMesh();

	// we'll extract the data from the FE mesh
	FESurfaceMesh* pm = m_surfmesh;

	// create nodes
	for (int i = 0; i<pm->Nodes(); ++i)
	{
		FENode& node = pm->Node(i);
		gmesh->AddNode(node.r, node.m_gid);
	}

	// create edges
	for (int i = 0; i<pm->Edges(); ++i)
	{
		FEEdge& es = pm->Edge(i);
		if (es.m_gid >= 0)
			gmesh->AddEdge(es.n, es.Nodes(), es.m_gid);
	}

	// create face data
	for (int i = 0; i<pm->Faces(); ++i)
	{
		FEFace& fs = pm->Face(i);
		gmesh->AddFace(fs.n, fs.Nodes(), fs.m_gid, fs.m_sid);
	}

	gmesh->Update();
	SetRenderMesh(gmesh);
}

FESurfaceMesh* GSurfaceMeshObject::GetSurfaceMesh()
{
	return m_surfmesh;
}

const FESurfaceMesh* GSurfaceMeshObject::GetSurfaceMesh() const
{
	return m_surfmesh;
}

void GSurfaceMeshObject::ReplaceSurfaceMesh(FESurfaceMesh* newMesh)
{
	m_surfmesh = newMesh;
	m_surfmesh->SetGObject(this);
	Update();
}

// get the mesh of an edge curve
FECurveMesh* GSurfaceMeshObject::GetFECurveMesh(int edgeId)
{
	FESurfaceMesh* mesh = GetSurfaceMesh();
	if (mesh == 0) return 0;

	mesh->TagAllNodes(-1);
	int NC = mesh->Edges();
	int ne = 0;
	for (int i = 0; i<NC; ++i)
	{
		FEEdge& e = mesh->Edge(i);
		if (e.m_gid == edgeId)
		{
			mesh->Node(e.n[0]).m_ntag = 0;
			mesh->Node(e.n[1]).m_ntag = 0;
			ne++;
		}
	}

	FECurveMesh* curve = new FECurveMesh;

	int NN = mesh->Nodes();
	int nn = 0;
	for (int i = 0; i<NN; ++i)
	{
		FENode& node = mesh->Node(i);
		if (node.m_ntag != -1)
		{
			node.m_ntag = nn++;
			vec3d r = Transform().LocalToGlobal(node.r);
			curve->AddNode(r, false);
		}
	}

	for (int i = 0; i<NC; ++i)
	{
		FEEdge& sedge = mesh->Edge(i);
		if (sedge.m_gid == edgeId)
		{
			int n0 = mesh->Node(sedge.n[0]).m_ntag;
			int n1 = mesh->Node(sedge.n[1]).m_ntag;
			curve->AddEdge(n0, n1);
		}
	}

	curve->Update();

	return curve;
}

// serialization
void GSurfaceMeshObject::Save(OArchive& ar)
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
		for (int i = 0; i<Parts(); ++i)
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
		for (int i = 0; i<Faces(); ++i)
		{
			ar.BeginChunk(CID_OBJ_FACE);
			{
				GFace& f = *Face(i);
				int nid = f.GetID();
				ar.WriteChunk(CID_OBJ_FACE_ID, nid);
				ar.WriteChunk(CID_OBJ_FACE_TYPE, f.m_ntype);
				ar.WriteChunk(CID_OBJ_FACE_NAME, f.GetName());
				ar.WriteChunk(CID_OBJ_FACE_PID0, f.m_nPID[0]);
				ar.WriteChunk(CID_OBJ_FACE_PID1, f.m_nPID[1]);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the edges
	ar.BeginChunk(CID_OBJ_EDGE_SECTION);
	{
		for (int i = 0; i<Edges(); ++i)
		{
			ar.BeginChunk(CID_OBJ_EDGE);
			{
				GEdge& e = *Edge(i);
				int nid = e.GetID();
				ar.WriteChunk(CID_OBJ_EDGE_ID, nid);
				ar.WriteChunk(CID_OBJ_EDGE_NAME, e.GetName());
				ar.WriteChunk(CID_OBJ_EDGE_TYPE, e.Type());
				ar.WriteChunk(CID_OBJ_EDGE_NODE0, e.m_node[0]);
				ar.WriteChunk(CID_OBJ_EDGE_NODE1, e.m_node[1]);
				ar.WriteChunk(CID_OBJ_EDGE_NODE2, e.m_cnode);
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
			for (int i = 0; i<Nodes(); ++i)
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
	if (GetFEMesh())
	{
		ar.BeginChunk(CID_MESH);
		{
			GetFEMesh()->Save(ar);
		}
		ar.EndChunk();
	}

	// save the surface mesh
	if (m_surfmesh)
	{
		ar.BeginChunk(CID_SURFACE_MESH);
		{
			m_surfmesh->Save(ar);
		}
		ar.EndChunk();
	}
}

void GSurfaceMeshObject::Load(IArchive& ar)
{
	TRACE("GSurfaceMeshObject::Load");

	int nparts = -1, nfaces = -1, nedges = -1, nnodes = -1;

	while (IArchive::IO_OK == ar.OpenChunk())
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
		// object info
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
			while (IArchive::IO_OK == ar.OpenChunk())
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
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				if (ar.GetChunkID() != CID_OBJ_PART) throw ReadError("error parsing CID_OBJ_PART_SECTION");

				GPart* p = new GPart(this);
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid, mid;
					switch (ar.GetChunkID())
					{
					case CID_OBJ_PART_ID: ar.read(nid); p->SetID(nid); break;
					case CID_OBJ_PART_MAT: ar.read(mid); p->SetMaterialID(mid); break;
					case CID_OBJ_PART_NAME:
					{
						char szname[256] = { 0 };
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
			assert((int)m_Part.size() == nparts);
		}
		break;
		// object surfaces
		case CID_OBJ_FACE_SECTION:
		{
			assert(nfaces > 0);
			m_Face.reserve(nfaces);
			int n = 0;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				if (ar.GetChunkID() != CID_OBJ_FACE) throw ReadError("error parsing CID_OBJ_FACE_SECTION");

				GFace* f = new GFace(this);
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid;
					switch (ar.GetChunkID())
					{
					case CID_OBJ_FACE_ID: ar.read(nid); f->SetID(nid); break;
					case CID_OBJ_FACE_TYPE: ar.read(f->m_ntype); break;
					case CID_OBJ_FACE_NAME:
					{
						char szname[256] = { 0 };
						ar.read(szname);
						f->SetName(szname);
					}
					break;
					case CID_OBJ_FACE_PID0: ar.read(f->m_nPID[0]); break;
					case CID_OBJ_FACE_PID1: ar.read(f->m_nPID[1]); break;
					}
					ar.CloseChunk();
				}
				ar.CloseChunk();

				f->SetLocalID(n++);

				m_Face.push_back(f);
			}
			assert((int)m_Face.size() == nfaces);
		}
		break;
		// object edges
		case CID_OBJ_EDGE_SECTION:
		{
			m_Edge.clear();
			if (nedges > 0) m_Edge.reserve(nedges);
			int n = 0;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				if (ar.GetChunkID() != CID_OBJ_EDGE) throw ReadError("error parsing CID_OBJ_EDGE_SECTION");

				GEdge* e = new GEdge(this);
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid;
					switch (ar.GetChunkID())
					{
					case CID_OBJ_EDGE_ID: ar.read(nid); e->SetID(nid); break;
					case CID_OBJ_EDGE_TYPE: ar.read(e->m_ntype); break;
					case CID_OBJ_EDGE_NODE0: ar.read(e->m_node[0]); break;
					case CID_OBJ_EDGE_NODE1: ar.read(e->m_node[1]); break;
					case CID_OBJ_EDGE_NODE2: ar.read(e->m_cnode); break;
					case CID_OBJ_EDGE_NAME:
					{
						char szname[256] = { 0 };
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
			assert((int)m_Edge.size() == nedges);
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
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_NODE) throw ReadError("error parsing CID_OBJ_NODE_SECTION");

					GNode* n = new GNode(this);
					while (IArchive::IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_NODE_ID: ar.read(nid); n->SetID(nid); break;
						case CID_OBJ_NODE_POS: ar.read(n->LocalPosition()); break;
						case CID_OBJ_NODE_NAME:
						{
							char szname[256] = { 0 };
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
				assert((int)m_Node.size() == nnodes);
			}
		}
		break;
		// the mesh object
		case CID_MESH:
			if (GetFEMesh()) delete GetFEMesh();
			SetFEMesh(new FEMesh);
			GetFEMesh()->Load(ar);
			break;
		case CID_SURFACE_MESH:
			if (m_surfmesh) delete m_surfmesh;
			m_surfmesh = new FESurfaceMesh;
			m_surfmesh->SetGObject(this);
			m_surfmesh->Load(ar);
			break;
		}
		ar.CloseChunk();
	}

	BuildGMesh();
}

void GSurfaceMeshObject::Attach(const GSurfaceMeshObject* po, bool weld, double weldTolerance)
{
	// Add the nodes
	int NN0 = Nodes();
	int NN = po->Nodes();
	for (int i = 0; i<NN; ++i)
	{
		GNode* n = new GNode(this);
		const GNode& no = *po->Node(i);
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
	for (int i = 0; i<NE; ++i)
	{
		GEdge* e = new GEdge(this);
		const GEdge& eo = *po->Edge(i);
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
	for (int i = 0; i<NP; ++i)
	{
		GPart* g = new GPart(this);
		const GPart& go = *po->Part(i);
		g->SetMaterialID(go.GetMaterialID());
		g->SetID(go.GetID());
		g->SetLocalID(i + NP0);
		g->SetName(go.GetName());
		m_Part.push_back(g);
	}

	// --- Faces ---
	int NF0 = Faces();
	int NF = po->Faces();
	for (int i = 0; i<NF; ++i)
	{
		GFace* f = new GFace(this);
		const GFace& fo = *po->Face(i);

		f->m_node = fo.m_node;
		for (int j = 0; j<(int)fo.m_node.size(); ++j)
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
	const FESurfaceMesh* oldMesh = po->GetSurfaceMesh();
	FESurfaceMesh* newMesh = GetSurfaceMesh();
	if (weld)
	{
		newMesh->AttachAndWeld(*oldMesh, weldTolerance);
		Update();
	}
	else
	{
		newMesh->Attach(*oldMesh);
	}

	newMesh->UpdateBox();
	newMesh->UpdateNormals();

	BuildGMesh();
}
