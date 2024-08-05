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
#include "PLYExport.h"
#include <GeomLib/GObject.h>
#include <GeomLib/GModel.h>
#include <FEMLib/FSProject.h>

PLYExport::PLYExport(FSProject& prj) : FSFileExport(prj)
{

}

PLYExport::~PLYExport(void)
{

}

struct PLY_FACE {
	int	nodes;
	int	n[4];
};

bool PLYExport::Write(const char* szfile)
{
	// get the GModel
	GModel& mdl = m_prj.GetFSModel().GetModel();

	// the vertex and face list
	std::vector<vec3d> vertices; vertices.reserve(100000);
	std::vector<PLY_FACE> faces; faces.reserve(100000);
	
	int objs = mdl.Objects();
	for (int i = 0; i < objs; ++i)
	{
		FSMesh* mesh = mdl.Object(i)->GetFEMesh();
		if (mesh)
		{
			int n0 = vertices.size();
			int NN = mesh->Nodes();
			for (int j = 0; j < NN; ++j)
			{
				FSNode& node = mesh->Node(j);
				vec3d r = mesh->LocalToGlobal(node.r);
				vertices.push_back(r);
			}

			int NE = mesh->Elements();
			for (int j = 0; j < NE; ++j)
			{
				FSElement& el = mesh->Element(j);
				if ((el.Type() == FE_TRI3) || (el.Type() == FE_QUAD4))
				{
					PLY_FACE f;
					f.nodes = el.Nodes();
					for (int k = 0; k < el.Nodes(); ++k)
					{
						f.n[k] = n0 + el.m_node[k];
					}
					faces.push_back(f);
				}
			}
		}
	}

	// open the file
	FILE* fp = fopen(szfile, "wt");
	if (fp == nullptr) return false;

	// write the header
	fprintf(fp, "ply\n");
	fprintf(fp, "format ascii 1.0\n");
	fprintf(fp, "comment PLY file generated by PreView\n");
	fprintf(fp, "element vertex %d\n", (int)vertices.size());
	fprintf(fp, "property float x\n");
	fprintf(fp, "property float y\n");
	fprintf(fp, "property float z\n");
	fprintf(fp, "element face %d\n", (int)faces.size());
	fprintf(fp, "property list uchar int vertex_index\n");
	fprintf(fp, "end_header\n");

	// write the vertices
	for (int i = 0; i < vertices.size(); ++i)
	{
		vec3d& ri = vertices[i];
		fprintf(fp, "%lg %lg %lg\n", ri.x, ri.y, ri.z);
	}

	// write the faces
	for (int i = 0; i < faces.size(); ++i)
	{
		PLY_FACE& fi = faces[i];
		if (fi.nodes == 3)
			fprintf(fp, "3 %d %d %d\n", fi.n[0], fi.n[1], fi.n[2]);
		else
			fprintf(fp, "4 %d %d %d %d\n", fi.n[0], fi.n[1], fi.n[2], fi.n[3]);
	}

	fclose(fp);

	return true;
}
