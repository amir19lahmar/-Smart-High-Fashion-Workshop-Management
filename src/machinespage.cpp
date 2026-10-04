#include "machinespage.h"
#include "ui_machinespage.h"

#include <algorithm>
#include <utility>

#include <QAction>
#include <QComboBox>
#include <QDateEdit>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QMarginsF>
#include <QMenu>
#include <QMessageBox>
#include <QPageSize>
#include <QPdfWriter>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStyle>
#include <QTableWidget>
#include <QTextDocument>
#include <QTimer>
#include <QToolButton>
#include <QUrl>

#include "chartwidgets.h"
#include "database.h"
#include "detailsdialog.h"
#include "uihelpers.h"

static const int kPageSize = 5;

// Version « maquette » : mettre true pour réactiver les boutons du CRUD
// (Ajouter, Afficher, Modifier, Supprimer). A false, ils restent visibles mais ne font rien.
static const bool kCrudActif = false;

static void repolish(QWidget *w)
{
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}

static void clearLayout(QLayout *l)
{
    while (QLayoutItem *item = l->takeAt(0)) {
        if (QWidget *w = item->widget()) w->deleteLater();
        delete item;
    }
}

/* ============================ construction ============================ */

MachinesPage::MachinesPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::MachinesPage)
{
    ui->setupUi(this);          // construit toute l'interface décrite dans machinespage.ui
    m_pager = ui->pagerLayout;

    setupIcons();
    setupTable();
    connectSignals();

    reloadData();
    clearForm();

    // Re-vérification périodique (une machine peut devenir « en retard » sans action de l'utilisateur)
    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MachinesPage::reloadData);
    timer->start(60 * 1000);
}

MachinesPage::~MachinesPage()
{
    delete ui;
}

// Les icônes sont des SVG recolorés à l'exécution : elles sont donc affectées ici et non dans le .ui.
void MachinesPage::setupIcons()
{
    const QColor ink("#2c3240");
    // cartes
    ui->iconCardForm->setPixmap(UI::svgPixmap(QStringLiteral("gear"),  Qt::white, 18));
    ui->iconCardList->setPixmap(UI::svgPixmap(QStringLiteral("clip"),  Qt::white, 18));
    ui->iconCardStats->setPixmap(UI::svgPixmap(QStringLiteral("chart"), Qt::white, 18));

    // champs du formulaire
    ui->iconId->setPixmap(UI::svgPixmap(QStringLiteral("idcard"),   ink, 18));
    ui->iconNom->setPixmap(UI::svgPixmap(QStringLiteral("gear"),    ink, 18));
    ui->iconType->setPixmap(UI::svgPixmap(QStringLiteral("box"),    ink, 18));
    ui->iconDate->setPixmap(UI::svgPixmap(QStringLiteral("calendar"), ink, 18));
    for (int i = 0; i < ui->comboEtat->count(); ++i)
        ui->comboEtat->setItemIcon(i, UI::dotIcon(UI::etatColor(ui->comboEtat->itemText(i))));

    // photo
    ui->btnPen->setIcon(UI::svgIcon(QStringLiteral("pen"), Qt::white, 16));
    ui->btnPlus->setIcon(UI::svgIcon(QStringLiteral("plus"), QColor("#c9a36a"), 22));
    ui->labelThumb1->setPixmap(UI::cover(QPixmap(QStringLiteral(":/images/thumb1.jpg")), QSize(44, 56), 5));
    ui->labelThumb2->setPixmap(UI::cover(QPixmap(QStringLiteral(":/images/thumb2.jpg")), QSize(44, 56), 5));

    // boutons
    ui->btnAdd->setIcon(UI::svgIcon(QStringLiteral("plus"),  Qt::white, 20));
    ui->btnMod->setIcon(UI::svgIcon(QStringLiteral("pen"),   Qt::white, 20));
    ui->btnDel->setIcon(UI::svgIcon(QStringLiteral("trash"), QColor("#4a3a26"), 20));
    ui->btnView->setIcon(UI::svgIcon(QStringLiteral("eye"),  QColor("#1b1d25"), 20));
    ui->btnReset->setIcon(UI::svgIcon(QStringLiteral("refresh"), QColor("#23252e"), 16));
    ui->btnExport->setIcon(UI::svgIcon(QStringLiteral("doc"), Qt::white, 16));
    ui->btnChip->setIcon(UI::svgIcon(QStringLiteral("warn"), QColor("#b9781a"), 16));

    ui->lineSearch->addAction(UI::svgIcon(QStringLiteral("search"), QColor("#6b7a90"), 18),
                              QLineEdit::LeadingPosition);
}

