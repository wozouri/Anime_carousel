#ifndef ANIME_HOMEPAGE_H
#define ANIME_HOMEPAGE_H

#include <QWidget>

#include "Carousel_card/carousel_card.h"
#include "Carousel_card/Card_text.h"
#include "Carousel_card/Card_button.h"


class Anime_Homepage : public QWidget
{
    Q_OBJECT

public:
    Anime_Homepage(QWidget *parent = nullptr);
    ~Anime_Homepage();

    Carousel_card* Anime_carrier_card;
};
#endif // ANIME_HOMEPAGE_H
