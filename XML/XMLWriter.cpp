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

// XMLWriter.cpp: implementation of the XMLWriter class.
//
//////////////////////////////////////////////////////////////////////

#include "XMLWriter.h"
#include <sstream>
using namespace std;

const char* XMLElement::intFormat = "%6d";

void XMLElement::setDefaultFormats()
{
	intFormat = "%6d";
}

void XMLElement::value(int    n) { stringstream ss; ss << n; m_val = ss.str(); }
void XMLElement::value(bool   b) { int n = (b ? 1 : 0); stringstream ss; ss << n; m_val = ss.str(); }
void XMLElement::value(double g) { stringstream ss; ss << g; m_val = ss.str(); }

void XMLElement::value(int* pi, int n)
{
	m_val.clear();
	if (n==0) return;

	stringstream ss;
	ss << pi[0];
	for (int i=1; i<n; ++i)
	{
		ss << "," << pi[i];
	}
	m_val = ss.str();
}

void XMLElement::value(double* pg, int n)
{
	m_val.clear();
	if (n==0) return;
	stringstream ss;
	ss << pg[0];
	for (int i=1; i<n; ++i)
	{
		ss << "," << pg[i];
	}
	m_val = ss.str();
}

void XMLElement::value(const vec3f& r)
{
	stringstream ss;
	ss << r.x << "," << r.y << "," << r.z;
	m_val = ss.str();
}

void XMLElement::value(const vec3d& r)
{ 
	stringstream ss;
	ss << r.x << "," << r.y << "," << r.z;
	m_val = ss.str();
}

void XMLElement::value(const vec2i& r)
{
	stringstream ss;
	ss << r.x << "," << r.y;
	m_val = ss.str();
}

void XMLElement::value(const mat3d& a)
{
	stringstream ss;
	ss << a(0,0) << "," << a(0,1) << "," << a(0,2) << ",";
	ss << a(1,0) << "," << a(1,1) << "," << a(1,2) << ",";
	ss << a(2,0) << "," << a(2,1) << "," << a(2,2);
	m_val = ss.str();
}

void XMLElement::value(const std::vector<int>& v)
{
	m_val.clear();
	if (v.empty()) return;

	stringstream ss;
	ss << v[0];
	for (int i = 1; i < v.size(); ++i)
	{
		ss << "," << v[i];
	}
	m_val = ss.str();
}

void XMLElement::value(const std::vector<double>& v)
{
	m_val.clear();
	if (v.empty()) return;

	stringstream ss;
	ss << v[0];
	for (int i = 1; i < v.size(); ++i)
	{
		ss << "," << v[i];
	}
	m_val = ss.str();
}

int XMLElement::add_attribute(const char* szn, const char* szv)
{
	m_att.push_back(XMLAtt(szn, szv));
	return (int) m_att.size()-1;
}

int XMLElement::add_attribute(const char* szn, int n)
{
	stringstream ss; ss << n;
	m_att.push_back(XMLAtt(szn, ss.str()));
	return (int)m_att.size() - 1;
}

int XMLElement::add_attribute(const char* szn, bool b)
{
	int n = (b ? 1 : 0);
	stringstream ss; ss << n;
	m_att.push_back(XMLAtt(szn, ss.str()));
	return (int)m_att.size() - 1;
}

int XMLElement::add_attribute(const char* szn, double g)
{
	stringstream ss; ss << g;
	m_att.push_back(XMLAtt(szn, ss.str()));
	return (int)m_att.size() - 1;
}

int XMLElement::add_attribute(const char* szn, const std::string& s)
{
	m_att.push_back(XMLAtt(szn, s));
	return (int)m_att.size() - 1;
}

void XMLElement::set_attribute(int nid, const char* szv)
{
	m_att[nid].m_val = szv;
}

void XMLElement::set_attribute(int nid, int n)
{
	stringstream ss; ss << n;
	m_att[nid].m_val = ss.str();
}

void XMLElement::set_attribute(int nid, bool b)
{
	stringstream ss; ss << (b?(int)1:(int)0);
	m_att[nid].m_val = ss.str();
}

void XMLElement::set_attribute(int nid, double g)
{
	stringstream ss; ss << g;
	m_att[nid].m_val = ss.str();
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

XMLWriter::XMLFloatFormat	XMLWriter::m_floatFormat = XMLWriter::FixedFormat;

void XMLWriter::SetFloatFormat(XMLFloatFormat fmt)
{
	m_floatFormat = fmt;
}

XMLWriter::XMLFloatFormat XMLWriter::GetFloatFormat()
{
	return m_floatFormat;
}

XMLWriter::XMLWriter()
{
	m_fp = 0;
	m_level = 0;

	m_sztab[0] = 0;

	XMLElement::setDefaultFormats();
}

XMLWriter::~XMLWriter()
{
	close();
}

bool XMLWriter::open(const char* szfile)
{
	if (m_fp) return false;
	if (szfile == nullptr) return false;

	m_fp = fopen(szfile, "wt");

	// write the first line
	if (m_fp) fprintf(m_fp, "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
	
	return (m_fp != nullptr);
}

void XMLWriter::close()
{
	if (m_fp) fclose(m_fp); m_fp = 0;
}

void XMLWriter::inc_level()
{
	++m_level;

	m_sztab[0] = 0;
	int l=0;
	for (int i=0; i<m_level; ++i) 
	{
		sprintf(m_sztab+l, "\t");
		++l;
	}
	m_sztab[l] = 0;
}

void XMLWriter::dec_level()
{
	if (m_level <= 0) return;

	--m_level;

	m_sztab[0] = 0;
	int l=0;
	for (int i=0; i<m_level; ++i) 
	{
		sprintf(m_sztab+l, "\t");
		++l;
	}
	m_sztab[l] = 0;
}


void XMLWriter::add_branch(XMLElement& el, bool bclear)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s", m_sztab);

	fprintf(m_fp, szformat, el.m_tag.c_str());

	for (int i=0; i<el.attributes(); ++i)
	{
		const XMLElement::XMLAtt& att = el.attribute(i);
		fprintf(m_fp, " %s=\"%s\"", att.name(), att.value());
	}

	fprintf(m_fp, ">%s\n", el.m_val.c_str());

	strcpy(m_tag[m_level], el.m_tag.c_str());

	inc_level();

	if (bclear) el.clear();
}

void XMLWriter::add_branch(const char* sz)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s>\n", m_sztab);
	fprintf(m_fp, szformat, sz);

	strcpy(m_tag[m_level], sz);
	inc_level();
}