void MachinesPage::setupTable()
{
    QTableWidget *t = ui->tableMachines;
    t->verticalHeader()->setDefaultSectionSize(50);
    t->setFixedHeight(40 + 50 * kPageSize + 4);
    QHeaderView *hh = t->horizontalHeader();
    hh->setHighlightSections(false);
    hh->setFixedHeight(40);
    hh->setSectionsClickable(true);
    hh->setSectionResizeMode(QHeaderView::Fixed);
    hh->setSectionResizeMode(2, QHeaderView::Stretch);
    hh->setStretchLastSection(false);
    const int widths[] = { 34, 70, 0, 78, 122, 118, 84 };
    for (int c = 0; c < 7; ++c) if (widths[c]) t->setColumnWidth(c, widths[c]);
    hh->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // case « tout cocher » de l'en-tête
    connect(hh, &QHeaderView::sectionClicked, this, [this](int c) {
        if (c != 0) return;
        QTableWidget *tw = ui->tableMachines;
        bool all = true;
        for (int r = 0; r < tw->rowCount(); ++r)
            if (tw->item(r, 0)->checkState() != Qt::Checked) all = false;
        for (int r = 0; r < tw->rowCount(); ++r)
            tw->item(r, 0)->setCheckState(all ? Qt::Unchecked : Qt::Checked);
    });
}

void MachinesPage::connectSignals()
{
    if (kCrudActif) {
        connect(ui->btnAdd,  &QPushButton::clicked, this, &MachinesPage::onAdd);      // Create
        connect(ui->btnView, &QPushButton::clicked, this, &MachinesPage::onView);     // Read
        connect(ui->btnMod,  &QPushButton::clicked, this, &MachinesPage::onModify);   // Update
        connect(ui->btnDel,  &QPushButton::clicked, this, &MachinesPage::onDelete);   // Delete
    }
    connect(ui->btnReset,  &QPushButton::clicked, this, &MachinesPage::onReset);
    connect(ui->btnExport, &QPushButton::clicked, this, &MachinesPage::onExportPdf);
    connect(ui->btnPen,    &QToolButton::clicked, this, &MachinesPage::onChoosePhoto);
    connect(ui->btnPlus,   &QPushButton::clicked, this, &MachinesPage::onChoosePhoto);

    connect(ui->comboEtat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] { updateDetection(); });
    connect(ui->dateMaintenance, &QDateEdit::dateChanged, this, [this] { updateDetection(); });

    connect(ui->tableMachines, &QTableWidget::itemSelectionChanged, this, &MachinesPage::onRowSelected);
    connect(ui->lineSearch, &QLineEdit::textChanged, this, &MachinesPage::onSearchChanged);
    connect(ui->comboSort, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this] { m_page = 0; applyFilter(); });
    connect(ui->btnChip, &QPushButton::clicked, this, [this] { m_onlyAlerts = !m_onlyAlerts; m_page = 0; applyFilter(); });
}

/* ============================== données =============================== */

const Machine *MachinesPage::findMachine(const QString &id) const
{
    for (const Machine &m : m_all)
        if (m.id == id) return &m;
    return nullptr;
}

void MachinesPage::reloadData()
{
    m_all = Database::all();
    checkAlerts();
    updateCharts();
    applyFilter();
}

void MachinesPage::onSearchChanged(const QString &text)
{
    // les deux champs de recherche restent synchronisés
    m_page = 0;
    applyFilter();
    emit searchTextChanged(text);      // la barre de recherche du haut se met à jour
}

void MachinesPage::setSearchText(const QString &text)
{
    if (ui->lineSearch->text() != text)
        ui->lineSearch->setText(text);  // déclenche onSearchChanged()
}

