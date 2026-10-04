#pragma once
#include <QDate>
#include <QString>
#include <QStringList>

// États possibles d'une machine (attribut « Etat » du cahier des charges)
namespace Etat {
inline QString disponible()   { return QStringLiteral("Disponible"); }
inline QString maintenance()  { return QStringLiteral("En maintenance"); }
inline QString indisponible() { return QStringLiteral("Indisponible"); }
inline QStringList all()      { return QStringList() << disponible() << maintenance() << indisponible(); }
}

inline QStringList typesMachine()
{
    return QStringList() << QStringLiteral("Couture") << QStringLiteral("Coupe")
                         << QStringLiteral("Finition") << QStringLiteral("Broderie")
                         << QStringLiteral("Repassage") << QStringLiteral("Autre");
}

// Entité « Machine » : ID_Machine, Nom_Machine, Type_Machine, Etat, Date_Prochaine_Maintenance
struct Machine
{
    QString id;
    QString nom;
    QString type;
    QString etat = Etat::disponible();
    QDate   prochaineMaintenance = QDate::currentDate().addDays(60);
    QString photo;                       // chemin d'une photo (optionnel)

    int  joursRestants() const { return QDate::currentDate().daysTo(prochaineMaintenance); }
    bool indisponible()  const { return etat == Etat::indisponible(); }

    // Détection automatique : texte vide = aucune maintenance nécessaire
    QString raisonMaintenance() const
    {
        const int j = joursRestants();
        if (indisponible())
            return QStringLiteral("Machine indisponible – maintenance requise");
        if (j < 0)
            return QStringLiteral("Maintenance en retard de %1 j").arg(-j);
        if (etat == Etat::maintenance())
            return QStringLiteral("Maintenance en cours");
        if (j <= 7)
            return QStringLiteral("Maintenance à prévoir sous %1 j").arg(j);
        return QString();
    }
    bool maintenanceRequise() const { return !raisonMaintenance().isEmpty(); }
    bool critique() const { return indisponible() || joursRestants() < 0; }
};
