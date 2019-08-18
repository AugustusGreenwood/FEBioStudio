#pragma once
#include <stdio.h>
#include <string.h>
#include <string>

#ifdef WIN32
typedef __int64 off_type;
#endif

#ifdef LINUX // same for Linux and Mac OS X
typedef off_t off_type;
#endif

#ifdef __APPLE__ // same for Linux and Mac OS X
typedef off_t off_type;
#endif

//-----------------------------------------------------------------------------
// forward declaration of model class
class FEModel;
class FEProject;

//-----------------------------------------------------------------------------
class FileReader
{
public:
	FileReader();
	virtual ~FileReader();

	// This function must be overloaded in derived classes
	virtual bool Load(const char* szfile) = 0;

public:
	// get the error string
	const std::string& GetErrorMessage();

	// get the number of errors
	int Errors();

	// get the amount of the file read so far
	// expressed in percentage of total file size
	float GetFileProgress() const;

	// get the file title, i.e. the file name w/o the path
	void FileTitle(char* sz);

	// set the file name
	void SetFileName(const std::string& fileName);

protected:
	// open the file
	bool Open(const char* szfile, const char* szmode);

	// close the file
	virtual void Close();

	// helper function that sets the error string
	bool errf(const char* szerr, ...);

protected:
	FILE*			m_fp;
	std::string		m_fileName;	//!< file name

private:
	std::string		m_err;		//!< error messages (separated by \n)
	int				m_nerrors;	//!< number of errors
	off_type		m_nfilesize;	// size of file
};

//-----------------------------------------------------------------------------
// class for reading FE file formats
class FEFileImport : public FileReader
{
public:
	FEFileImport(){}

	virtual bool Load(FEProject& prj, const char* szfile) = 0;

private:
	bool Load(const char* szfile) { return false; }
};

// helper function to compare strings
inline int szcmp(const char* sz1, const char* sz2)
{
	return strncmp(sz1, sz2, strlen(sz2));
}
