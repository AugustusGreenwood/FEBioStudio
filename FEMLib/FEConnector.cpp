#include "FEConnector.h"
#include <MeshTools/FEModel.h>
#include <set>
#include <memory>
using namespace std;

//=============================================================================
// FEConnector
//-----------------------------------------------------------------------------

FEConnector::FEConnector(int ntype, FEModel* ps, int nstep)
{
    m_ntype = ntype;
	SetStep(nstep);
    m_bActive = true;
    m_ps = ps;
    m_sztype = "(not defined)";

	m_rbA = m_rbB = -1;
}

//-----------------------------------------------------------------------------
FEConnector::~FEConnector()
{
    
}

//-----------------------------------------------------------------------------
void FEConnector::SetPosition(const vec3d& r)
{

}

//-----------------------------------------------------------------------------
int FEConnector::Type()
{ 
	return m_ntype; 
}

//-----------------------------------------------------------------------------
void FEConnector::SaveList(FEItemListBuilder* pitem, OArchive& ar)
{
    if (pitem)
    {
        ar.BeginChunk((int) pitem->Type());
        pitem->Save(ar);
        ar.EndChunk();
    }
}

//-----------------------------------------------------------------------------
FEItemListBuilder* FEConnector::LoadList(IArchive& ar)
{
    FEItemListBuilder* pitem = 0;
    
    if (ar.OpenChunk() != IO_OK) throw ReadError("error in FEConnector::LoadList");
    unsigned int ntype = ar.GetChunkID();
    switch (ntype)
    {
        case GO_NODE: pitem = new GNodeList(m_ps); break;
        case GO_EDGE: pitem = new GEdgeList(m_ps); break;
        case GO_FACE: pitem = new GFaceList(m_ps); break;
        case GO_PART: pitem = new GPartList(m_ps); break;
        case FE_NODESET: pitem = new FENodeSet((GObject*)0); break;
        case FE_EDGESET: pitem = new FEEdgeSet((GObject*)0); break;
        case FE_SURFACE: pitem = new FESurface((GObject*)0); break;
        case FE_PART   : pitem = new FEPart   ((GObject*)0); break;
        default:
            assert(false);
    }
    if (pitem == 0) throw ReadError("unknown item list type (FEInterface::LoadList)");
    
    pitem->Load(ar);
    ar.CloseChunk();
    
    int nret = ar.OpenChunk();
    if (nret != IO_END) throw ReadError("error in FEInterface::LoadList");
    
    // set the parent mesh for FEGroup's
    FEGroup* pg = dynamic_cast<FEGroup*>(pitem);
    if (pg)
    {
        if (m_ps->FindGroupParent(pg) == false) throw ReadError("Invalid object ID in FEInterface::Load");
    }
    
    return pitem;
}

//-----------------------------------------------------------------------------
void FEConnector::Save(OArchive& ar)
{
    ar.WriteChunk(CID_CONNECTOR_NAME, GetName());
	ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());
    ar.WriteChunk(CID_CONNECTOR_ACTIVE, m_bActive);
    ar.WriteChunk(CID_CONNECTOR_STEP, GetStep());
    ar.BeginChunk(CID_CONNECTOR_PARAMS);
    {
        ParamContainer::Save(ar);
    }
    ar.EndChunk();
    
    ar.WriteChunk(CID_RC_RIGIDBODY_A, m_rbA);
	ar.WriteChunk(CID_RC_RIGIDBODY_B, m_rbB);
}

//-----------------------------------------------------------------------------
void FEConnector::Load(IArchive& ar)
{
    TRACE("FEConnector::Load");
    while (IO_OK == ar.OpenChunk())
    {
        switch (ar.GetChunkID())
        {
            case CID_CONNECTOR_NAME: { string name; ar.read(name); SetName(name); } break;
			case CID_FEOBJ_INFO    : { string info; ar.read(info); SetInfo(info); } break;
            case CID_CONNECTOR_ACTIVE: ar.read(m_bActive); break;
            case CID_CONNECTOR_STEP: { int nstep; ar.read(nstep); SetStep(nstep); } break;
            case CID_CONNECTOR_PARAMS: ParamContainer::Load(ar); break;
			case CID_RC_RIGIDBODY_A: ar.read(m_rbA); break;
            case CID_RC_RIGIDBODY_B: ar.read(m_rbB); break;
            default:
                throw ReadError("unknown CID in FEConnector::Load");
        }
        ar.CloseChunk();
    }
}

