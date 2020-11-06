#include "gifdemo.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    gifDemo w;
    w.show();

    return a.exec();
}
