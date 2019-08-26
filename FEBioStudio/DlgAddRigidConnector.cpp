#include "stdafx.h"
#include "DlgAddRigidConnector.h"
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QFormLayout>
#include <QMessageBox>
#include <MeshTools/FEModel.h>
#include <MeshTools/FEProject.h>
#include <FEMLib/FEMultiMaterial.h>
#include <FEMLib/FEMKernel.h>

class Ui::CDlgAddRigidConnector
{
public:
	QLineEdit* name;
	QComboBox* step;
	QComboBox* matA;
	QComboBox* matB;
	QListWidget* list;

	int		m_ntype;
	int		m_matA, m_matB;

public:
	void setup(QDialog* dlg)
	{
		m_ntype = -1;
		m_matA = m_matB = -1;

		name = new QLineEdit;
		name->setPlaceholderText("(leave blank for default)");

		step = new QComboBox;

		matA = new QComboBox;
		matB = new QComboBox;

		list = new QListWidget;

		QFormLayout* form = new QFormLayout;
		form->addRow("Name:", name);
		form->addRow("Step:", step);
		form->addRow("Rigid material A:", matA);
		form->addRow("Rigid material B:", matB);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QVBoxLayout* mainLayout = new QVBoxLayout;
		mainLayout->addLayout(form);
		mainLayout->addWidget(list);
		mainLayout->addWidget(bb);

		dlg->setLayout(mainLayout);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));		
		QObject::connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), dlg, SLOT(accept()));
	}
};

CDlgAddRigidConnector::CDlgAddRigidConnector(FEProject& prj, QWidget* parent) : QDialog(parent), ui(new Ui::CDlgAddRigidConnector)
{
	setWindowTitle("Add Rigid Connector");

	ui->setup(this);

	// add the steps
	FEModel& fem = prj.GetFEModel();
	for (int i = 0; i<fem.Steps(); ++i)
	{
		ui->step->addItem(QString::fromStdString(fem.GetStep(i)->GetName()));
	}

	// set the materials
	for (int i = 0; i<fem.Materials(); ++i)
	{
		GMaterial* pm = fem.GetMaterial(i);
		if (dynamic_cast<FERigidMaterial*>(pm->GetMaterialProperties()))
		{
			ui->matA->addItem(QString::fromStdString(pm->GetName()), pm->GetID());
			ui->matB->addItem(QString::fromStdString(pm->GetName()), pm->GetID());
		}
	}

	// add the rigid connectors
	unsigned int mod = prj.GetModule();
	vector<FEClassFactory*> v =  FEMKernel::FindAllClasses(mod, FE_RIGID_CONNECTOR);
	for (int i=0; i<(int)v.size(); ++i)
	{
		FEClassFactory* fac = v[i];

		QListWidgetItem* item = new QListWidgetItem;
		item->setText(QString::fromStdString(fac->GetTypeStr()));
		item->setData(Qt::UserRole, fac->GetClassID());

		ui->list->addItem(item);
	}
}

void CDlgAddRigidConnector::accept()
{
	QListWidgetItem* item = ui->list->currentItem();
	if (item == 0)
	{
		QMessageBox::critical(this, "Add Rigid Connector", "Please select a rigid connector");
		return;
	}

	ui->m_ntype = item->data(Qt::UserRole).toInt();
	ui->m_matA = ui->matA->currentData().toInt();
	ui->m_matB = ui->matB->currentData().toInt();

	QDialog::accept();
}

int CDlgAddRigidConnector::GetType()
{
	return ui->m_ntype;
}

std::string CDlgAddRigidConnector::GetName()
{
	QString t = ui->name->text();
	return t.toStdString();
}

int CDlgAddRigidConnector::GetStep()
{
	return ui->step->currentIndex();
}

int CDlgAddRigidConnector::GetMaterialA()
{
	return ui->m_matA;
}

int CDlgAddRigidConnector::GetMaterialB()
{
	return ui->m_matB;
}
