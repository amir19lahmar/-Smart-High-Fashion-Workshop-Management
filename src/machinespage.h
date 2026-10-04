#pragma once
#include <QList>
#include <QSet>
#include <QWidget>
#include "machine.h"

class QHBoxLayout;

namespace Ui { class MachinesPage; }

// Page « Gestion des Machines » : formulaire, liste, statistiques.
// L'interface est définie dans machinespage.ui (Qt Designer) ; ce fichier ne contient que la logique.
class MachinesPage : public QWidget
{
    Q_OBJECT
public:
    explicit MachinesPage(QWidget *parent = nullptr);
    ~MachinesPage() override;

    int  alertCount() const { return m_alertCount; }
    void setSearchText(const QString &text);        // synchronise avec la recherche de la barre du haut
    void showAlertsMenu(QWidget *anchor);           // menu de la cloche

signals:
    void searchTextChanged(const QString &text);
    void alertCountChanged(int count);
    void notify(const QString &message, bool error);

private slots:
    void onAdd();
    void onModify();
    void onDelete();
    void onView();
    void onReset();
    void onExportPdf();
    void onChoosePhoto();
    void onRowSelected();
    void onSearchChanged(const QString &text);
    void updateDetection();

private:
    // initialisation (les widgets eux-mêmes viennent du .ui)
    void setupIcons();
    void setupTable();
    void connectSignals();

    // données
    void reloadData();
    void applyFilter();
    void renderPage();
    void buildPager(int pages);
    void updateCharts();
    void checkAlerts();
    void goToMachine(const QString &id);
    const Machine *findMachine(const QString &id) const;

    // formulaire
    bool readForm(Machine &m, QString *error) const;
    void loadForm(const Machine &m);
    void clearForm();
    void updatePreview();

    Ui::MachinesPage *ui;
    QHBoxLayout *m_pager = nullptr;

    QList<Machine> m_all;
    QList<Machine> m_view;
    QString m_currentId;
    QString m_formPhoto;
    int  m_page = 0;
    int  m_alertCount = 0;
    bool m_onlyAlerts = false;
    bool m_alertsReady = false;
    QSet<QString> m_knownUnavailable;
};
