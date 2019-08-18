#include "FESurfaceLoad.h"


//-----------------------------------------------------------------------------
FEPressureLoad::FEPressureLoad(FEModel* ps, FEItemListBuilder* pi, int nstep) : FESurfaceLoad(FE_PRESSURE_LOAD, ps, pi, nstep)
{
	SetTypeString("Pressure Load");
	Param* p = AddScienceParam(1, Param_STRESS, "pressure", "pressure");
	p->SetLoadCurve();
	p->MakeVariable(true);
	AddBoolParam(false, "linear", "Type")->SetEnumNames("Nonlinear\0Linear\0");
	AddBoolParam(true , "symmetric_stiffness", "Formulation")->SetEnumNames("Nonsymmetric\0Symmetric\0");
}

// used only for reading parameters for old file formats
void FEPressureLoad::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetLinearFlag(p.GetIntValue() == 1); break;
	case 1: SetLoad(p.GetFloatValue()); break;
	case 2: *GetLoadCurve() = *p.GetLoadCurve(); break;
	}
}

//-----------------------------------------------------------------------------

FEFluidFlux::FEFluidFlux(FEModel* ps, FEItemListBuilder* pi, int nstep) : FESurfaceLoad(FE_FLUID_FLUX, ps, pi, nstep)
{
	SetTypeString("Fluid volumetric flow rate");
	AddDoubleParam(1, "flux", "flux")->SetLoadCurve();
	AddBoolParam(0, "linear", "Type")->SetEnumNames("Nonlinear\0Linear\0");
	AddBoolParam(0, "mixture", "Flux")->SetEnumNames("Fluid\0Mixture\0");
}

// used only for reading parameters for old file formats
void FEFluidFlux::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetLinearFlag(p.GetIntValue() == 1); break;
	case 1: SetLoad(p.GetFloatValue()); break;
	case 2: *GetLoadCurve() = *p.GetLoadCurve(); break;
	case 3: SetMixtureFlag(p.GetIntValue() == 1); break;
	}
}

//-----------------------------------------------------------------------------

FEBPNormalTraction::FEBPNormalTraction(FEModel* ps, FEItemListBuilder* pi, int nstep) : FESurfaceLoad(FE_BP_NORMAL_TRACTION, ps, pi, nstep)
{
	SetTypeString("Mixture normal traction");
	AddDoubleParam(1, "traction", "traction")->SetLoadCurve();
	AddBoolParam(0, "linear", "Type")->SetEnumNames("Nonlinear\0Linear\0");
	AddBoolParam(0, "effective", "Traction")->SetEnumNames("Mixture\0Effective\0");
}

// used only for reading parameters for old file formats
void FEBPNormalTraction::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetLinearFlag(p.GetIntValue() == 1); break;
	case 1: SetLoad(p.GetFloatValue()); break;
	case 2: *GetLoadCurve() = *p.GetLoadCurve(); break;
	case 3: SetMixtureFlag(p.GetIntValue() == 1); break;
	}
}

//-----------------------------------------------------------------------------
FESoluteFlux::FESoluteFlux(FEModel* ps, FEItemListBuilder* pi, int nstep) : FESurfaceLoad(FE_SOLUTE_FLUX, ps, pi, nstep)
{
	SetTypeString("Solute molar flow rate");
	AddDoubleParam(1, "flux", "flux")->SetLoadCurve();
	AddBoolParam(0, "linear", "Type")->SetEnumNames("Nonlinear\0Linear\0");
	AddChoiceParam(0, "solute_id", "Solute")->SetEnumNames("$(Solutes)")->SetOffset(1);
}

// used only for reading parameters for old file formats
void FESoluteFlux::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetLinearFlag(p.GetIntValue() == 1); break;
	case 1: SetLoad(p.GetFloatValue()); break;
	case 2: *GetLoadCurve() = *p.GetLoadCurve(); break;
	case 3: SetBC(p.GetIntValue()); break;
	}
}

//-----------------------------------------------------------------------------

FEHeatFlux::FEHeatFlux(FEModel* ps, FEItemListBuilder* pi, int nstep) : FESurfaceLoad(FE_HEAT_FLUX, ps, pi, nstep)
{
	SetTypeString("Heat Flux");
	AddDoubleParam(0.0, "flux", "flux")->SetLoadCurve();
}

// used only for reading parameters for old file formats
void FEHeatFlux::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetLoad(p.GetFloatValue()); break;
	case 1: *GetLoadCurve() = *p.GetLoadCurve(); break;
	}
}


//-----------------------------------------------------------------------------

