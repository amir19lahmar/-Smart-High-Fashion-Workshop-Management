#include "uihelpers.h"

#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
#include <QSvgRenderer>

namespace UI {

QPixmap svgPixmap(const QString &name, const QColor &color, int size)
{
    QFile f(QStringLiteral(":/icons/%1.svg").arg(name));
    QByteArray data;
    if (f.open(QIODevice::ReadOnly))
        data = f.readAll();
    data.replace("currentColor", color.name().toUtf8());

    QSvgRenderer renderer(data);
    const int px = size * 2;                     // rendu 2x pour les écrans HiDPI
    QPixmap pm(px, px);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    renderer.render(&p);
    p.end();
    pm.setDevicePixelRatio(2.0);
    return pm;
}

QIcon svgIcon(const QString &name, const QColor &color, int size)
{
    return QIcon(svgPixmap(name, color, size));
}

QIcon dotIcon(const QColor &color, int size)
{
    const int px = size * 2;
    QPixmap pm(px, px);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(color);
    p.setPen(Qt::NoPen);
    p.drawEllipse(QRectF(px * 0.12, px * 0.12, px * 0.76, px * 0.76));
    p.end();
    pm.setDevicePixelRatio(2.0);
    return QIcon(pm);
}

QPixmap cover(const QPixmap &src, const QSize &size, int radius)
{
    const qreal dpr = 2.0;
    const QSize px(int(size.width() * dpr), int(size.height() * dpr));
    QPixmap scaled = src.scaled(px, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap out(px);
    out.fill(Qt::transparent);
    QPainter p(&out);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, px.width(), px.height()), radius * dpr, radius * dpr);
    p.setClipPath(path);
    p.drawPixmap((px.width() - scaled.width()) / 2, (px.height() - scaled.height()) / 2, scaled);
    p.end();
    out.setDevicePixelRatio(dpr);
    return out;
}

QPixmap machinePixmap(const QString &photoPath)
{
    if (!photoPath.isEmpty() && QFileInfo::exists(photoPath)) {
        QPixmap pm(photoPath);
        if (!pm.isNull())
            return pm;
    }
    return QPixmap(QStringLiteral(":/images/machine.jpg"));
}

QColor etatColor(const QString &etat)
{
    if (etat == QLatin1String("Disponible"))    return QColor("#2e9e5b");
    if (etat == QLatin1String("En maintenance")) return QColor("#e6a23c");
    return QColor("#e5484d");
}

QColor typeColor(const QString &type)
{
    if (type == QLatin1String("Couture"))   return QColor("#0a2f5b");
    if (type == QLatin1String("Coupe"))     return QColor("#6aa7e0");
    if (type == QLatin1String("Finition"))  return QColor("#f0c48a");
    if (type == QLatin1String("Broderie"))  return QColor("#d6a566");
    if (type == QLatin1String("Repassage")) return QColor("#b4c7dd");
    return QColor("#9a9a9a");
}

} // namespace UI
