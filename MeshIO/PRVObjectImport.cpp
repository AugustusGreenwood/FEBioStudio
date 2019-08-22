#include "stdafx.h"
#include "PRVObjectImport.h"
#include <FSCore/Serializable.h>
#include "PRVObjectFormat.h"
#include <MeshTools/FEProject.h>
#include <GeomLib/GObject.h>
#include <MeshTools/GDiscreteObject.h>

//-----------------------------------------------------------------------------
PRVObjectImport::PRVObjectImport()
{
}

//-----------------------------------------------------------------------------
bool PRVObjectImport::Load(FEProject& prj, const char* szfile)
{
	if (Open(szfile, "rb") == false) return false;

	// open archive         P V O
	if (m_ar.Open(m_fp, 0x0050564F) == false) return false;

	bool ret = LoadObjects(m_ar, prj);

	Close();

	return ret;
}

void PRVObjectImport::Close()
{
	m_ar.Close();

	FileReader::Close();
}

bool PRVObjectImport::LoadObjects(IArchive& ar, FEProject& prj)
{
	GModel& model = prj.GetFEModel().GetModel();
	m_objList.clear();
	IOResult nret = IO_OK;
	while (ar.OpenChunk() == IO_OK)
	{
		int nid = ar.GetChunkID();

		if (nid == PVO_VERSION)
		{
			unsigned int version = 0;
			nret = ar.read(version);
			if (nret != IO_OK) return errf("Error reading version number");

			if (version != PVO_VERSION_NUMBER) return false;

			ar.SetVersion(version);
		}
		else if (nid == PVO_OBJECT)
		{
			GObject* po = LoadObject(ar, prj);
			if (po == 0) return false;

			model.AddObject(po);
			po = 0;
		}
		else if (nid == PVO_DISCRETE_OBJECT)
		{
			GDiscreteObject* po = LoadDiscreteObject(ar, prj);
			if (po == 0) return false;
			
			model.AddDiscreteObject(po);
			po = 0;			
		}
		ar.CloseChunk();
	}

	return true;
}

GObject* PRVObjectImport::LoadObject(IArchive& ar, FEProject& prj)
{
	GObject* po = 0;
	while (ar.OpenChunk() == IO_OK)
	{
		int nid = ar.GetChunkID();

		if (nid == PVO_OBJECT_TYPE)
		{
			int ntype = -1;
			if (ar.read(ntype) != IO_OK) return 0;
			po = BuildObject(ntype);
			if (po == 0) return 0;
		}
		else if (nid == PVO_OBJECT_DATA)
		{
			if (po == 0) return 0;
			po->Load(ar);

			// The problem is that the ID of the object items may already be in use.
			// Therefore we need to reinded all the items.
			// The old IDs will be stored in the tags in case we need to figure out dependencies
			ReindexObject(po);

			// add it to the list
			m_objList.push_back(po);
		}

		ar.CloseChunk();
	}

	return po;
}

GDiscreteObject* PRVObjectImport::LoadDiscreteObject(IArchive& ar, FEProject& prj)
{
	GDiscreteObject* po = 0;
	while (ar.OpenChunk() == IO_OK)
	{
		int nid = ar.GetChunkID();

		if (nid == PVO_OBJECT_TYPE)
		{
			int ntype = -1;
			if (ar.read(ntype) != IO_OK) return 0;

			switch (ntype)
			{
			case FE_LINEAR_SPRING_SET: po = new GLinearSpringSet; break;
			case FE_NONLINEAR_SPRING_SET: po = new GNonlinearSpringSet; break;
			}
			if (po == 0) return 0;
		}
		else if (nid == PVO_OBJECT_DATA)
		{
			if (po == 0) return 0;
			po->Load(ar);

			// This discrete object will still reference the old IDs of the objects in the pvo file.
			// Therefore we need to reindex them
			ReindexDiscreteObject(po);
		}

		ar.CloseChunk();
	}

	return po;
}

void PRVObjectImport::ReindexObject(GObject* po)
{
	for (int i=0; i<po->Parts(); ++i)
	{
		GPart* pg = po->Part(i);
		pg->m_ntag = pg->GetID();
		pg->SetID(GPart::CreateUniqueID());
	}

	for (int i = 0; i<po->Faces(); ++i)
	{
		GFace* pg = po->Face(i);
		pg->m_ntag = pg->GetID();
		pg->SetID(GFace::CreateUniqueID());
	}

	for (int i = 0; i<po->Edges(); ++i)
	{
		GEdge* pg = po->Edge(i);
		pg->m_ntag = pg->GetID();
		pg->SetID(GEdge::CreateUniqueID());
	}

	for (int i = 0; i<po->Nodes(); ++i)
	{
		GNode* pg = po->Node(i);
		pg->m_ntag = pg->GetID();
		pg->SetID(GNode::CreateUniqueID());
	}
}

void PRVObjectImport::ReindexDiscreteObject(GDiscreteObject* po)
{
	// This only works for discrete element sets
	GDiscreteElementSet* ps = dynamic_cast<GDiscreteElementSet*>(po);
	assert(ps);

	int NE = ps->size();
	for (int i=0; i<NE; ++i)
	{
		GDiscreteElement& de = ps->element(i);
		int n[2] = {-1, -1};
		for (int j=0; j<2; ++j)
		{
			int nj = de.Node(j);
			for (int k=0; k<m_objList.size(); ++k)
			{
				GObject* pk = m_objList[k];

				GNode* node = pk->FindNodeFromTag(nj); assert(node);
				if (node)
				{
					n[j] = node->GetID();
				}
			}
		}
		de.SetNodes(n[0], n[1]);
	}
}
