#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName(QLatin1String("Filippo"));
    QApplication::setApplicationDisplayName(QLatin1String("SVG Station Plan Editor"));
    QApplication a(argc, argv);

    EditorMainWindow w;
    w.show();

    return a.exec();
}
