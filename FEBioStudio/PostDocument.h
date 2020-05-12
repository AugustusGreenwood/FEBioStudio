#pragma once
#include "Document.h"
#include <GLLib/GView.h>
#include <PostLib/FEMaterial.h>
#include "GraphData.h"

class CModelDocument;
class CPostObject;

namespace Post {
	class FEPostModel;
	class CPalette;
	class FEFileReader;
}

// Timer modes
#define MODE_FORWARD	0
#define MODE_REVERSE	1
#define MODE_CYLCE		2

//-----------------------------------------------------------------------------
struct TIMESETTINGS
{
	int		m_mode;		// play mode
	double	m_fps;		// frames per second
	int		m_start;	// start time
	int		m_end;		// end time
	int		m_inc;		// used when MODE_CYCLE
	bool	m_bloop;	// loop or not
	bool	m_bfix;		// use a fixed time step
	double	m_dt;		// fixed time step size

	void Defaults();
};

//-----------------------------------------------------------------------------
// model data which is used for file updates
class ModelData
{
private:
	struct MODEL
	{
		int		m_ntime;
		bool	m_bnorm;	// calculate normals or not
		bool	m_boutline;	// render as outline
		bool	m_bghost;	// render ghost
		bool	m_bShell2Hex; // render shells as hexes
		int		m_nshellref;	// shell reference surface
		int		m_nDivs;	// nr of element subdivisions
		int		m_nrender;	// render mode
		double	m_smooth;	// smoothing angle
	};

	struct COLORMAP
	{
		bool	m_bactive;
		int		m_ntype;
		int		m_ndivs;
		bool	m_bsmooth;	// smooth gradient or not
		float	m_min;
		float	m_max;
		float	m_user[2];	// user range
		bool	m_bDispNodeVals;	// render nodal values
		int		m_nRangeType;	// range type
		int		m_nField;
	};

	struct DISPLACEMENTMAP
	{
		int		m_nfield;	// vector field defining the displacement
		float	m_scale;	// displacement scale factor
	};

public:
	ModelData();
	void ReadData(Post::CGLModel* po);
	void WriteData(Post::CGLModel* po);

	bool IsValid() const;

protected:
	bool		m_isValid;
	MODEL						m_mdl;	// CGLModel data
	COLORMAP					m_cmap;	// CColorMap data
	DISPLACEMENTMAP				m_dmap;	// DisplacementMap data
	std::vector<Post::FEMaterial>		m_mat;	// material list
	std::vector<std::string>	m_data;	// data field strings
};


class CPostDocument : public CDocument
{
public:
	CPostDocument(CMainWindow* wnd, CModelDocument* parent = nullptr);
	~CPostDocument();

	void Clear() override;

	bool Initialize() override;

	GObject* GetActiveObject() override;

	bool IsValid();

public:
	int GetStates();

	void SetActiveState(int n);

	int GetActiveState();

	void SetDataField(int n);

	Post::FEPostModel* GetFEModel();

	Post::CGLModel* GetGLModel();

	CPostObject* GetPostObject();

	TIMESETTINGS& GetTimeSettings();

	std::string GetFieldString();

	float GetTimeValue();

	float GetTimeValue(int n);

	void SetCurrentTimeValue(float ftime);

	void UpdateAllStates();

	void UpdateFEModel(bool breset = false);

	int GetEvalField();

	void ActivateColormap(bool bchecked);

	void DeleteObject(Post::CGLObject* po);

	CGView* GetView();

	std::string GetFileName();

	// get the model's bounding box
	BOX GetBoundingBox();

	// get the selection bounding box
	BOX GetSelectionBox();

public:
	int Graphs() const;
	void AddGraph(const CGraphData& data);
	const CGraphData* GetGraphData(int i);
	int FindGraphData(const CGraphData* data);
	void ReplaceGraphData(int n, const CGraphData& data);
	void DeleteGraph(const CGraphData* data);

private:
	void ApplyPalette(const Post::CPalette& pal);

private:
	CModelDocument*	m_doc;

private:
	Post::CGLModel*		m_glm;
	Post::FEPostModel*	m_fem;
	CGView				m_view;
	std::string			m_fileName;

	std::vector<CGraphData*>	m_graphs;

	ModelData	m_MD;

	CPostObject*	m_postObj;

	TIMESETTINGS m_timeSettings;
};
