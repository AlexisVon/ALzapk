// Copyright (c) 2026 AlexisVon

#include "Alexis_Lza.h"

#include <QApplication>
#include <QFile>
#include <QFontDatabase>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QFontDatabase::addApplicationFont(":/FiraCode.ttf");
    app.setFont(QFont("Fira Code", 9));

    AlexisLza window;
    QFile qss_file(":/dark.qss");
    if (qss_file.open(QIODevice::ReadOnly)) window.setStyleSheet(QString::fromUtf8(qss_file.readAll()));
    window.setWindowIcon(QIcon(":/icon.ico"));
    window.setWindowTitle("LZA");
    window.show();
    return app.exec();
}
