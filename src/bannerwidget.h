#pragma once
#include <QPixmap>
#include <QWidget>

// Bandeau « Gestion des Machines » (tissu + machine à coudre + citation)
class BannerWidget : public QWidget
{
public:
    explicit BannerWidget(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;
private:
    void rebuild();
    QPixmap m_fabric;
    QPixmap m_machine;
};