void MachinesPage::applyFilter()
{
    const QString q = ui->lineSearch->text().trimmed();
    m_view.clear();
    for (const Machine &m : std::as_const(m_all)) {
        // recherche par ID_Machine, Nom_Machine, Type_Machine
        const bool match = q.isEmpty()
            || m.id.contains(q, Qt::CaseInsensitive)
            || m.nom.contains(q, Qt::CaseInsensitive)
            || m.type.contains(q, Qt::CaseInsensitive);
        if (match && (!m_onlyAlerts || m.maintenanceRequise()))
            m_view.append(m);
    }

    // tri par Nom_Machine, Type_Machine ou Etat
    const int key = ui->comboSort->currentIndex();
    if (key > 0) {
        std::stable_sort(m_view.begin(), m_view.end(), [key](const Machine &a, const Machine &b) {
            const QString x = key == 1 ? a.nom : key == 2 ? a.type : a.etat;
            const QString y = key == 1 ? b.nom : key == 2 ? b.type : b.etat;
            return QString::localeAwareCompare(x, y) < 0;
        });
    }

    int alerts = 0;
    for (const Machine &m : std::as_const(m_all)) if (m.maintenanceRequise()) ++alerts;
    ui->btnChip->setVisible(alerts > 0);
    ui->btnChip->setText(m_onlyAlerts
        ? QStringLiteral("  %1 machine(s) à maintenir – afficher tout").arg(alerts)
        : QStringLiteral("  %1 machine(s) nécessitent une maintenance").arg(alerts));

    renderPage();
}

void MachinesPage::renderPage()
{
    const int total = m_view.size();
    const int pages = qMax(1, (total + kPageSize - 1) / kPageSize);
    m_page = qBound(0, m_page, pages - 1);
    const int from = m_page * kPageSize;
    const int to = qMin(total, from + kPageSize);

    {
        QSignalBlocker blocker(ui->tableMachines);
        ui->tableMachines->clearContents();
        ui->tableMachines->setRowCount(to - from);

        for (int r = 0; r < to - from; ++r) {
            const Machine &m = m_view.at(from + r);

            auto *chk = new QTableWidgetItem;
            chk->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            chk->setCheckState(Qt::Unchecked);
            chk->setData(Qt::UserRole, m.id);
            ui->tableMachines->setItem(r, 0, chk);

            auto mk = [&](const QString &txt) {
                auto *it = new QTableWidgetItem(txt);
                it->setData(Qt::UserRole, m.id);
                return it;
            };
            ui->tableMachines->setItem(r, 1, mk(m.id));
            ui->tableMachines->setItem(r, 2, mk(m.nom));
            ui->tableMachines->setItem(r, 3, mk(m.type));

            // pastille d'état
            QString bg = QStringLiteral("#dcefd9"), fg = QStringLiteral("#3f8a45");
            if (m.etat == Etat::maintenance())  { bg = QStringLiteral("#f7e6cc"); fg = QStringLiteral("#b9781a"); }
            if (m.etat == Etat::indisponible()) { bg = QStringLiteral("#f8d6d2"); fg = QStringLiteral("#c8402f"); }
            auto *pill = new QLabel(m.etat);
            pill->setStyleSheet(QStringLiteral("background:%1;color:%2;border-radius:5px;padding:4px 9px;font-size:12px;")
                                    .arg(bg, fg));
            auto *pw = new QWidget;
            auto *pl = new QHBoxLayout(pw);
            pl->setContentsMargins(4, 0, 4, 0);
            pl->addWidget(pill);
            pl->addStretch();
            ui->tableMachines->setCellWidget(r, 4, pw);

            // date + détection automatique
            const QString reason = m.raisonMaintenance();
            auto *dt = mk((reason.isEmpty() ? QString() : QStringLiteral("⚠ "))
                          + m.prochaineMaintenance.toString(QStringLiteral("dd/MM/yyyy")));
            if (!reason.isEmpty()) {
                dt->setForeground(m.critique() ? QColor("#c8402f") : QColor("#b9781a"));
                dt->setToolTip(reason);
            }
            ui->tableMachines->setItem(r, 5, dt);

            auto *ap = new QLabel;
            ap->setPixmap(UI::cover(UI::machinePixmap(m.photo), QSize(64, 38), 4));
            auto *aw = new QWidget;
            auto *al = new QHBoxLayout(aw);
            al->setContentsMargins(4, 0, 4, 0);
            al->addWidget(ap);
            ui->tableMachines->setCellWidget(r, 6, aw);
        }

        for (int r = 0; r < to - from; ++r)
            if (ui->tableMachines->item(r, 1)->data(Qt::UserRole).toString() == m_currentId)
                ui->tableMachines->selectRow(r);
    }
    buildPager(pages);
}

