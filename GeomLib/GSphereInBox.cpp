#include "GPrimitive.h"
#include <MeshTools/FESphereInBox.h>

//-----------------------------------------------------------------------------
GSphereInBox::GSphereInBox() : GPrimitive(GSPHERE_IN_BOX)
{
	AddDoubleParam(1.0, "w", "width" ); // width
	AddDoubleParam(1.0, "h", "height"); // height
	AddDoubleParam(1.0, "d", "depth"); // depth
	AddDoubleParam(0.25, "R", "radius"); // radius (of cavity)
	
	m_pMesher = new FESphereInBox(this);

	Create();
}

//-----------------------------------------------------------------------------
bool GSphereInBox::Update(bool b)
{
	double W = GetFloatValue(WIDTH);
	double H = GetFloatValue(HEIGHT);
	double D = GetFloatValue(DEPTH);
	double R = GetFloatValue(RADIUS);

	double w = W*0.5;
	double h = H*0.5;
	double d = D*0.5;
	double r = R*1.0/sqrt(3.0);

	double x[16] = {-w,  w,  w, -w, -w,  w, w, -w, -1,  1,  1, -1, -1,  1, 1, -1};
	double y[16] = {-h, -h,  h,  h, -h, -h, h,  h, -1, -1,  1,  1, -1, -1, 1,  1};
	double z[16] = {-d, -d, -d, -d,  d,  d, d,  d, -1, -1, -1, -1,  1,  1, 1,  1};

	for (int i=0; i<16; ++i)
	{
		GNode& n = *m_Node[i];
		n.LocalPosition() = vec3d(x[i], y[i], z[i]);
	}

	BuildGMesh();

	// project nodes onto surface
	GLMesh* pm = GetRenderMesh();

	// find all nodes for the inner surface and project to a sphere
	for (int i=0; i<pm->Nodes(); ++i) pm->Node(i).tag = 0;
	for (int i=0; i<pm->Faces(); ++i)
	{
		GMesh::FACE& f = pm->Face(i);
		if (f.pid >= 6)
		{
			pm->Node(f.n[0]).tag = 1;
			pm->Node(f.n[1]).tag = 1;
			pm->Node(f.n[2]).tag = 1;
			f.sid = 6;
		}
	}
	for (int i=0; i<pm->Nodes(); ++i)
	{
		GMesh::NODE& n = pm->Node(i);
		if (n.tag == 1)
		{
			n.r.Normalize();
			n.r *= R;
		}
	}

	// move all nodes up so that the primitive lies on the xy-plane
	for (int i=0; i<pm->Nodes(); ++i) pm->Node(i).r.z += d;

	for (int i=8; i<16; ++i)
	{
		GNode& n = *m_Node[i];
		vec3d newPos = n.LocalPosition() * r;
		n.LocalPosition() = newPos;
	}

	for (int i=0; i<16; ++i)
	{
		GNode& n = *m_Node[i];
		vec3d pos = n.LocalPosition();
		pos.z += d;
		n.LocalPosition() = pos;
	}

	pm->UpdateBoundingBox();
	pm->UpdateNormals();

	return true;
}

//-----------------------------------------------------------------------------
// Define the Box geometry.
void GSphereInBox::Create()
{
	int i;

	// 1. build the nodes
	//-------------------
	assert(m_Node.empty());
	for (i=0; i<16; ++i) AddNode(vec3d(0,0,0), NODE_VERTEX, true);

	// 2. build the edges
	//-------------------
	int ET[24][2] = {
		{0,1},{1, 2},{ 2, 3},{ 3,0},{ 4, 5},{ 5, 6},{ 6, 7},{ 7, 4},{0, 4},{1, 5},{ 2, 6},{ 3, 7},
		{8,9},{9,10},{10,11},{11,8},{12,13},{13,14},{14,15},{15,12},{8,12},{9,13},{10,14},{11,15}
	};
	assert(m_Edge.empty());
	for (i= 0; i<24; ++i) AddLine(ET[i][0], ET[i][1]);

	// 3. build the parts
	//-------------------
	assert(m_Part.empty());
	AddPart();

	// 4. build the faces
	//-------------------
	int FET[12][4] = {
		{ 0,  9,  4,  8}, { 1, 10,  5,  9}, { 2, 11,  6, 10}, { 3,  8,  7, 11}, { 0,  1,  2,  3}, { 4,  5,  6,  7},
		{12, 21, 16, 20}, {13, 22, 17, 21}, {14, 23, 18, 22}, {15, 20, 19, 23}, {12, 13, 14, 15}, {16, 17, 18, 19}
	};

	assert(m_Face.empty());
	vector<int> edge;
	edge.resize(4);
	for (i=0; i<12; ++i)
	{
		edge[0] = FET[i][0];
		edge[1] = FET[i][1];
		edge[2] = FET[i][2];
		edge[3] = FET[i][3];
		AddFacet(edge, FACE_QUAD);
	}

	Update();
}
