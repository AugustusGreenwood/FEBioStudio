#include "GMultiBox.h"
#include <MeshTools/FEMultiBlockMesh.h>
#include "GPrimitive.h"

//-----------------------------------------------------------------------------
//! Constructor
GMultiBox::GMultiBox() : GObject(GMULTI_BLOCK)
{
	m_pMesher = new FEMultiBlockMesh;
}

//-----------------------------------------------------------------------------
// This function creates a new GMultiBox from an existing GObject
// The new GMultiBox will have the same ID as the existing GObject
// This is used in the CCmdConvertObject command that converts a GPrimitve
// a GMultiBox
GMultiBox::GMultiBox(GObject *po) : GObject(GMULTI_BLOCK)
{
	// This only works with boxes for now
	GBox* pb = dynamic_cast<GBox*>(po);
	assert(pb);

	// define the mesher
	m_pMesher = new FEMultiBlockMesh;

	// copy to old object's ID
	SetID(po->GetID());

	// creating a new object has increased the object counter
	// so we need to decrease it again
	GItem_T<GBaseObject>::DecreaseCounter();

	int i;

	// next, we copy the geometry info
	// --- Nodes ---
	int NN = po->Nodes();
	m_Node.reserve(NN);
	for (i=0; i<NN; ++i)
	{
		GNode* n = new GNode(this);
		GNode& no = *po->Node(i);
		n->LocalPosition() = no.LocalPosition();
		n->SetID(no.GetID());
		n->SetLocalID(i);
		assert(n->GetLocalID() == no.GetLocalID());
		m_Node.push_back(n);
	}

	// --- Edges ---
	int NE = po->Edges();
	m_Edge.reserve(NE);
	for (i=0; i<NE; ++i)
	{
		GEdge* e = new GEdge(this);
		GEdge& eo = *po->Edge(i);
		e->m_node[0] = eo.m_node[0];
		e->m_node[1] = eo.m_node[1];
		e->SetID(eo.GetID());
		e->SetLocalID(i);
		assert(e->GetLocalID() == eo.GetLocalID());
		m_Edge.push_back(e);
	}

	// --- Faces ---
	int NF = po->Faces();
	m_Face.reserve(NF);
	for (i=0; i<NF; ++i)
	{
		GFace* f = new GFace(this);
		GFace& fo = *po->Face(i);
		f->m_node = fo.m_node;
		f->m_edge = fo.m_edge;
		f->m_nPID[0] = fo.m_nPID[0];
		f->m_nPID[1] = fo.m_nPID[1];
		f->SetID(fo.GetID());
		f->SetLocalID(i);
		assert(f->GetLocalID() == fo.GetLocalID());
		f->m_ntype = FACE_QUAD;
		m_Face.push_back(f);
	}

	// --- Parts ---
	int NP = po->Parts();
	m_Part.reserve(NP);
	for (i=0; i<NP; ++i)
	{
		GPart* g = new GPart(this);
		GPart& go = *po->Part(i);
		g->SetMaterialID(go.GetMaterialID());
		g->SetID(go.GetID());
		g->SetLocalID(i);
		assert(g->GetLocalID() == go.GetLocalID());
		m_Part.push_back(g);
	}

	// rebuild the GMesh
	BuildGMesh();
}

//-----------------------------------------------------------------------------
// update geometry
bool GMultiBox::Update(bool b)
{
	return true;
}

//-----------------------------------------------------------------------------
// create the geometry
void GMultiBox::Create()
{
}

//-----------------------------------------------------------------------------
// build the FE mesh
FEMesh* GMultiBox::BuildMesh()
{
	return 0;
}

//-----------------------------------------------------------------------------
// update the FE mesh
void GMultiBox::UpdateFEMesh()
{
}

//-----------------------------------------------------------------------------
// save object data to archive
void GMultiBox::Save(OArchive& ar)
{
}

//-----------------------------------------------------------------------------
// load object from archive
void GMultiBox::Load(IArchive& ar)
{
}
