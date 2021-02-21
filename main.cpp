#include "mainwindow.h"
#include <QApplication>
#include <QTextCodec>
int main(int argc, char *argv[])
{
#if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
    QApplication::addLibraryPath("./plugins");
#if _MSC_VER
    QTextCodec *codec = QTextCodec::codecForName("gbk");
#else
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
#endif
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);
#else
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    QTextCodec::setCodecForLocale(codec);
#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
