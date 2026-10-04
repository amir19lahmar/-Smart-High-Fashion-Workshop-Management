#pragma once
#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QSize>
#include <QString>

namespace UI {
QPixmap svgPixmap(const QString &name, const QColor &color, int size);
QIcon   svgIcon(const QString &name, const QColor &color, int size = 24);
QIcon   dotIcon(const QColor &color, int size = 14);
QPixmap cover(const QPixmap &src, const QSize &size, int radius = 0);
QPixmap machinePixmap(const QString &photoPath);      // photo de la machine ou image par défaut
QColor  etatColor(const QString &etat);
QColor  typeColor(const QString &type);
}
