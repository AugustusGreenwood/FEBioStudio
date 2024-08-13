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
#include "CommandPanel.h"

namespace Ui {
	class CBuildPanel;
}

class CCreatePanel;

class CBuildPanel : public CCommandPanel
{
	Q_OBJECT

public:
	CBuildPanel(CMainWindow* wnd, QWidget* parent = 0);

	CCreatePanel* CreatePanel();
	CCommandPanel* GetActivePanel();

	void Update(bool breset) override;

	void Apply() override;

	//! Process Esc key event (return true if processed)
	bool OnEscapeEvent() override;
	bool OnDeleteEvent() override;

	bool IsEditPanelVisible();
	bool IsMeshPanelVisible();

public slots:
	void on_buildTab_currentChanged(int index);

private:
	Ui::CBuildPanel*	ui;
};
