#include "stdafx.h"
#include "GLCamera.h"
#ifdef WIN32
#include <Windows.h>
#include <GL/gl.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif
#ifdef LINUX
#include <GL/gl.h>
#endif
#include "glx.h"

//=============================================================================
GLCameraTransform::GLCameraTransform(const GLCameraTransform& key)
{
	pos = key.pos;
	trg = key.trg;
	rot = key.rot;
	name = key.name;
}

GLCameraTransform& GLCameraTransform::operator = (const GLCameraTransform& key)
{
	pos = key.pos;
	trg = key.trg;
	rot = key.rot;
	name = key.name;
	return *this;
}

//=============================================================================
CGLCamera::CGLCamera()
{
	Reset();
}

//-----------------------------------------------------------------------------
CGLCamera::~CGLCamera()
{

}

//-----------------------------------------------------------------------------
void CGLCamera::Reset()
{
	SetCameraSpeed(0.8f);
	SetCameraBias(0.8f);
	m_rot.Target(quatd(0, vec3d(1,0,0)));
	m_pos.Target(vec3d(0,0,0));
	m_trg.Target(vec3d(0,0,0));
	Update(true);

	m_bdecal = false;
}

//-----------------------------------------------------------------------------
void CGLCamera::SetCameraSpeed(double f)
{
	if (f > 1.0) f = 1.0;
	if (f < 0.0) f = 0.0;
	m_speed = f;
	Interpolator::m_nsteps = 5 + (int)((1.0 - f)*60.0);
}

//-----------------------------------------------------------------------------
void CGLCamera::SetCameraBias(double f)
{
	if (f > 1.f) f = 1.f;
	if (f < 0.f) f = 0.f;
	m_bias = f;
	Interpolator::m_smooth = 0.5f + f*0.45f;
}

//-----------------------------------------------------------------------------
bool CGLCamera::IsAnimating()
{
	bool banim = false;
	banim |= m_pos.m_banim;
	banim |= m_trg.m_banim;
	banim |= m_rot.m_banim;
	return banim;
}

//-----------------------------------------------------------------------------
void CGLCamera::Update(bool bhit)
{
	if (bhit == false)
	{
		m_pos.Update();
		m_trg.Update();
		m_rot.Update();
	}
	else
	{
		m_pos.HitTarget();
		m_trg.HitTarget();
		m_rot.HitTarget();
	}
}

//-----------------------------------------------------------------------------
// This sets up the GL matrix transformation for rendering
void CGLCamera::Transform()
{
	// reset the modelview matrix mode
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// position the light
	GLfloat l1[] = {0.5f, 0.5f, 1.f, 0.f};
	glLightfv(GL_LIGHT0, GL_POSITION, l1);

//	GLfloat l2[] = {-0.5f, -0.3f, 0.f, 0.f};
//	glLightfv(GL_LIGHT1, GL_POSITION, l2);

	// target in camera coordinates
	vec3d r = Target();
	
	// zoom-in a little when in decal mode
	if (m_bdecal) r.z *= .999;

	// position the target in camera coordinates
	GLX::translate(-r);

	// orient the camera
	GLX::rotate(m_rot.Value());

	// translate to world coordinates
	GLX::translate(-GetPosition());
}

//-----------------------------------------------------------------------------
void CGLCamera::Pan(const quatd& q)
{

}

//-----------------------------------------------------------------------------
void CGLCamera::Dolly(double f)
{
	vec3d dr(0, 0, -GetFinalTargetDistance()*f);
	m_rot.Target().Inverse().RotateVector(dr);
	SetTarget(FinalPosition() + dr);
}

//-----------------------------------------------------------------------------
void CGLCamera::Truck(vec3d& v)
{
	vec3d dr(v);
	m_rot.Target().Inverse().RotateVector(dr);
	SetTarget(FinalPosition() + dr);
}

//-----------------------------------------------------------------------------
void CGLCamera::Orbit(quatd& q)
{
	quatd o = q*m_rot.Target();
	o.MakeUnit();
	m_rot.Target(o);
}

//-----------------------------------------------------------------------------
void CGLCamera::Zoom(double f)
{
	SetTargetDistance(GetFinalTargetDistance() * f);
}

//-----------------------------------------------------------------------------
void CGLCamera::SetTarget(const vec3d& r)
{
	m_pos.Target(r);
}

// set the target in local coordinates
void CGLCamera::SetLocalTarget(const vec3d& r)
{
	m_trg.Target(r);
}

void CGLCamera::SetViewDirection(const vec3d &r)
{
	if (r.Length() != 0.f)
	{
		m_rot.Target(quatd(vec3f(0, 0, 1.f), r).Inverse());
	}
}

void CGLCamera::SetTransform(GLCameraTransform& t)
{
	m_pos.Target(t.pos);
	m_trg.Target(t.trg);
	m_rot.Target(t.rot);
}

void CGLCamera::GetTransform(GLCameraTransform& t)
{
	t.pos = m_pos.Value();
	t.trg = m_trg.Value();
	t.rot = m_rot.Value();
}

//-----------------------------------------------------------------------------
vec3d CGLCamera::WorldToCam(vec3d r) const
{
	r += Target();

	quatd q = m_rot.Value().Inverse();

	q.RotateVector(r);

	r += GetPosition();

	return r;
}

//-----------------------------------------------------------------------------
vec3d CGLCamera::CamToWorld(vec3d r) const
{
	r -= GetPosition();
	m_rot.Value().RotateVector(r);
	r -= Target();

	return r;
}

//-----------------------------------------------------------------------------
// get the position in global coordinates
vec3d CGLCamera::GlobalPosition() const
{
	return WorldToCam(vec3d(0, 0, 0));
}
