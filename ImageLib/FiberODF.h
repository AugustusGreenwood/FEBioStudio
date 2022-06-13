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

#ifdef HAS_ITK

#include <SimpleITK.h>
#include <string>

namespace sitk = itk::simple;
using std::string;

class matrix;

class CFiberODF 
{

public:
    CFiberODF(string& inFile, string& outFile);

    void Apply();

private:
    void butterworthFilter(sitk::Image img);
    sitk::Image powerSpectrum(sitk::Image img);
    void fftRadialFilter(sitk::Image img);
    void reduceAmp(sitk::Image img, std::vector<double>* reduced);

    std::unique_ptr<matrix> compSH(int size, double* theta, double*phi);
    double harmonicY(int degree, int order, double theta, double phi, int numType);
    std::unique_ptr<matrix> complLapBel_Coef();

    double GFA(std::vector<double> vals);

private:
    string m_inFile;
    string m_outFile;

    float m_tLow;
    float m_tHigh;

    int m_order;


};

#endif