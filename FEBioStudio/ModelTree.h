#pragma once
#include <QTreeWidget>
#include <vector>

class CModelViewer;
class CDocument;
class CPropertyList;
class FEModel;
class FEStep;
class FSObject;
class CObjectProps;
class FEMaterial;
class FEReactionMaterial;
class GMaterial;
class FEProject;
class FEComponent;

class CObjectValidator;

enum ModelTreeType
{
	MT_FEOBJECT,
	MT_OBJECT,
	MT_PART,
	MT_SURFACE,
	MT_EDGE,
	MT_NODE,
	MT_MATERIAL_LIST,
	MT_SOLUTES_LIST,
	MT_SOLUTE,
	MT_SBM_LIST,
	MT_SBM,
	MT_NAMED_SELECTION,
	MT_OBJECT_LIST,
	MT_PART_LIST,
	MT_FACE_LIST,
	MT_EDGE_LIST,
	MT_NODE_LIST,
	MT_BC_LIST,
	MT_LOAD_LIST,
	MT_IC_LIST,
	MT_CONTACT_LIST,
	MT_CONSTRAINT_LIST,
	MT_CONNECTOR_LIST,
	MT_DISCRETE_LIST,
	MT_DISCRETE_SET,
	MT_DISCRETE,
	MT_STEP_LIST,
	MT_PROJECT_OUTPUT,
	MT_PROJECT_OUTPUT_LOG,
	MT_PART_GROUP,
	MT_FACE_GROUP,
	MT_EDGE_GROUP,
	MT_NODE_GROUP,
	MT_BC,
	MT_LOAD,
	MT_IC,
	MT_CONTACT,
	MT_STEP,
	MT_CONSTRAINT,
	MT_CONNECTOR,
	MT_MATERIAL,
	MT_DATAMAP,
	MT_JOBLIST,
	MT_JOB,
	MT_POST_PLOT,
	MT_3DIMAGE,
	MT_3DIMAGE_RENDER
};

struct CModelTreeItem
{
	FSObject*			obj;	// the object
	CPropertyList*		props;	// the property list
	CObjectValidator*	val;	// the validator
	int					flag;	// 0 = list view, 1 = form view
	int					type;	// the object type
};

class CModelTree : public QTreeWidget
{
	Q_OBJECT

public:
	enum ModelFlags
	{
		SHOW_PROPERTY_FORM  = 1,	// show the properties in a form or in a list
		OBJECT_NOT_EDITABLE = 2,	// is the object associated with the item editable
		DUPLICATE_ITEM      = 4,	// this item is a duplicate
		NAME_NOT_EDITABLE   = 8		// the name cannot be edited
	};

public:
	CModelTree(CModelViewer* view, QWidget* parent = 0);

	// build the model tree from the document
	void Build(CDocument* doc);

	void ShowItem(QTreeWidgetItem* item);

	void Select(FSObject* po);
	void Select(const std::vector<FSObject*>& objList);

	void UpdateObject(FSObject* po);

	void contextMenuEvent(QContextMenuEvent* ev) override;

	int Items() const { return (int) m_data.size(); }
	CModelTreeItem& GetItem(int n) { return m_data[n]; }

	QTreeWidgetItem* FindItem(FSObject* o);

	bool GetSelection(std::vector<FSObject*>& sel);

protected:
	void ClearData();

	QTreeWidgetItem* AddTreeItem(QTreeWidgetItem* parent, const QString& name, int ntype = 0, int ncount = 0, FSObject* po = 0, CPropertyList* props = 0, CObjectValidator* val = 0, int flags = 0);

	void UpdateModelData (QTreeWidgetItem* t1, FEModel& fem);
	void UpdateObjects   (QTreeWidgetItem* t1, FEModel& fem);
	void UpdateGroups    (QTreeWidgetItem* t1, FEModel& fem);
	void UpdateBC        (QTreeWidgetItem* t1, FEModel& fem, FEStep* pstep);
	void UpdateLoads     (QTreeWidgetItem* t1, FEModel& fem, FEStep* pstep);
	void UpdateICs       (QTreeWidgetItem* t1, FEModel& fem, FEStep* pstep);
	void UpdateContact   (QTreeWidgetItem* t1, FEModel& fem, FEStep* pstep);
	void UpdateRC        (QTreeWidgetItem* t1, FEModel& fem, FEStep* pstep);
	void UpdateConnectors(QTreeWidgetItem* t1, FEModel& fem, FEStep* pstep);
	void UpdateSteps     (QTreeWidgetItem* t1, FEModel& fem);
	void UpdateMaterials (QTreeWidgetItem* t1, FEModel& fem);
	void UpdateDiscrete  (QTreeWidgetItem* t1, FEModel& fem);
	void UpdateOutput    (QTreeWidgetItem* t1, FEProject& prj);
	void UpdateJobs      (QTreeWidgetItem* t1, CDocument* doc);
	void UpdateImages    (QTreeWidgetItem* t1, CDocument* doc);

	void AddMaterial(QTreeWidgetItem* item, const QString& name, GMaterial* gmat, FEMaterial* pmat, FEModel& fem, bool topLevel);
	void AddReactionMaterial(QTreeWidgetItem* item, FEReactionMaterial* mat, FEModel& fem);

	void AddDataMaps(QTreeWidgetItem* t1, FEComponent* pc);

	CModelTreeItem* GetCurrentData();

private:
	std::vector<CModelTreeItem>	m_data;
	CModelViewer*				m_view;

	friend class CModelViewer;
};
