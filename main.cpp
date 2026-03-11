#include "v2raycpp.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    v2raycpp window;
    window.show();
    return app.exec();
}
