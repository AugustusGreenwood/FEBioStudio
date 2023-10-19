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
#pragma once
#include "CreatePanel.h"

//=============================================================================
class CCreateGeodesicCurvePane : public CCreatePane
{
	Q_OBJECT

public:
	CCreateGeodesicCurvePane(CCreatePanel* parent);

	void Activate() override;
	void Deactivate() override;

	bool Clear() override;

	FSObject* Create() override;

	// set the input event for this pane
	void setInput(const vec3d& r) override;

	void hideEvent(QHideEvent* ev) override;

private:
	void BuildGeodesic();

protected slots:
	void on_getNode_clicked();
	void on_newCurve_clicked();

private:
	GObject* m_tmp;	// temp object
	vec3d	m_r[2];

	QLineEdit* m_in[2];
	int	m_input;
};
