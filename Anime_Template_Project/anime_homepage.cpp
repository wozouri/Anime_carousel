#include "anime_homepage.h"

Anime_Homepage::Anime_Homepage(QWidget *parent)
    : QWidget(parent)
{
    Anime_carrier_card = new Carousel_card();
    //设置小部件在窗口正中央
    Anime_carrier_card->show();

}

Anime_Homepage::~Anime_Homepage()
{



}