FEConvectiveHeatFlux::FEConvectiveHeatFlux(FEModel* ps, FEItemListBuilder* pi, int nstep) : FESurfaceLoad(FE_CONV_HEAT_FLUX, ps, pi, nstep)
{
	SetTypeString("Convective Heat Flux");
	AddDoubleParam(1, "hc", "hc");
	AddDoubleParam(0, "Ta", "Ta")->SetLoadCurve();
}

// used only for reading parameters for old file formats
void FEConvectiveHeatFlux::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetCoefficient(p.GetFloatValue()); break;
	case 1: SetTemperature(p.GetFloatValue()); break;
	case 2: *GetLoadCurve() = *p.GetLoadCurve(); break;
	}
}

//-----------------------------------------------------------------------------

FESurfaceTraction::FESurfaceTraction(FEModel* ps, FEItemListBuilder* pi, int nstep) : FESurfaceLoad(FE_SURFACE_TRACTION, ps, pi, nstep)
{
	SetTypeString("Surface Traction");
	AddDoubleParam(1.0, "scale")->SetLoadCurve();
	AddVecParam(vec3d(0,0,1), "traction");
}

// used only for reading parameters for old file formats
void FESurfaceTraction::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetTraction(p.GetVecValue()); break;
	case 1: *GetLoadCurve() = *p.GetLoadCurve(); break;
	}
}

//-----------------------------------------------------------------------------

FEFluidTraction::FEFluidTraction(FEModel* ps, FEItemListBuilder* pi, int nstep) : FESurfaceLoad(FE_FLUID_TRACTION, ps, pi, nstep)
{
	SetTypeString("Fluid viscous traction");
	AddDoubleParam(1.0, "scale")->SetLoadCurve();
	AddVecParam(vec3d(0, 0, 0), "traction");
}

FELoadCurve* FEFluidTraction::GetLoadCurve() { return GetParamLC(LOAD); }

void FEFluidTraction::SetScale(double s)
{
	SetFloatValue(LOAD, s);
}

double FEFluidTraction::GetScale()
{
	return GetFloatValue(LOAD);
}

void FEFluidTraction::SetTraction(const vec3d& t) { SetVecValue(TRACTION, t); }
vec3d FEFluidTraction::GetTraction() { return GetVecValue(TRACTION); }

// used only for reading parameters for old file formats
void FEFluidTraction::LoadParam(const Param& p)
{
	switch (p.GetParamID())
	{
	case 0: SetTraction(p.GetVecValue()); break;
	case 1: *GetLoadCurve() = *p.GetLoadCurve(); break;
	}
}

//-----------------------------------------------------------------------------

FEFluidVelocity::FEFluidVelocity(FEModel* ps) : FESurfaceLoad(FE_FLUID_VELOCITY, ps)
{
    SetTypeString("Fluid Velocity Vector");
    AddVecParam(vec3d(0,0,0), "fluid velocity", "fluid velocity")->SetLoadCurve();
}

FEFluidVelocity::FEFluidVelocity(FEModel* ps, FEItemListBuilder* pi, vec3d t, int nstep) : FESurfaceLoad(FE_FLUID_VELOCITY, ps, pi, nstep)
{
    SetTypeString("Fluid Velocity Vector");
    AddVecParam(t, "fluid velocity", "fluid velocity")->SetLoadCurve();
}

//-----------------------------------------------------------------------------

FEFluidNormalVelocity::FEFluidNormalVelocity(FEModel* ps) : FESurfaceLoad(FE_FLUID_NORMAL_VELOCITY, ps)
{
    SetTypeString("Fluid Normal Velocity");
    AddDoubleParam(1, "velocity", "velocity")->SetLoadCurve();
    AddBoolParam(true, "prescribe_nodal_velocities", "prescribe_nodal_velocities");
    AddBoolParam(false, "parabolic", "parabolic");
}

FEFluidNormalVelocity::FEFluidNormalVelocity(FEModel* ps, FEItemListBuilder* pi, double vn, bool bp, bool bparab, int nstep) : FESurfaceLoad(FE_FLUID_NORMAL_VELOCITY, ps, pi, nstep)
{
    SetTypeString("Fluid Normal Velocity");
    AddDoubleParam(vn, "velocity", "velocity")->SetLoadCurve();
    AddBoolParam(bp, "prescribe_nodal_velocities", "prescribe_nodal_velocities");
    AddBoolParam(bparab, "parabolic", "parabolic");
}

//-----------------------------------------------------------------------------

FEFluidRotationalVelocity::FEFluidRotationalVelocity(FEModel* ps) : FESurfaceLoad(FE_FLUID_ROTATIONAL_VELOCITY, ps)
{
    SetTypeString("Fluid Rotational Velocity");
    AddDoubleParam(1, "angular_speed", "angular_speed")->SetLoadCurve();
    AddVecParam(vec3d(0,0,1), "axis", "axis");
    AddVecParam(vec3d(0,0,0), "origin", "origin");
}