void MachinesPage::buildPager(int pages)
{
    clearLayout(m_pager);
    m_pager->addStretch();
    auto add = [&](const QString &text, int target, bool current, bool enabled) {
        auto *b = new QPushButton(text);
        b->setObjectName(current ? QStringLiteral("pageCur") : QStringLiteral("page"));
        b->setFixedSize(36, 36);
        b->setEnabled(enabled);
        b->setCursor(Qt::PointingHandCursor);
        connect(b, &QPushButton::clicked, this, [this, target] { m_page = target; renderPage(); });
        m_pager->addWidget(b);
    };
    add(QStringLiteral("‹"), qMax(0, m_page - 1), false, m_page > 0);
    const int start = qMax(0, qMin(m_page - 2, pages - 5));
    const int end = qMin(pages, start + 5);
    if (start > 0) { add(QStringLiteral("1"), 0, false, true); if (start > 1) add(QStringLiteral("…"), start - 1, false, true); }
    for (int i = start; i < end; ++i) add(QString::number(i + 1), i, i == m_page, true);
    if (end < pages) { if (end < pages - 1) add(QStringLiteral("…"), end, false, true); add(QString::number(pages), pages - 1, false, true); }
    add(QStringLiteral("›"), qMin(pages - 1, m_page + 1), false, m_page < pages - 1);
}

void MachinesPage::goToMachine(const QString &id)
{
    for (int i = 0; i < m_view.size(); ++i)
        if (m_view[i].id == id) { m_page = i / kPageSize; break; }
    renderPage();
}

/* ============================ statistiques ============================ */

void MachinesPage::updateCharts()
{
    // Répartition par type
    QMap<QString, int> count;
    for (const Machine &m : std::as_const(m_all)) count[m.type]++;
    QVector<DonutChart::Slice> slices;
    for (const QString &t : typesMachine())
        if (count.value(t) > 0)
            slices.append(DonutChart::Slice{ t, count.value(t), UI::typeColor(t) });
    ui->donutChart->setData(slices, QStringLiteral("machines"));

    // Historique : le mois courant reflète l'état réel du parc
    Snapshot now;
    now.mois = QDate::currentDate().toString(QStringLiteral("yyyy-MM"));
    for (const Machine &m : std::as_const(m_all)) {
        if (m.indisponible()) ++now.horsService;
        else if (m.maintenanceRequise()) ++now.bientot;
        else ++now.operationnelles;
    }
    Database::saveSnapshot(now);

    static const char *mois[] = { "Jan", "Fév", "Mar", "Avr", "Mai", "Juin", "Juil", "Aoû", "Sep", "Oct", "Nov", "Déc" };
    QStringList labels;
    QVector<double> ok, soon, down;
    for (const Snapshot &s : Database::snapshots(6)) {
        labels << QString::fromUtf8(mois[qBound(1, s.mois.mid(5, 2).toInt(), 12) - 1]);
        ok << s.operationnelles; soon << s.bientot; down << s.horsService;
    }
    ui->lineChart->setData(labels, ok, soon, down);

    // Top 5 : machines dont la maintenance est la plus proche
    QList<Machine> sorted = m_all;
    std::stable_sort(sorted.begin(), sorted.end(), [](const Machine &a, const Machine &b) {
        return a.prochaineMaintenance < b.prochaineMaintenance;
    });
    QVector<TopBars::Entry> entries;
    for (int i = 0; i < qMin(5, sorted.size()); ++i) {
        const int j = sorted[i].joursRestants();
        const QString value = j < 0 ? QStringLiteral("retard") : QStringLiteral("%1 j").arg(j);
        entries.append(TopBars::Entry{ sorted[i].nom, value, (60.0 - qBound(0, j, 60)) / 60.0 });
    }
    ui->topBars->setData(entries);
}

/* ================================ alertes ================================ */

void MachinesPage::checkAlerts()
{
    QSet<QString> unavailable;
    int needing = 0;
    for (const Machine &m : std::as_const(m_all)) {
        if (m.indisponible()) unavailable.insert(m.id);
        if (m.maintenanceRequise()) ++needing;
    }
    // alerte : une machine vient de devenir indisponible
    if (m_alertsReady) {
        for (const QString &id : std::as_const(unavailable)) {
            if (!m_knownUnavailable.contains(id)) {
                const Machine *m = findMachine(id);
                if (m) emit notify(QStringLiteral("Alerte : la machine %1 – %2 est indisponible.").arg(m->id, m->nom), true);
            }
        }
    }
    m_knownUnavailable = unavailable;
    m_alertsReady = true;

    if (needing != m_alertCount) {
        m_alertCount = needing;
        emit alertCountChanged(m_alertCount);
    }
}

