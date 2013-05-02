/*
 * Copyright (C) 2008-2010 Alexei Chaloupov <alexei.chaloupov@gmail.com>
 */
#include "viewsource.h"
#include <QTextEdit>
#include <QFile>
#include <QMenu>
#include <QMenuBar>
#include <QTextCodec>
#include <QFileDialog>

HtmlHighlighter::HtmlHighlighter(QTextDocument *document)
    : QSyntaxHighlighter(document)
{
    QTextCharFormat entityFormat;
    entityFormat.setForeground(QColor(0, 128, 0));
    entityFormat.setFontWeight(QFont::Bold);
    setFormatFor(Entity, entityFormat);
 
    QTextCharFormat tagFormat;
    tagFormat.setForeground(QColor(192, 16, 112));
    tagFormat.setFontWeight(QFont::Bold);
    setFormatFor(Tag, tagFormat);

    QTextCharFormat attrFormat;
    attrFormat.setForeground(QColor(128, 0, 128));
    attrFormat.setFontWeight(QFont::Bold);
    setFormatFor(Attributes, attrFormat);

    QTextCharFormat commentFormat;
    commentFormat.setForeground(QColor(128, 10, 74));
    commentFormat.setFontItalic(true);
    setFormatFor(Comment, commentFormat);
}

void HtmlHighlighter::setFormatFor(Construct construct,
                            const QTextCharFormat &format)
{
    m_formats[construct] = format;
    rehighlight();
}


void HtmlHighlighter::highlightBlock(const QString &text)
{
    int state = previousBlockState();
    int len = text.length();
    int start = 0;
    int pos = 0;
	int space = -1;

	while (pos < len) 
	{
        switch (state) 
		{
        case NormalState:
        default:
            while (pos < len) {
                QChar ch = text.at(pos);
                if (ch == '<') {
                    if (text.mid(pos, 4) == "<!--") {
                        state = InComment;
                    } else {
                        state = InTag;
                    }
                    break;
                } else if (ch == '&') {
                    start = pos;
                    while (pos < len
                           && text.at(pos++) != ';')
                        ;
                    setFormat(start, pos - start,
                              m_formats[Entity]);
                } else {
                    ++pos;
                }
            }
            break;
		case InComment:
            start = pos;
            while (pos < len) {
                if (text.mid(pos, 3) == "-->") {
                    pos += 3;
                    state = NormalState;
                    break;
                } else {
                    ++pos;
                }
            }
            setFormat(start, pos - start,
                      m_formats[Comment]);
            break;
		case InTag:
            QChar quote = QChar::Null;
            start = pos;
			space = -1;
            while (pos < len) {
                QChar ch = text.at(pos);
                if (quote.isNull()) 
				{
                    if (ch == '\'' || ch == '"') 
					{
                        quote = ch;
                    }
					else
					if (ch.isSpace() && space == -1) 
					{
						space = pos;
					}
					else 
						if (ch == '>') 
						{
							++pos;
							state = NormalState;
							break;
						}
                } 
				else 
					if (ch.isSpace() && space == -1) 
					{
						space = pos;
					}
					else
					if (ch == quote) 
					{
						quote = QChar::Null;
					}
                ++pos;
            }
			if (space == -1)
			{
				setFormat(start, pos - start, m_formats[Tag]);
			}
			else
			{
				setFormat(start, pos - start, m_formats[Tag]);
				setFormat(space, pos - space - 1, m_formats[Attributes]);
			}
        }
    }

  setCurrentBlockState(state);
}


ViewSourceWindow::ViewSourceWindow(QWidget *parent, const QString& src)
    : QMainWindow(parent), source(src)
{
    setupFileMenu();
    setupEditor();

    setCentralWidget( (QWidget*)editor );
    setWindowTitle(tr("Source Viewer"));
}

void ViewSourceWindow::saveFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("File Save As"), "Source.html", "Html Files (*.html *.htm)");

    if (!fileName.isEmpty()) 
	{
        QFile file(fileName);
        if (file.open(QFile::WriteOnly | QFile::Text))
		{
			file.write(editor->toPlainText().toUtf8());
		}
    }
}

void ViewSourceWindow::setupEditor()
{
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);

    editor = new QTextEdit;
    editor->setFont(font);

    highlighter = new HtmlHighlighter(editor->document());

	QTextCodec* codec = QTextCodec::codecForHtml(source.toUtf8(), QTextCodec::codecForName( "utf-8" ));
	if (codec)
	{
		editor->setPlainText(codec->toUnicode(source.toUtf8()));
	}
	else
	{
		editor->setPlainText(source.toUtf8());
	}
}

void ViewSourceWindow::setupFileMenu()
{
    QMenu *fileMenu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(fileMenu);

    fileMenu->addAction(tr("&Save As..."), this, SLOT(saveFile()), QKeySequence::SaveAs);
                        
    fileMenu->addAction(tr("&Close"), this, SLOT(close()),	QKeySequence::Close);
}

