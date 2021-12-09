#pragma once
#include <FEMLib/FECoreMaterial.h>

class FSUserMaterial : public FSMaterial
{
	enum { SZTYPE, PARAMDATA, PARAMNAME, PARAMVALUE, PARAMTYPE };

public:
	FSUserMaterial(int ntype) : FSMaterial(ntype) {}
	~FSUserMaterial();

	void SetTypeString(const char* sz) override;
	const char* GetTypeString() override { return m_sztype; }

	void AddParameter(const char* szname, double v);

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	void copy(FSMaterial* pm);

protected:
	char			m_sztype[256];	// type name
	vector<char*>	m_pname;		// list of parameter names
};
