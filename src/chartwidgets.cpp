#include "chartwidgets.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QtMath>

static QFont px(int pixelSize, bool bold = false)
{
    QFont f;
    f.setPixelSize(pixelSize);
    f.setBold(bold);
    return f;
}

/* ------------------------------ DonutChart ------------------------------ */
DonutChart::DonutChart(QWidget *parent) : QWidget(parent) {}

void DonutChart::setData(const QVector<Slice> &slices, const QString &unit)
{
    m_slices = slices;
    m_unit = unit;
    update();
}

void DonutChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int total = 0;
    for (const Slice &s : m_slices) total += s.value;

    const int side = qMin(height() - 8, 168);
    const QRectF rc(6, (height() - side) / 2.0, side, side);
    const qreal pw = side * 0.22;
    const QRectF arc = rc.adjusted(pw / 2, pw / 2, -pw / 2, -pw / 2);

    if (total == 0) {
        QPen pen(QColor("#e6dfd1"), pw);
        p.setPen(pen);
        p.drawEllipse(arc);
    } else {
        int start = 90 * 16;
        for (const Slice &s : m_slices) {
            const int span = -qRound(360.0 * 16 * s.value / total);
            QPen pen(s.color, pw);
            pen.setCapStyle(Qt::FlatCap);
            p.setPen(pen);
            p.drawArc(arc, start, span);
            start += span;
        }
    }

    // Total au centre
    p.setPen(QColor("#0a2748"));
    p.setFont(px(24, true));
    p.drawText(QRectF(rc.x(), rc.center().y() - 26, side, 28), Qt::AlignCenter, QString::number(total));
    p.setFont(px(12));
    p.setPen(QColor("#555555"));
    p.drawText(QRectF(rc.x(), rc.center().y() + 2, side, 18), Qt::AlignCenter, m_unit);

    // Légende
    const qreal lx = rc.right() + 26;
    const qreal rowH = 29;
    qreal y = height() / 2.0 - rowH * m_slices.size() / 2.0;
    p.setFont(px(13));
    for (const Slice &s : m_slices) {
        p.setPen(Qt::NoPen);
        p.setBrush(s.color);
        p.drawEllipse(QPointF(lx + 7, y + rowH / 2), 7, 7);
        p.setPen(QColor("#23252e"));
        p.drawText(QRectF(lx + 22, y, width() - lx - 110, rowH), Qt::AlignVCenter | Qt::AlignLeft, s.label);
        const int pct = total ? qRound(100.0 * s.value / total) : 0;
        p.drawText(QRectF(width() - 92, y, 84, rowH), Qt::AlignVCenter | Qt::AlignRight,
                   QStringLiteral("%1 (%2%)").arg(s.value).arg(pct));
        y += rowH;
    }
}

/* ------------------------------- LineChart ------------------------------- */
LineChart::LineChart(QWidget *parent) : QWidget(parent) {}

void LineChart::setData(const QStringList &labels, const QVector<double> &ok,
                        const QVector<double> &soon, const QVector<double> &down)
{
    m_labels = labels; m_ok = ok; m_soon = soon; m_down = down;
    update();
}

void LineChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int n = m_labels.size();
    if (n < 2) return;

    double maxv = 1;
    for (double v : m_ok)   maxv = qMax(maxv, v);
    for (double v : m_soon) maxv = qMax(maxv, v);
    for (double v : m_down) maxv = qMax(maxv, v);
    const int step = maxv <= 15 ? 5 : 10;
    const int yMax = qMax(step * 2, int(qCeil(maxv / step)) * step);

    const QRectF plot(34, 8, width() - 34 - 14, height() - 8 - 52);
    const qreal padX = 14;

    p.setFont(px(12));
    for (int v = 0; v <= yMax; v += step) {
        const qreal y = plot.bottom() - plot.height() * v / yMax;
        p.setPen(QColor("#ece4d4"));
        p.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
        p.setPen(QColor("#6b6b6b"));
        p.drawText(QRectF(0, y - 9, 28, 18), Qt::AlignRight | Qt::AlignVCenter, QString::number(v));
    }

    auto xAt = [&](int i) { return plot.left() + padX + (plot.width() - 2 * padX) * i / (n - 1); };
    auto yAt = [&](double v) { return plot.bottom() - plot.height() * v / yMax; };

    for (int i = 0; i < n; ++i) {
        p.setPen(QColor("#6b6b6b"));
        p.drawText(QRectF(xAt(i) - 25, plot.bottom() + 4, 50, 18), Qt::AlignCenter, m_labels[i]);
    }

    struct S { const QVector<double> *v; QColor c; bool fill; };
    const S series[] = { { &m_ok, QColor("#2e9e5b"), true },
                         { &m_soon, QColor("#f0a020"), true },
                         { &m_down, QColor("#e5484d"), false } };
    for (const S &s : series) {
        if (s.v->size() != n) continue;
        QPolygonF poly;
        for (int i = 0; i < n; ++i) poly << QPointF(xAt(i), yAt(s.v->at(i)));
        if (s.fill) {
            QPolygonF area = poly;
            area << QPointF(xAt(n - 1), plot.bottom()) << QPointF(xAt(0), plot.bottom());
            QColor fc = s.c; fc.setAlpha(38);
            p.setPen(Qt::NoPen); p.setBrush(fc);
            p.drawPolygon(area);
        }
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(s.c, 1.6));
        p.drawPolyline(poly);
        p.setPen(Qt::NoPen); p.setBrush(s.c);
        for (const QPointF &pt : poly) p.drawEllipse(pt, 3.2, 3.2);
    }

    // Légende
    const QString names[] = { QStringLiteral("Opérationnelles"), QStringLiteral("Maintenance bientôt"),
                              QStringLiteral("Hors service") };
    const QColor cols[] = { QColor("#2e9e5b"), QColor("#f0a020"), QColor("#e5484d") };
    p.setFont(px(12));
    QFontMetrics fm(p.font());
    qreal tw = 0;
    for (const QString &s : names) tw += fm.horizontalAdvance(s) + 26;
    qreal x = (width() - tw) / 2;
    const qreal y = height() - 16;
    for (int i = 0; i < 3; ++i) {
        p.setPen(Qt::NoPen); p.setBrush(cols[i]);
        p.drawEllipse(QPointF(x + 5, y), 5, 5);
        p.setPen(cols[i].darker(130));
        p.drawText(QPointF(x + 16, y + 4), names[i]);
        x += fm.horizontalAdvance(names[i]) + 26;
    }
}

/* -------------------------------- TopBars -------------------------------- */
TopBars::TopBars(QWidget *parent) : QWidget(parent) {}

void TopBars::setData(const QVector<Entry> &entries)
{
    m_entries = entries;
    update();
}

void TopBars::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QColor fills[] = { QColor("#0a2f5b"), QColor("#8fb8e6"), QColor("#b7d0ec"),
                             QColor("#a7bfdc"), QColor("#e8c9a0") };
    const int n = qMax(5, m_entries.size());
    const qreal rowH = height() / qreal(n);
    const qreal nameW = qMin<qreal>(150, width() * 0.42);
    const qreal valW = 46;
    const QRectF track(nameW + 6, 0, width() - nameW - valW - 10, 12);

    for (int i = 0; i < m_entries.size(); ++i) {
        const Entry &e = m_entries[i];
        const qreal cy = rowH * i + rowH / 2;

        p.setFont(px(13));
        p.setPen(QColor("#23252e"));
        QFontMetrics fm(p.font());
        p.drawText(QRectF(0, cy - 12, nameW, 24), Qt::AlignVCenter | Qt::AlignLeft,
                   fm.elidedText(e.name, Qt::ElideRight, int(nameW)));

        QRectF t = track; t.moveCenter(QPointF(track.center().x(), cy));
        p.setPen(Qt::NoPen);
        p.setBrush(QColor("#eee8dc"));
        p.drawRoundedRect(t, 6, 6);
        QRectF f = t; f.setWidth(qMax<qreal>(10, t.width() * qBound(0.0, e.ratio, 1.0)));
        p.setBrush(fills[i % 5]);
        p.drawRoundedRect(f, 6, 6);

        p.setPen(QColor("#23252e"));
        p.drawText(QRectF(width() - valW, cy - 12, valW, 24), Qt::AlignVCenter | Qt::AlignRight, e.value);
    }
}
