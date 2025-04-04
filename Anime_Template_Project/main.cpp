#include "anime_homepage.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Anime_Homepage w;
    w.show();
    return a.exec();
}
