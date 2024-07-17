/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
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

#include "stdafx.h"
#include "3PointAngleTool.h"
#include <GLLib/GDecoration.h>
#include <MeshLib/FEMesh.h>

//-----------------------------------------------------------------------------
class C3PointAngleDecoration : public GDecoration
{
public:
	C3PointAngleDecoration()
	{
		point[0] = new GPointDecoration(vec3f(0,0,0));
		point[1] = new GPointDecoration(vec3f(0,0,0));
		point[2] = new GPointDecoration(vec3f(0,0,0));
		line[0] = new GLineDecoration(point[0], point[1]);
		line[1] = new GLineDecoration(point[1], point[2]);
		arc = nullptr;
		setVisible(false);
	}

	~C3PointAngleDecoration()
	{
		delete line[1]; 
		delete line[0];
		delete point[2];
		delete point[1];
		delete point[0];
		if (arc) delete arc;
	}

	void setPosition(const vec3f& a, const vec3f& b, const vec3f& c)
	{
		point[0]->setPosition(a);
		point[1]->setPosition(b);
		point[2]->setPosition(c);

		if (arc) delete arc;
		arc = new GArcDecoration(b, a, c);
	}

	void render()
	{
		point[0]->render();
		point[1]->render();
		point[2]->render();
		line[0]->render();
		line[1]->render();
		if (arc) arc->render();
	}

private:
	GPointDecoration*	point[3];
	GLineDecoration*	line[2];
	GArcDecoration*		arc;
};

//-----------------------------------------------------------------------------
C3PointAngleTool::C3PointAngleTool(CMainWindow* wnd) : CBasicTool(wnd, "3 Point Angle")
{
	addProperty("node 1", CProperty::Int);
	addProperty("node 2", CProperty::Int);
	addProperty("node 3", CProperty::Int);
	addProperty("angle", CProperty::Float)->setFlags(CProperty::Visible);

	m_node[0] = 0;
	m_node[1] = 0;
	m_node[2] = 0;
	m_angle = 0.0;

	SetInfo("Calculates the angle (in degrees) defined by 3 nodes.");
}

//-----------------------------------------------------------------------------
QVariant C3PointAngleTool::GetPropertyValue(int i)
{
	switch (i)
	{
	case 0: return m_node[0]; break;
	case 1: return m_node[1]; break;
	case 2: return m_node[2]; break;
	case 3: return m_angle; break;
	}
	return QVariant();
}

//-----------------------------------------------------------------------------
void C3PointAngleTool::SetPropertyValue(int i, const QVariant& v)
{
	if (i == 0) m_node[0] = v.toInt();
	if (i == 1) m_node[1] = v.toInt();
	if (i == 2) m_node[2] = v.toInt();
}

//-----------------------------------------------------------------------------
void C3PointAngleTool::addPoint(int n)
{
	if (n <= 0) return;

	// see if we have this point already
	for (int i = 0; i < 3; ++i) if (m_node[i] == n) return;

	// we don't so add it to the back
	if (m_node[2] == 0)
	{
		int m = 2;
		while ((m > 0) && (m_node[m - 1] == 0)) m--;
		m_node[m] = n;
	}
	else
	{
		m_node[0] = m_node[1];
		m_node[1] = m_node[2];
		m_node[2] = n;
	}
}

//-----------------------------------------------------------------------------
void C3PointAngleTool::Update()
{
	SetDecoration(nullptr);
	m_angle = 0.0;

	FSMesh* mesh = GetActiveMesh();
	if (mesh == nullptr) return;

	int nsel = 0;
	int N = mesh->Nodes();
	for (int i = 0; i<N; ++i)
	{
		FSNode& node = mesh->Node(i);
		if (node.IsSelected())
		{
			nsel++;
			int nid = i+1;
			addPoint(nid);
		}
	}

	if (nsel == 0)
	{
		m_node[0] = m_node[1] = m_node[2] = 0;
	}

	if ((m_node[0]>0)&& (m_node[1]>0)&& (m_node[2]>0))
	{
		vec3f a = to_vec3f(mesh->NodePosition(m_node[0]-1));
		vec3f b = to_vec3f(mesh->NodePosition(m_node[1]-1));
		vec3f c = to_vec3f(mesh->NodePosition(m_node[2]-1));

		vec3f e1 = a - b; e1.Normalize();
		vec3f e2 = c - b; e2.Normalize();

		m_angle = 180.0*acos(e1*e2) / PI;

		C3PointAngleDecoration* deco = new C3PointAngleDecoration;
		deco->setPosition(a, b, c);
		SetDecoration(deco);
	}
	else if ((m_node[0] > 0) && (m_node[1] > 0))
	{
		vec3f a = to_vec3f(mesh->NodePosition(m_node[0] - 1));
		vec3f b = to_vec3f(mesh->NodePosition(m_node[1] - 1));

		GLineDecoration* deco = new GLineDecoration(a,  b);
		SetDecoration(deco);
	}
}
