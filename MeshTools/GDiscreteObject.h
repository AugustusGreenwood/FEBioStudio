#pragma once

#include "FEObject.h"
#include "GeomLib/GItem.h"

#define FE_DISCRETE_OBJECT		0
#define FE_DISCRETE_SPRING		1
#define FE_GENERAL_SPRING		2
#define FE_ARMATURE				3
#define FE_LINEAR_SPRING_SET	4
#define FE_NONLINEAR_SPRING_SET	5
#define FE_DISCRETE_ELEMENT		6
#define FE_DEFORMABLE_SPRING	7

class GDiscreteObject : public FEObject
{
public:
	GDiscreteObject(int ntype);
	~GDiscreteObject(void);

	int GetType() { return m_ntype; }

	// check selection state
	bool IsSelected() const { return ((m_state & GEO_SELECTED) != 0); }
	virtual void Select() { m_state = m_state | GEO_SELECTED; }
	virtual void UnSelect() { m_state = m_state & ~GEO_SELECTED; }

	virtual void SelectComponent(int n) { Select(); }
	virtual void UnselectComponent(int n) { UnSelect(); }

	// get/set state
	unsigned int GetState() const { return m_state; }
	void SetState(unsigned int state) { m_state = state; }

	// check visibility state (only used by GBaseObject's)
	bool IsVisible() const { return ((m_state & GEO_VISIBLE) != 0); }
	void Show() { m_state = m_state | GEO_VISIBLE; }
	void Hide() { m_state = 0; }

	// get/set object color
	GLCOLOR GetColor() const;
	void SetColor(const GLCOLOR& c);

protected:
	int		m_ntype;
	int		m_state;
	GLCOLOR	m_col;
};

//-----------------------------------------------------------------------------

class GLinearSpring : public GDiscreteObject
{
public:
	enum { MP_E };

public:
	GLinearSpring();
	GLinearSpring(int n1, int n2);

	void Save(OArchive& ar);
	void Load(IArchive& ar);
	
public:
	int	m_node[2];	// the two nodes
};

//-----------------------------------------------------------------------------

class GGeneralSpring : public GDiscreteObject
{
public:
	enum { MP_F };

public:
	GGeneralSpring();
	GGeneralSpring(int n1, int n2);

	void Save(OArchive& ar);
	void Load(IArchive& ar);

public:
	int	m_node[2];
};

//-----------------------------------------------------------------------------
// A discrete element can only be defined as a member of a discrete element set
class GDiscreteElement : public GDiscreteObject
{
public:
	GDiscreteElement();
	GDiscreteElement(int n0, int n1);
	GDiscreteElement(const GDiscreteElement& el);
	void operator = (const GDiscreteElement& el);

	int GetID() const { return m_nid; }

	void SetNodes(int n0, int n1);

public:
	const int& Node(int n) const { return m_node[n]; }

protected:
	int		m_nid;
	int		m_node[2];

	static	int	m_ncount;
};

//-----------------------------------------------------------------------------
// base class for discrete element sets
class GDiscreteElementSet : public GDiscreteObject
{
public:
	GDiscreteElementSet(int ntype);
	~GDiscreteElementSet();

	int size() const;

	GDiscreteElement& element(int i);

	void AddElement(int n0, int n1);
	
	void AddElement(const GDiscreteElement& el);

	void RemoveElement(int index);

	int FindElement(const GDiscreteElement& el) const;
	int FindElement(int id) const;

	void UnSelect();
	void Select();

	void SelectComponent(int n);
	void UnselectComponent(int n);

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

protected:
	vector<GDiscreteElement*>	m_elem;
};

//-----------------------------------------------------------------------------
class GLinearSpringSet : public GDiscreteElementSet
{
public:
	enum { MP_E };

public:
	GLinearSpringSet();

	void SetSpringConstant(double E);

	void Save(OArchive& ar);
	void Load(IArchive& ar);
};

//-----------------------------------------------------------------------------
class GNonlinearSpringSet : public GDiscreteElementSet
{
public:
	enum { MP_F };

public:
	GNonlinearSpringSet();

	void Save(OArchive& ar);
	void Load(IArchive& ar);
};

//-----------------------------------------------------------------------------
class GDeformableSpring : public GDiscreteObject
{
public:
	enum { MP_E, MP_DIV };

public:
	GDeformableSpring();
	GDeformableSpring(int n0, int n1);

	int NodeID(int i) const { return m_node[i]; }

	int Divisions() const { return GetIntValue(MP_DIV); }

protected:
	int		m_node[2];

public:
	int		m_ntag;
};

//-----------------------------------------------------------------------------
// This class is not really used, except to define the armature feature to
// the framework
class GArmature : public GDiscreteObject
{
public:
	GArmature() : GDiscreteObject(FE_ARMATURE) {}
};
