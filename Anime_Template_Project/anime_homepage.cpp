#include "anime_homepage.h"

Anime_Homepage::Anime_Homepage(QWidget *parent)
    : QWidget(parent)
{

    this->setPalette(QPalette(QColor(222,222,222,222)));
    setAutoFillBackground(true);
    this->resize(800,600);

    Diffusion_button1 = new Diffusion_button(this);
    Diffusion_button1->move(200,300);
    Diffusion_button1->show();

    Wave_buttons = new Wave_button(this);
    Wave_buttons->move(450, 300);
    Wave_buttons->show();





}

Anime_Homepage::~Anime_Homepage()
{



}
