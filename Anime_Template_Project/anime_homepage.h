#ifndef ANIME_HOMEPAGE_H
#define ANIME_HOMEPAGE_H

#include <QWidget>
//轮播
#include "Carousel_card/carousel_card.h"
#include "Carousel_card/Card_text.h"
#include "Carousel_card/Card_button.h"
//闪亮章鱼
#include "button_class/Diffusion_button.h"
#include "button_class/Wave_button.h"




class Anime_Homepage : public QWidget
{
    Q_OBJECT

public:
    Anime_Homepage(QWidget *parent = nullptr);
    ~Anime_Homepage();

    Diffusion_button* Diffusion_button1;
    Wave_button* Wave_buttons;

};
#endif // ANIME_HOMEPAGE_H
