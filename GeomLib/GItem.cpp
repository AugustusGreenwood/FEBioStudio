#include "GItem.h"
#include "GBaseObject.h"
#include "geom.h"

//-----------------------------------------------------------------------------
// initialize static variables
template <> int GItem_T<GPart  >::m_ncount = 0;
template <> int GItem_T<GFace  >::m_ncount = 0;
template <> int GItem_T<GEdge  >::m_ncount = 0;
template <> int GItem_T<GNode  >::m_ncount = 0;


//=============================================================================
// GNode
//-----------------------------------------------------------------------------
GNode::GNode(const GNode &n)
{
	m_r = n.m_r;

	m_state = n.m_state;
	m_gid = n.m_gid;
	m_lid = n.m_lid;
	m_ntype = n.m_ntype;
	m_ntag = n.m_ntag;
	m_po = n.m_po;
	SetName(n.GetName());
}

//-----------------------------------------------------------------------------
void GNode::operator =(const GNode &n)
{
	m_r = n.m_r;

	m_state = n.m_state;
	m_gid = n.m_gid;
	m_lid = n.m_lid;
	m_ntype = n.m_ntype;
	m_ntag = n.m_ntag;
//	m_po = n.m_po;
	SetName(n.GetName());
}

//-----------------------------------------------------------------------------
vec3d GNode::Position() const
{
	const GBaseObject* po = Object(); assert(po);
	if (po)
	{
		return po->GetTransform().LocalToGlobal(m_r);
	}
	else return m_r;
}

//=============================================================================
// GEdge
//-----------------------------------------------------------------------------
GEdge::GEdge(const GEdge& e)
{
	m_node[0] = e.m_node[0];
	m_node[1] = e.m_node[1];
	m_cnode = e.m_cnode;
	m_ntype = e.m_ntype;

	m_state = e.m_state;
	m_gid = e.m_gid;
	m_lid = e.m_lid;
	m_po = e.m_po;
	SetName(e.GetName());
}

//-----------------------------------------------------------------------------
void GEdge::operator =(const GEdge &e)
{
	m_node[0] = e.m_node[0];
	m_node[1] = e.m_node[1];
	m_cnode = e.m_cnode;
	m_ntype = e.m_ntype;

	m_state = e.m_state;
	m_gid = e.m_gid;
	m_lid = e.m_lid;
//	m_po = e.m_po;
	SetName(e.GetName());
}

//-----------------------------------------------------------------------------
bool GEdge::operator==(const GEdge& e)
{
	if (e.m_ntype != m_ntype) return false;
	switch (m_ntype)
	{
	case EDGE_LINE:
	case EDGE_YARC:
	case EDGE_ZARC:
		if ((m_node[0] != e.m_node[0])&&(m_node[0] != e.m_node[1])) return false;
		if ((m_node[1] != e.m_node[0])&&(m_node[1] != e.m_node[1])) return false;
		break;
	case EDGE_3P_CIRC_ARC:
		if ((m_node[0] != e.m_node[0])&&(m_node[0] != e.m_node[1])) return false;
		if ((m_node[1] != e.m_node[0])&&(m_node[1] != e.m_node[1])) return false;
		if (m_cnode != e.m_cnode) return false;
		break;
	case EDGE_3P_ARC:
		if ((m_node[0] != e.m_node[0])&&(m_node[0] != e.m_node[1])) return false;
		if ((m_node[1] != e.m_node[0])&&(m_node[1] != e.m_node[1])) return false;
		if (m_cnode != e.m_cnode) return false;
		break;
	default:
//		assert(false);
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
double GEdge::Length()
{
	double L = 0;
	switch (m_ntype)
	{
	case EDGE_UNKNOWN: assert(false); L = 0; break;
	case EDGE_LINE:
		{
			vec3d r0 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y);
			vec2d b(r1.x, r1.y);

			GM_LINE l(a, b);

			L = l.Length();
		}
		break;
	case EDGE_3P_CIRC_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode)->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y); 
			vec2d b(r1.x, r1.y); 
			vec2d c(r2.x, r2.y); 

			GM_CIRCLE_ARC ca(a, b, c);
			L = ca.Length();
		}
		break;
	case EDGE_3P_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode)->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y); 
			vec2d b(r1.x, r1.y); 
			vec2d c(r2.x, r2.y); 

			GM_ARC ca(a, b, c);
			L = ca.Length();
		}
		break;
	default:
		assert(false);
	}
	return L;
}