void XMLWriter::add_empty(XMLElement& el, bool bclear)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s", m_sztab);

	fprintf(m_fp, szformat, el.m_tag.c_str());

	for (int i=0; i<el.attributes(); ++i)
	{
		const XMLElement::XMLAtt& att = el.attribute(i);
		fprintf(m_fp, " %s=\"%s\"", att.name(), att.value());
	}

	fprintf(m_fp, "/>\n");

	if (bclear) el.clear();
}

void XMLWriter::add_leaf(XMLElement& el, bool bclear)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s", m_sztab);

	fprintf(m_fp, szformat, el.m_tag.c_str());

	for (int i=0; i<el.attributes(); ++i)
	{
		const XMLElement::XMLAtt& att = el.attribute(i);
		fprintf(m_fp, " %s=\"%s\"", att.name(), att.value());
	}

	fprintf(m_fp, ">%s</%s>\n", el.m_val.c_str(), el.m_tag.c_str());

	if (bclear) el.clear();
}

void XMLWriter::add_leaf(const char* szn, const char* szv)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s", m_sztab);

	fprintf(m_fp, szformat, szn);

	fprintf(m_fp, ">%s</%s>\n", szv, szn);
}

void XMLWriter::add_leaf(const char* szn, const std::string& s)
{
	add_leaf(szn, s.c_str());
}

void XMLWriter::add_leaf(const char* szn, double* pg, int n)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s>", m_sztab);

	fprintf(m_fp, szformat, szn);

	if (n>0)
	{
		fprintf(m_fp, "%.12lg", pg[0]);
		for (int i=1; i<n; ++i) fprintf(m_fp, ",%.12lg", pg[i]);
	}

	fprintf(m_fp, "</%s>\n", szn);

}

void XMLWriter::add_leaf(const char* szn, float* pg, int n)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s>", m_sztab);

	fprintf(m_fp, szformat, szn);

	if (n>0)
	{
		fprintf(m_fp, "%.12g", pg[0]);
		for (int i=1; i<n; ++i) fprintf(m_fp, ",%.12g", pg[i]);
	}

	fprintf(m_fp, "</%s>\n", szn);

}


void XMLWriter::add_leaf(const char* szn, int* pi, int n)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s>", m_sztab);

	fprintf(m_fp, szformat, szn);

	if (n>0)
	{
		fprintf(m_fp, "%d", pi[0]);
		for (int i=1; i<n; ++i) fprintf(m_fp, ",%d", pi[i]);
	}

	fprintf(m_fp, "</%s>\n", szn);
}

void XMLWriter::add_leaf(XMLElement& el, const std::vector<int>& A)
{
	char szformat[256] = {0};
	sprintf(szformat, "%s<%%s", m_sztab);

	fprintf(m_fp, szformat, el.m_tag.c_str());

	for (int i=0; i<el.attributes(); ++i)
	{
		const XMLElement::XMLAtt& att = el.attribute(i);
		fprintf(m_fp, " %s=\"%s\"", att.name(), att.value());
	}

	fprintf(m_fp, ">\n%s", m_sztab);

	int n = (int) A.size(), l = 0;
	for (int i=0; i<n; ++i)
	{
		l += fprintf(m_fp, "%5d", A[i]);
		if (i < n-1)
		{
			fprintf(m_fp, ",");
			if (l > 80) { fprintf(m_fp, "\n%s", m_sztab); l=0; }
		}
	}
	fprintf(m_fp,"\n%s</%s>\n", m_sztab, el.m_tag.c_str());
}


void XMLWriter::close_branch()
{
	if (m_level > 0)
	{
		dec_level();

		char szformat[256] = {0};
		sprintf(szformat, "%s</%%s>\n", m_sztab);

		fprintf(m_fp, szformat, m_tag[m_level]);
	}
}

void XMLWriter::add_comment(const std::string& s, bool singleLine)
{
	if (s.empty()) return;

	if (singleLine)
	{
		fprintf(m_fp, "<!-- %s -->\n", s.c_str());
	}
	else
	{
		fprintf(m_fp, "<!--\n%s\n-->\n", s.c_str());
	}
}
