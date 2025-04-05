#include "carrier_card.h"

Carrier_card::Carrier_card(QPixmap m_carrier_card, QString main_title, QString sub_title,QWidget *parent)
    : QWidget{parent}
{
     
    this->setMaximumSize(860, 480); //最大宽高
     
    this->resize(320, 480); //设置宽和高

    this->m_carrier_card = m_carrier_card;

    this->font_related_construction(main_title, sub_title); //字体相关构造

    this->set_gradient_color();
     
    setMouseTracking(true); // 启用持续鼠标跟踪（无需按下按钮）
     
    connect(&m_clickTimer, &QTimer::timeout, this, &Carrier_card::onClickTimerTimeout); // 初始化定时器

    connect(&timer4s, &QTimer::timeout, this, &Carrier_card::circle_reset);

    shadow = new QGraphicsDropShadowEffect(Anime_button);
    shadow->setOffset(0, 0); // 设置阴影偏移量
    shadow->setBlurRadius(150); // 设置阴影模糊半径
    shadow->setColor(Qt::black); // 设置阴影颜色
    Anime_button->setGraphicsEffect(shadow); // 将阴影效果应用到按钮上
}

void Carrier_card::font_related_construction(QString main_title, QString sub_title) //字体相关构造
{
    Anime_small_text = new Card_text(main_title, sub_title,this);
    Anime_small_text->reset_animation(0);
    qDebug() << "Card_text::" << Anime_small_text->height();
    
}

void Carrier_card::mousePressEvent(QMouseEvent* event)//鼠标点击
{
    if (event->button() == Qt::LeftButton)
    {
        // 检查是否在限制时间内
        if (!m_clickTimer.isActive()) // 如果定时器未启动，则允许点击
        {
            emit carrier_card_idChanged(this->m_carrier_card_id);
            m_clickTimer.start(500); // 1000毫秒 // 启动定时器，限制1秒内再次点击
        }
        else qDebug() << "点击频率过快，请稍后再试"; // 如果在限制时间内，可以选择忽略点击事件或给出提示

    }
    
    //调用父类的鼠标点击事件
    QWidget::mousePressEvent(event);
}

void Carrier_card::mouseMoveEvent(QMouseEvent* event)
{
    //判断鼠标是否在窗口内
    if (this->isClicked == true)
    {
        mousePos = event->pos();
        qDebug() << "mousePos:" << mousePos;
        update();
    }
}

bool Carrier_card::event(QEvent* e)
{

    if (e->type() == QEvent::Enter)
    {
        this->isClicked = true;
        
        if (this->m_carrier_card_id == 2) timer4s.stop();


        qDebug() << "鼠标进入";

    }
    else if (e->type() == QEvent::Leave)
    {
        this->isClicked = false;

        if (this->m_carrier_card_id == 2) timer4s.start(100);

        qDebug() << "鼠标离开";
        update();
    }
    return QWidget::event(e);    //执行父类的event()，处理其他类型事件
}






void Carrier_card::draw_radial_gradient_circle() //绘制跟随鼠标位置的圆形渐变
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRadialGradient gradient1(mousePos.rx(), mousePos.ry(), qMax(width() , height() ) / 2); 
    gradient1.setColorAt(0.0, QColor(255, 255, 255, 15)); // 中心颜色为白色
    gradient1.setColorAt(0.8, QColor(255, 255, 255, 0)); // 边缘颜色为黑色
    gradient1.setColorAt(1.0, QColor(255, 255, 255, 0)); // 边缘颜色为黑色

    // 设置画刷为渐变
    painter.setBrush(gradient1);
    painter.setPen(Qt::NoPen); // 去除边框

    // 绘制一个矩形，填充渐变颜色
    painter.drawRect(rect());
}

