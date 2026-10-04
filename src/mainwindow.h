#pragma once
#include <QMainWindow>

class QLabel;

namespace Ui { class MainWindow; }

// Fenêtre principale : menu latéral, barre du haut, pile de pages.
// L'interface est définie dans mainwindow.ui (Qt Designer) ;
// la page « Machines » est un widget promu : voir machinespage.ui / machinespage.cpp.
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onBellClicked();
    void updateBadge(int count);
    void showToast(const QString &message, bool error);

private:
    void setupSidebar();
    void setupTopBar();

    Ui::MainWindow *ui;
    QLabel *m_badge = nullptr;   // pastille de la cloche (superposée au bouton)
    QLabel *m_toast = nullptr;   // notification flottante
};
