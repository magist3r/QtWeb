/*
 * Copyright (C) 2008-2010 Alexei Chaloupov <alexei.chaloupov@gmail.com>
 */
#ifndef VIEWSOURCE_H
#define VIEWSOURCE_H

#include <QMainWindow>
#include <QSyntaxHighlighter>

class HtmlHighlighter : public QSyntaxHighlighter
{
  Q_OBJECT
 
public:
    enum Construct {
        Entity,
        Tag,
		Attributes,
        Comment,
        LastConstruct = Comment
    };
 
    HtmlHighlighter(QTextDocument *document);
 
    void setFormatFor(Construct construct,
                      const QTextCharFormat &format);
    QTextCharFormat formatFor(Construct construct) const
        { return m_formats[construct]; }
 
protected:
    enum State {
        NormalState = -1,
        InComment,
        InTag
    };
 
    void highlightBlock(const QString &text);
 
private:
    QTextCharFormat m_formats[LastConstruct + 1];
	
};


class ViewSourceWindow : public QMainWindow
{
    Q_OBJECT

public:
    ViewSourceWindow(QWidget *parent, const QString&);

public slots:
    void saveFile();

private:
    void setupEditor();
    void setupFileMenu();

    QTextEdit *editor;
    HtmlHighlighter *highlighter;
	QString	source;
};

#endif // VIEWSOURCE_H
