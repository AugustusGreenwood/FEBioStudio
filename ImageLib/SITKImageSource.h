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

#include "ImageSource.h"

class CITKImageSource : public CImageSource
{
public:
    enum FileType
    {
        RAW, // Currently unused
        DICOM, 
        TIFF, // Currently unused. We use our own TIFF reader.
        NRRD, 
        OTHER
    };

public:
    CITKImageSource(CImageModel* imgModel, const std::string& filename, int type);
    CITKImageSource(CImageModel* imgModel);

    bool Load() override;

    void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

    void SetFileName(const std::string& filename) { m_filename = filename; }

    int GetFileType() const { return m_fileType; }

private:
    std::string m_filename;

    int m_fileType;
};

class CITKSeriesImageSource : public CImageSource
{
public:
    CITKSeriesImageSource(CImageModel* imgModel, const std::vector<std::string>& filenames);
    CITKSeriesImageSource(CImageModel* imgModel);

    bool Load() override;

    void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

    void SetFileNames(const std::vector<std::string>& filenames) { m_filenames = filenames; }

private:
    std::vector<std::string> m_filenames;
};