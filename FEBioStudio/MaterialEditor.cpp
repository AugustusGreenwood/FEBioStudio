#include "stdafx.h"
#include "ui_materialeditor.h"
#include <QComboBox>
#include <MeshTools/FEMaterialFactory.h>
#include <assert.h>

void FillComboBox(QComboBox* pc, int nclass, int module, bool btoplevelonly)
{
	FEMaterialFactory& MF = *FEMaterialFactory::GetInstance();

	FEMatDescIter it = MF.FirstMaterial();
	int NMAT = MF.Materials();
	for (int i = 0; i<NMAT; ++i, ++it)
	{
		unsigned int nflags = (*it)->GetFlags();
		int mod = (*it)->GetModule();

		if (mod & module)
		{
			if ((btoplevelonly==false) || (nflags & MaterialFlags::TOPLEVEL))
			{
				if (nclass < 0x0100)
				{
					if ((*it)->GetClassID() & nclass)
					{
						const char* szname = (*it)->GetTypeString();
						pc->addItem(QString(szname), (*it)->GetTypeID());
					}
				}
				else
				{
					if ((*it)->GetClassID() == nclass)
					{
						const char* szname = (*it)->GetTypeString();
						pc->addItem(QString(szname), (*it)->GetTypeID());
					}
				}
			}
		}
	}
}

void CMaterialEditor::SetModules(int module)
{
	ui->m_module = module;
}

void CMaterialEditor::SetMaterial(FEMaterial* mat)
{
	ui->ClearTree();
	MaterialEditorItem* item = ui->topLevelItem(0);
	FEMaterialFactory& MF = *FEMaterialFactory::GetInstance();
	int nclass = MF.ClassID(mat);
	item->SetClassID(nclass);
	item->SetMaterial(mat);
	ui->setMaterialCategory(nclass);
}

QString CMaterialEditor::GetMaterialName() const
{
	return ui->name->text();
}

FEMaterial* CMaterialEditor::GetMaterial()
{
	MaterialEditorItem* it = ui->topLevelItem(0);
	return it->GetMaterial();
}

CMaterialEditor::CMaterialEditor(QWidget* parent) : QDialog(parent), ui(new Ui::CMaterialEditor)
{
	setMinimumSize(400, 400);
	setWindowTitle("Add Material");
	ui->setupUi(this);
}

void CMaterialEditor::showEvent(QShowEvent* ev)
{
	ui->matClass->clear();

	for (int i=0; i<FEMaterialFactory::Categories(); ++i)
	{
		FEMatCategory& mc = FEMaterialFactory::GetCategory(i);

		if (mc.GetModule() & ui->m_module)
		{
			ui->addMaterialCategory(mc.GetName(), mc.GetID());
		}
	}

	GMaterial* gmat = ui->mat;
	if (gmat)
	{
		setWindowTitle("Change Material");
		ui->name->setText(QString::fromStdString(gmat->GetName()));

		// create a copy of the material (just in case we cancel)
		FEMaterial* pmat = gmat->GetMaterialProperties();
		if (pmat)
		{
			FEMaterial* pmCopy = FEMaterialFactory::Create(pmat->Type());
			pmCopy->copy(pmat);
			SetMaterial(pmCopy);
			ui->mat = 0;
		}
	}
}

void CMaterialEditor::SetInitMaterial(GMaterial* pm)
{	
	ui->mat = pm;
}

void CMaterialEditor::on_matClass_currentIndexChanged(int n)
{
	ui->ClearTree();
}

void CMaterialEditor::on_tree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous)
{
	if (previous)
	{
		ui->tree->removeItemWidget(previous, 1);
		ui->matList = 0;
	}

	if (current)
	{
		// get the material editor item
		MaterialEditorItem* item = dynamic_cast<MaterialEditorItem*>(current);
		assert(item);

		// get the class ID
		int nclass = item->GetClassID();

		ui->matList = new QComboBox;
		FillComboBox(ui->matList, nclass, ui->m_module, (item->ParentMaterial() == 0));

		int index = -1;
		FEMaterial* pm = item->GetMaterial();
		if (pm)
		{
			index = ui->matList->findData(pm->Type());
			assert(index != -1);
		}
		ui->matList->setCurrentIndex(index);

		QObject::connect(ui->matList, SIGNAL(currentIndexChanged(int)), this, SLOT(materialChanged(int)));
		ui->tree->setItemWidget(current, 1, ui->matList);
	}
}

void CMaterialEditor::materialChanged(int n)
{
	MaterialEditorItem* it = ui->currentItem();
	int ntype = ui->matList->itemData(n).toInt();

	FEMaterialFactory& MF = *FEMaterialFactory::GetInstance();
	FEMaterial* pmat = MF.Create(ntype); assert(pmat);

	it->SetMaterial(pmat);
}

void CMaterialEditor::accept()
{
	QDialog::accept();
}
