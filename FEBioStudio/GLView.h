#pragma once
#include <QOpenGLWidget>
#include <QNativeGestureEvent>
#include <GLLib/GLCamera.h>
#include "CommandManager.h"
#include "GManipulator.h"
#include "GTriad.h"
#include "GGrid.h"
#include <MeshLib/Intersect.h>
#include "GLViewRender.h"
#include <MeshTools/FEExtrudeFaces.h>
#include <GLWLib/GLWidgetManager.h>
#include <PostLib/Animation.h>

class CMainWindow;
class CDocument;
class GCurveMeshObject;
class CPostDoc;
class GMaterial;

// coordinate system modes
#define COORD_GLOBAL	0
#define COORD_LOCAL		1
#define COORD_SCREEN	2

//-----------------------------------------------------------------------------
// Animation modes
enum ANIMATION_MODE {
	ANIM_RECORDING,
	ANIM_PAUSED,
	ANIM_STOPPED
};

// preset views
enum View_Mode {
	VIEW_USER,
	VIEW_TOP,
	VIEW_BOTTOM,
	VIEW_LEFT,
	VIEW_RIGHT,
	VIEW_FRONT,
	VIEW_BACK
};

// view conventions
enum View_Convention {
    CONV_FR_XZ,
    CONV_FR_XY,
    CONV_US_XY
};

// snap modes
enum Snap_Mode
{
	SNAP_NONE,
	SNAP_GRID
};

//-----------------------------------------------------------------------------
// tag structure
struct GLTAG
{
	char	sztag[32];	// name of tag
	float	wx, wy;		// window coordinates for tag
	vec3d	r;			// world coordinates of tag
	bool	bvis;		// is the tag visible or not
	int		ntag;		// tag value
};

//-----------------------------------------------------------------------------
class SelectRegion
{
public:
	SelectRegion(){}
	virtual ~SelectRegion(){}

	virtual bool IsInside(int x, int y) const = 0;

	// see if a line intersects this region
	// default implementation only checks if one of the end points is inside.
	// derived classes should provide a better implementation.
	virtual bool LineIntersects(int x0, int y0, int x1, int y1) const;

	// see if a triangle intersects this region
	// default implementation checks for line intersections
	virtual bool TriangleIntersect(int x0, int y0, int x1, int y1, int x2, int y2) const;
};

class BoxRegion : public SelectRegion
{
public:
	BoxRegion(int x0, int x1, int y0, int y1);
	bool IsInside(int x, int y) const;
	bool LineIntersects(int x0, int y0, int x1, int y1) const;
private:
	int	m_x0, m_x1;
	int	m_y0, m_y1;
};

class CircleRegion : public SelectRegion
{
public:
	CircleRegion(int x0, int x1, int y0, int y1);
	bool IsInside(int x, int y) const;
	bool LineIntersects(int x0, int y0, int x1, int y1) const;
private:
	int	m_xc, m_yc;
	int	m_R;
};

class FreeRegion : public SelectRegion
{
public:
	FreeRegion(vector<pair<int, int> >& pl);
	bool IsInside(int x, int y) const;
private:
	vector<pair<int, int> >& m_pl;
	int m_x0, m_x1;
	int m_y0, m_y1;
};

class CGLView : public QOpenGLWidget
{
	Q_OBJECT

public:
	CGLView(CMainWindow* pwnd, QWidget* parent = 0);
	~CGLView();

public:
	double GetGridScale() { return m_grid.GetScale(); }
	quatd GetGridOrientation() { return m_grid.m_q; }

	CGLCamera& GetCamera() { return m_Cam; }

	CDocument* GetDocument();

	GObject* GetActiveObject();

	void Reset();

	void SelectParts   (int x, int y);
	void SelectSurfaces(int x, int y);
	void SelectEdges   (int x, int y);
	void SelectNodes   (int x, int y);
	void SelectDiscrete(int x, int y);

	void HighlightEdge(int x, int y);

	void SelectObjects   (int x, int y);
	void SelectFEElements(int x, int y);
	void SelectFEFaces   (int x, int y);
	void SelectFEEdges   (int x, int y);
	void SelectFENodes   (int x, int y);
	bool SelectPivot(int x, int y);

	void RegionSelectObjects (const SelectRegion& region);
	void RegionSelectParts   (const SelectRegion& region);
	void RegionSelectSurfaces(const SelectRegion& region);
	void RegionSelectEdges   (const SelectRegion& region);
	void RegionSelectNodes   (const SelectRegion& region);
	void RegionSelectDiscrete(const SelectRegion& region);

