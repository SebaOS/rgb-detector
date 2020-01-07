#include "dialog.h"
#include <iostream>
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icons/main/icon.png"));
    Dialog w;
    w.setWindowTitle("RGB DETECTOR");
    w.show();
    return a.exec();
}
