# PLAN DE RÉFACTORING CLI - DEBUGGER

## 🎯 OBJECTIF

Améliorer significativement l'interface CLI du debugger pour la rendre professionnelle, modulaire et adaptée au long-running.

**Abandon de la TUI ncurses** - Focus uniquement sur la CLI.

---

## 📋 BESOINS FONCTIONNELS

### 1. Affichage clair et structuré
- **Problème actuel** : sortie texte brute, illisible, informations perdues
- **Besoin** : sortie structurée, sections clairement identifiées, hiérarchie visuelle
- **Contrainte** : ASCII uniquement (pas d'Unicode/smilies)

### 2. Adaptée au long-running
- **Problème** : trop de sortie pollue stdout
- **Besoin** : modes de verbosité, log vers fichier, résumés périodiques
- **Usage** : programmes qui tournent plusieurs heures

### 3. Mode interactif
- **Problème actuel** : une seule commande au démarrage
- **Besoin** : boucle interactive (REPL), historique, auto-complétion
- **Contrainte** : doit pouvoir tester en mode non-interactif

### 4. Modularité
- **Problème actuel** : code monolithique (debug_console.c = 22k lignes)
- **Besoin** : architecture modulaire, sections expandables
- **But** : pouvoir afficher/masquer des sections selon besoin

---

## 🏗️ ARCHITECTURE CIBLÉ

```
.
├── CMakeLists.txt           # Build system (remplace makefile)
├── cmake/
│   ├── Dependencies.cmake   # Gestion des dépendances externes
│   └── CompilerFlags.cmake  # Flags de compilation
├── src/
│   ├── debug_console.c      # Main entry point (orchestrator)
│   ├── core/
│   │   ├── debugger.h/c     # Ptrace, signal handling core
│   │   ├── registers.h/c    # Register read/write operations
│   │   ├── memory.h/c       # Memory read/write operations
│   │   └── breakpoints.h/c  # Breakpoint management
│   ├── display/
│   │   ├── output.h/c       # Output system with levels (quiet/normal/verbose)
│   │   ├── sections.h/c     # Modular sections (backtrace, regs, mem)
│   │   ├── theme.h/c        # Color detection (auto), ANSI codes
│   │   └── formatter.h/c    # ASCII formatting helpers
│   └── cli/
│       ├── parser.h/c       # Command parsing
│       ├── repl.h/c         # Interactive mode (linenoise)
│       └── config.h/c       # User preferences/settings
├── include/                 # Headers publics
├── build/                   # CMake build directory
└── config/                  # Fichiers de configuration par défaut
    └── debuggerrc.default
```

---

## 🎨 SPÉCIFICATIONS D'AFFICHAGE

### Niveaux de sortie

| Mode | Usage | Affiche |
|------|-------|---------|
| `QUIET` | Scripts, long-running | Erreurs + signaux uniquement |
| `NORMAL` | Défaut | Informations essentielles |
| `VERBOSE` | Debug | Tous les détails |
| `CUSTOM` | User | Configurable par l'utilisateur |

### Format ASCII (exemples)

**Mode QUIET :**
```
[SIGSEGV] Division by zero @ test.c:58
#0 funcdetest2+0xcc  test.c:58
#1 funcdetest+0x21   test.c:65
#2 main+0x24         test.c:80
```

**Mode NORMAL :**
```
[PROCESS] target/test  PID:1071754  Offset:0x555555554000

[SIGSEGV] Division by zero @ test.c:58

[BACKTRACE]
  #0 funcdetest2+0xcc  test.c:58
  #1 funcdetest+0x21   test.c:65
  #2 main+0x24         test.c:80

dbg>
```

**Mode VERBOSE :**
```
+-----------------------------------------------------------------+
| PROCESS: target/test                                            |
| PID: 1071754  GID: 1000  PPID: 1071752                         |
| Offset: 0x555555554000  Base: 0x7ffff7fc2000                    |
+-----------------------------------------------------------------+

[SIGSEGV] Signal 11 @ 0x0000000000000000
  si_signo: 11  si_errno: 0  si_code: 0
  Fault addr: (nil)

[BACKTRACE - 7 frames]
  #0 funcdetest2+0xcc   [test.c:58:11]      <-- Division by zero
  #1 funcdetest+0x21    [test.c:65:5]
  #2 main+0x24          [test.c:80:5]
  ...

[REGISTERS - General Purpose]
  rax: 0x0000000000000000   rbx: 0x00007fffffffdde8
  rcx: 0x0000000000000000   rdx: 0x0000000000000000
  ...

dbg>
```

---

## 🔧 COMMANDES

### Mode non-interactif (actuel, conservé)
```bash
./target/debug_console ./target/test
continue
register dump
memory read 0x...
```

### Mode interactif (nouveau)
```bash
./target/debug_console -i ./target/test
dbg> backtrace
dbg> registers --expand rax,rbx
dbg> set output quiet
dbg> help
```

### Options de ligne de commande
```bash
--quiet, -q          # Mode quiet (erreurs/signaux seulement)
--verbose, -v        # Mode verbose (tous les détails)
--log=FILE           # Log vers fichier
--log-append=FILE    # Append au fichier de log
--summary=N          # Résumé périodique toutes les N secondes
--color=WHEN         # never, auto, always
--expand-all         # Expand toutes les sections
--help, -h           # Aide
```

---

## 🔨 FONCTIONNALITÉS À IMPLÉMENTER

### Phase 1 : Refactorisation (Structure)
- [ ] Créer l'architecture modulaire (dossiers core/, display/, cli/)
- [ ] Extraire le code de debug_console.c vers les modules
- [ ] Créer les fichiers headers pour chaque module
- [ ] Définir les structures de données communes

### Phase 2 : Système d'affichage
- [ ] Implémenter `output.c` avec niveaux (quiet/normal/verbose)
- [ ] Implémenter `theme.c` (détection auto couleurs, codes ANSI)
- [ ] Implémenter `formatter.c` (ASCII, séparateurs, box-drawing)
- [ ] Créer les sections modulaires (backtrace, registers, memory)
- [ ] Système d'expand/collapse des sections

### Phase 3 : Mode interactif
- [ ] Intégrer la bibliothèque linenoise-ng
- [ ] Implémenter le REPL (Read-Eval-Print Loop)
- [ ] Ajouter l'historique des commandes
- [ ] Ajouter l'auto-complétion (commandes, noms de fonctions, registres)
- [ ] Commande `help`

### Phase 4 : Fonctionnalités avancées
- [ ] Expansion des sections (`--expand`, `--expand-all`)
- [ ] Filtres par catégorie (backtrace, registers, memory, signals)
- [ ] Log vers fichier avec rotation
- [ ] Résumé périodique pour long-running
- [ ] Configuration persistante (fichier .debuggerrc ?)

### Phase 5 : Tests et validation
- [ ] Tester toutes les commandes existantes (régression)
- [ ] Tester le mode interactif
- [ ] Tester les redirections (> fichier)
- [ ] Tester long-running (résumés périodiques)
- [ ] Valider la détection automatique des couleurs

---

## 🔧 BUILD SYSTEM : CMake

### Pourquoi CMake ?

| Make | CMake |
|------|-------|
| Gestion manuelle des dépendances | `find_package()` automatique |
| Pas de configurations Debug/Release | Multi-configuration (Debug/Release/RelWithDebInfo) |
| Makefile maison | CMake standard, tous les IDEs le supportent |
| Difficile d'intégrer des deps externes | `FetchContent` pour télécharger deps automatiquement |

### Structure CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(debugger VERSION 1.0 LANGUAGES C)

# Options de build
option(BUILD_INTERACTIVE "Build with linenoise (interactive mode)" ON)
option(BUILD_CONFIG "Build with libconfig (config file support)" ON)
option(ENABLE_TESTS "Build tests" OFF)

# Dépendances
find_package(PkgConfig REQUIRED)

# linenoise-ng
pkg_check_modules(LINENOISE REQUIRED linenoise)

# argtable3
pkg_check_modules(ARGTABLE3 REQUIRED argtable3)

# libconfig
if(BUILD_CONFIG)
    find_package(LibConfig REQUIRED)
endif()

# stb_ds.h (header-only, pas de package)
include_directories(/usr/local/include)

# Cibles
add_executable(debug_console src/debug_console.c ...)
target_link_libraries(debug_console ${LINENOISE_LIBRARIES} ...)

# Installation
install(TARGETS debug_console DESTINATION bin)
```

### Commandes de build

```bash
# Configuration
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Compilation
cmake --build .

# Installation
sudo cmake --install .
```

---

## 📦 DÉPENDANCES EXTERNES (Option A - Maximaliste)

| Librairie | Usage | Code économisé |
|-----------|-------|----------------|
| **linenoise-ng** | REPL, historique, auto-complétion | ~200 lignes |
| **argtable3** | Parsing arguments ligne de commande | ~100 lignes |
| **stb_ds.h** | Data structures (arrays, hashmaps) | ~200 lignes |
| **libconfig** | Fichier de configuration `.debuggerrc` | ~150 lignes |

**Total : ~650 lignes de code en moins !**

**Installation :**
```bash
# linenoise-ng (REPL)
git clone https://github.com/arcanedmaragos/linenoise-ng.git
cd linenoise-ng && make && sudo make install

# argtable3 (Arguments)
wget http://sourceforge.net/projects/argtable/files/argtable/argtable-3.2/argtable3-3.2.2.tar.gz
tar xvf argtable3-3.2.2.tar.gz && cd argtable3-3.2.2
./configure && make && sudo make install

# stb_ds.h (Data structures, header-only)
wget https://github.com/nothings/stb/raw/master/stb_ds.h -O /usr/local/include/stb_ds.h

# libconfig (Config fichier)
sudo apt install libconfig-dev  # Debian/Ubuntu
# ou
brew install libconfig          # macOS
```

---

## ❓ DÉCISIONS À PRENDRE

### 1. Prompt du mode interactif
- A) `dbg> ` (simple, court) **← RECOMMANDÉ**
- B) `(dbg) ` (style GDB)
- C) `> ` (minimaliste)
- **À décider**

### 2. Nom de l'exécutable
- A) Garder `debug_console`
- B) Renommer en `debug` ou `dbg`
- **À décider**

### 3. Fichier de configuration
- A) Créer un système de config (`~/.debuggerrc`)
- B) Pas de fichier de config (options ligne de commande seulement)
- **À décider**

---

## 📊 MÉTRIQUES DE SUCCÈS

### Avant (actuel)
- [ ] Code monolithique (debug_console.c = 22k lignes)
- [ ] Sortie texte brute illisible
- [ ] Pas de mode interactif
- [ ] Pas adapté au long-running

### Après (cible)
- [ ] Architecture modulaire (8-10 fichiers)
- [ ] Sortie structurée avec sections
- [ ] Mode interactif avec historique
- [ ] Modes quiet/verbose + log fichier
- [ ] Toutes les commandes actuelles préservées

---

## 🚢 PLAN D'IMPLÉMENTATION

1. **Validation du plan** : Relecture et approbation ✅
2. **Phase 1** : Création de l'architecture modulaire ✅
   - Headers créés (10 fichiers)
   - Sources créés (10 fichiers)
   - Erreurs de compilation corrigées (4)
3. **Phase 2** : Système d'affichage ✅ TERMINÉE
   - theme.c: Détection multi-niveaux, 16 couleurs, styles combinés
   - output.c: Préfixes, couleurs par catégorie, statistiques
   - sections.c: Intégration avec output/theme
4. **Phase 3** : Mode interactif ✅ TERMINÉE
   - parser.c: Table de lookup, commandes set/filter
   - repl.c: Intégration output/theme, mode fallback
   - config.c/h: Configuration unifiée
5. **Phase 4** : Fonctionnalités avancées ⏳ PENDING
6. **Phase 5** : Tests et validation ⏳ PENDING
7. **Documentation** : README mis à jour ⏳ PENDING

---

## 📊 PROGRESSION

| Phase | Statut | Détails |
|-------|--------|---------|
| Phase 1 | ✅ TERMINÉE | Architecture modulaire créée, 20 fichiers créés |
| Phase 2 | ✅ TERMINÉE | Détection terminale, 16 couleurs, styles combinés, préfixes, statistiques |
| Phase 3 | ✅ TERMINÉE | Parser avec lookup, REPL intégré, configuration unifiée |
| Phase 4 | ⏳ PENDING | Expansion sections, filtres, log fichier |
| Phase 5 | ⏳ PENDING | Tests régression, validation |

---

## 📝 PHASE 3 - DÉTAILS

### parser.c/h (Terminé)
- **Table de lookup**: Recherche rapide des commandes par nom
- **Commandes supportées**:
  - `set output <level>`: quiet|normal|verbose|debug
  - `set expand <level>`: none|normal|full
  - `filter <cats>`: Liste séparée par virgules
- **Aide**: `parser_print_usage()` avec affichage coloré
- **Aide détaillée**: `parser_print_command_help()` par commande

### repl.c/h (Terminé)
- **Intégration output/theme**: Affichage coloré des messages
- **Fonction execute_command()**: Traitement centralisé
- **Commandes implémentées**:
  - `set output`: Change le niveau de verbosité
  - `set expand`: Change le niveau d'expansion
  - `filter`: Filtre les catégories de sortie
- **Mode fallback**: Basic fgets() sans linenoise
- **Auto-complétion**: Suggestions pour set output/expand
- **Hints inline**: Aide contextuelle pendant la frappe

### config.c/h (Terminé)
- **Configuration unifiée**: output + theme + sections + REPL
- **config_apply()**: Applique toute la configuration aux systèmes
- **Fonctions de modification**:
  - `config_set_output_level()`
  - `config_set_color_mode()`
  - `config_set_timestamps()`
  - `config_set_expand()`
- **config_reset()**: Retour aux valeurs par défaut

---

## 📝 PHASE 2 - DÉTAILS

### theme.c/h (Terminé)
- Détection multi-niveaux: isatty(), NO_COLOR, TERM, COLORTERM
- 16 couleurs: 8 normales + 8 bright variants
- Styles combinés: bold + color (theme_format)
- Mapping automatique des couleurs par catégorie

### output.c/h (Terminé)
- Préfixes par catégorie: [PROCESS], [SIGNAL], [BACKTRACE], etc.
- Couleurs automatiques par catégorie
- Statistiques: signals_received, breakpoints_hit, steps_executed, memory_reads/writes
- Fonctions spécialisées: output_signal(), output_process_info(), output_error()
- Timestamps optionnels
- Niveau de verbosité: QUIET/NORMAL/VERBOSE/DEBUG

### sections.c/h (Terminé)
- Intégration complète avec output.c et theme.c
- signal_info_print(): Affichage des signaux avec couleurs
- breakpoint_hit_print(): Affichage des breakpoints
- Expansion: EXPAND_NONE (compact), EXPAND_NORMAL, EXPAND_FULL
- Affichage conditionnel selon le niveau de verbosité

---

*Créé le 2026-01-28*
*Dernière mise à jour: Phase 2 terminée*
