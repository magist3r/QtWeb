#include <QApplication>
#include <QString>
#include <QUrl>
#include <QWebView>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWebView view;
    view.setWindowTitle("QtWebKit Smoke Test");
    view.resize(800, 600);
    QUrl url = QUrl("http://example.com/");
    if (argc > 1)
        url = QUrl::fromUserInput(QString::fromLocal8Bit(argv[1]));
    view.load(url);
    view.show();

    return app.exec();
}
