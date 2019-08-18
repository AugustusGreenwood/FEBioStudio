#pragma once
#include <FSCore/ParamBlock.h>
#include <FSCore/Serializable.h>
#include <MeshLib/box.h>
#include <FSCore/FSObject.h>
#include <list>
using namespace std;

//-----------------------------------------------------------------------------
class GObject;
class FEMesh;
class GMesh;
class FEModel;

//-----------------------------------------------------------------------------
class GModifier : public FSObject
{
public:
	GModifier(void);
	virtual ~GModifier(void);

	virtual void Apply(GObject* po) = 0;
	virtual GMesh* BuildGMesh(GObject* po) = 0;
	virtual FEMesh* BuildFEMesh(GObject* po) = 0; 
};

//-----------------------------------------------------------------------------
class GTwistModifier : public GModifier
{
public:
	enum { ORIENT, TWIST, SCALE, SMIN, SMAX };
public:
	GTwistModifier(FEModel* ps = 0);
	void Apply(GObject* po);
	virtual GMesh* BuildGMesh(GObject* po);
	FEMesh* BuildFEMesh(GObject* po);
};

//-----------------------------------------------------------------------------
class GPinchModifier : public GModifier
{
public:
	enum {SCALE, ORIENT};

public:
	GPinchModifier(FEModel* ps = 0);
	void Apply(GObject* po);
	virtual GMesh* BuildGMesh(GObject* po);
	FEMesh* BuildFEMesh(GObject* po);
};

//-----------------------------------------------------------------------------
class GBendModifier : public GModifier
{
public:
	enum { ORIENT, ANGLE, SMIN, SMAX, XPIVOT, YPIVOT, ZPIVOT };

public:
	GBendModifier(FEModel* ps = 0);
	void Apply(GObject* po);
	virtual GMesh* BuildGMesh(GObject* po);
	FEMesh* BuildFEMesh(GObject* po);

protected:
	void UpdateParams();
	void Apply(vec3d& r);

protected:
	BOX		m_box;
	vec3d	m_rc;
	vec3d	m_pvt;
	quatd	m_q;
	double	m_R0, m_L;
	double	m_smin, m_smax, m_a;
};

//-----------------------------------------------------------------------------
class GSkewModifier : public GModifier
{
public:
	enum { ORIENT, SKEW };

public:
	GSkewModifier(FEModel* ps = 0);
	void Apply(GObject* po);
	virtual GMesh* BuildGMesh(GObject* po);
	FEMesh* BuildFEMesh(GObject* po);
};

//-----------------------------------------------------------------------------
class GWrapModifier : public GModifier
{
public:
	enum { TRG_ID, METHOD, NSTEPS };
public:
	GWrapModifier();
	void Apply(GObject* po);
	virtual GMesh* BuildGMesh(GObject* po) { return 0; }
	FEMesh* BuildFEMesh(GObject* po) { return 0; }
	void SetTarget(GObject* ptrg) { m_po = ptrg; }

protected:
	void ClosestPoint(FEMesh* pm, vector<vec3d>& DS, vector<int>& tag);
	void NormalProjection(FEMesh* pm, vector<vec3d>& DS, vector<int>& tag, int nsteps);

protected:
	GObject*	m_po;	//!< target object
};

//-----------------------------------------------------------------------------
class GExtrudeModifier : public GModifier
{
public:
	enum { DIST };

public:
	GExtrudeModifier();
	void Apply(GObject* po);
	GMesh* BuildGMesh(GObject* po);
	FEMesh* BuildFEMesh(GObject* po);
};


//-----------------------------------------------------------------------------
class GRevolveModifier : public GModifier
{
public:
	enum { ANGLE, DIVS };

public:
	GRevolveModifier();
	void Apply(GObject* po);
	GMesh* BuildGMesh(GObject* po);
	FEMesh* BuildFEMesh(GObject* po);
};

//-----------------------------------------------------------------------------
class GModifierStack : public CSerializable
{
public:
	GModifierStack(GObject* po);
	~GModifierStack();

	GObject* GetOwner() { return m_po; }
	void SetOwner(GObject* po) { m_po = po; }

	void Add(GModifier* pmod) { m_Mod.push_back(pmod); }
	void Remove(GModifier* pmod);
	int Size() { return (int) m_Mod.size(); }

	FEMesh* GetFEMesh() { return m_pmesh; }
	void ClearMesh();

	void SetFEMesh(FEMesh* pm) { ClearMesh(); m_pmesh = pm; }

	GModifier* Modifier(int n)
	{
		list<GModifier*>::iterator pi = m_Mod.begin();
		for (int i=0; i<n; ++i) ++pi;
		return (*pi);
	}

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	void Apply();

	void Clear();

	void Copy(GModifierStack* ps);

protected:
	GObject*	m_po;		// the object that owns this stack
	FEMesh*		m_pmesh;	// the original mesh

	list<GModifier*>	m_Mod;	// the actual modifier stack
};
