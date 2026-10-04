#pragma once
#include <QFrame>
#include <QPixmap>

// Menu latéral avec image de fond (logo en haut, photo en bas).
// L'image garde sa largeur : le haut (logo) et le bas (photo) restent à taille fixe,
// seule la zone du milieu (bleu uni) s'étire selon la hauteur de la fenêtre.
// Utilisé comme widget promu dans mainwindow.ui.
class SidebarWidget : public QFrame
{
    Q_OBJECT
public:
    explicit SidebarWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap m_bg;
};
