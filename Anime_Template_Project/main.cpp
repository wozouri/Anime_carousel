#include "anime_homepage.h"
#include "dial_class/Knob_page.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Knob_page w;
    w.show();
    return a.exec();
}
