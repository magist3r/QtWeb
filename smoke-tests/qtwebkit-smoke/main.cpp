#include <QApplication>
#include <QTimer>
#include <QUrl>
#include <QWebView>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    int result = 2;

    QWebView view;
    view.setWindowTitle("QtWebKit Smoke Test");
    view.resize(800, 600);
    QObject::connect(&view, &QWebView::loadFinished, [&app, &result](bool ok) {
        result = ok ? 0 : 1;
        app.quit();
    });
    QTimer::singleShot(10000, &app, SLOT(quit()));
    view.setHtml(
        "<html><body><h1>QtWebKit smoke test</h1>"
        "<p>If this renders, QtWebKitWidgets is functional.</p>"
        "</body></html>",
        QUrl("about:blank"));
    view.show();

    app.exec();
    return result;
}