void MachinesPage::showAlertsMenu(QWidget *anchor)
{
    QMenu menu(this);
    bool any = false;
    for (const Machine &m : std::as_const(m_all)) {
        const QString reason = m.raisonMaintenance();
        if (reason.isEmpty()) continue;
        any = true;
        QAction *a = menu.addAction(UI::dotIcon(m.critique() ? QColor("#e5484d") : QColor("#e6a23c")),
                                    QStringLiteral("%1 – %2 : %3").arg(m.id, m.nom, reason));
        const QString id = m.id;
        connect(a, &QAction::triggered, this, [this, id] {
            m_currentId = id;
            ui->lineSearch->clear();
            goToMachine(id);
            if (const Machine *mm = findMachine(id)) loadForm(*mm);
        });
    }
    if (!any)
        menu.addAction(QStringLiteral("Aucune alerte – tout le parc est à jour"))->setEnabled(false);
    menu.exec(anchor->mapToGlobal(QPoint(-260, anchor->height() + 4)));
}

/* ============================== formulaire ============================== */

bool MachinesPage::readForm(Machine &m, QString *error) const
{
    m.nom = ui->lineNom->text().trimmed();
    if (m.nom.isEmpty()) {
        *error = QStringLiteral("Le nom de la machine est obligatoire.");
        return false;
    }
    m.type = ui->comboType->currentText();
    m.etat = ui->comboEtat->currentText();
    m.prochaineMaintenance = ui->dateMaintenance->date();
    m.photo = m_formPhoto;
    return true;
}

void MachinesPage::loadForm(const Machine &m)
{
    ui->lineId->setText(m.id);
    ui->lineNom->setText(m.nom);
    int i = ui->comboType->findText(m.type);
    if (i < 0) { ui->comboType->addItem(m.type); i = ui->comboType->findText(m.type); }
    ui->comboType->setCurrentIndex(i);
    ui->comboEtat->setCurrentIndex(qMax(0, ui->comboEtat->findText(m.etat)));
    ui->dateMaintenance->setDate(m.prochaineMaintenance);
    m_formPhoto = m.photo;
    updatePreview();
    updateDetection();
}

void MachinesPage::clearForm()
{
    m_currentId.clear();
    m_formPhoto.clear();
    ui->lineId->setText(Database::nextId());
    ui->lineNom->clear();
    ui->comboType->setCurrentIndex(0);
    ui->comboEtat->setCurrentIndex(0);
    ui->dateMaintenance->setDate(QDate::currentDate().addDays(60));
    updatePreview();
    updateDetection();
}

void MachinesPage::updatePreview()
{
    ui->labelPhoto->setPixmap(UI::cover(UI::machinePixmap(m_formPhoto), QSize(142, 214), 8));
}

void MachinesPage::updateDetection()
{
    if (!ui->labelDetection) return;
    Machine tmp;
    tmp.etat = ui->comboEtat->currentText();
    tmp.prochaineMaintenance = ui->dateMaintenance->date();
    const QString reason = tmp.raisonMaintenance();
    if (reason.isEmpty()) {
        ui->labelDetection->setText(QStringLiteral("✔  Détection automatique : maintenance à jour"));
        ui->labelDetection->setProperty("level", QStringLiteral("ok"));
    } else {
        ui->labelDetection->setText(QStringLiteral("⚠  Détection automatique : %1").arg(reason));
        ui->labelDetection->setProperty("level", tmp.critique() ? QStringLiteral("crit") : QStringLiteral("warn"));
    }
    repolish(ui->labelDetection);
}

void MachinesPage::onRowSelected()
{
    const QList<QTableWidgetItem *> items = ui->tableMachines->selectedItems();
    if (items.isEmpty()) return;
    const QString id = items.first()->data(Qt::UserRole).toString();
    if (const Machine *m = findMachine(id)) {
        m_currentId = id;
        loadForm(*m);
    }
}

/* =============================== actions =============================== */

