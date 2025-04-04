#include "carousel_card.h"


Carousel_card::Carousel_card(QWidget *parent)
    : QWidget{parent}
{
    this->setMaximumSize(1900, 530);
    this->setMinimumSize(1300, 530);
    this->Anime_basic_information();
    this->Anime_cards_sorting(Anime_seven_cards_list,
        Anime_cards_startX,
        Anime_cards_cardWidth,
        Anime_cards_spacing);
}

void Carousel_card::Anime_card_position_update(int m_carrier_card_id)
{
    qDebug() << "m_carrier_card_id" << m_carrier_card_id;
    int timeA = animation_duration;
    if (m_carrier_card_id > 2)
    {
        int x = m_carrier_card_id - 2;
        int j = x;
        for (int i = 0; i < x; i++)
        {
            timeA = timeA - 70;
            Anime_seven_cards_list.append(Anime_seven_cards_list.takeFirst());
            for (Carrier_card* Anime_cards : Anime_seven_cards_list)
            {
                if (Anime_cards == nullptr) continue;
                int j = Anime_seven_cards_list.indexOf(Anime_cards);
                Anime_cards->setcarrier_card_id(j);
                if (Anime_seven_cards_list.length() - 1 == j) Anime_cards->move(Anime_seven_cards_zasyo_list[j]);
                this->Anime_Anima_set(Anime_cards, Anime_seven_cards_zasyo_list[j], timeA);
            }
        }
        j--;
    }
    else if (m_carrier_card_id < 2)
    {
        int x = 2 - m_carrier_card_id;
        Anime_seven_cards_list.prepend(Anime_seven_cards_list.takeLast());
        for (Carrier_card* Anime_cards : Anime_seven_cards_list)
        {
            if (Anime_cards == nullptr) continue;
            int j = Anime_seven_cards_list.indexOf(Anime_cards);
            Anime_cards->setcarrier_card_id(j);
            if (Anime_cards->carrier_card_id() == 0) Anime_cards->move(Anime_seven_cards_zasyo_list.first());
            if (Anime_seven_cards_list.length() - 1 == j) Anime_cards->move(Anime_seven_cards_zasyo_list[j]);
            this->Anime_Anima_set(Anime_cards, Anime_seven_cards_zasyo_list[j], timeA);
        }
    }
}

void Carousel_card::onwheel_TimerTimeout()
{
    clickTimer.stop();
}
void  Carousel_card::Anime_Anima_set(Carrier_card* Anime_cards,QPoint Anime_zasyo, int Anime_time)
{
    Anime_cards->raise();
    Anime_cards->Anime_card_transformation(Anime_time);
    QPropertyAnimation* Anime_anima = new QPropertyAnimation(Anime_cards, "pos");
    Anime_anima->setDuration(Anime_time);
    Anime_anima->setStartValue(Anime_cards->pos());
    Anime_anima->setEndValue(Anime_zasyo);
    Anime_anima->setEasingCurve(QEasingCurve::Linear);
    Anime_anima->start(QAbstractAnimation::DeleteWhenStopped);
}
void Carousel_card::Anime_basic_information()
{
    QFile file(":/json/json/Anime_care_attributes.json");
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "打开文件失败";
        return;
    }
    QByteArray jsonData = file.readAll();
    file.close();
    QJsonDocument document = QJsonDocument::fromJson(jsonData);
    if (document.isNull() && !document.isObject())
    {
        qDebug() << "解析JSON失败";
        return;
    }
    QJsonObject rootObject = document.object();
    QJsonArray hobbiesArray = rootObject["Anime_img_str_list"].toArray();

    for (const QJsonValue& value : hobbiesArray) {
        QJsonObject addressObject = value.toObject();
        if (!QFile::exists(addressObject["img"].toString())) //判断图片资源文件是否存在
        {
            qDebug() << "图片资源文件不存在";
            continue;
        }
        Carrier_card* Anime_cards = new Carrier_card(QPixmap(addressObject["img"].toString()),
            addressObject["str_1"].toString(),
            addressObject["str_2"].toString(),
            this);
        Anime_cards->Anime_button->setWebsite_url(addressObject["anime_url"].toString());
        connect(Anime_cards, &Carrier_card::carrier_card_idChanged, this, &Carousel_card::Anime_card_position_update);
        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(Anime_cards);
        shadow->setOffset(0, 0); // 设置阴影偏移量
        shadow->setBlurRadius(80); // 设置阴影模糊半径
        shadow->setColor(Anime_cards->m_color3); // 设置阴影颜色
        Anime_cards->setGraphicsEffect(shadow);
        Anime_seven_cards_list.append(Anime_cards);
    }
    connect(this, &Carousel_card::wheel_signalChanged, this, &Carousel_card::Anime_card_position_update);
    connect(&clickTimer, &QTimer::timeout, this, &Carousel_card::onwheel_TimerTimeout); // 初始化定时器
}

void Carousel_card::Anime_cards_sorting(QList<Carrier_card *> &Anime_seven_cards_list,
    int Anime_cards_startX, 
    int Anime_cards_cardWidth, 
    int Anime_cards_spacing)
{
    int currentX = Anime_cards_startX;
    for (Carrier_card* Anime_cards : Anime_seven_cards_list)
    {
        if (Anime_cards == nullptr) continue;
        Anime_cards->setcarrier_card_id(Anime_seven_cards_list.indexOf(Anime_cards));
        if (Anime_cards->carrier_card_id() == 2) Anime_cards->Anime_card_1();
        if (Anime_cards->carrier_card_id() == 3) currentX += 540;
        Anime_cards->move(currentX, 13);
        Anime_seven_cards_zasyo_list.append(QPoint(currentX, 13));
        currentX += Anime_cards_cardWidth + Anime_cards_spacing;
    }
}

void Carousel_card::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton) emit wheel_signalChanged(San);
    QWidget::mousePressEvent(event);
}

void Carousel_card::wheelEvent(QWheelEvent* event)
{
    if (event->angleDelta().y() > 0)
    {
        if (!clickTimer.isActive())
        {
            emit wheel_signalChanged(Yizi);
            clickTimer.start(310);
        }            
    }
    else if (event->angleDelta().y() < 0)
    {
        if (!clickTimer.isActive())
        {
            emit wheel_signalChanged(San);
            clickTimer.start(170);
        }
    }   
}













