#include "FECoreMaterial.h"
#include <MeshTools/FEMaterialFactory.h>
#include <MeshTools/FEMaterial.h>

//=============================================================================
// FEMaterialProperty
//=============================================================================

//-----------------------------------------------------------------------------
FEMaterialProperty::FEMaterialProperty()
{
	m_parent = 0;
	m_nClassID = -1;
	m_maxSize = NO_FIXED_SIZE;
}

//-----------------------------------------------------------------------------
FEMaterialProperty::FEMaterialProperty(const std::string& name, int nClassID, FEMaterial* parent, int nsize, unsigned int flags) : m_parent(parent)
{
	m_nClassID = nClassID;
	m_name = name;
	m_flag = flags;
	m_maxSize = nsize;
	if (nsize > 0)
	{
		m_mat.assign(nsize, 0);
	}
}

//-----------------------------------------------------------------------------
FEMaterialProperty::~FEMaterialProperty()
{
	Clear();
}

//-----------------------------------------------------------------------------
void FEMaterialProperty::SetName(const std::string& name)
{
	m_name = name;
}

//-----------------------------------------------------------------------------
const std::string& FEMaterialProperty::GetName()
{ 
	return m_name; 
}

//-----------------------------------------------------------------------------
void FEMaterialProperty::Clear()
{
	for (int i = 0; i<(int)m_mat.size(); ++i) { delete m_mat[i]; m_mat[i] = 0; }
	if (m_maxSize == NO_FIXED_SIZE) m_mat.clear();
}