void Carrier_card::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::LosslessImageRendering);

    // 裁剪出圆角矩形
    QPainterPath path;
    path.addRoundedRect(0, 0, this->width(), this->height(), 10, 10);
    painter.setClipPath(path);

    // 绘制图片
    int windowWidth = width(); // 获取窗口大小
    int windowHeight = height();
    double aspectRatio = static_cast<double>(m_carrier_card.width()) / m_carrier_card.height(); // 计算图片的新宽度和高度
    int newWidth = static_cast<int>(windowHeight * aspectRatio);
    int newHeight = windowHeight;
    int posX = (windowWidth - newWidth) / 2; // 计算图片的绘制位置，使其居中
    int posY = 0; // 图片位于顶部
    painter.drawPixmap(posX, posY, newWidth, newHeight, m_carrier_card, 0, 0, m_carrier_card.width(), m_carrier_card.height());

    //从左至右渐变色
    QLinearGradient gradient(0, 480, 860, 480);
    gradient.setColorAt(0, m_color1); 
    gradient.setColorAt(0.35, m_color2); 
    gradient.setColorAt(0.55, QColor(0, 0, 0, 0)); 
    gradient.setColorAt(1, QColor(0, 0, 0, 0)); 
    gradient.setSpread(QGradient::PadSpread);
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen); 
    painter.drawRect(0, 0, width(), height());

    //绘制低透明度黑色前景
    QColor semiTransparentRed(0, 0, 0, m_opacity);
    painter.setBrush(semiTransparentRed);
    painter.drawRect(0, 0, width(), height()); 


    //绘制跟随鼠标位置的圆形渐变
    if (isClicked == true) this->draw_radial_gradient_circle();

    //绘制扇形倒计时
    if (m_carrier_card_id == 2 && display_circle == true) draw_sector_circle();
}


void Carrier_card::draw_sector_circle() //绘制扇形倒计时
{
    QPainter    painter(this);//创建QPainter对象
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen); // 去除边框
    painter.setBrush(QColor(255, 255, 255, 140)); // 设置画刷颜色为白色

    QRect   rect(20, 450, 15, 15);
    int startAngle = 0 * 16;///*起始 0°*/
    int spanAngle = m_circle_degrees * 16;//旋转360°
    painter.drawPie(rect, startAngle, spanAngle);
}
void Carrier_card::circle_reset() //扇形圆形倒计时
{
    TimerTimeout++;
    update();
    if (TimerTimeout == 15)
    {
        display_circle = true;
    }
    if (TimerTimeout < 87 && display_circle == true)
    {
        m_circle_degrees -= 5;
    }

    if (TimerTimeout == 87)
    {
        restore_disc();
        emit carrier_card_idChanged(3);
    }
}

void Carrier_card::set_gradient_color() //设置最亮的颜色和次亮的颜色
{
    QColor clr = this->getMostFrequentColor(m_carrier_card);
    Anime_button = new Card_button(QString("查看详情"), clr, this);
    Anime_button->move(38, 315);

    clr.setAlpha(100);
    m_color1 = clr;

    clr.setAlpha(68);
    m_color2 = clr;

    clr.setAlpha(255);
    m_color3 = clr;

    qDebug() << "m_color1" << m_color1.red() << m_color1.green() << m_color1.blue() << m_color1.alpha();
    qDebug() << "m_color2" << m_color2.red() << m_color2.green() << m_color2.blue() << m_color2.alpha();;
}


QColor Carrier_card::getMostFrequentColor(const QPixmap& pixmap) //获取图片中最频繁的颜色
{
    // 将 QPixmap 转换为 QImage
    QImage image = pixmap.toImage();

    if (image.isNull()) {
        return QColor(); // 返回无效颜色表示加载失败
    }

    // 转换为RGB32格式以忽略Alpha通道
    image = image.convertToFormat(QImage::Format_RGB32);

    QHash<QRgb, int> colorCounts;
    const uchar* bits = image.constBits();
    const int width = image.width();
    const int height = image.height();
    const int bytesPerLine = image.bytesPerLine();

    // 定义过滤阈值
    const qreal minSaturation = 8.1; // 饱和度低于此值视为灰度
    const qreal maxLightnessForWhite = 0.99; // 亮度高于此值视为接近白色

    for (int y = 0; y < height; ++y) {
        const QRgb* line = reinterpret_cast<const QRgb*>(bits + y * bytesPerLine);
        for (int x = 0; x < width; ++x) {
            QRgb rgb = line[x];
            QColor color = QColor::fromRgb(rgb);

            // 转换为HSL颜色空间
            float h, s, l;
            color.getHslF(&h, &s, &l);

            // 过滤条件：饱和度低且亮度高（白色到灰色）
            if (s < minSaturation && l > maxLightnessForWhite) {
                continue; // 跳过灰度或接近白色的颜色
            }

            colorCounts[rgb]++;
        }
    }

    if (colorCounts.isEmpty()) {
        return QColor(); // 所有颜色均被过滤
    }

    // 查找最高频率颜色
    QHashIterator<QRgb, int> it(colorCounts);
    QRgb maxColor = it.peekNext().key();
    int maxCount = it.peekNext().value();

    while (it.hasNext()) {
        it.next();
        if (it.value() > maxCount) {
            maxCount = it.value();
            maxColor = it.key();
        }
    }
    return QColor::fromRgb(maxColor);
}


