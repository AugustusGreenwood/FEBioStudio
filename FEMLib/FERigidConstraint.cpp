#include "stdafx.h"
#include "FERigidConstraint.h"

FERigidConstraintOld::FERigidConstraintOld(int ntype, int nstep)
{
	m_ntype = ntype;
	m_mid = -1;
	m_nstep = nstep;
	for (int i=0; i<6; ++i)
	{
		m_BC[i] = 0;
		m_val[i] = 0;
	}
}

FERigidConstraintOld::~FERigidConstraintOld(void)
{
}

void FERigidConstraintOld::Save(OArchive &ar)
{
	ar.WriteChunk(NAME, GetName());
	ar.WriteChunk(MATID, m_mid);
	for (int i=0; i<6; ++i)
	{
		ar.BeginChunk(CONSTRAINT);
		{
			ar.WriteChunk(BC, m_BC[i]);
			ar.WriteChunk(VAL, m_val[i]);
			ar.BeginChunk(LC);
			{
				m_LC[i].Save(ar);
			}
			ar.EndChunk();
		}
		ar.EndChunk();
	}
}

void FERigidConstraintOld::Load(IArchive &ar)
{
	TRACE("FERigidConstraintOld::Load");

	char sz[256] = {0};
	int n = 0;
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		unsigned int nid = ar.GetChunkID();
		switch (nid)
		{
		case NAME: ar.read(sz); SetName(sz); break;
		case MATID: ar.read(m_mid); break;
		case CONSTRAINT:
			{
				while (ar.OpenChunk() == IArchive::IO_OK)
				{
					unsigned int nid = ar.GetChunkID();
					switch (nid)
					{
					case BC: ar.read(m_BC[n]); break;
					case VAL: ar.read(m_val[n]); break;
					case LC: m_LC[n].Load(ar); break;
					}
					ar.CloseChunk();
				}
				++n;
			}
			break;
		}
		ar.CloseChunk();
	}
}

//=============================================================================
FERigidConstraint::FERigidConstraint(int ntype, int nstep)
{
	m_matid = -1;
	m_ntype = ntype;
	m_nstepID = nstep;
}

FERigidConstraint::~FERigidConstraint(void)
{
}

