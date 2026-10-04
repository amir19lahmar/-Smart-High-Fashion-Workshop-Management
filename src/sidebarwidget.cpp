#include "sidebarwidget.h"

#include <QPainter>

// Limites des 3 zones dans l'image d'origine (534 x 1600 px) :
//   0 .. kTopEnd        : logo FashioNova
//   kTopEnd .. kBotStart : fond bleu uni (zone étirable, derrière les boutons)
//   kBotStart .. fin    : photo + slogan « Fashion is a lifestyle »
static const int kTopEnd   = 420;
static const int kBotStart = 840;

SidebarWidget::SidebarWidget(QWidget *parent)
    : QFrame(parent), m_bg(QStringLiteral(":/images/sidebar_bg.jpg"))
{
}

void SidebarWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.fillRect(rect(), QColor("#07213f"));
    if (m_bg.isNull())
        return;

    const qreal w  = width();
    const qreal h  = height();
    const qreal bw = m_bg.width();
    const qreal bh = m_bg.height();
    const qreal s  = w / bw;                         // échelle : l'image occupe toute la largeur

    const qreal dstTop = kTopEnd * s;
    const qreal dstBot = (bh - kBotStart) * s;

    if (h < dstTop + dstBot) {                       // fenêtre vraiment très basse : image entière
        p.drawPixmap(QRectF(0, 0, w, h), m_bg, QRectF(0, 0, bw, bh));
        return;
    }
    p.drawPixmap(QRectF(0, 0, w, dstTop), m_bg, QRectF(0, 0, bw, kTopEnd));
    p.drawPixmap(QRectF(0, dstTop, w, h - dstTop - dstBot), m_bg,
                 QRectF(0, kTopEnd, bw, kBotStart - kTopEnd));
    p.drawPixmap(QRectF(0, h - dstBot, w, dstBot), m_bg, QRectF(0, kBotStart, bw, bh - kBotStart));
}
