#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStyle>
#include <QTimer>

#include "machinespage.h"
#include "uihelpers.h"

static const QColor kNavy("#0a2748");

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);          // construit toute l'interface décrite dans mainwindow.ui

    setupSidebar();
    setupTopBar();

    // notification flottante (superposée au contenu, donc créée en code)
    m_toast = new QLabel(ui->central);
    m_toast->setObjectName(QStringLiteral("toast"));
    m_toast->setWordWrap(true);
    m_toast->setMargin(12);
    m_toast->hide();

    // liaison avec la page Machines
    connect(ui->pageMachines, &MachinesPage::notify, this, &MainWindow::showToast);
    connect(ui->pageMachines, &MachinesPage::alertCountChanged, this, &MainWindow::updateBadge);
    connect(ui->pageMachines, &MachinesPage::searchTextChanged, this, [this](const QString &t) {
        if (ui->topSearch->text() != t) {
            QSignalBlocker b(ui->topSearch);
            ui->topSearch->setText(t);
        }
    });
    connect(ui->topSearch, &QLineEdit::textChanged, ui->pageMachines, &MachinesPage::setSearchText);
    updateBadge(ui->pageMachines->alertCount());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupSidebar()
{
    // le logo FashioNova fait partie de l'image de fond (sidebarwidget.cpp)

    struct Item { QPushButton *button; const char *icon; };
    const Item items[] = {
        { ui->btnNavClients,   "users" }, { ui->btnNavEmployes, "users" },
        { ui->btnNavCommandes, "clip"  }, { ui->btnNavMaquettes, "man" },
        { ui->btnNavMachines,  "gear"  }, { ui->btnNavArticles, "tag"  } };

    for (int i = 0; i < 6; ++i) {
        QPushButton *b = items[i].button;
        const QString icon = QString::fromLatin1(items[i].icon);
        b->setIcon(UI::svgIcon(icon, b->isChecked() ? kNavy : QColor(Qt::white), 26));
        // l'icône change de couleur quand le bouton est sélectionné
        connect(b, &QPushButton::toggled, this, [b, icon](bool on) {
            b->setIcon(UI::svgIcon(icon, on ? kNavy : QColor(Qt::white), 26));
        });
        connect(b, &QPushButton::clicked, this, [this, i] { ui->stackedPages->setCurrentIndex(i); });
    }
}

void MainWindow::setupTopBar()
{
    ui->btnMenu->setIcon(UI::svgIcon(QStringLiteral("menu"), QColor("#23252e"), 24));
    connect(ui->btnMenu, &QToolButton::clicked, this, [this] {
        ui->sidebar->setVisible(!ui->sidebar->isVisible());
    });

    ui->topSearch->addAction(UI::svgIcon(QStringLiteral("search"), QColor("#6b7a90"), 20),
                             QLineEdit::LeadingPosition);

    ui->btnBell->setIcon(UI::svgIcon(QStringLiteral("bell"), QColor("#23252e"), 26));
    m_badge = new QLabel(QStringLiteral("0"), ui->btnBell);
    m_badge->setObjectName(QStringLiteral("badge"));
    m_badge->setAlignment(Qt::AlignCenter);
    m_badge->setFixedSize(17, 17);
    m_badge->move(21, 1);
    m_badge->setAttribute(Qt::WA_TransparentForMouseEvents);
    connect(ui->btnBell, &QToolButton::clicked, this, &MainWindow::onBellClicked);

    QPixmap av(72, 72);
    av.fill(Qt::transparent);
    {
        QPainter p(&av);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(kNavy);
        p.setPen(Qt::NoPen);
        p.drawEllipse(0, 0, 72, 72);
        p.drawPixmap(18, 18, UI::svgPixmap(QStringLiteral("user"), Qt::white, 18));
    }
    av.setDevicePixelRatio(2.0);
    ui->labelAvatar->setPixmap(av);

    ui->labelChevron->setPixmap(UI::svgPixmap(QStringLiteral("chevron"), QColor("#23252e"), 16));
}

void MainWindow::onBellClicked()
{
    ui->pageMachines->showAlertsMenu(ui->btnBell);
}

void MainWindow::updateBadge(int count)
{
    m_badge->setText(QString::number(count));
    m_badge->setVisible(count > 0);
}

void MainWindow::showToast(const QString &message, bool error)
{
    m_toast->setText(message);
    m_toast->setProperty("error", error);
    m_toast->style()->unpolish(m_toast);
    m_toast->style()->polish(m_toast);
    m_toast->update();
    const int w = 360;
    m_toast->setFixedWidth(w);
    m_toast->resize(w, qMax(48, m_toast->heightForWidth(w)));
    m_toast->move(ui->central->width() - w - 22, 72);
    m_toast->show();
    m_toast->raise();
    QTimer::singleShot(5000, m_toast, &QLabel::hide);
}
