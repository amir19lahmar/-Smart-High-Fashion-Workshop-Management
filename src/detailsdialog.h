#pragma once
#include <QDialog>
#include "machine.h"

namespace Ui { class DetailsDialog; }

// Boîte « Afficher » : détails d'une machine + état de la maintenance (detailsdialog.ui)
class DetailsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DetailsDialog(const Machine &machine, QWidget *parent = nullptr);
    ~DetailsDialog() override;

private:
    Ui::DetailsDialog *ui;
};
