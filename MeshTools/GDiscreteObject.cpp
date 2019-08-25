#include "stdafx.h"
#include "GDiscreteObject.h"
#include <FSCore/Archive.h>

GDiscreteObject::GDiscreteObject(int ntype)
{
	m_ntype = ntype;
	m_state = 0;
	m_col = GLColor(0, 255, 0);
}

GDiscreteObject::~GDiscreteObject(void)
{
}

GLColor GDiscreteObject::GetColor() const { return m_col; }
void GDiscreteObject::SetColor(const GLColor& c) { m_col = c; }

//-----------------------------------------------------------------------------

GLinearSpring::GLinearSpring() : GDiscreteObject(FE_DISCRETE_SPRING)
{
	m_node[0] = m_node[1] = -1;
	AddDoubleParam(0, "E", "spring constant");
}

GLinearSpring::GLinearSpring(int n1, int n2) : GDiscreteObject(FE_DISCRETE_SPRING)
{
	m_node[0] = n1;
	m_node[1] = n2;
	AddDoubleParam(0, "E", "spring constant");
}

void GLinearSpring::Save(OArchive& ar)
{
	double E = GetFloatValue(MP_E);
	ar.WriteChunk(0, GetName());
	ar.WriteChunk(1, m_node[0]);
	ar.WriteChunk(2, m_node[1]);
	ar.WriteChunk(3, E);
}