	void RegionSelectFENodes(const SelectRegion& region);
	void RegionSelectFEFaces(const SelectRegion& region);
	void RegionSelectFEEdges(const SelectRegion& region);
	void RegionSelectFEElems(const SelectRegion& region);

	void SetModelView(GObject* po);

	void SetCoordinateSystem(int nmode)
	{
		m_coord = nmode;
	}

	void UndoViewChange()
	{
		if (m_Cmd.CanUndo()) m_Cmd.UndoCommand();
		repaint();
	}

	void RedoViewChange()
	{
		if (m_Cmd.CanRedo()) m_Cmd.RedoCommand();
		repaint();
	}

	void RenderTooltip(int x, int y);

	vec3d PickPoint(int x, int y, bool* success = 0);

	void SetViewMode(View_Mode n);
	View_Mode GetViewMode() { return m_nview; }

	void TogglePerspective(bool b);
	void ToggleDisplayNormals();

	bool OrhographicProjection() { return m_bortho; }

	double GetFOV() { return m_fov; }
	double GetAspectRatio() { return m_ar; }
	double GetNearPlane() { return m_fnear; }
	double GetFarPlane() { return m_ffar; }

	void GetViewport(int vp[4])
	{
		vp[0] = m_viewport[0];
		vp[1] = m_viewport[1];
		vp[2] = m_viewport[2];
		vp[3] = m_viewport[3];
	}

protected:
	void mousePressEvent  (QMouseEvent* ev);
	void mouseMoveEvent   (QMouseEvent* ev);
	void mouseReleaseEvent(QMouseEvent* ev);
	void mouseDoubleClickEvent(QMouseEvent* ev);
	void wheelEvent       (QWheelEvent* ev);
    bool gestureEvent     (QNativeGestureEvent* ev);
    bool event            (QEvent* event);

signals:
	void pointPicked(const vec3d& p);
	void selectionChanged();

public:
	//! Zoom out on current selection
	void ZoomSelection(bool forceZoom = true);

	//! zoom in on a box
	void ZoomTo(const BOX& box);

	//! Zoom in on an object
	void ZoomToObject(GObject* po);

	// zoom to the models extents
	void ZoomExtents(bool banimate = true);

	// prep the GL view for rendering
	void PrepModel();

	// setup the projection matrix
	void SetupProjection();

	// overridden from Glx_View
	void SetDefaultMaterial() { SetMatProps(0); }

	// get device pixel ration
	int GetDevicePixelRatio() { return m_dpr; }

	// set the GL material properties based on the material
	void SetMatProps(GMaterial* pm);

	// set some default GL material properties
	void SetDefaultMatProps();

	// render functions
public:
	// rendering functions for GObjects
	void RenderObject(GObject* po);
	void RenderParts(GObject* po);
	void RenderSurfaces(GObject* po);
	void RenderEdges(GObject* po);
	void RenderNodes(GObject* po);
	void RenderSelectedParts(GObject* po);
	void RenderSelectedSurfaces(GObject* po);
	void RenderSelectedEdges(GObject* po);
	void RenderSelectedNodes(GObject* po);

	// rendering functions for FEMeshes
	void RenderFEElements(GObject* po);
	void RenderFEAllElements(FEMesh* pm, bool bext = false);
	void RenderFEFaces(GObject* po);
	void RenderFEMeshFaces(GObject* po);
	void RenderFESurfaceMeshFaces(GObject* po);
	void RenderFEEdges(GObject* po);
	void RenderFENodes(GObject* po);
	void RenderMeshLines();
	void RenderMeshLines(GObject* pm);
	void RenderMeshLines(FEMeshBase* pm);
	void RenderFEFace(FEFace& face, FEMeshBase* pm);

	// other rendering functions
	void RenderDiscrete();
	void RenderBackground();
	void RenderNormals(GObject* po, double scale);
	void RenderFeatureEdges();
	void RenderModel();
	void RenderRubberBand();
	void RenderPivot(bool bpick = false);
	void RenderRigidJoints();
	void RenderRigidConnectors();
	void RenderRigidWalls();
	void RenderRigidBodies();
	void RenderMaterialFibers();
	void RenderLocalMaterialAxes();
	void Render3DCursor(const vec3d& r, double R);
	void RenderSelectionBox();
	void RenderDefaultView();
	void RenderPostView(CPostDoc* postDoc);
	void RenderTags();
	void RenderImageData();

	void ScreenToView(int x, int y, double& fx, double& fy);
	vec3d ScreenToGrid(int x, int y);
	vec3d GetViewDirection(double fx, double fy);

