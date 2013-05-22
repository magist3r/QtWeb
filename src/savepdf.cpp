#include "savepdf.h"
#include <QSettings>
#include <QFileDialog>
#include <QPrinter>
#include <QWebFrame>

extern QString dirDownloads(bool create_dir);

SavePDF::SavePDF(const QString& title, QWebFrame* frame, QWidget *parent)
	: QDialog(parent), m_frame(frame)
{
	ui.setupUi(this);
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(save()));
    connect(ui.buttonBrowse, SIGNAL(clicked()), this, SLOT(getFileName()));
	
	//ui.pageSize->addItem("A0 (841 x 1189)", 5);
	//ui.pageSize->addItem("A1 (594 x 841)", 6);
	ui.pageSize->addItem("A2 (420 x 594)", 7);
	ui.pageSize->addItem("A3 (297 x 420)", 8);
	ui.pageSize->addItem("A4 (210 x 297)", 0);
	ui.pageSize->addItem("A5 (148 x 210)", 9); // ? size seems to be incorrect
	//ui.pageSize->addItem("A6 (105 x 148)", 10);
	//ui.pageSize->addItem("A7 (74 x 105)", 11);
	//ui.pageSize->addItem("A8 (52 x 74)", 12);
	//ui.pageSize->addItem("A9 (37 x 52)", 13);
	//ui.pageSize->addItem("B0 (1030 x 1456)", 14);
	//ui.pageSize->addItem("B1 (728 x 1030)", 15);
	//ui.pageSize->addItem("B2 (515 x 728)", 17);
	//ui.pageSize->addItem("B3 (364 x 515)", 18);
	//ui.pageSize->addItem("B4 (257 x 364)", 19);
	//ui.pageSize->addItem("B5 (182 x 257)", 1);
	//ui.pageSize->addItem("B6 (128 x 182)", 20);
	//ui.pageSize->addItem("B7 (91 x 128)", 21);
	//ui.pageSize->addItem("B8 (64 x 91)", 22);
	//ui.pageSize->addItem("B9 (45 x 64)", 23);
	//ui.pageSize->addItem("B10 (32 x 45)", 16);
	ui.pageSize->addItem("C5E (163 x 229)", 24);
	ui.pageSize->addItem("Comm10E (105 x 241)", 25);
	ui.pageSize->addItem("DLE (110 x 220)", 26);
	ui.pageSize->addItem("Executive (191 x 254)", 4);
	ui.pageSize->addItem("Folio (210 x 330)", 27);
	ui.pageSize->addItem("Ledger (432 x 279)", 28);
	ui.pageSize->addItem("Legal (216 x 356)", 3);
	ui.pageSize->addItem("Letter (216 x 279)", 2);
	ui.pageSize->addItem("Tabloid (279 x 432)", 29);

	QSettings settings;
	settings.beginGroup(QLatin1String("savePDF"));

	ui.pageSize->setCurrentIndex(ui.pageSize->findData(settings.value(QLatin1String("size"),0).toString())); // Letter by default
	ui.colorMode->setCurrentIndex(settings.value(QLatin1String("color"),0).toInt()); // Color by default

	ui.portrait->setChecked(settings.value(QLatin1String("portrait"),true).toBool()); // Portrait by default
	ui.landscape->setChecked(!settings.value(QLatin1String("portrait"),true).toBool());

	ui.leftMargin->setValue(settings.value(QLatin1String("left"),10).toInt());
	ui.rightMargin->setValue(settings.value(QLatin1String("right"),10).toInt());
	ui.topMargin->setValue(settings.value(QLatin1String("top"),15).toInt());
	ui.bottomMargin->setValue(settings.value(QLatin1String("bottom"),25).toInt());

	QString path = settings.value(QLatin1String("lastPath"),"").toString();

	settings.endGroup();

	if (path.isEmpty())
		path = dirDownloads(true);
	
	if (path.right(1) != QDir::separator())
		path += QDir::separator();

	
	if (title.isEmpty())
		path += "Document.PDF";
	else
		path += (QString(title).replace(QRegExp("[:/<>?*|]"), "_").replace(QDir::separator(), '_') + ".PDF");

	ui.fileName->setText( QDir::toNativeSeparators(path) );
}

SavePDF::~SavePDF()
{
}

extern bool ShellOpenExplorer(QString path);

void SavePDF::save()
{
	QSettings settings;
	settings.beginGroup(QLatin1String("savePDF"));

	
	int size = ui.pageSize->itemData( ui.pageSize->currentIndex() ).toInt();
	settings.setValue(QLatin1String("size"), size ); 

	int colorMode = ui.colorMode->currentIndex();
	settings.setValue(QLatin1String("color"), colorMode); 

	bool portrait = ui.portrait->isChecked();
	settings.setValue(QLatin1String("portrait"), portrait);

	int left = ui.leftMargin->value();
	settings.setValue(QLatin1String("left"), left);

	int right = ui.rightMargin->value();
	settings.setValue(QLatin1String("right"), right);

	int top = ui.topMargin->value();
	settings.setValue(QLatin1String("top"), top);
	
	int bottom = ui.bottomMargin->value();
	settings.setValue(QLatin1String("bottom"), bottom);

	QFileInfo fi(ui.fileName->text());
	QString path =  fi.absolutePath();
	settings.setValue(QLatin1String("lastPath"), QDir::toNativeSeparators(path));

	settings.endGroup();

	QPrinter p;
	
	p.setColorMode(colorMode == 0 ? QPrinter::Color : QPrinter::GrayScale );
	p.setOrientation( portrait ? QPrinter::Portrait : QPrinter::Landscape);
	p.setPaperSize ( (QPrinter::PaperSize) size );
	p.setOutputFormat ( QPrinter::PdfFormat );
	p.setOutputFileName( ui.fileName->text() );
	p.setPageMargins(left, top, right, bottom, QPrinter::Millimeter);

	m_frame->print( &p );

	accept();

	ShellOpenExplorer(ui.fileName->text());
}

void SavePDF::getFileName()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Save File"), QString(),
            tr("PDF Documents (*.pdf);;All files (*.*)"));

    if (file.isEmpty())
		return;

	
	ui.fileName->setText( QDir::toNativeSeparators(file) );
}
