// Image.cpp: implementation of the CImage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Image.h"
#include <stdio.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////
// CImage
//////////////////////////////////////////////////////////////////////

CImage::CImage()
{
	m_pb = 0;
	m_cx = m_cy = 0;
	m_bdel = true;
}

CImage::CImage(int nx, int ny)
{
	m_pb = new byte[nx*ny];
	for (int i=0; i<nx*ny; i++) m_pb[i] = 0;

	m_cx = nx;
	m_cy = ny;

	m_bdel = true;
}

CImage::CImage(const CImage& im)
{
	m_cx = im.m_cx;
	m_cy = im.m_cy;

	m_pb = new byte[m_cx*m_cy];
	memcpy(m_pb, im.m_pb, m_cx*m_cy);

	m_bdel = true;
}

CImage& CImage::operator = (const CImage& im)
{
	if (m_bdel) delete [] m_pb;

	m_cx = im.m_cx;
	m_cy = im.m_cy;

	m_pb = new byte[m_cx*m_cy];
	memcpy(m_pb, im.m_pb, m_cx*m_cy);

	m_bdel = true;

	return (*this);
}

CImage& CImage::operator -= (const CImage& im)
{
	byte* pbS = im.m_pb;
	byte* pbD = m_pb;

	int nsize = m_cx*m_cy;

	for (int i=0; i<nsize; i++, pbS++, pbD++)
		*pbD = byte((((int) *pbD - (int) *pbS) + 255) >> 1);

	return (*this);
}

CImage::~CImage()
{
	if (m_bdel) delete [] m_pb;
}

void CImage::Create(int nx, int ny, byte* pb)
{
	if (pb)
	{
		m_pb = pb;
		m_bdel = false;
	}
	else
	{
		if (m_bdel) delete [] m_pb;

		m_pb = new byte[nx*ny];
		for (int i=0; i<nx*ny; i++) m_pb[i] = 0;
		m_bdel = true;
	}

	m_cx = nx;
	m_cy = ny;
}

void CImage::StretchBlt(CImage& im)
{
	byte* pd = im.m_pb;

	int nx = im.Width();
	int ny = im.Height();

	int i0 = 0;
	int j0 = 0;

	byte* p0, *p1, *p2, *p3;
	int h0, h1, h2, h3;
	int w = 0, h = 0;

	int Hx = nx - 1;
	int Hy = ny - 1;
	int H = Hx*Hy;

	for (int y=0; y<ny; y++)
	{
		i0 = 0;
		w = 0;

		while (y*(m_cy-1)>(j0+1)*(ny-1)) { j0++; h -= (ny-1); }

		p0 = m_pb + (j0*m_cx);
		p1 = p0 + 1;
		p2 = p0 + (m_cy != 1 ? m_cx : 0);
		p3 = p2 + 1;

		for (int x=0; x<nx; x++)
		{
			while (x*(m_cx-1)>(i0+1)*(nx-1)) { i0++; w -= (nx-1); p0++; p1++; p2++; p3++; }

			h0 = (Hx - w)*(Hy - h);
			h1 = w*(Hy - h);
			h2 = (Hx - w)*h;
			h3 = w*h;

			*pd++ = (h0*p0[0] + h1*p1[0] + h2*p2[0] + h3*p3[0])/H;

			w += (m_cx-1);
		}

		h += (m_cy-1);
	}
}
