#include "FEEdge.h"
#include <assert.h>

//-----------------------------------------------------------------------------
FEEdge::FEEdge()
{
	m_elem = -1;
	n[0] = n[1] = n[2] = n[3] = -1;
	m_nbr[0] = m_nbr[1] = -1;
	m_face[0] = m_face[1] = -1;
	m_type = FE_EDGE_INVALID;
	m_gid = -1;
}

//-----------------------------------------------------------------------------
FEEdge::FEEdge(const FEEdge& e) : FEItem(e)
{
	m_type = e.m_type;
	m_elem = e.m_elem;
	n[0] = e.n[0];
	n[1] = e.n[1];
	n[2] = e.n[2];
	n[3] = e.n[3];
	m_nbr[0] = e.m_nbr[0];
	m_nbr[1] = e.m_nbr[1];
	m_face[0] = e.m_face[0];
	m_face[1] = e.m_face[1];
}

//-----------------------------------------------------------------------------
void FEEdge::operator = (const FEEdge& e)
{
	m_type = e.m_type;
	m_elem = e.m_elem;
	n[0] = e.n[0];
	n[1] = e.n[1];
	n[2] = e.n[2];
	n[3] = e.n[3];
	m_nbr[0] = e.m_nbr[0];
	m_nbr[1] = e.m_nbr[1];
	m_face[0] = e.m_face[0];
	m_face[1] = e.m_face[1];
	FEItem::operator=(e);
}

//-----------------------------------------------------------------------------
// Tests equality between edges
bool FEEdge::operator == (const FEEdge& e) const
{
	if (e.m_type != m_type) return false;
	assert(m_type != FE_EDGE_INVALID);
	if ((n[0] != e.n[0]) && (n[0] != e.n[1])) return false;
	if ((n[1] != e.n[0]) && (n[1] != e.n[1])) return false;

	if (m_type == FE_EDGE3) 
	{
		if (n[2] != e.n[2]) return false;
	}
	else if (m_type == FE_EDGE4)
	{
		if ((n[2] != e.n[2]) && (n[2] != e.n[3])) return false;
		if ((n[3] != e.n[2]) && (n[3] != e.n[3])) return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Returns the local index for a given node number.
int FEEdge::FindNodeIndex(int node) const
{
	assert(m_type != FE_EDGE_INVALID);
	if (node == n[0]) return 0;
	if (node == n[1]) return 1;
	if (node == n[2]) return 2;
	if (node == n[3]) return 3;
	return -1;
}

//-----------------------------------------------------------------------------
void FEEdge::SetType(FEEdgeType type)
{ 
	assert(m_type == FE_EDGE_INVALID);
	assert(type != FE_EDGE_INVALID);
	m_type = type;
}

//-----------------------------------------------------------------------------
int FEEdge::Nodes() const
{ 
	static int nodeCount[] = {2, 3, 4, 0};
	assert(m_type != FE_EDGE_INVALID);
	return nodeCount[m_type];
}

//-----------------------------------------------------------------------------
//! Evaluate the shape function values at the iso-parametric point r = [0,1]
void FEEdge::shape(double* H, double r)
{
	switch (m_type)
	{
	case FE_EDGE2:
		H[0] = 1.0 - r;
		H[1] = r;
		break;
	case FE_EDGE3:
		H[0] = (1 - r)*(2 * (1 - r) - 1);
		H[1] = r*(2 * r - 1);
		H[2] = 4 * (1 - r)*r;
		break;
	case FE_EDGE4:
		H[0] = 0.5f*(1.f - r)*(3.f*r - 1.f)*(3.f*r - 2.f);
		H[1] = 0.5f*r*(3.f*r - 1.f)*(3.f*r - 2.f);
		H[2] = 9.f / 2.f*r*(r - 1.f)*(3.f*r - 2.f);
		H[3] = 9.f / 2.f*r*(1.f - r)*(3.f*r - 1.f);
		break;
	default:
		assert(false);
	}
}

//-----------------------------------------------------------------------------
double FEEdge::eval(double* d, double r)
{
	double H[FEEdge::MAX_NODES];
	shape(H, r);
	double a = 0.0;
	for (int i = 0; i<Nodes(); ++i) a += H[i] * d[i];
	return a;
}

//-----------------------------------------------------------------------------
vec3f FEEdge::eval(vec3f* d, double r)
{
	double H[FEEdge::MAX_NODES];
	shape(H, r);
	vec3f a(0, 0, 0);
	for (int i = 0; i<Nodes(); ++i) a += d[i] * ((float)H[i]);
	return a;
}