void Carrier_card::Anime_card_transformation(int animation_duration) //动画开始
{
    if (m_carrier_card_id != 2)
    {
        timer4s.stop();
        restore_disc();

        Anime_small_text->reset_animation(animation_duration);

        QPropertyAnimation* animation1 = new QPropertyAnimation(this, "size");
        animation1->setDuration(animation_duration);
        animation1->setStartValue(this->size());
        animation1->setEndValue(QSize(320, 480));
        animation1->setEasingCurve(QEasingCurve::Linear);

        QPropertyAnimation* animation3 = new QPropertyAnimation(this, "opacity");
        animation3->setDuration(animation_duration);
        animation3->setStartValue(this->opacity());
        animation3->setEndValue(90);
        animation3->setEasingCurve(QEasingCurve::Linear);

        animation1->start(QAbstractAnimation::DeleteWhenStopped);
        animation3->start(QAbstractAnimation::DeleteWhenStopped);
    }
    else
    {
        timer4s.start(100);

        Anime_small_text->start_animation(animation_duration);

        QPropertyAnimation* animation1 = new QPropertyAnimation(this, "size");
        animation1->setDuration(animation_duration);
        animation1->setStartValue(this->size());
        animation1->setEndValue(QSize(860, 480));
        animation1->setEasingCurve(QEasingCurve::Linear);

        QPropertyAnimation* animation3 = new QPropertyAnimation(this, "opacity");
        animation3->setDuration(animation_duration);
        animation3->setStartValue(this->opacity());
        animation3->setEndValue(0);
        animation3->setEasingCurve(QEasingCurve::Linear);

        animation1->start(QAbstractAnimation::DeleteWhenStopped);
        animation3->start(QAbstractAnimation::DeleteWhenStopped);
    }
}


void Carrier_card::Anime_card_1() //卡片形态初始化
{
    this->resize(860, 480);
    Anime_small_text->start_animation(0);
    this->setOpacity(0);
    timer4s.start(100);
}
void Carrier_card::onClickTimerTimeout()  // 定时器超时后停止
{
    m_clickTimer.stop();
}

void Carrier_card::restore_disc() //扇形圆形恢复
{
    display_circle = false;
    TimerTimeout = 0;
    m_circle_degrees = 360;
}

int Carrier_card::carrier_card_id() const
{
    return m_carrier_card_id;
}

void Carrier_card::setcarrier_card_id(int newCarrier_card_id)
{
    if (m_carrier_card_id == newCarrier_card_id)
        return;
    m_carrier_card_id = newCarrier_card_id;
    update();
}

int Carrier_card::opacity() const
{
    return m_opacity;
}

void Carrier_card::setOpacity(int newOpacity)
{
    m_opacity = newOpacity;
    update();
}

int Carrier_card::circle_degrees() const
{
    return m_circle_degrees;
}

void Carrier_card::setCircle_degrees(int newCircle_degrees)
{
    if (m_circle_degrees == newCircle_degrees)
        return;
    m_circle_degrees = newCircle_degrees;
    qDebug() << "circle_degrees changed to:" << m_circle_degrees;
    update();
}
