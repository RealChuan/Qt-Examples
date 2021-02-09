#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 套用了旧的样式，具体细节自己调整
    const QString filePath("D:/Mine/CODE/Qt/AppTools/app-subdir/bin-64/debug/qss/Common.css");
    if(QFile::exists(filePath)){
        QFile file(filePath);
        if(file.open(QFile::ReadOnly)){
            qApp->setStyleSheet(file.readAll());
            file.close();
        }
    }

    MainWindow w;
    w.show();

    return a.exec();
}
