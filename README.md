# FashioNova – Gestion des Machines (Qt / C++)

Application de bureau Qt Widgets reproduisant l'interface « Gestion des Machines » de FashioNova (menu latéral avec logo, bandeau, formulaire, liste, statistiques) avec une base de données SQLite.

## Prérequis

- Qt **5.15** ou **Qt 6.x** avec les modules **Widgets, Sql (pilote QSQLITE), Svg**
- Un compilateur C++17 (MinGW, MSVC, GCC ou Clang)

## Compilation

**Avec Qt Creator** : ouvrir `GestionMachines.pro` (qmake) ou `CMakeLists.txt` (CMake), choisir un kit, puis *Exécuter*.

**En ligne de commande (qmake)** :

```text
qmake GestionMachines.pro
make
```

**En ligne de commande (CMake)** :

```text
cmake -S . -B build
cmake --build build
```

## Fonctionnalités

Entité **Machine** : `ID_Machine`, `Nom_Machine`, `Type_Machine`, `Etat`, `Date_Prochaine_Maintenance`.

- Ajouter : bouton *Ajouter*
- Afficher : bouton *Afficher*
- Modifier : sélectionner une ligne puis *Modifier*
- Supprimer : bouton *Supprimer*
- Trier : liste déroulante *Trier par*
- Rechercher : champ de recherche
- Exporter PDF : bouton *Exporter PDF*
- Alertes de maintenance et indisponibilité
- Statistiques par type, état et maintenance

## Données

La base SQLite `fashionova_machines.db` est créée au premier lancement avec 12 machines de démonstration.

## Structure

```text
GestionMachines.pro / CMakeLists.txt   projet
resources.qrc                          images, icônes SVG, feuille de style
resources/style.qss                    thème

src/mainwindow.ui + mainwindow.h/.cpp  fenêtre principale
src/machinespage.ui + .h/.cpp          page Machines
src/detailsdialog.ui + .h/.cpp         boîte de détails
src/main.cpp                            démarrage
src/database.*                          accès SQLite
src/machine.h                           entité Machine
src/chartwidgets.*                      graphiques
src/bannerwidget.*                      bandeau
src/uihelpers.*                         icônes et images
```

## Personnalisation

- Menu : `src/mainwindow.ui`
- Seuil d'alerte : `src/machine.h`
- Couleurs : `resources/style.qss`