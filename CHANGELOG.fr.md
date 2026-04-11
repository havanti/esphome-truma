# Journal des modifications

[🇩🇪 Deutsch](CHANGELOG.md) | [🇬🇧 English](CHANGELOG.en.md) | 🇫🇷 Français

Toutes les modifications notables de ce projet sont documentées dans ce fichier.

Le format est basé sur [Keep a Changelog](https://keepachangelog.com/fr/1.0.0/).

---

## État de compatibilité

### Résumé

Cette version rétablit la compatibilité avec ESPHome 2025.8 à 2026.3.x.
La cause principale était la suppression de `get_uart_event_queue()` du composant
`IDFUARTComponent` upstream dans ESPHome 2025.8, ce qui rendait la détection des
signaux BREAK du bus LIN non fonctionnelle pour les builds ESP-IDF. Des changements
cassants supplémentaires liés à ESP-IDF 5.x (chaîne d'outils ESP32) et aux
modifications d'API ESPHome 2026.x ont également été corrigés.

Testé avec :
- ESPHome **2026.3.3** — ESP-IDF ✅
- ESPHome **2026.3.2** — ESP-IDF ✅
- ESPHome **2026.3.1** — ESP-IDF ✅
- ESPHome **2026.3.0** — ESP-IDF ✅
- ESPHome **2026.2.2** — ESP-IDF ✅
- ESPHome **2025.9.3** — ESP-IDF ✅

---

## [1.0.10] — 2026-04-11 — Nettoyage supplémentaire

### Modifié
- Définitions de macros dupliquées `DIAGNOSTIC_FRAME_MASTER` / `DIAGNOSTIC_FRAME_SLAVE` supprimées de deux fichiers `.cpp` et regroupées en une seule `constexpr` dans `LinBusListener.h`
- Constante magique `1440` remplacée par la constante nommée `MINUTES_PER_DAY`

---

## [1.0.9] — 2026-04-02 — Nettoyage

### Modifié
- Timeouts codés en dur remplacés par des constantes `constexpr` nommées
- `const` ajouté à la variable locale `lin_identifier`

---

## [1.0.8] — 2026-03-30 — Qualité du code

### Corrigé
- Assignations incorrectes des champs dans les trames de réponse pour le mix énergétique et le niveau de puissance électrique corrigées

### Modifié
- Fautes de frappe nettoyées dans l'ensemble du codebase
- Commentaires obsolètes et résolus supprimés
- Commentaires du code révisés et harmonisés

### Documentation
- Référence à @kamahat et son fork ajoutée dans tous les READMEs

---

## [1.0.7] — 2026-03-28 — Améliorations mineures

### Corrigé
- Niveau de log pour « LIN CRC error on SID » abaissé de WARN à VERBOSE — pas une vraie erreur, juste une réponse trop lente de la Truma (suggéré par @kamahat)

### Documentation
- `min_version: 2026.3.1` ajouté dans tous les YAMLs d'exemple
- Fichiers CONTRIBUTING ajoutés (DE/EN/FR)

---

## [1.0.6] — 2026-03-27 — Robustesse

### Corrigé

#### `components/truma_inetbox/LinBusListener_esp_idf.cpp`
- `uartEventTask_` : crash au démarrage corrigé sur ESP32 double cœur, où le task pouvait
  appeler `xQueueReceive()` avec un handle de queue NULL avant que `uart_driver_install()`
  sur le cœur 1 n'ait terminé
- Timeout de 5 secondes ajouté à la boucle d'attente de la queue : si le pilote UART ne
  devient jamais disponible (ex. échec du setup UART), le task journalise désormais une
  erreur explicite et se termine proprement au lieu de boucler silencieusement

---

## [1.0.5] — 2026-03-27 — Améliorations

### Modifié

#### Fichiers YAML d'exemple (les quatre)
- `refresh` dans `external_components` modifié de `0s` à `24h` — ESPHome vérifie les mises à jour une fois par jour
- Deux alternatives commentées ajoutées : `refresh: always` (pour le développement) et `refresh: 0s` (sans mise à jour automatique)

---

## [1.0.4] — 2026-03-23 — Corrections de bogues

### Corrigé

#### `components/uart/__init__.py`
- `validate_raw_data()` : second contrôle `isinstance(value, str)` (code mort, jamais atteint) corrigé en `isinstance(value, bytes)`

#### `README.md` / `README.en.md`
- Référence de fichier obsolète `ESP32-S3_truma_6DE_example.yaml` → `ESP32-S3_truma_6DE_Diesel_example.yaml` (fichier renommé précédemment)

---

## [1.0.3] — 2026-03-22 — OTA, nettoyage

### Ajouté

#### Tous les fichiers YAML d'exemple basés sur WiFi
- Bloc `ota` ajouté (`platform: esphome`, mot de passe fictif) à toutes les configurations d'exemple WiFi

#### `README.md` / `README.en.md`
- Section OTA ajoutée : explication des mises à jour Over-the-Air et note sur le mot de passe fictif

### Supprimé

- `WomoLinControllerEthernet.yaml` — supprimé (spécifique Ethernet, non maintenu ici)
- `WomoLinControllerEthernetMqtt.yaml` — supprimé (spécifique Ethernet, non maintenu ici)
- Répertoire `examples/` — supprimé (remplacé par les fichiers YAML d'exemple à la racine)

---

## [1.0.2] — 2026-03-19 — Configurations d'exemple et documentation

### Ajouté

#### `ESP32_truma_4-6_Gas_example.yaml` / `ESP32-S3_truma_4-6_Gas_example.yaml` (nouveau)
- Variante gaz des configurations d'exemple avec `HEATER_GAS` et `HEATER_ENERGY_MIX_GAS`
- Script de décokéfaction diesel, capteur et boutons non inclus (fonctionnement gaz uniquement)

### Modifié

#### `ESP32_truma_6DE_example.yaml` → `ESP32_truma_6DE_Diesel_example.yaml`
#### `ESP32-S3_truma_6DE_example.yaml` → `ESP32-S3_truma_6DE_Diesel_example.yaml`
- Renommés pour identifier explicitement la variante diesel

#### `components/truma_inetbox/__init__.py` / `components/uart/__init__.py`
- `synchronous=True` ajouté à tous les appels `register_action()`
  (ESPHome 2026.3.0 exige ce paramètre ; toutes les méthodes `play()` sont synchrones)

#### `README.md` / `README.en.md`
- Section configuration d'exemple restructurée en sélection en 2 étapes (mix énergétique → matériel)
- Tableau récapitulatif variantes gaz/diesel ajouté
- Note de compatibilité Truma Combi 4 ajoutée
- Avertissement de compatibilité ajouté : testé avec Truma Combi 6DE (2018, brûleur Eberspächer) ;
  les générations diesel Truma plus récentes sans Eberspächer ne sont pas vérifiées
- Introduction upstream (dépôt Fabian-Schmidt) supprimée
- Révision éditoriale

---

## [1.0.1] — 2026-03-14 — Compatibilité ESPHome 2026.6 (suivi des dépréciations)

### Modifié

#### `components/truma_inetbox/__init__.py`
- `CORE.using_esp_idf` → `CORE.is_esp32 and not CORE.using_arduino`
  Déprécié depuis ESPHome 2026.1 (changement de comportement en 2026.6). La condition cible
  les builds ESP-IDF uniquement, où les macros `ARDUINO_SERIAL_EVENT_TASK_*` ne sont pas
  fournies par le framework.

#### `components/uart/__init__.py`
- `CORE.using_esp_idf` → `not CORE.using_arduino`
  Même correction de dépréciation dans le sélecteur de type UART (`_uart_declare_type`).

#### `components/truma_inetbox/LinBusListener_esp_idf.cpp`
- Defines de repli `#ifndef` ajoutés pour `ARDUINO_SERIAL_EVENT_TASK_STACK_SIZE` (4096)
  et `ARDUINO_SERIAL_EVENT_TASK_RUNNING_CORE` (0) afin que le fichier compile même sans
  les flags de build (filet de sécurité).

#### `components/truma_inetbox/climate/TrumaWaterClimate.cpp`
#### `components/truma_inetbox/climate/TrumaRoomClimate.cpp`
- `traits.set_supports_current_temperature(true)`
  → `traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE)`
  `set_supports_current_temperature` est déprécié depuis ESPHome 2025+.

---

## [1.0.0] — 2026-03-02 — Compatibilité ESPHome 2025.8+ / 2026.3.x — Détails

### Modifié — `components/uart/`

#### `uart_component.h`
- `virtual int available()` → `virtual size_t available()` conformément à la signature ESPHome 2025.8+
- Implémentations par défaut (no-op) ajoutées pour les nouvelles méthodes virtuelles introduites
  dans ESPHome 2025.8 : `set_rx_full_threshold()`, `set_rx_timeout()`, `load_settings(bool)`, `load_settings()`

#### `uart_component.cpp`
- `check_read_timeout_()` utilise désormais des comparaisons `size_t` (sans casts `int` inutiles)

#### `uart_component_esp_idf.h` _(critique)_
- Guard préprocesseur modifié : `USE_ESP_IDF` → `USE_ESP32_FRAMEWORK_ESP_IDF`
- Membre `SemaphoreHandle_t lock_` supprimé (mutex retiré de l'upstream en 2025.8)
- `int available()` → `size_t available()`
- `get_hw_serial_number()` ajouté directement à la classe de base `IDFUARTComponent`
- Déclarations pour `load_settings()`, `set_rx_full_threshold()`, `set_rx_timeout()` ajoutées
- `uart_event_queue_` conservé **inconditionnellement** (non protégé par `USE_UART_WAKE_LOOP_ON_RX`),
  car la tâche de détection des signaux BREAK du bus LIN en a besoin en permanence

#### `uart_component_esp_idf.cpp` _(critique)_
- Guard préprocesseur modifié : `USE_ESP_IDF` → `USE_ESP32_FRAMEWORK_ESP_IDF`
- `UART_SCLK_APB` → `UART_SCLK_DEFAULT` (changement d'API ESP-IDF 5.x)
- `portTICK_RATE_MS` → `pdMS_TO_TICKS(20)` (supprimé d'ESP-IDF 5.x)
- Tous les appels mutex take/give `lock_` supprimés (~12 emplacements)
- `static uint8_t next_uart_num` → `static uart_port_t next_uart_num = UART_NUM_0`
  (ESP-IDF 5.x : `uart_port_t` est un enum de portée, sans conversion implicite vers `uint8_t`)
- Postfix `++` sur `uart_port_t` remplacé par un cast explicite :
  `next_uart_num = (uart_port_t)(next_uart_num + 1)`
- `int available()` → `size_t available()`
- Implémentations pour `load_settings()`, `set_rx_full_threshold()`, `set_rx_timeout()` ajoutées

#### `truma_uart_component_esp_idf.h`
- Guard préprocesseur modifié : `USE_ESP_IDF` → `USE_ESP32_FRAMEWORK_ESP_IDF`
- `get_hw_serial_number()` supprimé (désormais fourni par la classe de base `IDFUARTComponent`)
- `get_uart_event_queue()` conservé, expose `&uart_event_queue_`

#### `uart_component_esp32_arduino.h` / `.cpp`
- `int available()` → `size_t available()`
- `check_logger_conflict()` : `logger::global_logger->get_hw_serial()` protégé par
  `#if defined(USE_LOGGER) && !defined(USE_ESP32)` — ESPHome 2026.1 a supprimé
  `get_hw_serial()` de `Logger` pour ESP32 (Arduino sur ESP32 se base désormais sur IDF)

#### `uart_component_rp2040.h` / `.cpp`
- `int available()` → `size_t available()`

#### `uart_component_esp8266.h` / `.cpp`
- `ESP8266UartComponent::available()` : `int` → `size_t`

---

### Modifié — `components/truma_inetbox/`

#### Remplacement des types entiers POSIX (30 fichiers concernés)
- `u_int8_t` → `uint8_t`
- `u_int16_t` → `uint16_t`
- `u_int32_t` → `uint32_t`

Ces types POSIX (`u_int*_t`) sont définis implicitement par les headers glibc / BSD libc
qu'inclut automatiquement la chaîne d'outils Arduino. La chaîne d'outils GCC d'ESP-IDF 5.x
ne les fournit **pas**, ce qui provoquait 294 erreurs de compilation dans 30 fichiers.

Fichiers concernés :
`LinBusProtocol.h`, `LinBusProtocol.cpp`, `LinBusListener.h`, `LinBusListener.cpp`,
`TrumaiNetBoxApp.h`, `TrumaiNetBoxApp.cpp`, `TrumaiNetBoxAppHeater.h/cpp`,
`TrumaiNetBoxAppAirconManual.h/cpp`, `TrumaiNetBoxAppAirconAuto.h/cpp`,
`TrumaiNetBoxAppClock.h/cpp`, `TrumaiNetBoxAppTimer.h/cpp`,
`TrumaStructs.h`, `TrumaEnums.h`, `TrumaStatusFrameBuilder.h`,
`TrumaStausFrameResponseStorage.h`, `helpers.h`, `helpers.cpp`,
`automation.h`, `time/TrumaTime.h` ainsi que les sous-composants sensor/number/select/climate.

#### `LinBusListener_esp_idf.cpp`
- `#define QUEUE_WAIT_BLOCKING (portTickType) portMAX_DELAY`
  → `(TickType_t) portMAX_DELAY`
  (`portTickType` a été renommé en `TickType_t` dans FreeRTOS 10 / ESP-IDF 5.x)
- `uart_intr_config(uart_num, &uart_intr)` → `uart_intr_config((uart_port_t) uart_num, &uart_intr)`
  (ESP-IDF 5.x : `uart_intr_config` exige un `uart_port_t`, sans conversion implicite depuis `uint8_t`)

#### `LinBusListener_esp32_arduino.cpp`
- `#define QUEUE_WAIT_BLOCKING (portTickType) portMAX_DELAY`
  → `(TickType_t) portMAX_DELAY`
  (même renommage FreeRTOS, affecte aussi Arduino sur ESP32 qui s'appuie sur ESP-IDF 5.x)

---

### Ajouté

- `test_compile.yaml` — configuration de test minimale pour les builds framework Arduino ESP32
- `test_compile_idf.yaml` — configuration de test minimale pour les builds framework ESP-IDF ESP32

---

### Notes

- ESPHome **2026.1.x n'existe pas** sur PyPI — la numérotation des versions saute directement
  de 2025.10.x à 2026.2.x.
- ESPHome 2026.1 a déprécié `CORE.using_esp_idf` (avertissement uniquement ; changement de
  comportement en 2026.6). Arduino sur ESP32 se base désormais officiellement sur ESP-IDF,
  ce qui rend les fonctionnalités IDF disponibles dans les deux frameworks. Le composant
  `uart_component_esp32_arduino` continue de fonctionner comme override personnalisé pour ce projet.
- L'installation d'ESPHome 2026.2.x dans un venv Python nécessite également le paquet
  `fatfs-ng` (`pip install fatfs-ng`) en tant que dépendance transitive PlatformIO.
