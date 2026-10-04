# FashioNova – Gestion des Machines (Qt / C++)

Application de bureau Qt Widgets reproduisant l'interface « Gestion des Machines » de FashioNova
(menu latéral avec logo, bandeau, formulaire, liste, statistiques) avec une base de données SQLite.

## Prérequis
- Qt **5.15** ou **Qt 6.x** avec les modules **Widgets, Sql (pilote QSQLITE), Svg**
- Un compilateur C++17 (MinGW, MSVC, GCC ou Clang)

## Compilation
**Avec Qt Creator** : ouvrir `GestionMachines.pro` (qmake) ou `CMakeLists.txt` (CMake), choisir un kit, puis *Exécuter*.

**En ligne de commande (qmake)** :
```
qmake GestionMachines.pro
make            # ou mingw32-make / nmake
```

**En ligne de commande (CMake)** :
```
cmake -S . -B build
cmake --build build
```

## Fonctionnalités (cahier des charges)
Entité **Machine** : `ID_Machine`, `Nom_Machine`, `Type_Machine`, `Etat`, `Date_Prochaine_Maintenance`.

| Fonction | Où |
|---|---|
| Ajouter | bouton *Ajouter* (ID généré : MCH001, MCH002…) |
| Afficher | bouton *Afficher* (détails + état de la maintenance) |
| Modifier | sélectionner une ligne, changer les champs, *Modifier* |
| Supprimer | bouton *Supprimer* (avec confirmation) |
| Trier | liste déroulante *Trier par* : Nom_Machine, Type_Machine, Etat |
| Rechercher | champ de recherche (ID_Machine, Nom_Machine, Type_Machine) – la barre du haut et celle de la liste sont synchronisées |
| Exporter PDF | bouton *Exporter PDF* (liste filtrée/triée courante) |
| Détection automatique de la maintenance | une machine est signalée si l'état est « En maintenance » ou « Indisponible », si la date est dépassée, ou si la maintenance est à moins de 7 jours : icône ⚠ et date colorée dans la liste, bandeau « n machine(s) nécessitent une maintenance » (cliquable pour filtrer), message dans le formulaire et cloche avec compteur |
| Alerte d'indisponibilité | quand une machine devient *Indisponible*, une notification rouge s'affiche ; la cloche liste toutes les alertes (un clic ouvre la machine) |

Statistiques : répartition par type, évolution de l'état du parc (le mois courant est enregistré automatiquement
dans la base ; les 5 mois précédents sont des valeurs d'exemple créées au premier lancement) et Top 5 des machines
par prochaine maintenance.

## Données
La base SQLite `fashionova_machines.db` est créée au premier lancement dans le dossier de données de l'application
(`QStandardPaths::AppDataLocation`), avec 12 machines de démonstration. Supprimez ce fichier pour repartir de zéro.
Le bouton ✎ (ou « + ») sous la photo permet d'associer une image à une machine (à valider avec *Modifier* / *Ajouter*).

## Structure
L'interface est décrite dans des fichiers **Qt Designer (`.ui`)** ; les `.cpp` ne contiennent que la logique.
```
GestionMachines.pro / CMakeLists.txt   projet
resources.qrc                          images, icônes SVG, feuille de style
resources/style.qss                    thème (couleurs, boutons, tableau…) modifiable sans recompiler le code

src/mainwindow.ui    + mainwindow.h/.cpp     fenêtre : menu latéral, barre du haut, pile de pages
src/sidebarwidget.h/.cpp                menu latéral avec image de fond (resources/images/sidebar_bg.jpg), widget promu dans mainwindow.ui
src/machinespage.ui  + machinespage.h/.cpp   page Machines : formulaire, liste, statistiques + CRUD, recherche, tri, PDF, alertes
src/detailsdialog.ui + detailsdialog.h/.cpp   boîte « Afficher » (détails d'une machine)
src/main.cpp                           démarrage, thème, ouverture de la base
src/database.*                         accès SQLite (table machine, historique)
src/machine.h                          entité Machine + règles de détection de maintenance
src/chartwidgets.*                     graphiques (anneau, courbes, barres) – widgets promus dans machinespage.ui
src/bannerwidget.*                     bandeau d'en-tête – widget promu dans machinespage.ui
src/uihelpers.*                        icônes SVG, couleurs, images arrondies
```

### Modifier l'interface avec Qt Designer
Ouvrir `src/mainwindow.ui`, `src/machinespage.ui` ou `src/detailsdialog.ui` (double-clic dans Qt Creator).
- `MachinesPage`, `BannerWidget`, `DonutChart`, `LineChart` et `TopBars` sont des **widgets promus** (conteneurs vides dans Designer, dessinés par le code).
- Les couleurs/bordures viennent de `style.qss` ; les widgets répétés (cartes, champs, libellés…) sont repérés par la propriété dynamique `role` (ex. `role = "card"`).
- Les icônes SVG sont recolorées à l'exécution : elles sont affectées dans le code (`setupIcons()`), pas dans Designer.
- Les listes *Type* et *État* du formulaire se modifient dans Designer, mais restent à garder cohérentes avec `machine.h` (`typesMachine()`, `Etat`).

## Personnalisation
- Noms du menu : boutons `btnNav…` dans `src/mainwindow.ui` (libellés « Gestion … »).
- Seuil d'alerte (7 jours) : `Machine::raisonMaintenance()` dans `src/machine.h`.
- Couleurs : `resources/style.qss`.


## Version « boutons CRUD inactifs »
Dans cette version, les boutons **Ajouter, Afficher, Modifier, Supprimer** sont visibles mais ne font rien.
Pour les réactiver : `src/machinespage.cpp` → `static const bool kCrudActif = true;`
Les autres fonctions (réinitialiser, recherche, tri, export PDF, sélection dans le tableau, statistiques) restent actives.
