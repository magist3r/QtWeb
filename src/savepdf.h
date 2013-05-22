#ifndef SAVEPDF_H
#define SAVEPDF_H

#include <QDialog>
#include "ui_savepdf.h"

class QWebFrame;

class SavePDF : public QDialog
{
	Q_OBJECT

public:
	SavePDF(const QString& title, QWebFrame* frame, QWidget *parent = 0);
	~SavePDF();

protected slots:
	void save();
	void getFileName();

private:
	Ui::savepdfClass ui;
	QWebFrame* m_frame;
};

#endif // SAVEPDF_H
