#pragma once
#include <MathLib/math3d.h>
#include <MathLib/Interpolator.h>
#include <string>

//-----------------------------------------------------------------------------
class GLCameraTransform
{
public:
	GLCameraTransform() {}
	GLCameraTransform(const GLCameraTransform& key);
	GLCameraTransform& operator = (const GLCameraTransform& key);

public:
	std::string	name;
	vec3d		pos;	// position
	vec3d		trg;	// target
	quatd		rot;	// rotation
};

//=============================================================================
// This class implements a camera that can be used to navigate a 3D world.
// It uses the interpolater class to allow animatable transistions between
// two viewpoints.
class CGLCamera  
{
public:
	// constructor/destructor
	CGLCamera();

	//! destructor
	virtual ~CGLCamera();

	// reset the camera
	void Reset();

	// set the GL transformation matrix
	void Transform();

	// update camera position (for animations)
	void Update(bool bhit = false);

	// set line-draw or decal mode
	void LineDrawMode(bool b) { m_bdecal = b; }

public:
	void SetCameraSpeed(double f);
	double GetCameraSpeed() { return m_speed; }

	void SetCameraBias(double f);
	double GetCameraBias() { return m_bias; }

public:
	// rotate aroun camera's axis
	void Pan(const quatd& q);

	// move camera forward or backward
	void Dolly(double f);

	// move camera in camera plane
	void Truck(vec3d& v);

	// rotate around target
	void Orbit(quatd& q);

	// zoom in or out
	void Zoom(double f);

public:
	// sets the distance to the target
	void SetTargetDistance(double z) { vec3d r = m_trg.Target(); r.z = z; m_trg.Target(r); }

	// gets the distance to the target
	double GetTargetDistance() { return m_trg.Value().z; }

	// gets the distance to the target
	double GetFinalTargetDistance() { return m_trg.Target().z; }

	// set the camera's target
	void SetTarget(const vec3d& r);

	// set the target in local coordinates
	void SetLocalTarget(const vec3d& r);

	// set the orientation of the camera
	void SetOrientation(quatd q) { m_rot.Target(q); }

	// get the camera's orientation
	quatd GetOrientation() { return m_rot.Value(); }

	// get the target position
	vec3d GetPosition() const { return m_pos.Value(); }

	vec3d FinalPosition() const { return m_pos.Target(); }

	vec3d Target() const { return m_trg.Value(); }
	vec3d FinalTarget() const { return m_trg.Target(); }

	// set the view direction
	void SetViewDirection(const vec3d& r);

	// set the camera transformation
	void SetTransform(GLCameraTransform& t);

	void GetTransform(GLCameraTransform& t);

	// see if the camera is still animating
	bool IsAnimating();

	// convert world coordinates to camera coordinates
	vec3d WorldToCam(vec3d r) const;

	// convert camera coordinates to world coordinates
	vec3d CamToWorld(vec3d r) const;

	// get the position in global coordinates
	vec3d GlobalPosition() const;

public:
	VecInterpolator		m_pos;	// position of target in global coordinates
	VecInterpolator		m_trg;	// position of target in local coordinates
	QuatInterpolator	m_rot;	// orientation of camera
	bool	m_bdecal;			// decal or line draw mode

private:
	double	m_speed;
	double	m_bias;
};
