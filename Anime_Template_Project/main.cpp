#include "anime_homepage.h"
#include "Login_interface/responsive_form.h"


#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Responsive_form r;
    r.show();

    //Anime_Homepage w;
    //w.show();

    return a.exec();
}