void GLinearSpring::Load(IArchive& ar)
{
	TRACE("GLinearSpring::Load");

	char sz[256] = {0};
	double E;

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();

		switch (nid)
		{
		case 0: ar.read(sz); SetName(sz); break;
		case 1: ar.read(m_node[0]); break;
		case 2: ar.read(m_node[1]); break;
		case 3: ar.read(E); SetFloatValue(MP_E, E); break;
		}

		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------

GGeneralSpring::GGeneralSpring() : GDiscreteObject(FE_GENERAL_SPRING)
{
	m_node[0] = m_node[1] = -1;
	AddDoubleParam(1, "force", "spring force")->SetLoadCurve();

	// create an initial linear ramp
	LOADPOINT p0(0,0), p1(1,1);
	GetParamLC(MP_F)->Add(p0);
	GetParamLC(MP_F)->Add(p1);
}

GGeneralSpring::GGeneralSpring(int n1, int n2) : GDiscreteObject(FE_GENERAL_SPRING)
{
	m_node[0] = n1;
	m_node[1] = n2;
	AddDoubleParam(1, "force", "spring force")->SetLoadCurve();

	// create an initial linear ramp
	LOADPOINT p0(0,0), p1(1,1);
	GetParamLC(MP_F)->Add(p0);
	GetParamLC(MP_F)->Add(p1);
}

void GGeneralSpring::Save(OArchive& ar)
{
	ar.WriteChunk(0, GetName());
	ar.WriteChunk(1, m_node[0]);
	ar.WriteChunk(2, m_node[1]);
	ar.BeginChunk(3);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
}

void GGeneralSpring::Load(IArchive& ar)
{
	TRACE("GGeneralSpring::Load");

	char sz[256] = {0};

	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();

		switch (nid)
		{
		case 0: ar.read(sz); SetName(sz); break;
		case 1: ar.read(m_node[0]); break;
		case 2: ar.read(m_node[1]); break;
		case 3: ParamContainer::Load(ar); break;
		}
		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
int GDiscreteElement::m_ncount = 1;

GDiscreteElement::GDiscreteElement() : GDiscreteObject(FE_DISCRETE_ELEMENT)
{ 
	m_node[0] = m_node[1] = -1; m_state = 0; 
	m_nid = m_ncount++;
}

GDiscreteElement::GDiscreteElement(int n0, int n1) : GDiscreteObject(FE_DISCRETE_ELEMENT)
{ 
	m_node[0] = n0; 
	m_node[1] = n1; 
	m_state = 0; 
	m_nid = m_ncount++;
}

GDiscreteElement::GDiscreteElement(const GDiscreteElement& el) : GDiscreteObject(FE_DISCRETE_ELEMENT)
{
	m_node[0] = el.m_node[0]; 
	m_node[1] = el.m_node[1]; 
	m_state = el.m_state; 
	m_nid = el.m_nid;
}

void GDiscreteElement::operator = (const GDiscreteElement& el)
{
	m_node[0] = el.m_node[0]; 
	m_node[1] = el.m_node[1]; 
	m_state = el.m_state; 
	m_nid = el.m_nid;
}

void GDiscreteElement::SetNodes(int n0, int n1)
{
	m_node[0] = n0;
	m_node[1] = n1;
}

//=================================================================================================

GDiscreteElementSet::GDiscreteElementSet(int ntype) : GDiscreteObject(ntype) 
{
}

//-----------------------------------------------------------------------------
GDiscreteElementSet::~GDiscreteElementSet()
{
	for (size_t i=0; i<m_elem.size(); ++i) delete m_elem[i];
	m_elem.clear();
}

//-----------------------------------------------------------------------------
int GDiscreteElementSet::size() const 
{ 
	return (int)m_elem.size(); 
}

//-----------------------------------------------------------------------------
GDiscreteElement& GDiscreteElementSet::element(int i)
{ 
	return *m_elem[i]; 
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::AddElement(int n0, int n1)
{
	// create new discrete element
	GDiscreteElement* el = new GDiscreteElement(n0, n1);

	// set default name
	char szbuf[32] = {0};
	sprintf(szbuf, "spring%d", (int)m_elem.size()+1);
	el->SetName(szbuf);

	// add it to the pile
	m_elem.push_back(el);
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::AddElement(const GDiscreteElement& el)
{
	GDiscreteElement* newElem = new GDiscreteElement(el);
	for (int i=0; i<size(); ++i) 
		if (newElem->GetID() < m_elem[i]->GetID())
		{
			m_elem.insert(m_elem.begin()+i, newElem);
			return;
		}
	m_elem.push_back(newElem);
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::UnSelect()
{
	for (int i=0; i<(int) m_elem.size(); ++i) m_elem[i]->UnSelect();
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::Select()
{
	for (int i=0; i<(int) m_elem.size(); ++i) m_elem[i]->Select();
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::SelectComponent(int n)
{
	m_elem[n]->Select();
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::UnselectComponent(int n)
{
	m_elem[n]->UnSelect();
}

//-----------------------------------------------------------------------------
int GDiscreteElementSet::FindElement(const GDiscreteElement& el) const
{
	for (int i=0; i<(int)m_elem.size(); ++i)
	{
		const GDiscreteElement& eli = *m_elem[i];
		if ((eli.Node(0) == el.Node(0))&&
			(eli.Node(1) == el.Node(1))) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
int GDiscreteElementSet::FindElement(int id) const
{
	for (int i=0; i<(int)m_elem.size(); ++i)
	{
		const GDiscreteElement& eli = *m_elem[i];
		if (eli.GetID() == id) return i;
	}
	return -1;
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::RemoveElement(int index)
{
	m_elem.erase(m_elem.begin() + index);
}

//-----------------------------------------------------------------------------
void GDiscreteElementSet::Save(OArchive& ar)
{
	int N = m_elem.size();
	for (int i=0; i<N; ++i)
	{
		ar.BeginChunk(0);
		{
			GDiscreteElement& ei = *m_elem[i];
			int n0 = ei.Node(0);
			int n1 = ei.Node(1);
			ar.WriteChunk(0, n0);
			ar.WriteChunk(1, n1);
		}
		ar.EndChunk();
	}
}

void GDiscreteElementSet::Load(IArchive& ar)
{
	TRACE("GSpringSet::Load");
	
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		if (nid == 0)
		{
			int n0, n1;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				int nid = ar.GetChunkID();
				if      (nid == 0) ar.read(n0);
				else if (nid == 1) ar.read(n1);
				ar.CloseChunk();
			}
			AddElement(n0, n1);
		}
		ar.CloseChunk();
	}
}

//-----------------------------------------------------------------------------
GLinearSpringSet::GLinearSpringSet() : GDiscreteElementSet(FE_LINEAR_SPRING_SET)
{
	AddDoubleParam(1, "E", "spring constant");
}

void GLinearSpringSet::SetSpringConstant(double E)
{
	SetFloatValue(MP_E, E);
}

void GLinearSpringSet::Save(OArchive& ar)
{
	ar.WriteChunk(0, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());
	if (m_elem.size() > 0)
	{
		ar.BeginChunk(1);
		{
			GDiscreteElementSet::Save(ar);
		}
		ar.EndChunk();
	}
	ar.BeginChunk(2);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
	ar.WriteChunk(3, GetColor());
}

void GLinearSpringSet::Load(IArchive& ar)
{
	TRACE("GSpringSet::Load");

	string s;
	GLColor col = GetColor();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case 0: ar.read(s); SetName(s); break;
		case 1: GDiscreteElementSet::Load(ar); break;
		case 2: ParamContainer::Load(ar); break;
		case 3: ar.read(col); break;
		case CID_FEOBJ_INFO: ar.read(s); SetInfo(s); break;
		}
		ar.CloseChunk();
	}

	SetColor(col);
}

//-----------------------------------------------------------------------------
GNonlinearSpringSet::GNonlinearSpringSet() : GDiscreteElementSet(FE_NONLINEAR_SPRING_SET)
{
	AddDoubleParam(1, "force", "spring force")->SetLoadCurve();

	// create an initial linear ramp
	LOADPOINT p0(0,0), p1(1,1);
	GetParamLC(MP_F)->Add(p0);
	GetParamLC(MP_F)->Add(p1);
}

void GNonlinearSpringSet::Save(OArchive& ar)
{
	ar.WriteChunk(0, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());
	if (m_elem.size() > 0)
	{
		ar.BeginChunk(1);
		{
			GDiscreteElementSet::Save(ar);
		}
		ar.EndChunk();
	}
	ar.BeginChunk(2);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
	ar.WriteChunk(3, GetColor());
}

void GNonlinearSpringSet::Load(IArchive& ar)
{
	TRACE("GNonlinearSpringSet::Load");

	string s;
	GLColor col = GetColor();
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case 0: ar.read(s); SetName(s); break;
		case 1: GDiscreteElementSet::Load(ar); break;
		case 2: ParamContainer::Load(ar); break;
		case 3: ar.read(col); break;
		case CID_FEOBJ_INFO: ar.read(s); SetInfo(s); break;
		}
		ar.CloseChunk();
	}

	SetColor(col);
}

//-----------------------------------------------------------------------------
GDeformableSpring::GDeformableSpring() : GDiscreteObject(FE_DEFORMABLE_SPRING)
{
	AddDoubleParam(1, "E", "spring constant");
	AddIntParam(1, "divs", "Divisions");

	m_node[0] = m_node[1] = -1;
}

//-----------------------------------------------------------------------------
GDeformableSpring::GDeformableSpring(int n0, int n1) : GDiscreteObject(FE_DEFORMABLE_SPRING)
{
	AddDoubleParam(1, "E", "spring constant");
	AddIntParam(1, "divs", "Divisions")->SetState(Param_EDITABLE);

	m_node[0] = n0;
	m_node[1] = n1;
}
