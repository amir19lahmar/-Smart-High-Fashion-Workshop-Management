#include "detailsdialog.h"
#include "ui_detailsdialog.h"

#include <QStyle>

#include "uihelpers.h"

DetailsDialog::DetailsDialog(const Machine &m, QWidget *parent)
    : QDialog(parent), ui(new Ui::DetailsDialog)
{
    ui->setupUi(this);

    ui->labelTitle->setText(QStringLiteral("%1 – %2").arg(m.id, m.nom));
    ui->valueType->setText(m.type);
    ui->valueEtat->setText(m.etat);
    ui->valueDate->setText(QStringLiteral("%1 (%2 j)")
                               .arg(m.prochaineMaintenance.toString(QStringLiteral("dd/MM/yyyy")))
                               .arg(m.joursRestants()));
    ui->labelPhoto->setPixmap(UI::cover(UI::machinePixmap(m.photo), QSize(90, 120), 8));

    const QString reason = m.raisonMaintenance();
    if (reason.isEmpty()) {
        ui->labelStatus->setText(QStringLiteral("✔ Maintenance à jour."));
        ui->labelStatus->setProperty("level", QStringLiteral("ok"));
    } else {
        ui->labelStatus->setText(QStringLiteral("⚠ %1").arg(reason));
        ui->labelStatus->setProperty("level", QStringLiteral("crit"));
    }
    ui->labelStatus->style()->unpolish(ui->labelStatus);
    ui->labelStatus->style()->polish(ui->labelStatus);
}

DetailsDialog::~DetailsDialog()
{
    delete ui;
}
