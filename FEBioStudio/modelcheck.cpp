#include "stdafx.h"
#include "modelcheck.h"
#include "Document.h"
#include <PostGL/GLModel.h>
#include <GeomLib/GObject.h>
#include <FEMLib/FEMaterial.h>
#include <FEMLib/FERigidConstraint.h>
#include <stdarg.h>

// are there any objects?
void check_000(CDocument* doc, std::vector<FSObject*>& objList);

// are all geometries meshed?
void check_001(CDocument* doc, std::vector<FSObject*>& objList);

// are there any materials
void check_002(CDocument* doc, std::vector<FSObject*>& objList);

// is a material assigned to all parts ?
void check_003(CDocument* doc, std::vector<FSObject*>& objList);

// Are there any analysis steps? 
void check_004(CDocument* doc, std::vector<FSObject*>& objList);

// check for unused materials
void check_005(CDocument* doc, std::vector<FSObject*>& objList);

// check for unconstrained rigid bodies
void check_006(CDocument* doc, std::vector<FSObject*>& objList);

// see if surfaces of solo interfaces are assigned.
void check_007(CDocument* doc, std::vector<FSObject*>& objList);

// see if surfaces of paired interfaces are assigned.
void check_008(CDocument* doc, std::vector<FSObject*>& objList);

// see if loads have assigned selections.
void check_009(CDocument* doc, std::vector<FSObject*>& objList);

// see if BCs have assigned selections.
void check_010(CDocument* doc, std::vector<FSObject*>& objList);

// see if ICs have assigned selections.
void check_011(CDocument* doc, std::vector<FSObject*>& objList);

// see if there are any output variables defined
void check_012(CDocument* doc, std::vector<FSObject*>& objList);

typedef void(*ERROR_FUNC)(CDocument*, std::vector<FSObject*>&);

struct ERROR_DATA
{
	ERROR_TYPE	errType;
	const char* errStr;
	ERROR_FUNC	errFnc;
};

vector<ERROR_DATA> error = {
	{ CRITICAL, "This model does not contain any geometry.", check_000},
	{ CRITICAL, "The object \"%s\" is not meshed."         , check_001 },
	{ CRITICAL, "There are no materials defined in this model.", check_002 },
	{ CRITICAL, "Part \"%s\" does not have a material assigned.", check_003 },
	{ CRITICAL, "There are no analysis steps defined.", check_004 },
	{ WARNING , "Material \"%s\" is not used by this model.", check_005 },
	{ WARNING , "Rigid body \"%s\" might be underconstrained.", check_006 },
	{ CRITICAL, "No selection was assigned to interface \"%s\".", check_007 },
	{ CRITICAL, "One or both of the selections of interface \"%s\" is unassigned.", check_008 },
	{ CRITICAL, "Load \"%s\" has no selection assigned.", check_009 },
	{ CRITICAL, "Boundary condition \"%s\" has no selection assigned.", check_010 },
	{ CRITICAL, "Initial condition \"%s\" has no selection assigned.", check_011 },
	{ WARNING , "This model does not define any output variables.", check_012 }
};

bool isWarning(int error_code)
{
	return (error[error_code].errType == ERROR_TYPE::CRITICAL);
}

const char* errorString(int error_code)
{
	if (error_code >= error.size())
	{
		assert(false);
		return "undefined error";
	}
	else return error[error_code].errStr;
}

MODEL_ERROR make_error(int error_code, FSObject* po = nullptr)
{
	// make the message
	char sztxt[1024] = { 0 };
	if (po)
		sprintf(sztxt, error[error_code].errStr, po->GetName().c_str());
	else 
		sprintf(sztxt, error[error_code].errStr, "(null)");

	MODEL_ERROR err;
	err.first = error[error_code].errType;
	err.second = string(sztxt);
	return err;
}

