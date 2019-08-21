#pragma once
#include "FEMLib/FEStepComponent.h"
#include "MeshTools/FEItemListBuilder.h"
#include "MeshTools/GMaterial.h"
#include <list>
using namespace std;

//-----------------------------------------------------------------------------
// Base class for contact interfaces
class FEConnector : public FEStepComponent
{
public:
	FEConnector(int ntype, FEModel* ps, int nstep);
	virtual ~FEConnector();

	int Type();

	virtual void SetPosition(const vec3d& r);

	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

protected:
	void SaveList(FEItemListBuilder* pitem, OArchive& ar);
	FEItemListBuilder* LoadList(IArchive& ar);

protected:
	FEModel* m_ps;
	int		m_ntype;

public:
	int	m_rbA;
	int	m_rbB;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid spherical joint
//
class FERigidSphericalJoint : public FEConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, J_ORIG,
        MIN_AUG, MAX_AUG, B_ROT, ROT_X, ROT_Y, ROT_Z, MX, MY, MZ };
    
public:
    FERigidSphericalJoint(FEModel* ps, int nstep = 0);
	void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid revolute joint
//
class FERigidRevoluteJoint : public FEConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, J_ORIG, J_AXIS, T_AXIS,
        MIN_AUG, MAX_AUG, B_ROT, ROT, MOMENT };
    
public:
    FERigidRevoluteJoint(FEModel* ps, int nstep = 0);
	void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid prismatic joint
//
class FERigidPrismaticJoint : public FEConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, J_ORIG, J_AXIS, T_AXIS,
        MIN_AUG, MAX_AUG, B_TRANS, TRANS, FORCE };
    
public:
    FERigidPrismaticJoint(FEModel* ps, int nstep = 0);
	void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid cylindrical joint
//
class FERigidCylindricalJoint : public FEConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, J_ORIG, J_AXIS, T_AXIS,
        MIN_AUG, MAX_AUG, B_TRANS, TRANS, FORCE, B_ROT, ROT, MOMENT };
    
public:
    FERigidCylindricalJoint(FEModel* ps, int nstep = 0);
	void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid planar joint
//
class FERigidPlanarJoint : public FEConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, J_ORIG, J_AXIS, T_AXIS,
        MIN_AUG, MAX_AUG, B_TRANS1, TRANS1, B_TRANS2, TRANS2, B_ROT, ROT };
    
public:
    FERigidPlanarJoint(FEModel* ps, int nstep = 0);
	void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid lock joint
//
class FERigidLock : public FEConnector
{
public:
    enum {TOL, GTOL, ATOL, F_PENALTY, M_PENALTY, J_ORIG, J_AXIS, T_AXIS,
        MIN_AUG, MAX_AUG };
    
public:
    FERigidLock(FEModel* ps, int nstep = 0);
    void SetPosition(const vec3d& r) override;
};

//-----------------------------------------------------------------------------
//  This class implements a rigid spring
//
class FERigidSpring : public FEConnector
{
public:
    enum { K, XA, XB };
    
public:
    FERigidSpring(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
//  This class implements a rigid damper
//
class FERigidDamper : public FEConnector
{
public:
    enum { C, XA, XB };
    
public:
    FERigidDamper(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
//  This class implements a rigid angular damper
//
class FERigidAngularDamper : public FEConnector
{
public:
    enum { C };
    
public:
    FERigidAngularDamper(FEModel* ps, int nstep = 0);
};

//-----------------------------------------------------------------------------
//  This class implements a rigid contractile force
//
class FERigidContractileForce : public FEConnector
{
public:
    enum { F, XA, XB };
    
public:
    FERigidContractileForce(FEModel* ps, int nstep = 0);
};
