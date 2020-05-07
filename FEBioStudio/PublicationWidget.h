//#include <QPushButton>
#pragma once
#include <QFrame>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace Ui{
class CPublicationWidget;
}

//enum pubWidgetType {EXPANDABLE, SELECTABLE, CHECKABLE};

class CPublicationWidget : public QFrame
{
	Q_OBJECT

public:

	enum Selection {NONE, CHECKBOX, BUTTON};

	CPublicationWidget(Selection selection = NONE, bool expandable = true);
	CPublicationWidget(QVariantMap& data, Selection selection = NONE, bool expandable = true);
	CPublicationWidget(QString title, QString year, QString journal, QString volume,
			QString issue, QString pages, QString DOI, QStringList authorGiven,
			QStringList authorFamily, Selection selection = NONE, bool expandable = true);

	CPublicationWidget(const CPublicationWidget& obj);

	void init();

	QString ShortText();
	QString FullText();

	const QStringList& getAuthorFamily() const;
	void setAuthorFamily(const QStringList &authorFamily);
	const QStringList& getAuthorGiven() const;
	void setAuthorGiven(const QStringList &authorGiven);
	const QString& getDOI() const;
	void setDOI(const QString &doi);
	const QString& getIssue() const;
	void setIssue(const QString &issue);
	const QString& getJournal() const;
	void setJournal(const QString &journal);
	const QString& getPages() const;
	void setPages(const QString &pages);
	const QString& getTitle() const;
	void setTitle(const QString &title);
	const QString& getVolume() const;
	void setVolume(const QString &volume);
	const QString& getYear() const;
	void setYear(const QString &year);
	bool isChecked() const;
	int getSelection() const;



public slots:
	void on_expand_triggered();
	void on_shrink_triggered();
	bool isExpandable() const;

	void on_select_triggered();
	void checkBox_stateChanged(int state);

signals:
	void chosen_publication(CPublicationWidget* pub);


private:
	Ui::CPublicationWidget* ui;


	QString title;
	QString year;
	QString journal;
	QString volume;
	QString issue;
	QString pages;
	QString DOI;

	QStringList authorGiven;
	QStringList authorFamily;

	bool expandable;
	Selection selection;
};