FEFluidRotationalVelocity::FEFluidRotationalVelocity(FEModel* ps, FEItemListBuilder* pi, double w, vec3d n, vec3d p, int nstep) : FESurfaceLoad(FE_FLUID_ROTATIONAL_VELOCITY, ps, pi, nstep)
{
    SetTypeString("Fluid Normal Velocity");
    AddDoubleParam(w, "angular_speed", "angular_speed")->SetLoadCurve();
    AddVecParam(n, "axis", "axis");
    AddVecParam(p, "origin", "origin");
}

//-----------------------------------------------------------------------------

FEFluidFlowResistance::FEFluidFlowResistance(FEModel* ps) : FESurfaceLoad(FE_FLUID_FLOW_RESISTANCE, ps)
{
    SetTypeString("Fluid Flow Resistance");
    AddDoubleParam(0, "R", "resistance")->SetLoadCurve();
    AddDoubleParam(0, "pressure_offset", "pressure_offset")->SetLoadCurve();
}

FEFluidFlowResistance::FEFluidFlowResistance(FEModel* ps, FEItemListBuilder* pi, double b, double po, int nstep) : FESurfaceLoad(FE_FLUID_FLOW_RESISTANCE, ps, pi, nstep)
{
    SetTypeString("Fluid Flow Resistance");
    AddDoubleParam(b, "R", "resistance")->SetLoadCurve();
    AddDoubleParam(po, "pressure_offset", "pressure_offset")->SetLoadCurve();
}

//-----------------------------------------------------------------------------

FEFluidBackflowStabilization::FEFluidBackflowStabilization(FEModel* ps) : FESurfaceLoad(FE_FLUID_BACKFLOW_STABIL, ps)
{
    SetTypeString("Fluid Backflow Stabilization");
    AddDoubleParam(1, "beta", "beta")->SetLoadCurve();
}

FEFluidBackflowStabilization::FEFluidBackflowStabilization(FEModel* ps, FEItemListBuilder* pi, double b, int nstep) : FESurfaceLoad(FE_FLUID_BACKFLOW_STABIL, ps, pi, nstep)
{
    SetTypeString("Fluid Backflow Stabilization");
    AddDoubleParam(b, "beta", "beta")->SetLoadCurve();
}

//-----------------------------------------------------------------------------

FEFluidTangentialStabilization::FEFluidTangentialStabilization(FEModel* ps) : FESurfaceLoad(FE_FLUID_TANGENTIAL_STABIL, ps)
{
    SetTypeString("Fluid Tangential Stabilization");
    AddDoubleParam(1, "beta", "beta")->SetLoadCurve();
}

FEFluidTangentialStabilization::FEFluidTangentialStabilization(FEModel* ps, FEItemListBuilder* pi, double b, int nstep) : FESurfaceLoad(FE_FLUID_TANGENTIAL_STABIL, ps, pi, nstep)
{
    SetTypeString("Fluid Tangential Stabilization");
    AddDoubleParam(b, "beta", "beta")->SetLoadCurve();
}

//-----------------------------------------------------------------------------

FEFSITraction::FEFSITraction(FEModel* ps) : FESurfaceLoad(FE_FSI_TRACTION, ps)
{
    SetTypeString("FSI Interface Traction");
}

FEFSITraction::FEFSITraction(FEModel* ps, FEItemListBuilder* pi, int nstep) : FESurfaceLoad(FE_FSI_TRACTION, ps, pi, nstep)
{
    SetTypeString("FSI Interface Traction");
}

//=======================================================================================
FEConcentrationFlux::FEConcentrationFlux(FEModel* ps) : FESurfaceLoad(FE_CONCENTRATION_FLUX, ps)
{
	SetTypeString("concentration flux");
	AddChoiceParam(0, "solute_id", "Solute")->SetEnumNames("$(Solutes)")->SetOffset(1);
	AddDoubleParam(0, "flux");
}

FELoadCurve* FEConcentrationFlux::GetLoadCurve() 
{ 
	return GetParamLC(FLUX); 
}

void FEConcentrationFlux::SetFlux(double f)
{ 
	SetFloatValue(FLUX, f); 
}

double FEConcentrationFlux::GetFlux()
{ 
	return GetFloatValue(FLUX); 
}

int FEConcentrationFlux::GetSoluteID()
{ 
	return GetIntValue(SOL_ID);
}

void FEConcentrationFlux::SetSoluteID(int n)
{
	SetIntValue(SOL_ID, n);
}