void checkModel(CDocument* doc, std::vector<MODEL_ERROR>& errorList)
{
	for (size_t i = 0; i < error.size(); ++i)
	{
		vector<FSObject*> objList;
		error[i].errFnc(doc, objList);

		for (size_t j = 0; j < objList.size(); ++j)
		{
			errorList.push_back(make_error(i, objList[j]));
		}
	}
}


void check_000(CDocument* doc, std::vector<FSObject*>& objList)
{
	// are there any objects?
	GModel& mdl = *doc->GetGModel();
	if (mdl.Objects() == 0)
	{
		objList.push_back(&mdl);
	}
}

void check_001(CDocument* doc, std::vector<FSObject*>& objList)
{
	GModel& mdl = *doc->GetGModel();
	for (int i = 0; i < mdl.Objects(); ++i)
	{
		GObject* po = mdl.Object(i);
		if (po->GetFEMesh() == nullptr)
		{
			objList.push_back(po);
		}
	}
}

void check_002(CDocument* doc, std::vector<FSObject*>& objList)
{
	FEModel& fem = *doc->GetFEModel();
	if (fem.Materials() == 0)
	{
		objList.push_back(&fem);
	}
}

void check_003(CDocument* doc, std::vector<FSObject*>& objList)
{
	GModel& mdl = *doc->GetGModel();
	for (int i = 0; i < mdl.Objects(); ++i)
	{
		GObject* po = mdl.Object(i);
		for (int j = 0; j < po->Parts(); ++j)
		{
			GPart* pj = po->Part(j);
			if (pj->GetMaterialID() == -1)
			{
				objList.push_back(pj);
			}
		}
	}
}

void check_004(CDocument* doc, std::vector<FSObject*>& objList)
{
	FEModel& fem = *doc->GetFEModel();
	if (fem.Steps() <= 1)
	{
		objList.push_back(&fem);
	}
}

void check_005(CDocument* doc, std::vector<FSObject*>& objList)
{
	GModel& mdl = *doc->GetGModel();
	FEModel& fem = *doc->GetFEModel();
	for (int i = 0; i < fem.Materials(); ++i)
	{
		GMaterial* mat = fem.GetMaterial(i);

		// see if this material is used
		list<GPart*> partList = mdl.FindPartsFromMaterial(mat->GetID());
		if (partList.empty())
		{
			objList.push_back(mat);
		}
	}
}

void check_006(CDocument* doc, std::vector<FSObject*>& objList)
{
	GModel& mdl = *doc->GetGModel();
	FEModel& fem = *doc->GetFEModel();
	for (int i = 0; i < fem.Materials(); ++i)
	{
		GMaterial* mat = fem.GetMaterial(i);
		if (dynamic_cast<FERigidMaterial*>(mat->GetMaterialProperties()))
		{
			int matId = mat->GetID();
			bool matUsed = false;
			for (int n = 0; n < fem.Steps(); ++n)
			{
				FEStep* pstep = fem.GetStep(n);
				
				// see if this material is referenced by any rigid constraints
				for (int j = 0; j < pstep->RCs(); ++j)
				{
					FERigidConstraint* prc = pstep->RC(j);
					if (prc->GetMaterialID() == matId)
					{
						matUsed = true;
						break;
					}
				}
				if (matUsed) break;

				// see if this material is referenced by any rigid connector
				for (int j = 0; j < pstep->Connectors(); ++j)
				{
					FEConnector* prc = pstep->Connector(j);
					if ((prc->m_rbA == matId) || (prc->m_rbB == matId))
					{
						matUsed = true;
						break;
					}
				}
				if (matUsed) break;

				// see if this material is used by any rigid interface
				for (int j = 0; j < pstep->Interfaces(); ++j)
				{
					FERigidInterface* ri = dynamic_cast<FERigidInterface*>(pstep->Interface(j));
					if (ri && (ri->GetRigidBody() == mat))
					{
						matUsed = true;
						break;
					}
				}

				// see if this material is used by a part that is connected to another part
				for (int j = 0; j < mdl.Objects(); ++j)
				{
					GObject* po = mdl.Object(j);
					for (int k = 0; k < po->Faces(); ++k)
					{
						GFace* face = po->Face(k);
						if ((face->m_nPID[0] != -1) && (face->m_nPID[1] != -1))
						{
							GPart* pg0 = po->Part(face->m_nPID[0]);
							GPart* pg1 = po->Part(face->m_nPID[1]);
							if ((pg0->GetMaterialID() == matId) || (pg1->GetMaterialID() == matId))
							{
								matUsed = true;
								break;
							}
						}
					}
					if (matUsed) break;
				}
				if (matUsed) break;
			}

			if (matUsed == false)
			{
				objList.push_back(mat);
			}
		}
	}
}