void MachinesPage::onAdd()
{
    Machine m;
    QString err;
    if (!readForm(m, &err)) { QMessageBox::warning(this, QStringLiteral("Ajouter"), err); return; }
    m.id = Database::nextId();
    if (!Database::insert(m, &err)) { QMessageBox::critical(this, QStringLiteral("Ajouter"), err); return; }
    m_currentId = m.id;
    reloadData();
    goToMachine(m.id);
    loadForm(m);
    emit notify(QStringLiteral("Machine %1 ajoutée avec succès.").arg(m.id), false);
}

void MachinesPage::onModify()
{
    if (m_currentId.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Modifier"), QStringLiteral("Sélectionnez d'abord une machine dans la liste."));
        return;
    }
    Machine m;
    QString err;
    if (!readForm(m, &err)) { QMessageBox::warning(this, QStringLiteral("Modifier"), err); return; }
    m.id = m_currentId;
    if (!Database::update(m, &err)) { QMessageBox::critical(this, QStringLiteral("Modifier"), err); return; }
    reloadData();
    goToMachine(m.id);
    emit notify(QStringLiteral("Machine %1 modifiée.").arg(m.id), false);
}

void MachinesPage::onDelete()
{
    if (m_currentId.isEmpty()) {
        QMessageBox::information(this, QStringLiteral("Supprimer"), QStringLiteral("Sélectionnez d'abord une machine dans la liste."));
        return;
    }
    const auto r = QMessageBox::question(this, QStringLiteral("Supprimer"),
                                         QStringLiteral("Supprimer définitivement la machine %1 ?").arg(m_currentId));
    if (r != QMessageBox::Yes) return;
    QString err;
    if (!Database::remove(m_currentId, &err)) { QMessageBox::critical(this, QStringLiteral("Supprimer"), err); return; }
    const QString id = m_currentId;
    clearForm();
    reloadData();
    emit notify(QStringLiteral("Machine %1 supprimée.").arg(id), false);
}

void MachinesPage::onView()
{
    const Machine *m = m_currentId.isEmpty() ? nullptr : findMachine(m_currentId);
    if (!m) {
        QMessageBox::information(this, QStringLiteral("Afficher"), QStringLiteral("Sélectionnez d'abord une machine dans la liste."));
        return;
    }
    DetailsDialog dlg(*m, this);
    dlg.exec();
}

void MachinesPage::onReset()
{
    clearForm();
    ui->tableMachines->clearSelection();
}

void MachinesPage::onChoosePhoto()
{
    const QString f = QFileDialog::getOpenFileName(this, QStringLiteral("Choisir une photo"), QString(),
                                                   QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp)"));
    if (f.isEmpty()) return;
    m_formPhoto = f;
    updatePreview();
    if (!m_currentId.isEmpty())
        emit notify(QStringLiteral("Photo sélectionnée : cliquez sur « Modifier » pour l'enregistrer."), false);
}

void MachinesPage::onExportPdf()
{
    const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Exporter la liste des machines"),
                                                      QStringLiteral("liste_machines.pdf"),
                                                      QStringLiteral("PDF (*.pdf)"));
    if (path.isEmpty()) return;

    QString rows;
    for (const Machine &m : std::as_const(m_view)) {
        const QString alert = m.raisonMaintenance();
        rows += QStringLiteral("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td></tr>")
                    .arg(m.id, m.nom.toHtmlEscaped(), m.type, m.etat,
                         m.prochaineMaintenance.toString(QStringLiteral("dd/MM/yyyy")), alert);
    }
    const QString html = QStringLiteral(
        "<h1 style='color:#0a2748'>FashioNova – Liste des machines</h1>"
        "<p>Édité le %1 – %2 machine(s)</p>"
        "<table border='1' cellspacing='0' cellpadding='5' width='100%'>"
        "<tr style='background:#efe3cf'><th>ID_Machine</th><th>Nom_Machine</th><th>Type_Machine</th>"
        "<th>Etat</th><th>Date_Prochaine_Maintenance</th><th>Alerte</th></tr>%3</table>")
        .arg(QDate::currentDate().toString(QStringLiteral("dd/MM/yyyy")))
        .arg(m_view.size())
        .arg(rows);

    QPdfWriter writer(path);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(15, 15, 15, 15));
    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&writer);

    emit notify(QStringLiteral("PDF exporté : %1").arg(path), false);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}