//-----------------------------------------------------------------------------
vec3d GEdge::Point(double l)
{
	assert((l>=0)&&(l<=1.0));
	if (l<0.0) l = 0.0;
	if (l>1.0) l = 1.0;
	vec3d p;
	switch (m_ntype)
	{
	case EDGE_LINE:
		{
			vec3d r0 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[1])->LocalPosition();
			p = r0*(1.0 - l) + r1*l;
		}
		break;
	case EDGE_3P_CIRC_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode)->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition() - r0;
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition() - r0;
			vec3d n = r1^r2; n.Normalize();
			quatd q(n, vec3d(0,0,1)), qi = q.Inverse();
			q.RotateVector(r1);
			q.RotateVector(r2);
			GM_CIRCLE_ARC c(vec2d(0,0), vec2d(r1.x, r1.y), vec2d(r2.x, r2.y));
			vec2d a = c.Point(l);
			p.x = a.x;
			p.y = a.y;
			p.z = 0;
			qi.RotateVector(p);
			p += r0;
		};
		break;
	case EDGE_3P_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode)->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition() - r0;
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition() - r0;
			vec3d n = r1^r2; n.Normalize();
			quatd q(n, vec3d(0,0,1)), qi = q.Inverse();
			q.RotateVector(r1);
			q.RotateVector(r2);
			GM_ARC c(vec2d(0,0), vec2d(r1.x, r1.y), vec2d(r2.x, r2.y));
			vec2d a = c.Point(l);
			p.x = a.x;
			p.y = a.y;
			p.z = 0;
			qi.RotateVector(p);
			p += r0;
		};
		break;
	default:
		assert(false);
	}
	return p;
}

//-----------------------------------------------------------------------------
vec2d GEdge::Tangent(double l)
{
	vec2d t;
	switch (m_ntype)
	{
	case EDGE_UNKNOWN: assert(false); break;
	case EDGE_LINE:
		{
			vec3d r0 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r1 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y);
			vec2d b(r1.x, r1.y);

			GM_LINE line(a, b);

			t = line.Tangent(l);
		}
		break;
	case EDGE_3P_CIRC_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode)->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y); 
			vec2d b(r1.x, r1.y); 
			vec2d c(r2.x, r2.y); 

			GM_CIRCLE_ARC ca(a, b, c);

			t = ca.Tangent(l);
		}
		break;
	case EDGE_3P_ARC:
		{
			vec3d r0 = m_po->Node(m_cnode)->LocalPosition();
			vec3d r1 = m_po->Node(m_node[0])->LocalPosition();
			vec3d r2 = m_po->Node(m_node[1])->LocalPosition();

			vec2d a(r0.x, r0.y); 
			vec2d b(r1.x, r1.y); 
			vec2d c(r2.x, r2.y); 

			GM_ARC ca(a, b, c);

			t = ca.Tangent(l);
		}
		break;
	default:
		assert(false);
	}

	return t;
}

//=============================================================================
// GFace
//-----------------------------------------------------------------------------
GFace::GFace(const GFace& f) 
{
	m_ntype = f.m_ntype;
	m_nPID[0] = f.m_nPID[0];
	m_nPID[1] = f.m_nPID[1];
	m_node = f.m_node;
	m_edge = f.m_edge;

	m_state = f.m_state;
	m_gid = f.m_gid;
	m_lid = f.m_lid;
	m_po = f.m_po;
	SetName(f.GetName());
}

//-----------------------------------------------------------------------------
void GFace::operator = (const GFace& f)
{
	m_ntype = f.m_ntype;
	m_nPID[0] = f.m_nPID[0];
	m_nPID[1] = f.m_nPID[1];
	m_node = f.m_node;
	m_edge = f.m_edge;

	m_state = f.m_state;
	m_gid = f.m_gid;
	m_lid = f.m_lid;
	SetName(f.GetName());
//	m_po = f.m_po;
}

//-----------------------------------------------------------------------------
bool GFace::HasEdge(int nid)
{
	for (int i=0; i<(int) m_edge.size(); ++i)
	{
		if (m_edge[i].nid == nid) return true;
	}
	return false;
}

//=============================================================================
// GPart
//-----------------------------------------------------------------------------
GPart::GPart(const GPart& p)
{
	m_matid = p.m_matid;

	m_state = p.m_state;
	m_gid = p.m_gid;
	m_lid = p.m_lid;
	m_po = p.m_po;
	SetName(p.GetName());
}

//-----------------------------------------------------------------------------
void GPart::operator =(const GPart &p)
{
	m_matid = p.m_matid;

	m_state = p.m_state;
	m_gid = p.m_gid;
	m_lid = p.m_lid;
	SetName(p.GetName());
//	m_po = p.m_po;
}