//-----------------------------------------------------------------------------
void FEMaterialProperty::AddMaterial(FEMaterial* pm)
{
	if (pm) pm->SetParentMaterial(m_parent);
	if (m_maxSize == NO_FIXED_SIZE)
		m_mat.push_back(pm);
	else
	{
		// find a zero component
		for (int i=0; i<(int)m_mat.size(); ++i)
		{
			if (m_mat[i] == 0) { m_mat[i] = pm; return; }
		}
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void FEMaterialProperty::SetMaterial(FEMaterial* pm, int i) 
{ 
	if (pm) assert(pm->ClassID() & m_nClassID);
	if (pm) pm->SetParentMaterial(m_parent);
	if (m_mat.empty() == false)
	{
		if (m_mat[i] != pm)
		{
			delete m_mat[i];
			m_mat[i] = pm;
		}
	}
}

//-----------------------------------------------------------------------------
// remove a material from the list (returns false if pm is not part of the list)
bool FEMaterialProperty::RemoveMaterial(FEMaterial* pm)
{
	// find the material
	for (int i=0; i<(int)m_mat.size(); ++i)
	{
		if (m_mat[i] == pm)
		{
			m_mat.erase(m_mat.begin() + i);
			delete pm;
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
FEMaterial* FEMaterialProperty::GetMaterial(int i)
{	
	if ((i<0) || (i>=(int)m_mat.size())) return 0;
	return m_mat[i]; 
}

//-----------------------------------------------------------------------------
int FEMaterialProperty::GetMaterialIndex(FEMaterial* mat)
{
	for (int i=0; i<(int)m_mat.size(); ++i)
	{
		if (m_mat[i] == mat) return i;
	}
	return -1;
}

//=============================================================================
// FEMaterial
//=============================================================================

//-----------------------------------------------------------------------------
FEMaterial::FEMaterial(int ntype) : m_ntype(ntype)
{
	m_naopt = -1;
	m_parent = 0;
	m_owner = 0;
}

//-----------------------------------------------------------------------------
FEMaterial::~FEMaterial()
{
	ClearProperties();
}

//-----------------------------------------------------------------------------
int FEMaterial::ClassID()
{
	FEMaterialFactory& MF = *FEMaterialFactory::GetInstance();
	return MF.ClassID(this);
}

//-----------------------------------------------------------------------------
const char* FEMaterial::TypeStr()
{
	FEMaterialFactory& MF = *FEMaterialFactory::GetInstance();
	return MF.TypeStr(this);
}

//-----------------------------------------------------------------------------
void FEMaterial::SetParentMaterial(FEMaterial* pmat)
{
	assert((m_parent==0) || (m_parent == pmat));
	m_parent = pmat;
}

//-----------------------------------------------------------------------------
const FEMaterial* FEMaterial::GetParentMaterial() const
{
	return m_parent;
}

//-----------------------------------------------------------------------------
const FEMaterial* FEMaterial::GetAncestor() const
{
	if (m_parent) return m_parent->GetAncestor();
	return this;
}

//-----------------------------------------------------------------------------
GMaterial* FEMaterial::GetOwner() const 
{ 
	return m_owner; 
}

//-----------------------------------------------------------------------------
void FEMaterial::SetOwner(GMaterial* owner)
{
	m_owner = owner;
}

//-----------------------------------------------------------------------------
// delete all material properties
void FEMaterial::ClearProperties()
{
	vector<FEMaterialProperty*>::iterator it;
	for (it = m_Mat.begin(); it != m_Mat.end(); ++it) (*it)->Clear();
}

//-----------------------------------------------------------------------------
// Add a component to the material
void FEMaterial::AddProperty(const std::string& name, int nClassID, int maxSize, unsigned int flags)
{
	FEMaterialProperty* m = new FEMaterialProperty(name, nClassID, this, maxSize, flags);
	m_Mat.push_back(m);
}

//-----------------------------------------------------------------------------
void FEMaterial::AddProperty(const std::string& name, FEMaterial* pm)
{
	FEMaterialProperty* p = FindProperty(name);
	assert(p);
	if (p) p->AddMaterial(pm);
}

//-----------------------------------------------------------------------------
int FEMaterial::AddProperty(int propID, FEMaterial* pm)
{
	FEMaterialProperty& p = GetProperty(propID);
	p.AddMaterial(pm);
	return (p.Size() - 1);
}

//-----------------------------------------------------------------------------
// Replace the material of a component
void FEMaterial::ReplaceProperty(int propID, FEMaterial* pm, int matID)
{
	m_Mat[propID]->SetMaterial(pm, matID);
}

//-----------------------------------------------------------------------------
FEMaterialProperty* FEMaterial::FindProperty(const std::string& name)
{
	int n = (int) m_Mat.size();
	for (int i=0; i<n; ++i)
	{
		FEMaterialProperty* pm = m_Mat[i];
		if (pm->GetName() == name) return pm;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// find the property by type
FEMaterialProperty* FEMaterial::FindProperty(int ntype)
{
	int n = (int)m_Mat.size();
	for (int i = 0; i<n; ++i)
	{
		FEMaterialProperty* pm = m_Mat[i];
		if (pm->GetClassID() == ntype) return pm;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// find the property by the material
FEMaterialProperty* FEMaterial::FindProperty(FEMaterial* pm)
{
	int NP = Properties();
	for (int i=0; i<NP; ++i)
	{
		FEMaterialProperty& p = GetProperty(i);
		int nmat = p.Size();
		for (int j=0; j<nmat; ++j)
		{
			if (p.GetMaterial(j) == pm) return &p;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
void FEMaterial::copy(FEMaterial* pm)
{
	// make sure these materials are the same
	assert(m_ntype == pm->m_ntype);

	// copy this material's parameters first
	GetParamBlock() = pm->GetParamBlock();

	// copy the individual components
	int NC = (int) pm->m_Mat.size();
	m_Mat.resize(NC);
	for (int i=0; i<NC; ++i)
	{
		FEMaterialProperty& mcd = GetProperty(i);
		FEMaterialProperty& mcs = pm->GetProperty(i);

		if (mcs.Size() == 1)
		{
			if (mcs.GetMaterial())
			{
				FEMaterial* pm = FEMaterialFactory::Create(mcs.GetMaterial()->Type());
				pm->copy(mcs.GetMaterial());
				mcd.SetMaterial(pm);
			}
			else mcs.SetMaterial(0);
		}
		else
		{
			for (int j=0; j<mcs.Size(); ++j)
			{
				FEMaterial* pmj = mcs.GetMaterial(j);
				FEMaterial* pm = FEMaterialFactory::Create(pmj->Type());
				pm->copy(pmj);
				mcd.AddMaterial(pm);
			}
		}
	}
}

//-----------------------------------------------------------------------------
mat3d FEMaterial::GetMatAxes(FEElementRef& el)
{
	switch (m_naopt)
	{
		case FE_AXES_LOCAL:
		{
			FECoreMesh* pm = el.m_pmesh;
			vec3d r1 = pm->Node(el->m_node[ m_n[0]-1 ]).r;
			vec3d r2 = pm->Node(el->m_node[ m_n[1]-1 ]).r;
			vec3d r3 = pm->Node(el->m_node[ m_n[2]-1 ]).r;
			vec3d a = r2 - r1;
			vec3d d = r3 - r1;
			vec3d c = a^d;
			vec3d b = c^a;
			a.Normalize();
			b.Normalize();
			c.Normalize();
			mat3d Q;
			Q.zero();
			Q[0][0] = a.x; Q[0][1] = b.x; Q[0][2] = c.x;
			Q[1][0] = a.y; Q[1][1] = b.y; Q[1][2] = c.y;
			Q[2][0] = a.z; Q[2][1] = b.z; Q[2][2] = c.z;
			
			return Q;
		}
		break;
		case FE_AXES_VECTOR:
		{
			vec3d a = m_a;
			vec3d d = m_d;
			vec3d c = a^d;
			vec3d b = c^a;
			a.Normalize();
			b.Normalize();
			c.Normalize();
			mat3d Q;
			Q.zero();
			Q[0][0] = a.x; Q[0][1] = b.x; Q[0][2] = c.x;
			Q[1][0] = a.y; Q[1][1] = b.y; Q[1][2] = c.y;
			Q[2][0] = a.z; Q[2][1] = b.z; Q[2][2] = c.z;
			
			return Q;
		}
	}
	
	assert(false);
	mat3d Q;
	Q.unit();
	return Q;
}

//-----------------------------------------------------------------------------
// Save the material data to the archive
void FEMaterial::Save(OArchive& ar)
{
	// save the name if there is one
	string name = GetName();
	if (name.empty() == false)
	{
		ar.WriteChunk(CID_MAT_NAME, name);
	}

	// save material parameters (if any)
	if (Parameters() > 0)
	{
		ar.BeginChunk(CID_MAT_PARAMS);
		{
			ParamContainer::Save(ar);
		}
		ar.EndChunk();
	}

	// write the material properties (if any)
	if (!m_Mat.empty())
	{
		ar.BeginChunk(CID_MAT_PROPERTY);
		{
			int n = (int) m_Mat.size();
			for (int i=0; i<n; ++i)
			{
				FEMaterialProperty& mpi = GetProperty(i);

				// store the property name
				ar.WriteChunk(CID_MAT_PROPERTY_NAME, mpi.GetName());

				// store the property data
				ar.BeginChunk(CID_MAT_PROPERTY_MAT);
				{
					for (int j = 0; j<mpi.Size(); ++j)
					{
						FEMaterial* pm = mpi.GetMaterial(j);
						if (pm)
						{
							ar.BeginChunk(pm->Type());
							{
								pm->Save(ar);
							}
							ar.EndChunk();
						}
					}
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
// Load the material data from the archive
void FEMaterial::Load(IArchive &ar)
{
	TRACE("FEMaterial::Load");

	char szname[256];
	while (IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_MAT_NAME: { ar.read(szname); SetName(szname); } break;
		case CID_MAT_PARAMS: ParamContainer::Load(ar); break;
        case CID_MAT_PROPERTY:
			{
				FEMaterialProperty* prop = 0;
				while (IO_OK == ar.OpenChunk())
				{
					int nid = (int) ar.GetChunkID();
					if (nid == CID_MAT_PROPERTY_NAME) 
					{
						ar.read(szname);
						prop = FindProperty(szname);
						assert(prop);
						prop->Clear();
					}
					else if (nid == CID_MAT_PROPERTY_MAT)
					{
						int n = 0;
						while (IO_OK == ar.OpenChunk())
						{
							int nid = ar.GetChunkID();

							FEMaterial* pm = FEMaterialFactory::Create(nid);
							assert(pm);
							pm->Load(ar);
							if (prop)
							{
								if (prop->maxSize() == FEMaterialProperty::NO_FIXED_SIZE)
									prop->AddMaterial(pm);
								else prop->SetMaterial(pm, n);
								n++;
							}

							ar.CloseChunk();
						}
					}
					else if (ar.Version() < 0x00020000)
					{
						// Note that some materials are considered obsolete. Since they are no longer registered
						// we need to check for them explicitly.
						FEMaterial* pm = 0;
						switch (nid)
						{
						case FE_TRANS_MOONEY_RIVLIN_OLD   : pm = new FETransMooneyRivlinOld; break;
						case FE_TRANS_VERONDA_WESTMANN_OLD: pm = new FETransVerondaWestmannOld; break;
						case FE_COUPLED_TRANS_ISO_MR_OLD  : pm = new FECoupledTransIsoMooneyRivlinOld; break;
						default:
							pm = FEMaterialFactory::Create(nid);
						}
						assert(pm);
						if (pm) 
						{
							pm->Load(ar);
							prop->AddMaterial(pm);
						}
					}
					ar.CloseChunk();
				}
			}
			break;
		}
		ar.CloseChunk();
	}
}
