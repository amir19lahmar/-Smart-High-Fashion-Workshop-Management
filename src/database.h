#pragma once
#include <QList>
#include <QString>
#include "machine.h"

struct Snapshot {            // historique mensuel de l'état du parc
    QString mois;            // "yyyy-MM"
    int operationnelles = 0;
    int bientot = 0;
    int horsService = 0;
};

class Database
{
public:
    static bool open(QString *error = nullptr);

    static QList<Machine> all();
    static bool insert(const Machine &m, QString *error = nullptr);
    static bool update(const Machine &m, QString *error = nullptr);
    static bool remove(const QString &id, QString *error = nullptr);
    static QString nextId();

    static void saveSnapshot(const Snapshot &s);
    static QList<Snapshot> snapshots(int count);
};