// see if surfaces of solo interfaces are assigned.
void check_007(CDocument* doc, std::vector<FSObject*>& objList)
{
	FEModel& fem = *doc->GetFEModel();
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FEStep* step = fem.GetStep(i);
		for (int j = 0; j < step->Interfaces(); ++j)
		{
			FEInterface* pi = step->Interface(j);
			if (dynamic_cast<FESoloInterface*>(pi))
			{
				FESoloInterface* psi = dynamic_cast<FESoloInterface*>(pi);
				if (psi->GetItemList() == nullptr)
				{
					objList.push_back(pi);
				}
			}
		}
	}
}

// see if surfaces of paired interfaces are assigned.
void check_008(CDocument* doc, std::vector<FSObject*>& objList)
{
	FEModel& fem = *doc->GetFEModel();
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FEStep* step = fem.GetStep(i);
		for (int j = 0; j < step->Interfaces(); ++j)
		{
			FEInterface* pi = step->Interface(j);
			if (dynamic_cast<FEPairedInterface*>(pi))
			{
				FEPairedInterface* psi = dynamic_cast<FEPairedInterface*>(pi);
				if ((psi->GetMasterSurfaceList() == nullptr) || (psi->GetSlaveSurfaceList() == nullptr))
				{
					objList.push_back(pi);
				}
			}
		}
	}
}

// see if loads have assigned selections.
void check_009(CDocument* doc, std::vector<FSObject*>& objList)
{
	FEModel& fem = *doc->GetFEModel();
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FEStep* step = fem.GetStep(i);
		for (int j = 0; j < step->Loads(); ++j)
		{
			FEBoundaryCondition* pl = step->Load(j);
			if ((dynamic_cast<FEBodyLoad*>(pl) == nullptr) && (pl->GetItemList() == nullptr))
			{
				objList.push_back(pl);
			}
		}
	}
}

// see if BCs have assigned selections.
void check_010(CDocument* doc, std::vector<FSObject*>& objList)
{
	FEModel& fem = *doc->GetFEModel();
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FEStep* step = fem.GetStep(i);
		for (int j = 0; j < step->BCs(); ++j)
		{
			FEBoundaryCondition* pl = step->BC(j);
			if (pl->GetItemList() == nullptr)
			{
				objList.push_back(pl);
			}
		}
	}
}

// see if ICs have assigned selections.
void check_011(CDocument* doc, std::vector<FSObject*>& objList)
{
	FEModel& fem = *doc->GetFEModel();
	for (int i = 0; i < fem.Steps(); ++i)
	{
		FEStep* step = fem.GetStep(i);
		for (int j = 0; j < step->ICs(); ++j)
		{
			FEInitialCondition* pl = step->IC(j);
			if (pl->GetItemList() == nullptr)
			{
				objList.push_back(pl);
			}
		}
	}
}

// see if there are any output variables defined
void check_012(CDocument* doc, std::vector<FSObject*>& objList)
{
	FEModel& fem = *doc->GetFEModel();
	if (fem.DataVariables() == 0)
	{
		objList.push_back(&fem);
	}
}
