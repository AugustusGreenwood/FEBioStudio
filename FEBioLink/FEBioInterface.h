/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#pragma once
#include <vector>
#include <string>

class FEModelComponent;
class FEMaterial;
class FEBioMaterial;
class FEBioDiscreteMaterial;
class FEStep;
class FEBoundaryCondition;
class FENodalLoad;
class FESurfaceLoad;
class FEBodyLoad;
class FEInitialCondition;
class FEPairedInterface;
class FEModelConstraint;
class FERigidConstraint;
class FERigidConnector;
class FSModel;

namespace FEBio {

	bool CreateModelComponent(int classId, FEModelComponent* po);
	bool CreateModelComponent(int superClassId, const std::string& typeStr, FEModelComponent* po);
	void CreateMaterial(int classId, FEBioMaterial* po);
	bool CreateMaterial(const char* sztype, FEBioMaterial* po);
	void CreateMaterialProperty(int superClassID, const char* sztype, FEBioMaterial* po);
	bool CreateDiscreteMaterial(int superClassID, const char* sztype, FEBioDiscreteMaterial* po);
	void CreateStep(int classId, FEStep* po, bool initDefaultProps = true);
	void CreateStep(const char* sztype, FEStep* po);

	void UpdateFEBioMaterial(FEBioMaterial* pm);
	void UpdateFEBioDiscreteMaterial(FEBioDiscreteMaterial* pm);

	// helper functions for creating FEBio classes.
	FEMaterial*          CreateMaterial         (const char* sztype, FSModel* fem);
	FEBoundaryCondition* CreateBoundaryCondition(const char* sztype, FSModel* fem);
	FENodalLoad*         CreateNodalLoad        (const char* sztype, FSModel* fem);
	FESurfaceLoad*       CreateSurfaceLoad      (const char* sztype, FSModel* fem);
	FEBodyLoad*          CreateBodyLoad         (const char* sztype, FSModel* fem);
	FEPairedInterface*   CreatePairedInterface  (const char* sztype, FSModel* fem);
	FEModelConstraint*	 CreateNLConstraint     (const char* sztype, FSModel* fem);
	FERigidConstraint*	 CreateRigidConstraint  (const char* sztype, FSModel* fem);
	FERigidConnector*	 CreateRigidConnector   (const char* sztype, FSModel* fem);
	FEInitialCondition*  CreateInitialCondition (const char* sztype, FSModel* fem);
}