void FERigidConstraint::Save(OArchive &ar)
{
	ar.WriteChunk(NAME, GetName());
	ar.WriteChunk(MATID, m_matid);
	ar.BeginChunk(PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();
}

void FERigidConstraint::Load(IArchive &ar)
{
	TRACE("FERigidConstraint::Load");

	char sz[256] = {0};
	int n = 0;
	while (ar.OpenChunk() == IArchive::IO_OK)
	{
		unsigned int nid = ar.GetChunkID();
		switch (nid)
		{
		case NAME: ar.read(sz); SetName(sz); break;
		case MATID: ar.read(m_matid); break;
		case PARAMS:
			{
				ParamContainer::Load(ar);
			}
			break;
		}
		ar.CloseChunk();
	}
}

FERigidFixed::FERigidFixed(FEModel* fem, int nstep) : FERigidConstraint(FE_RIGID_FIXED, nstep)
{
	SetTypeString("Rigid fixed");

	AddBoolParam(false, "bc1", "X-displacement");
	AddBoolParam(false, "bc2", "Y-displacement");
	AddBoolParam(false, "bc3", "Z-displacement");
	AddBoolParam(false, "bc4", "X-rotation");
	AddBoolParam(false, "bc5", "Y-rotation");
	AddBoolParam(false, "bc6", "Z-rotation");
}

FERigidPrescribed::FERigidPrescribed(int ntype, int nstep) : FERigidConstraint(ntype, nstep)
{
}

FERigidDisplacement::FERigidDisplacement(FEModel* fem, int nstep) : FERigidPrescribed(FE_RIGID_DISPLACEMENT, nstep)
{
	SetTypeString("Rigid displacement/rotation");

	AddIntParam(0, "var", "var")->SetEnumNames("X-displacement\0Y-displacement\0Z-displacement\0X-rotation\0Y-rotation\0Z-rotation\0");
	AddDoubleParam(0.0, "val", "value")->SetLoadCurve();
}

FERigidDisplacement::FERigidDisplacement(int bc, int matid, double v, int nstep) : FERigidPrescribed(FE_RIGID_DISPLACEMENT, nstep)
{
	SetTypeString("Rigid displacement/rotation");

	AddIntParam(bc, "var", "var")->SetEnumNames("X-displacement\0Y-displacement\0Z-displacement\0X-rotation\0Y-rotation\0Z-rotation\0");
	AddDoubleParam(v, "val", "value")->SetLoadCurve();
	SetMaterialID(matid);
}


FERigidForce::FERigidForce(FEModel* fem, int nstep) : FERigidPrescribed(FE_RIGID_FORCE, nstep)
{
	SetTypeString("Rigid force");

	AddIntParam(0, "var", "var")->SetEnumNames("X-force\0Y-force\0Z-force\0X-torque\0Y-torque\0Z-torque\0");
	AddDoubleParam(0.0, "val", "value")->SetLoadCurve();
}

FERigidForce::FERigidForce(int bc, int matid, double v, int nstep) : FERigidPrescribed(FE_RIGID_FORCE, nstep)
{
	SetTypeString("Rigid force");

	AddIntParam(bc, "var", "var")->SetEnumNames("X-force\0Y-force\0Z-force\0X-torque\0Y-torque\0Z-torque\0");
	AddDoubleParam(v, "val", "value")->SetLoadCurve();
	SetMaterialID(matid);
}

FERigidVelocity::FERigidVelocity(FEModel* fem, int nstep) : FERigidConstraint(FE_RIGID_INIT_VELOCITY, nstep)
{
	SetTypeString("Rigid velocity");

	AddVecParam(vec3d(0,0,0), "value", "velocity");
}

FERigidAngularVelocity::FERigidAngularVelocity(FEModel* fem, int nstep) : FERigidConstraint(FE_RIGID_INIT_ANG_VELOCITY, nstep)
{
	SetTypeString("Rigid angular velocity");

	AddVecParam(vec3d(0, 0, 0), "value", "angular velocity");
}


vector<FERigidConstraint*> convertOldToNewRigidConstraint(FEModel* fem, FERigidConstraintOld* rc)
{
	vector<FERigidConstraint*> rc_new;
	switch (rc->Type())
	{
	case FE_RIGID_FIXED:
		{
			FERigidFixed* rf = new FERigidFixed(fem, rc->GetStep());
			if (rc->m_BC[0]) rf->SetDOF(0, true);
			if (rc->m_BC[1]) rf->SetDOF(1, true);
			if (rc->m_BC[2]) rf->SetDOF(2, true);
			if (rc->m_BC[3]) rf->SetDOF(3, true);
			if (rc->m_BC[4]) rf->SetDOF(4, true);
			if (rc->m_BC[5]) rf->SetDOF(5, true);

			rf->SetMaterialID(rc->m_mid);			
			rc_new.push_back(rf);
		}
		break;
	case FE_RIGID_DISPLACEMENT:
	case FE_RIGID_FORCE:
		{
			for (int i=0; i<6; ++i)
			{
				if (rc->m_BC[i])
				{
					FERigidPrescribed* rp = 0;
					if      (rc->Type() == FE_RIGID_DISPLACEMENT) rp = new FERigidDisplacement(fem, rc->GetStep());
					else if (rc->Type() == FE_RIGID_FORCE       ) rp = new FERigidForce       (fem, rc->GetStep());
					else { assert(false); }

					rp->SetMaterialID(rc->m_mid);
					rp->SetDOF(i);
					rp->SetValue(rc->m_val[i]);
					rp->SetLoadCurve(rc->m_LC[i]);

					rc_new.push_back(rp);
				}
			}
		}
		break;
	case FE_RIGID_INIT_VELOCITY:
		{	
			FERigidVelocity* rv = new FERigidVelocity(fem, rc->GetStep());
			rv->SetVelocity(vec3d(rc->m_val[0], rc->m_val[1], rc->m_val[2]));
			rv->SetMaterialID(rc->m_mid);

			FERigidAngularVelocity* ra = new FERigidAngularVelocity(fem, rc->GetStep());
			ra->SetVelocity(vec3d(rc->m_val[3], rc->m_val[4], rc->m_val[5]));

			rc_new.push_back(rv);
			rc_new.push_back(ra);
		}
		break;
	}

	return rc_new;
}