//=============================================================================
// FERigidSphericalJoint
//-----------------------------------------------------------------------------

FERigidSphericalJoint::FERigidSphericalJoint(FEModel* ps, int nstep) : FEConnector(FE_RC_SPHERICAL_JOINT, ps, nstep)
{
    SetTypeString("rigid spherical joint");
    
    AddDoubleParam(0.1         , "tolerance"            , "tolerance");
    AddDoubleParam(0           , "gaptol"               , "gap tolerance");
    AddDoubleParam(0           , "angtol"               , "angular tolerance");
    AddDoubleParam(1           , "force_penalty"        , "force penalty factor");
    AddDoubleParam(0           , "moment_penalty"       , "moment penalty factor");
    AddVecParam   (vec3d(0,0,0), "joint_origin"         , "joint origin"        );
    AddIntParam   (0           , "minaug"               , "minimum augmentations");
    AddIntParam   (10          , "maxaug"               , "maximum augmentations");
    AddBoolParam  (0           , "prescribed_rotation"  , "prescribed rotation flag");
    AddDoubleParam(0           , "rotation_x"           , "x-rotation");
    AddDoubleParam(0           , "rotation_y"           , "y-rotation");
    AddDoubleParam(0           , "rotation_z"           , "z-rotation");
    AddDoubleParam(0           , "moment_x"             , "x-moment");
    AddDoubleParam(0           , "moment_y"             , "y-moment");
    AddDoubleParam(0           , "moment_z"             , "z-moment");
}

//-----------------------------------------------------------------------------
void FERigidSphericalJoint::SetPosition(const vec3d& r)
{
	SetVecValue(J_ORIG, r);
}

//=============================================================================
// FERigidRevoluteJoint
//-----------------------------------------------------------------------------

FERigidRevoluteJoint::FERigidRevoluteJoint(FEModel* ps, int nstep) : FEConnector(FE_RC_REVOLUTE_JOINT, ps, nstep)
{
    SetTypeString("rigid revolute joint");
    
    AddDoubleParam(0.1         , "tolerance"            , "tolerance");
    AddDoubleParam(0           , "gaptol"               , "gap tolerance");
    AddDoubleParam(0           , "angtol"               , "angular tolerance");
    AddDoubleParam(1           , "force_penalty"        , "force penalty factor");
    AddDoubleParam(0           , "moment_penalty"       , "moment penalty factor");
    AddVecParam   (vec3d(0,0,0), "joint_origin"         , "joint origin"        );
    AddVecParam   (vec3d(0,0,0), "rotation_axis"        , "rotation axis"       );
    AddVecParam   (vec3d(0,0,0), "transverse_axis"      , "transverse axis"     );
    AddIntParam   (0           , "minaug"              , "minimum augmentations");
    AddIntParam   (10          , "maxaug"              , "maximum augmentations");
    AddBoolParam  (0           , "prescribed_rotation"  , "prescribed rotation flag");
    AddDoubleParam(0           , "rotation"             , "rotation angle");
    AddDoubleParam(0           , "moment"               , "prescribed moment");
}

//-----------------------------------------------------------------------------
void FERigidRevoluteJoint::SetPosition(const vec3d& r)
{
	SetVecValue(J_ORIG, r);
}

//=============================================================================
// FERigidPrismaticJoint
//-----------------------------------------------------------------------------

