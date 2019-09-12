#include "FECoreMesh.h"

//-----------------------------------------------------------------------------
//! constructor
FECoreMesh::FECoreMesh()
{
}

//-----------------------------------------------------------------------------
//! destructor
FECoreMesh::~FECoreMesh()
{
}

//-----------------------------------------------------------------------------
//! This function checks if all elements are of the type specified in the argument
bool FECoreMesh::IsType(int ntype) const
{
	int NE = Elements();
	for (int i=0; i<NE; ++i)
	{
		const FEElement_& el = ElementRef(i);
		if (el.Type() != ntype) return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
vec3d FECoreMesh::ElementCenter(FEElement_& el) const
{
	vec3d r;
	int N = el.Nodes();
	for (int i = 0; i<N; i++) r += m_Node[el.m_node[i]].r;
	return r / (float)N;
}

//-----------------------------------------------------------------------------
// Count nr of beam elements
int FECoreMesh::BeamElements()
{
	int n = 0;
	for (int i = 0; i<Elements(); ++i)
	{
		if (ElementRef(i).IsBeam()) n++;
	}

	return n;
}

//-----------------------------------------------------------------------------
// Count nr of shell elements
int FECoreMesh::ShellElements()
{
	int n = 0;
	for (int i = 0; i<Elements(); ++i)
	{
		if (ElementRef(i).IsShell()) n++;
	}

	return n;
}

//-----------------------------------------------------------------------------
// Count nr of solid elements
int FECoreMesh::SolidElements()
{
	int n = 0;
	for (int i = 0; i<Elements(); ++i)
	{
		if (ElementRef(i).IsSolid()) n++;
	}

	return n;
}

//-----------------------------------------------------------------------------
//! Is an element exterior or not
bool FECoreMesh::IsExterior(FEElement_* pe) const
{
	// make sure the element is visible
	if (pe->IsVisible() == false) return false;

	// get number of faces
	int NF = pe->Faces();

	// a shell has 0 faces and is always exterior
	if (NF == 0) return true;

	// solid elements
	for (int i = 0; i<NF; ++i)
	{
		const FEElement_* ei = ElementPtr(pe->m_nbr[i]);
		if ((ei == 0) || (ei->IsVisible() == false)) return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Calculate the volume of an element
double FECoreMesh::ElementVolume(int iel)
{
	FEElement_& el = ElementRef(iel);
	switch (el.Type())
	{
	case FE_HEX8: return HexVolume(el); break;
	case FE_HEX20: return HexVolume(el); break;
	case FE_HEX27: return HexVolume(el); break;
	case FE_TET4: return TetVolume(el); break;
	case FE_TET10: return TetVolume(el); break;
	case FE_TET15: return TetVolume(el); break;
	case FE_TET20: return TetVolume(el); break;
	case FE_PENTA6: return PentaVolume(el); break;
	case FE_PENTA15: return PentaVolume(el); break;
	case FE_PYRA5: return PyramidVolume(el); break;
	}

	return 0.f;
}

//-----------------------------------------------------------------------------
// Calculate the volume of a hex element
double FECoreMesh::HexVolume(const FEElement_& el)
{
	assert((el.Type() == FE_HEX8) || (el.Type() == FE_HEX20) || (el.Type() == FE_HEX27));

	// gauss-point data
	const double a = 1.f / (float)sqrt(3.0);
	const int NELN = 8;
	const int NINT = 8;
	static double gr[NINT] = { -a,  a,  a, -a, -a,  a, a, -a };
	static double gs[NINT] = { -a, -a,  a,  a, -a, -a, a,  a };
	static double gt[NINT] = { -a, -a, -a, -a,  a,  a, a,  a };
	static double gw[NINT] = { 1,  1,  1,  1,  1,  1,  1, 1 };

	static double H[NINT][NELN] = { 0 };
	static double Gr[NINT][NELN] = { 0 };
	static double Gs[NINT][NELN] = { 0 };
	static double Gt[NINT][NELN] = { 0 };
	static bool bfirst = true;

	if (bfirst)
	{
		int n;

		// calculate shape function values at gauss points
		for (n = 0; n<NINT; ++n)
		{
			H[n][0] = 0.125*(1 - gr[n])*(1 - gs[n])*(1 - gt[n]);
			H[n][1] = 0.125*(1 + gr[n])*(1 - gs[n])*(1 - gt[n]);
			H[n][2] = 0.125*(1 + gr[n])*(1 + gs[n])*(1 - gt[n]);
			H[n][3] = 0.125*(1 - gr[n])*(1 + gs[n])*(1 - gt[n]);
			H[n][4] = 0.125*(1 - gr[n])*(1 - gs[n])*(1 + gt[n]);
			H[n][5] = 0.125*(1 + gr[n])*(1 - gs[n])*(1 + gt[n]);
			H[n][6] = 0.125*(1 + gr[n])*(1 + gs[n])*(1 + gt[n]);
			H[n][7] = 0.125*(1 - gr[n])*(1 + gs[n])*(1 + gt[n]);
		}

		// calculate local derivatives of shape functions at gauss points
		for (n = 0; n<NINT; ++n)
		{
			Gr[n][0] = -0.125*(1 - gs[n])*(1 - gt[n]);
			Gr[n][1] = 0.125*(1 - gs[n])*(1 - gt[n]);
			Gr[n][2] = 0.125*(1 + gs[n])*(1 - gt[n]);
			Gr[n][3] = -0.125*(1 + gs[n])*(1 - gt[n]);
			Gr[n][4] = -0.125*(1 - gs[n])*(1 + gt[n]);
			Gr[n][5] = 0.125*(1 - gs[n])*(1 + gt[n]);
			Gr[n][6] = 0.125*(1 + gs[n])*(1 + gt[n]);
			Gr[n][7] = -0.125*(1 + gs[n])*(1 + gt[n]);

			Gs[n][0] = -0.125*(1 - gr[n])*(1 - gt[n]);
			Gs[n][1] = -0.125*(1 + gr[n])*(1 - gt[n]);
			Gs[n][2] = 0.125*(1 + gr[n])*(1 - gt[n]);
			Gs[n][3] = 0.125*(1 - gr[n])*(1 - gt[n]);
			Gs[n][4] = -0.125*(1 - gr[n])*(1 + gt[n]);
			Gs[n][5] = -0.125*(1 + gr[n])*(1 + gt[n]);
			Gs[n][6] = 0.125*(1 + gr[n])*(1 + gt[n]);
			Gs[n][7] = 0.125*(1 - gr[n])*(1 + gt[n]);

			Gt[n][0] = -0.125*(1 - gr[n])*(1 - gs[n]);
			Gt[n][1] = -0.125*(1 + gr[n])*(1 - gs[n]);
			Gt[n][2] = -0.125*(1 + gr[n])*(1 + gs[n]);
			Gt[n][3] = -0.125*(1 - gr[n])*(1 + gs[n]);
			Gt[n][4] = 0.125*(1 - gr[n])*(1 - gs[n]);
			Gt[n][5] = 0.125*(1 + gr[n])*(1 - gs[n]);
			Gt[n][6] = 0.125*(1 + gr[n])*(1 + gs[n]);
			Gt[n][7] = 0.125*(1 - gr[n])*(1 + gs[n]);
		}

		bfirst = false;
	}

	double *Grn, *Gsn, *Gtn;
	double vol = 0, detJ;
	double J[3][3];
	int i, n;

	vec3d rt[NELN];
	for (i = 0; i<NELN; ++i) rt[i] = m_Node[el.m_node[i]].r;

	for (n = 0; n<NINT; ++n)
	{
		Grn = Gr[n];
		Gsn = Gs[n];
		Gtn = Gt[n];

		J[0][0] = J[0][1] = J[0][2] = 0.0;
		J[1][0] = J[1][1] = J[1][2] = 0.0;
		J[2][0] = J[2][1] = J[2][2] = 0.0;
		for (i = 0; i<NELN; ++i)
		{
			double Gri = Grn[i];
			double Gsi = Gsn[i];
			double Gti = Gtn[i];

			double x = rt[i].x;
			double y = rt[i].y;
			double z = rt[i].z;

			J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
			J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
			J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
		}

		// calculate the determinant
		detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
			+ J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
			+ J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);

		vol += detJ*gw[n];
	}

	return vol;
}

//-----------------------------------------------------------------------------
// Calculate the volume of a pentahedral element
double FECoreMesh::PentaVolume(const FEElement_& el)
{
	assert((el.Type() == FE_PENTA6) || (el.Type() == FE_PENTA15));

	// gauss-point data
	//gauss intergration points
	const double a = 1.f / 6.f;
	const double b = 2.f / 3.f;
	const double c = 1.f / (float)sqrt(3.0);
	const double w = 1.f / 6.f;

	const int NELN = 6;
	const int NINT = 6;

	static double gr[NINT] = { a, b, a, a, b, a };
	static double gs[NINT] = { a, a, b, a, a, b };
	static double gt[NINT] = { -c, -c, -c, c, c, c };
	static double gw[NINT] = { w, w, w, w, w, w };

	static double H[NINT][NELN] = { 0 };
	static double Gr[NINT][NELN] = { 0 };
	static double Gs[NINT][NELN] = { 0 };
	static double Gt[NINT][NELN] = { 0 };
	static bool bfirst = true;

	if (bfirst)
	{
		int n;

		// calculate shape function values at gauss points
		for (n = 0; n<NINT; ++n)
		{
			H[n][0] = 0.5*(1.f - gt[n])*(1.0 - gr[n] - gs[n]);
			H[n][1] = 0.5*(1.f - gt[n])*gr[n];
			H[n][2] = 0.5*(1.f - gt[n])*gs[n];
			H[n][3] = 0.5*(1.f + gt[n])*(1.0 - gr[n] - gs[n]);
			H[n][4] = 0.5*(1.f + gt[n])*gr[n];
			H[n][5] = 0.5*(1.f + gt[n])*gs[n];
		}

		// calculate local derivatives of shape functions at gauss points
		for (n = 0; n<NINT; ++n)
		{
			Gr[n][0] = -0.5*(1.0 - gt[n]);
			Gr[n][1] = 0.5*(1.0 - gt[n]);
			Gr[n][2] = 0.0;
			Gr[n][3] = -0.5*(1.0 + gt[n]);
			Gr[n][4] = 0.5*(1.0 + gt[n]);
			Gr[n][5] = 0.0;

			Gs[n][0] = -0.5*(1.0 - gt[n]);
			Gs[n][1] = 0.0;
			Gs[n][2] = 0.5*(1.0 - gt[n]);
			Gs[n][3] = -0.5*(1.0 + gt[n]);
			Gs[n][4] = 0.0;
			Gs[n][5] = 0.5*(1.0 + gt[n]);

			Gt[n][0] = -0.5*(1.0 - gr[n] - gs[n]);
			Gt[n][1] = -0.5*gr[n];
			Gt[n][2] = -0.5*gs[n];
			Gt[n][3] = 0.5*(1.0 - gr[n] - gs[n]);
			Gt[n][4] = 0.5*gr[n];
			Gt[n][5] = 0.5*gs[n];
		}

		bfirst = false;
	}

	double *Grn, *Gsn, *Gtn;
	double vol = 0, detJ;
	double J[3][3];
	int i, n;

	vec3d rt[NELN];
	for (i = 0; i<NELN; ++i) rt[i] = m_Node[el.m_node[i]].r;

	for (n = 0; n<NINT; ++n)
	{
		Grn = Gr[n];
		Gsn = Gs[n];
		Gtn = Gt[n];

		J[0][0] = J[0][1] = J[0][2] = 0.0;
		J[1][0] = J[1][1] = J[1][2] = 0.0;
		J[2][0] = J[2][1] = J[2][2] = 0.0;
		for (i = 0; i<NELN; ++i)
		{
			double Gri = Grn[i];
			double Gsi = Gsn[i];
			double Gti = Gtn[i];

			double x = rt[i].x;
			double y = rt[i].y;
			double z = rt[i].z;

			J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
			J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
			J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
		}

		// calculate the determinant
		detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
			+ J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
			+ J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);

		vol += detJ*gw[n];
	}

	return vol;
}

//-----------------------------------------------------------------------------
// Calculate the volume of a pyramid element
double FECoreMesh::PyramidVolume(const FEElement_& el)
{
	assert(el.Type() == FE_PYRA5);

	// gauss-point data
	const double a = 1.0 / sqrt(3.0);
	const int NELN = 5;
	const int NINT = 8;
	static double gr[NINT] = { -a, a, a, -a, -a, a, a, -a };
	static double gs[NINT] = { -a, -a, a, a, -a, -a, a, a };
	static double gt[NINT] = { -a, -a, -a, -a, a, a, a, a };
	static double gw[NINT] = { 1, 1, 1, 1, 1, 1, 1, 1 };

	static double H[NINT][NELN] = { 0 };
	static double Gr[NINT][NELN] = { 0 };
	static double Gs[NINT][NELN] = { 0 };
	static double Gt[NINT][NELN] = { 0 };
	static bool bfirst = true;

	if (bfirst)
	{
		int n;

		// calculate shape function values at gauss points
		for (n = 0; n<NINT; ++n)
		{
			H[n][0] = 0.125*(1 - gr[n])*(1 - gs[n])*(1 - gt[n]);
			H[n][1] = 0.125*(1 + gr[n])*(1 - gs[n])*(1 - gt[n]);
			H[n][2] = 0.125*(1 + gr[n])*(1 + gs[n])*(1 - gt[n]);
			H[n][3] = 0.125*(1 - gr[n])*(1 + gs[n])*(1 - gt[n]);
			H[n][4] = 0.5*(1 + gt[n]);
		}

		// calculate local derivatives of shape functions at gauss points
		for (n = 0; n<NINT; ++n)
		{
			double r = gr[n], s = gs[n], t = gt[n];
			Gr[n][0] = -0.125*(1.0 - s)*(1.0 - t);
			Gr[n][1] = 0.125*(1.0 - s)*(1.0 - t);
			Gr[n][2] = 0.125*(1.0 + s)*(1.0 - t);
			Gr[n][3] = -0.125*(1.0 + s)*(1.0 - t);
			Gr[n][4] = 0.0;

			Gs[n][0] = -0.125*(1.0 - r)*(1.0 - t);
			Gs[n][1] = -0.125*(1.0 + r)*(1.0 - t);
			Gs[n][2] = 0.125*(1.0 + r)*(1.0 - t);
			Gs[n][3] = 0.125*(1.0 - r)*(1.0 - t);
			Gs[n][4] = 0.0;

			Gt[n][0] = -0.125*(1.0 - r)*(1.0 - s);
			Gt[n][1] = -0.125*(1.0 + r)*(1.0 - s);
			Gt[n][2] = -0.125*(1.0 + r)*(1.0 + s);
			Gt[n][3] = -0.125*(1.0 - r)*(1.0 + s);
			Gt[n][4] = 0.5;
		}

		bfirst = false;
	}

	double *Grn, *Gsn, *Gtn;
	double vol = 0, detJ;
	double J[3][3];
	int i, n;

	vec3d rt[NELN];
	for (i = 0; i<NELN; ++i) rt[i] = m_Node[el.m_node[i]].r;

	for (n = 0; n<NINT; ++n)
	{
		Grn = Gr[n];
		Gsn = Gs[n];
		Gtn = Gt[n];

		J[0][0] = J[0][1] = J[0][2] = 0.0;
		J[1][0] = J[1][1] = J[1][2] = 0.0;
		J[2][0] = J[2][1] = J[2][2] = 0.0;
		for (i = 0; i<NELN; ++i)
		{
			double Gri = Grn[i];
			double Gsi = Gsn[i];
			double Gti = Gtn[i];

			double x = rt[i].x;
			double y = rt[i].y;
			double z = rt[i].z;

			J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
			J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
			J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
		}

		// calculate the determinant
		detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
			+ J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
			+ J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);

		vol += detJ*gw[n];
	}

	return vol;
}

//-----------------------------------------------------------------------------
// Calculate the volume of a tetrahedral element
double FECoreMesh::TetVolume(const FEElement_& el)
{
	assert((el.Type() == FE_TET4) || (el.Type() == FE_TET10)
		|| (el.Type() == FE_TET15) || (el.Type() == FE_TET20));

	// gauss-point data
	const double a = 0.58541020f;
	const double b = 0.13819660f;
	const double w = 1.f / 24.f;

	const int NELN = 4;
	const int NINT = 4;

	static double gr[NINT] = { b, a, b, b };
	static double gs[NINT] = { b, b, a, b };
	static double gt[NINT] = { b, b, b, a };
	static double gw[NINT] = { w, w, w, w };

	static double H[NINT][NELN] = { 0 };
	static double Gr[NINT][NELN] = { 0 };
	static double Gs[NINT][NELN] = { 0 };
	static double Gt[NINT][NELN] = { 0 };
	static bool bfirst = true;

	if (bfirst)
	{
		int n;

		// calculate shape function values at gauss points
		for (n = 0; n<NINT; ++n)
		{
			H[n][0] = 1.0 - gr[n] - gs[n] - gt[n];
			H[n][1] = gr[n];
			H[n][2] = gs[n];
			H[n][3] = gt[n];
		}

		// calculate local derivatives of shape functions at gauss points
		for (n = 0; n<NINT; ++n)
		{
			Gr[n][0] = -1.0;
			Gr[n][1] = 1.0;
			Gr[n][2] = 0.0;
			Gr[n][3] = 0.0;

			Gs[n][0] = -1.0;
			Gs[n][1] = 0.0;
			Gs[n][2] = 1.0;
			Gs[n][3] = 0.0;

			Gt[n][0] = -1.0;
			Gt[n][1] = 0.0;
			Gt[n][2] = 0.0;
			Gt[n][3] = 1.0;
		}

		bfirst = false;
	}

	double *Grn, *Gsn, *Gtn;
	double vol = 0, detJ;
	double J[3][3];
	int i, n;

	vec3d rt[NELN];
	for (i = 0; i<NELN; ++i) rt[i] = m_Node[el.m_node[i]].r;

	for (n = 0; n<NINT; ++n)
	{
		Grn = Gr[n];
		Gsn = Gs[n];
		Gtn = Gt[n];

		J[0][0] = J[0][1] = J[0][2] = 0.0;
		J[1][0] = J[1][1] = J[1][2] = 0.0;
		J[2][0] = J[2][1] = J[2][2] = 0.0;
		for (i = 0; i<NELN; ++i)
		{
			double Gri = Grn[i];
			double Gsi = Gsn[i];
			double Gti = Gtn[i];

			double x = rt[i].x;
			double y = rt[i].y;
			double z = rt[i].z;

			J[0][0] += Gri*x; J[0][1] += Gsi*x; J[0][2] += Gti*x;
			J[1][0] += Gri*y; J[1][1] += Gsi*y; J[1][2] += Gti*y;
			J[2][0] += Gri*z; J[2][1] += Gsi*z; J[2][2] += Gti*z;
		}

		// calculate the determinant
		detJ = J[0][0] * (J[1][1] * J[2][2] - J[1][2] * J[2][1])
			+ J[0][1] * (J[1][2] * J[2][0] - J[2][2] * J[1][0])
			+ J[0][2] * (J[1][0] * J[2][1] - J[1][1] * J[2][0]);

		vol += detJ*gw[n];
	}

	return vol;
}

//-----------------------------------------------------------------------------
// Tag all elements
void FECoreMesh::TagAllElements(int ntag)
{
	const int NE = Elements();
	for (int i = 0; i<NE; ++i) ElementRef(i).m_ntag = ntag;
}

//-----------------------------------------------------------------------------
// get the local node positions of a element
void FECoreMesh::ElementNodeLocalPositions(const FEElement_& e, vec3d* r) const
{
	int ne = e.Nodes();
	for (int i = 0; i<ne; ++i) r[i] = m_Node[e.m_node[i]].r;
}

//-----------------------------------------------------------------------------
// See if this is a shell mesh.
bool FECoreMesh::IsShell() const
{
	int NE = Elements();
	for (int i = 0; i<NE; ++i)
	{
		const FEElement_& el = ElementRef(i);
		if (el.IsShell() == false) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// See if this is a solid mesh.
bool FECoreMesh::IsSolid() const
{
	int NE = Elements();
	for (int i = 0; i<NE; ++i)
	{
		const FEElement_& el = ElementRef(i);
		if (el.IsSolid() == false) return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Find a face of an element.
int FECoreMesh::FindFace(FEElement_ *pe, FEFace &f, FEFace& fe)
{
	if (pe->IsSolid())
	{
		int n = pe->Faces();
		for (int i = 0; i<n; ++i)
		{
			fe = pe->GetFace(i);
			if (fe == f) return i;
		}
	}
	if (pe->IsShell())
	{
		pe->GetShellFace(fe);
		if (fe == f) return 0;
	}

	return -1;
}

//-------------------------------------------------------------------------------------------------
void FECoreMesh::SelectElements(const vector<int>& elem)
{
	for (int i : elem) ElementRef(i).Select();
}

//-----------------------------------------------------------------------------
// This function finds the interior and exterior nodes.
void FECoreMesh::MarkExteriorNodes()
{
	int nodes = Nodes();
	int faces = Faces();
	int elems = Elements();

	for (int i = 0; i<nodes; ++i) Node(i).m_bext = false;

	for (int i = 0; i<faces; ++i)
	{
		FEFace& face = Face(i);
		for (int j = 0; j<face.Nodes(); ++j)
			m_Node[face.n[j]].m_bext = true;
	}

	// mark all nodes attached to beams as exterior
	for (int i = 0; i<elems; ++i)
	{
		FEElement_& el = ElementRef(i);
		if (el.IsType(FE_BEAM2))
		{
			Node(el.m_node[0]).m_bext = true;
			Node(el.m_node[1]).m_bext = true;
		}
	}
}

//-----------------------------------------------------------------------------
void FECoreMesh::FindNodesFromPart(int gid, vector<int>& node)
{
	for (int i = 0; i<Nodes(); ++i) Node(i).m_ntag = 0;
	for (int i = 0; i<Elements(); ++i)
	{
		FEElement_& e = ElementRef(i);
		if (e.m_gid == gid)
		{
			int ne = e.Nodes();
			for (int j = 0; j<ne; ++j) Node(e.m_node[j]).m_ntag = 1;
		}
	}

	int nodes = 0;
	for (int i = 0; i<Nodes(); ++i) if (Node(i).m_ntag == 1) nodes++;

	node.resize(nodes);
	nodes = 0;
	for (int i = 0; i<Nodes(); ++i)
		if (Node(i).m_ntag == 1) node[nodes++] = i;
}

//-------------------------------------------------------------------------------------------------
void FECoreMesh::ShowAllElements()
{
	for (int i = 0; i<Nodes(); ++i) Node(i).Show();
	for (int i = 0; i<Edges(); ++i) Edge(i).Show();
	for (int i = 0; i<Faces(); ++i) Face(i).Show();
	for (int i = 0; i<Elements(); ++i) ElementRef(i).Show();
}

//-------------------------------------------------------------------------------------------------
void FECoreMesh::ShowElements(vector<int>& elem, bool show)
{
	// show or hide all the elements
	if (show)
		for (int i : elem) ElementRef(i).Unhide();
	else
		for (int i : elem) ElementRef(i).Hide();

	// update visibility of all other items
	UpdateItemVisibility();
}

//-------------------------------------------------------------------------------------------------
void FECoreMesh::UpdateItemVisibility()
{
	// tag all visible nodes
	TagAllNodes(0);
	for (int i = 0; i<Elements(); ++i)
	{
		FEElement_& el = ElementRef(i);
		if (el.IsVisible())
		{
			int ne = el.Nodes();
			for (int j = 0; j<ne; ++j) Node(el.m_node[j]).m_ntag = 1;
		}
	}

	// update face visibility
	for (int i = 0; i<Faces(); ++i)
	{
		FEFace& face = Face(i);

		FEElement_* e0 = ElementPtr(face.m_elem[0]); assert(e0);
		FEElement_* e1 = ElementPtr(face.m_elem[1]);

		if (!e0->IsVisible() && ((e1 == 0) || !e1->IsVisible())) face.Hide(); else face.Show();
	}

	// update visibility of all other items
	for (int i = 0; i<Nodes(); ++i)
	{
		FENode& node = Node(i);
		if (node.m_ntag == 1) node.Show(); else node.Hide();
	}

	for (int i = 0; i<Edges(); ++i)
	{
		FEEdge& edge = Edge(i);
		if ((Node(edge.n[0]).m_ntag == 0) || (Node(edge.n[1]).m_ntag == 0)) edge.Hide();
		else edge.Show();
	}
}
