#include "bannerwidget.h"
#include "uihelpers.h"

#include <QFontMetrics>
#include <QImage>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>

static QPixmap faded(const QPixmap &src, qreal leftFrac, qreal rightFrac)
{
    QImage img = src.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QPainter p(&img);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    QLinearGradient g(0, 0, img.width(), 0);
    g.setColorAt(0, leftFrac > 0 ? QColor(0, 0, 0, 0) : QColor(0, 0, 0, 255));
    if (leftFrac > 0)  g.setColorAt(leftFrac, QColor(0, 0, 0, 255));
    if (rightFrac > 0) { g.setColorAt(1 - rightFrac, QColor(0, 0, 0, 255)); g.setColorAt(1, QColor(0, 0, 0, 0)); }
    else               g.setColorAt(1, QColor(0, 0, 0, 255));
    p.fillRect(img.rect(), g);
    p.end();
    return QPixmap::fromImage(img);
}

BannerWidget::BannerWidget(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(132);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void BannerWidget::resizeEvent(QResizeEvent *)
{
    rebuild();
}

void BannerWidget::rebuild()
{
    const int h = height();
    QPixmap fabric(QStringLiteral(":/images/banner_fabric.jpg"));
    QPixmap machine(QStringLiteral(":/images/banner_machine.jpg"));
    m_fabric  = faded(fabric.scaledToHeight(h, Qt::SmoothTransformation), 0.0, 0.55);
    m_machine = faded(machine.scaledToHeight(h, Qt::SmoothTransformation), 0.25, 0.12);
}

void BannerWidget::paintEvent(QPaintEvent *)
{
    if (m_fabric.isNull()) rebuild();

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    const qreal w = width(), h = height();

    QLinearGradient g(0, 0, w, 0);
    g.setColorAt(0.00, QColor("#d9cdb9"));
    g.setColorAt(0.20, QColor("#ebe0cd"));
    g.setColorAt(0.55, QColor("#f2e9d9"));
    g.setColorAt(1.00, QColor("#e6d8c0"));
    p.fillRect(rect(), g);

    p.drawPixmap(0, 0, m_fabric);
    p.drawPixmap(int(w * 0.81 - m_machine.width() + 14), 0, m_machine);

    // --- Titre (engrenage + texte)
    QFont title(QStringLiteral("Georgia"));
    title.setStyleHint(QFont::Serif);
    title.setBold(true);
    title.setPixelSize(int(h * 0.36));
    QFontMetrics tm(title);
    const QString t = QStringLiteral("Gestion des Machines");
    const int gear = int(h * 0.34);
    const qreal totalW = gear + 12 + tm.horizontalAdvance(t);
    const qreal cx = w * 0.385;
    qreal x0 = cx - totalW / 2;
    const qreal baseY = h * 0.46;

    p.drawPixmap(int(x0), int(baseY - gear * 0.78), UI::svgPixmap(QStringLiteral("gear"), QColor("#0f2b50"), gear));
    p.setFont(title);
    p.setPen(QColor("#13233f"));
    p.drawText(QPointF(x0 + gear + 12, baseY + tm.ascent() * 0.36), t);

    // --- Sous-titre
    QFont sub(QStringLiteral("Georgia"));
    sub.setStyleHint(QFont::Serif);
    sub.setItalic(true);
    sub.setPixelSize(int(h * 0.135));
    p.setFont(sub);
    p.setPen(QColor("#a98652"));
    p.drawText(QRectF(cx - w * 0.2, h * 0.58, w * 0.4, h * 0.18), Qt::AlignCenter,
               QStringLiteral("Contrôlez  ·  Suivez  ·  Anticipez"));

    // --- Séparateur avec étoile
    const qreal ly = h * 0.86;
    p.setPen(QPen(QColor("#b89a68"), 1));
    p.drawLine(QPointF(cx - 130, ly), QPointF(cx - 14, ly));
    p.drawLine(QPointF(cx + 14, ly), QPointF(cx + 130, ly));
    QPolygonF star;
    const qreal r = 8, ri = 2.4;
    star << QPointF(cx, ly - r) << QPointF(cx + ri, ly - ri) << QPointF(cx + r, ly) << QPointF(cx + ri, ly + ri)
         << QPointF(cx, ly + r) << QPointF(cx - ri, ly + ri) << QPointF(cx - r, ly) << QPointF(cx - ri, ly - ri);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#13233f"));
    p.drawPolygon(star);

    // --- Citation
    const qreal qx = w * 0.825, qw = w * 0.15;
    QFont q(QStringLiteral("Georgia"));
    q.setStyleHint(QFont::Serif);
    q.setItalic(true);
    q.setPixelSize(int(h * 0.135));
    p.setFont(q);
    p.setPen(QColor("#9a7a45"));
    p.drawText(QRectF(qx, h * 0.22, qw, h * 0.56), Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap,
               QStringLiteral("Une machine bien gérée est une production sans arrêt"));
    QFont big(QStringLiteral("Georgia"));
    big.setBold(true);
    big.setPixelSize(int(h * 0.3));
    p.setFont(big);
    p.setPen(QColor("#c9a36a"));
    p.drawText(QPointF(qx - 18, h * 0.42), QStringLiteral("\u201C"));
    p.drawText(QPointF(w - 34, h * 0.86), QStringLiteral("\u201D"));
}