FERigidPrismaticJoint::FERigidPrismaticJoint(FEModel* ps, int nstep) : FEConnector(FE_RC_PRISMATIC_JOINT, ps, nstep)
{
    SetTypeString("rigid prismatic joint");
    
    AddDoubleParam(0.1         , "tolerance"            , "tolerance");
    AddDoubleParam(0           , "gaptol"               , "gap tolerance");
    AddDoubleParam(0           , "angtol"               , "angular tolerance");
    AddDoubleParam(1           , "force_penalty"        , "force penalty factor");
    AddDoubleParam(0           , "moment_penalty"       , "moment penalty factor");
    AddVecParam   (vec3d(0,0,0), "joint_origin"         , "joint origin"       );
    AddVecParam   (vec3d(0,0,0), "translation_axis"     , "translation axis"   );
    AddVecParam   (vec3d(0,0,0), "transverse_axis"      , "transverse axis"    );
    AddIntParam   (0           , "minaug"              , "minimum augmentations");
    AddIntParam   (10          , "maxaug"              , "maximum augmentations");
    AddBoolParam  (0           , "prescribed_translation", "prescribed translation flag");
    AddDoubleParam(0           , "translation"          , "translation");
    AddDoubleParam(0           , "force"             , "prescribed force");
}

//-----------------------------------------------------------------------------
void FERigidPrismaticJoint::SetPosition(const vec3d& r)
{
	SetVecValue(J_ORIG, r);
}

//=============================================================================
// FERigidCylindricalJoint
//-----------------------------------------------------------------------------

FERigidCylindricalJoint::FERigidCylindricalJoint(FEModel* ps, int nstep) : FEConnector(FE_RC_CYLINDRICAL_JOINT, ps, nstep)
{
    SetTypeString("rigid cylindrical joint");
    
    AddDoubleParam(0.1         , "tolerance"            , "tolerance");
    AddDoubleParam(0           , "gaptol"               , "gap tolerance");
    AddDoubleParam(0           , "angtol"               , "angular tolerance");
    AddDoubleParam(1           , "force_penalty"        , "force penalty factor");
    AddDoubleParam(0           , "moment_penalty"       , "moment penalty factor");
    AddVecParam   (vec3d(0,0,0), "joint_origin"         , "joint origin"        );
    AddVecParam   (vec3d(0,0,0), "joint_axis"           , "joint axis"          );
    AddVecParam   (vec3d(0,0,0), "transverse_axis"      , "transverse axis"     );
    AddIntParam   (0           , "minaug"              , "minimum augmentations");
    AddIntParam   (10          , "maxaug"              , "maximum augmentations");
    AddBoolParam  (0           , "prescribed_translation", "prescribed translation flag");
    AddDoubleParam(0           , "translation"          , "translation");
    AddDoubleParam(0           , "force"             , "prescribed force");
    AddBoolParam  (0           , "prescribed_rotation"  , "prescribed rotation flag");
    AddDoubleParam(0           , "rotation"             , "rotation angle");
    AddDoubleParam(0           , "moment"               , "prescribed moment");
}

//-----------------------------------------------------------------------------
void FERigidCylindricalJoint::SetPosition(const vec3d& r)
{
	SetVecValue(J_ORIG, r);
}

//=============================================================================
// FERigidPlanarJoint
//-----------------------------------------------------------------------------

FERigidPlanarJoint::FERigidPlanarJoint(FEModel* ps, int nstep) : FEConnector(FE_RC_PLANAR_JOINT, ps, nstep)
{
    SetTypeString("rigid planar joint");
    
    AddDoubleParam(0.1         , "tolerance"            , "tolerance");
    AddDoubleParam(0           , "gaptol"               , "gap tolerance");
    AddDoubleParam(0           , "angtol"               , "angular tolerance");
    AddDoubleParam(1           , "force_penalty"        , "force penalty factor");
    AddDoubleParam(0           , "moment_penalty"       , "moment penalty factor");
    AddVecParam   (vec3d(0,0,0), "joint_origin"         , "joint origin"        );
    AddVecParam   (vec3d(0,0,0), "rotation_axis"        , "rotation axis"       );
    AddVecParam   (vec3d(0,0,0), "translation_axis_1"   , "translation axis 1"  );
    AddIntParam   (0           , "minaug"              , "minimum augmentations");
    AddIntParam   (10          , "maxaug"              , "maximum augmentations");
    AddBoolParam  (0           , "prescribed_translation_1", "prescribed translation flag along axis 1");
    AddDoubleParam(0           , "translation_1"        , "translation 1");
    AddBoolParam  (0           , "prescribed_translation_2", "prescribed translation flag along axis 2");
    AddDoubleParam(0           , "translation_2"        , "translation 2");
    AddBoolParam  (0           , "prescribed_rotation"  , "prescribed rotation flag");
    AddDoubleParam(0           , "rotation"             , "rotation angle");
}

