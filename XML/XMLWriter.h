#pragma once
#include <stdio.h>
#include <string.h>
#include <MathLib/math3d.h>
#include <vector>
#include <string>

#define MAX_TAGS	32
#define MAX_ATTR	32

class XMLWriter;

class XMLElement
{
public:
	XMLElement(const char* szname = 0)
	{
		clear();
		if (szname) strcpy(m_sztag, szname);
	}

	void clear()
	{
		m_natt = 0;
		m_sztag[0] = 0;
		m_szval[0] = 0;
	}

	void name(const char* sz) { strcpy(m_sztag, sz); }

	void value(const char* sz) { strcpy(m_szval, sz); }
	void value(int    n) { sprintf(m_szval, "%d" , n); }
	void value(int* pi, int n);
	void value(bool   b) { sprintf(m_szval, "%d" , (int) b); }
	void value(double g) { sprintf(m_szval, "%lg", g); }
	void value(double* pg, int n);
	void value(const vec3d& r);

	int add_attribute(const char* szn, const char* szv);
	int add_attribute(const char* szn, int n);
	int add_attribute(const char* szn, bool b);
	int add_attribute(const char* szn, double g);
	int add_attribute(const char* szn, const std::string& s);

	void set_attribute(int nid, const char* szv);
	void set_attribute(int nid, int n);
	void set_attribute(int nid, bool b);
	void set_attribute(int nid, double g);

protected:
	char	m_sztag[128];	// element name
	char	m_szval[512];	// element value

	int	m_natt;	// number of attributes
	char m_attn[MAX_ATTR][128];	// attribute name
	char m_attv[MAX_ATTR][128]; // attribute value

public:
	static void setDefautlFormats();
	static const char* intFormat;

	friend class XMLWriter;
};

class XMLWriter  
{
public:
	enum XMLFloatFormat {
		ScientificFormat,
		FixedFormat
	};

public:
	XMLWriter();
	virtual ~XMLWriter();
	
	bool open(const char* szfile);

	void close();

	void add_branch(XMLElement& el, bool bclear = true);
	void add_branch(const char* szname);

	void add_empty(XMLElement& el, bool bclear = true);

	void add_leaf  (XMLElement& el, bool bclear = true);

	void add_leaf(const char* szn, const char* szv);

	void add_leaf(const char* szn, int    n){ char szv[256]; sprintf(szv, "%d" , n); add_leaf(szn, szv); }
	void add_leaf(const char* szn, bool   b){ char szv[256]; sprintf(szv, "%d" , b); add_leaf(szn, szv); }
	void add_leaf(const char* szn, double g){ char szv[256]; sprintf(szv, "%lg", g); add_leaf(szn, szv); }
	void add_leaf(const char* szn, int *pi, int n);
	void add_leaf(const char* szn, float* pg, int n);
	void add_leaf(const char* szn, double* pg, int n);
	void add_leaf(const char* szn, const vec3d& r){ char szv[256]; sprintf(szv, "%g,%g,%g", r.x, r.y, r.z); add_leaf(szn, szv); }
	void add_leaf(const char* szn, const quatd& q){ char szv[256]; sprintf(szv, "%g,%g,%g,%g", q.x, q.y, q.z, q.w); add_leaf(szn, szv); }
	void add_leaf(XMLElement& el, const std::vector<int>& A);

	void close_branch();

public:
	static void SetFloatFormat(XMLFloatFormat fmt);
	static XMLFloatFormat GetFloatFormat();

protected:
	void inc_level();
	void dec_level();

protected:
	FILE*	m_fp;
	int		m_level;

	char	m_tag[MAX_TAGS][256];
	char	m_sztab[256];

	static XMLFloatFormat	m_floatFormat;
};
