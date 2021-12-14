#pragma once
#include "FEDomainComponent.h"

//=============================================================================
// Base class for all nodal, edge, surface, and body loads
class FSLoad : public FSDomainComponent
{
public:
	FSLoad(int ntype, FSModel* fem) : FSDomainComponent(ntype, fem) {}
	FSLoad(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep) : FSDomainComponent(ntype, ps, pi, nstep) {}
};

//=============================================================================
// NODAL LOADS
//=============================================================================

class FSNodalLoad : public FSLoad
{
public:
	FSNodalLoad(int ntype, FSModel* fem) : FSLoad(ntype, fem) { m_superClassID = FENODALLOAD_ID; }
	FSNodalLoad(int ntype, FSModel* ps, FEItemListBuilder* pi, int nstep) : FSLoad(ntype, ps, pi, nstep) { m_superClassID = FENODALLOAD_ID; }
};

class FSNodalDOFLoad : public FSNodalLoad
{
public:
	enum { DOF, LOAD };

public:
	FSNodalDOFLoad(FSModel* ps);
	FSNodalDOFLoad(FSModel* ps, FEItemListBuilder* pi, int bc, double f, int nstep = 0);

	LoadCurve* GetLoadCurve() { return GetParamLC(LOAD); }

	int GetDOF() { return GetIntValue(DOF); }
	void SetDOF(int n) { SetIntValue(DOF, n); }

	void SetLoad(double f) { SetFloatValue(LOAD, f); }
	double GetLoad() { return GetFloatValue(LOAD); }
};

class FEBioNodalLoad : public FSNodalLoad
{
public:
	FEBioNodalLoad(FSModel* ps);
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};