	void showSafeFrame(bool b);

	vec3d ViewToWorld(double fx, double fy);
	vec3d ViewToGrid(double fx, double fy);

	vec3d WorldToPlane(vec3d r);

	vec3d GetPickPosition();

	vec3d GetPivotPosition();
	quatd GetPivotRotation();

	void SetPivot(const vec3d& r);

	bool GetPivotMode() { return m_bpivot; }
	void SetPivotMode(bool b) { m_bpivot = b; }

	GLViewRender& GetViewRenderer() { return m_renderer; } 

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

private:
	void TagConnectedNodes(FEMeshBase* pm, int n);
	void TagNodesByShortestPath(FEMeshBase* pm, int n0, int n1);

	void SetSnapMode(Snap_Mode snap) { m_nsnap = snap; }
	Snap_Mode GetSnapMode() { return m_nsnap; }

	// convert from device pixel to physical pixel
	QPoint DeviceToPhysical(int x, int y)
	{
		return QPoint(m_dpr*x, m_viewport[3] - m_dpr*y);
	}

	void TagBackfacingFaces(FEMeshBase& mesh);
	void TagBackfacingNodes(FEMeshBase& mesh);
	void TagBackfacingEdges(FEMeshBase& mesh);
	void TagBackfacingElements(FEMesh& mesh);

public:
	QImage CaptureScreen();

	bool NewAnimation(const char* szfile, CAnimation* panim, GLenum fmt = GL_RGB);
	void StartAnimation();
	void StopAnimation();
	void PauseAnimation();
	void SetVideoFormat(GLenum fmt) { m_video_fmt = fmt; }

	ANIMATION_MODE AnimationMode() const;
	bool HasRecording() const;

	void UpdateWidgets(bool bposition = true);

protected:
	void PanView(vec3d r);

	void AddRegionPoint(int x, int y);

	void RenderGLProgress(CPostDoc* postDoc);

	bool TrackGLProgress(int x, CPostDoc* postDoc);

protected slots:
	void repaintEvent();

protected:
	CMainWindow*	m_pWnd;	// parent window
	CGLCamera		m_Cam;		// the camera
	CGLCamera		m_oldCam;	// the previous camera position
	GLViewRender	m_renderer; // the renderer for this view

	CCommandManager	m_Cmd;	// view command history

	GGrid		m_grid;		// the grid object

	vector<pair<int, int> >		m_pl;
	int			m_x0, m_y0, m_x1, m_y1;
	int			m_xp, m_yp;
	int			m_dxp, m_dyp;
	View_Mode	m_nview;
	Snap_Mode	m_nsnap;

	vec3d	m_rt;	// total translation
	vec3d	m_rg;

	double	m_st;	// total scale
	double	m_sa;	// accumulated scale
	vec3d	m_ds;	// direction of scale

	double	m_wt;	// total rotation
	double	m_wa;	// total accumulated rotation

	bool	m_bshift;
	bool	m_bctrl;
	bool	m_bsel;		// selection mode
	bool	m_bextrude;	// extrusion mode

	bool	m_btooltip;	// show tooltips

	bool	m_bortho;

	int		m_pivot;	// pivot selection mode

public:
	bool	m_bpick;

protected:
	FEExtrudeFaces*	m_pmod;

	double	m_fnear;
	double	m_ffar;
	double	m_fov;
	double	m_ar;

	double	m_ox;
	double	m_oy;

	bool	m_bsnap;	// snap to grid

	int		m_coord;	// coordinate system

	bool	m_bpivot;	// user-pivot = true
	vec3d	m_pv;		// pivot point

	// manipulators
	GTranslator		m_Ttor;	//!< the translate manipulator
	GRotator		m_Rtor;	//!< the rotate manipulator
	GScalor			m_Stor;	//!< the scale manipulator

	// triad
	GLBox*			m_ptitle;
	GLBox*			m_psubtitle;
	GLTriad*		m_ptriad;
	GLSafeFrame*	m_pframe;
	char			m_szsubtitle[256];

	CGLWidgetManager*	m_Widget;

	bool	m_drawGLProgress;
	int		m_GLProgressHeight;
	bool	m_trackGLProgress;

private:
	GLenum	m_video_fmt;

	ANIMATION_MODE	m_nanim;	// the animation mode
	CAnimation*		m_panim;	// animation object

private:
	int	m_viewport[4];		//!< store viewport coordinates
	int m_dpr;				//!< device pixel ratio for converting from physical to device-independent pixels
};

bool intersectsRect(const QPoint& p0, const QPoint& p1, const QRect& rt);
