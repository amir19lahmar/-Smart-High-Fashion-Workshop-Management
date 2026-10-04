#pragma once
#include <QColor>
#include <QStringList>
#include <QVector>
#include <QWidget>

// Anneau « Répartition par type » avec légende
class DonutChart : public QWidget
{
public:
    struct Slice { QString label; int value; QColor color; };
    explicit DonutChart(QWidget *parent = nullptr);
    void setData(const QVector<Slice> &slices, const QString &unit);
    QSize minimumSizeHint() const override { return QSize(300, 190); }
protected:
    void paintEvent(QPaintEvent *) override;
private:
    QVector<Slice> m_slices;
    QString m_unit;
};

// Courbes « Évolution de l'état des machines »
class LineChart : public QWidget
{
public:
    explicit LineChart(QWidget *parent = nullptr);
    void setData(const QStringList &labels, const QVector<double> &ok,
                 const QVector<double> &soon, const QVector<double> &down);
    QSize minimumSizeHint() const override { return QSize(320, 190); }
protected:
    void paintEvent(QPaintEvent *) override;
private:
    QStringList m_labels;
    QVector<double> m_ok, m_soon, m_down;
};

// Barres « Top 5 des machines (par prochaine maintenance) »
class TopBars : public QWidget
{
public:
    struct Entry { QString name; QString value; double ratio; };
    explicit TopBars(QWidget *parent = nullptr);
    void setData(const QVector<Entry> &entries);
    QSize minimumSizeHint() const override { return QSize(280, 190); }
protected:
    void paintEvent(QPaintEvent *) override;
private:
    QVector<Entry> m_entries;
};
