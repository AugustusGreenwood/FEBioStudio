/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#pragma once
#include <MathLib/math3d.h>
#include <FSCore/color.h>

//-----------------------------------------------------------------------------
//! view settings
struct VIEW_SETTINGS
{
	bool	m_bcull;	//!< cull backface flag
	bool	m_bconn;	//!< select connected
	bool	m_bmax;		//!< max angle constraint for select connected
	bool	m_bpart;	//!< respect partition boundaries flag
	bool	m_bhide;	//!< auto hide
	bool	m_bext;		//!< ignore interior nodes
	bool	m_bsoft;	//!< soft selection mode
	bool	m_bcullSel;	//!< ignore backfacing when selecting

	bool	m_bselpath;	//!< select by shortest path

	float	m_fconn;	//!< max connectivity angle (for selecting faces)

	bool	m_bgrid;	//!< show grid flag
	bool	m_bmesh;	//!< show edged faces
	bool	m_bfeat;	//!< show feature edges
	bool	m_bnorm;	//!< show face normals
	double	m_scaleNormals;	//!< scale factor for normals
	int		m_nrender;	//!< render mode
	int     m_nconv;    //!< multiview projection convention

	int		m_apply;	//!< emulate apply via middle mouse button

	bool	m_bjoint;	//!< show rigid joints
	bool	m_bwall;	//!< show rigid walls
	bool	m_brigid;	//!< show rigid kinematics
	bool	m_bfiber;	//!< show material fibers
	bool	m_bcontour;	//!< show contour plot
	bool	m_blma;		//!< show local material axes
	double	m_fiber_scale;	//!< scale factor for rendering fibers
	bool	m_showHiddenFibers;	//!< show fibers/axes on hidden parts
	bool	m_showDiscrete;		//!< render discrete sets
	bool	m_showRigidLabels;	//!< show labels on rigid bodies

	GLColor	m_col1, m_col2;		//!< background colors
	GLColor	m_fgcol;			//!< foreground color
	GLColor	m_mcol;				//!< mesh line color
	int		m_nbgstyle;			//!< back ground style
	float	m_node_size;		//!< size of nodes when displayed
	float	m_line_size;		//!< line size
	bool	m_bline_smooth;		//!< line smoothing flag
	bool	m_bpoint_smooth;	//!< point smoothing flag
	bool	m_bzsorting;

	bool	m_snapToGrid;		//!< snap to grid
	bool	m_snapToNode;		//!< snap to nodes

	vec3d	m_pos3d;	// The location of the 3D cursor

	bool	m_bTags;
	int		m_ntagInfo;	// amount of info shown on tags

	// lighting settings
	bool	m_bLighting;	// use lighting or not
	bool	m_bShadows;		// use shadows or not
	float	m_shadow_intensity;	// shadow intensity
	float	m_ambient;		// scene light ambient intensity
	float	m_diffuse;		// scene light diffuse inentisty

	// object appearance
	int		m_transparencyMode;		// 0 = off, 1 = selected only, 2 = unselected only
	int		m_objectColor;			// 0 = default (by material), 1 = by object

	void Defaults(int ntheme = 0);
};

//-----------------------------------------------------------------------------
// view state
// This stores the variables that define the state of the UI 
struct VIEW_STATE
{
	int		nselect;	// selection mode
	int		nstyle;		// selection style
	int		ntrans;		// transform mode
	int		nitem;		// modify mode
};
