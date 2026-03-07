#include <QApplication>
#include <QCoreApplication>
#include <QString>
#include <QUrl>
#include <QWebView>
#include <QSslSocket>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (QSslSocket::supportsSsl())
        QSslSocket::addDefaultCaCertificates(QCoreApplication::applicationDirPath() + QStringLiteral("/ca-certificates.crt"));

    QWebView view;
    view.setWindowTitle("QtWebKit Smoke Test");
    view.resize(800, 600);
    QUrl url(QStringLiteral("https://example.com/"));
    if (argc > 1)
        url = QUrl::fromUserInput(QString::fromLocal8Bit(argv[1]));

    view.load(url);
    view.show();

    return app.exec();
}
