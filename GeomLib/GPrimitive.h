#pragma once
#include "GObject.h"
#include <FSCore/ParamBlock.h>

class FEMesh;

//-----------------------------------------------------------------------------
// The GPrimitive class manages the parts, surfaces and nodesets automatically
// The user can not alter this data. This class assumes that all parts, surfaces,
// and nodesets are created by whatever class created the mesh. For example, since
// most procedural meshes automatically create their own partitions, they use
// auto-objects to manager their geometry. This is contrast for instance with the
// the GMeshObject which uses the FE mesh to define the geometry.

class GPrimitive : public GObject
{
public:
	GPrimitive(int ntype) : GObject(ntype) {}

	// get the editable mesh
	FEMeshBase* GetEditableMesh() override;
	FELineMesh* GetEditableLineMesh() override;

	// update the object's data
	bool Update(bool b = true) override;

	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

	GObject* Clone() override;
};

//-----------------------------------------------------------------------------
// Simple rectangular box
class GBox : public GPrimitive
{
public:
	enum { WIDTH, HEIGHT, DEPTH };
	double	m_w, m_h, m_d;

public:
	GBox();
	bool Update(bool b = true);

private:
	void Create();
};

//-----------------------------------------------------------------------------
// Tapered cone
class GCone : public GPrimitive
{
public:
	enum {R0, R1, H};
	double	m_R0, m_R1, m_h;

public:
	GCone();
	bool Update(bool b = true);

private:
	void Create();
	void BuildGMesh();
};

//-----------------------------------------------------------------------------
// Circular cylinder
class GCylinder : public GPrimitive
{
public:
	enum {RADIUS, HEIGHT};
	double	m_R, m_h;

public:
	GCylinder();
	bool Update(bool b = true);

private:
	void Create();
};

//-----------------------------------------------------------------------------
// Ellipsoidal cylinder
class GCylinder2 : public GPrimitive
{
public:
	enum {RADIUSX, RADIUSY, HEIGHT};
	double	m_Rx, m_Ry, m_h;

public:
	GCylinder2();
	bool Update(bool b = true);

private:
	void Create();
};

//-----------------------------------------------------------------------------
// hollow concentric sphere
class GHollowSphere: public GPrimitive
{
public:
	enum {RIN, ROUT};
	double	m_Ri, m_Ro;

public:
	GHollowSphere();
	bool Update(bool b = true);

private:
	void Create();
	void BuildGMesh();
	int NodeIndex(int i, int j, int ND, int NZ);
};

//-----------------------------------------------------------------------------
// hollow truncated concentric ellipsoid
class GTruncatedEllipsoid: public GPrimitive
{
public:
	enum {RA, RB, RC, WT, VEND};
	double	m_Ra, m_Rb, m_Rc;	// ellipsoid radii
	double	m_wt;				// wall thickness
	double	m_vend;				// end angle

public:
	GTruncatedEllipsoid();
	bool Update(bool b = true);

private:
	void Create();
	void BuildGMesh();
	int NodeIndex(int i, int j, int NS);
};

//-----------------------------------------------------------------------------
// sphere
class GSphere : public GPrimitive
{
public:
	enum {RADIUS};
	double	m_R;

public:
	GSphere();
	bool Update(bool b = true);

private:
	void Create();
	void BuildGMesh();
	int NodeIndex(int i, int j, int ND, int NZ);
};

//-----------------------------------------------------------------------------
// circular torus
class GTorus : public GPrimitive
{
public:
	enum {RIN, ROUT};
	double	m_R0, m_R1;

public:
	GTorus();
	bool Update(bool b = true);

private:
	void Create();
};

//-----------------------------------------------------------------------------
// box with a cylindrical cavity
class GCylinderInBox : public GPrimitive
{
public:
	enum {WIDTH, HEIGHT, DEPTH, RADIUS};
	double	m_W, m_H, m_D, m_R;

public:
	GCylinderInBox();
	bool Update(bool b = true);

private:
	void Create();
};

//-----------------------------------------------------------------------------
// box with a spherical cavity
class GSphereInBox : public GPrimitive
{
public:
	enum {WIDTH, HEIGHT, DEPTH, RADIUS};

public:
	GSphereInBox();
	bool Update(bool b = true);

private:
	void Create();
};

//-----------------------------------------------------------------------------
// Tube, that is a hollow cylinder
class GTube : public GPrimitive
{
public:
	enum {RIN, ROUT, HEIGHT};
	double	m_Ri, m_Ro, m_h;

public:
	GTube();
	bool Update(bool b = true);

private:
	void Create();
};

//-----------------------------------------------------------------------------
// Elliptical Tube, that is a hollow cylinder
class GTube2 : public GPrimitive
{
public:
	enum {RINX, RINY, ROUTX, ROUTY, HEIGHT};
	double	m_Rix, m_Riy, m_Rox, m_Roy, m_h;

public:
	GTube2();
	bool Update(bool b = true);

private:
	void Create();
};

//-----------------------------------------------------------------------------
// Cylindrical slice
class GSlice : public GPrimitive
{
public:
	enum {RADIUS, HEIGHT, ANGLE};
	double	m_R, m_H, m_w;

public:
	GSlice();
	bool Update(bool b = true);

private:
	void Create();
};

//-----------------------------------------------------------------------------
//! Quarter symmetry dog-bone
class GQuartDogBone : public GPrimitive
{
public:
	enum { CWIDTH, CHEIGHT, RADIUS, LENGTH, DEPTH, WING };

public:
	GQuartDogBone();
	bool Update(bool b = true);

protected:
	void Create();
};

//-----------------------------------------------------------------------------
//! a 3D Solid Arc 
class GSolidArc : public GPrimitive
{
public:
	enum { RIN, ROUT, HEIGHT, ARC };

public:
	GSolidArc();
	bool Update(bool b = true);

protected:
	void Create();
};

//-----------------------------------------------------------------------------
class GHexagon : public GPrimitive
{
public:
	enum { RADIUS, HEIGHT };

public:
	GHexagon();
	bool Update(bool b = true);

protected:
	void Create();
};

//-----------------------------------------------------------------------------
// 2D circular disc
class GDisc : public GPrimitive
{
public:
	enum {RADIUS};

public:
	GDisc();
	bool Update(bool b = true);

protected:
	void Create();
};

//-----------------------------------------------------------------------------
// 2D rectangular patch
class GPatch : public GPrimitive
{
public:
	enum {W, H};
	double	m_w, m_h;

public:
	GPatch();
	bool Update(bool b = true);

private:
	void Create();
};

//-----------------------------------------------------------------------------
// 2D ring
class GRing : public GPrimitive
{
public:
	enum {RIN, ROUT};
	double	m_Ri, m_Ro;

public:
	GRing();
	bool Update(bool b = true);

private:
	void Create();
};

//-----------------------------------------------------------------------------
// a shell tube (cylinder without capped ends)
class GThinTube  : public GPrimitive
{
public:
	enum {RAD, H};
	double	m_R, m_h;

public:
	GThinTube();
	bool Update(bool b = true);

private:
	void Create();
};

//-----------------------------------------------------------------------------
// Gregory patch
class GGregoryPatch : public GPrimitive
{
public:
	GGregoryPatch(FEMesh* pm) : GPrimitive(GGREGORY_PATCH) { SetFEMesh(pm); }

public:
	void UpdateMesh();
};