//-----------------------------------------------------------------------------
void FERigidPlanarJoint::SetPosition(const vec3d& r)
{
	SetVecValue(J_ORIG, r);
}

//=============================================================================
// FERigidLock
//-----------------------------------------------------------------------------

FERigidLock::FERigidLock(FEModel* ps, int nstep) : FEConnector(FE_RC_RIGID_LOCK, ps, nstep)
{
    SetTypeString("rigid lock");
    
    AddDoubleParam(0.1         , "tolerance"            , "tolerance");
    AddDoubleParam(0           , "gaptol"               , "gap tolerance");
    AddDoubleParam(0           , "angtol"               , "angular tolerance");
    AddDoubleParam(1           , "force_penalty"        , "force penalty factor");
    AddDoubleParam(0           , "moment_penalty"       , "moment penalty factor");
    AddVecParam   (vec3d(0,0,0), "joint_origin"         , "joint origin"         );
    AddVecParam   (vec3d(0,0,0), "first_axis"           , "first axis"           );
    AddVecParam   (vec3d(0,0,0), "second_axis"          , "second axis"          );
    AddIntParam   (0           , "minaug"               , "minimum augmentations");
    AddIntParam   (10          , "maxaug"               , "maximum augmentations");
}

//-----------------------------------------------------------------------------
void FERigidLock::SetPosition(const vec3d& r)
{
    SetVecValue(J_ORIG, r);
}

//=============================================================================
// FERigidSpring
//-----------------------------------------------------------------------------

FERigidSpring::FERigidSpring(FEModel* ps, int nstep) : FEConnector(FE_RC_SPRING, ps, nstep)
{
    SetTypeString("rigid spring");
    
    AddDoubleParam(0           , "k", "stiffness");
    AddVecParam   (vec3d(0,0,0), "insertion_a" , "insertion point a");
    AddVecParam   (vec3d(0,0,0), "insertion_b" , "insertion point b");
}

//=============================================================================
// FERigidDamper
//-----------------------------------------------------------------------------

FERigidDamper::FERigidDamper(FEModel* ps, int nstep) : FEConnector(FE_RC_DAMPER, ps, nstep)
{
    SetTypeString("rigid damper");
    
    AddDoubleParam(0           , "c", "damping");
    AddVecParam   (vec3d(0,0,0), "insertion_a" , "insertion point a");
    AddVecParam   (vec3d(0,0,0), "insertion_b" , "insertion point b");
}

//=============================================================================
// FERigidAngularDamper
//-----------------------------------------------------------------------------

FERigidAngularDamper::FERigidAngularDamper(FEModel* ps, int nstep) : FEConnector(FE_RC_ANGULAR_DAMPER, ps, nstep)
{
    SetTypeString("rigid angular damper");
    
    AddDoubleParam(0.0 , "c", "damping");
}

//=============================================================================
// FERigidContractileForce
//-----------------------------------------------------------------------------

FERigidContractileForce::FERigidContractileForce(FEModel* ps, int nstep) : FEConnector(FE_RC_CONTRACTILE_FORCE, ps, nstep)
{
    SetTypeString("rigid contractile force");
    
    AddDoubleParam(0           , "f0", "force");
    AddVecParam   (vec3d(0,0,0), "insertion_a" , "insertion point a");
    AddVecParam   (vec3d(0,0,0), "insertion_b" , "insertion point b");
}
