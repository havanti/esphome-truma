# ESPHome & Truma CP Plus — chauffage intelligent et plus encore 🚐

[🇩🇪 Deutsch](README.md) | [🇬🇧 English](README.en.md) | 🇫🇷 Français

> Contributions bienvenues — merci d'[ouvrir une issue](../../issues) avant de soumettre une PR. Voir [CONTRIBUTING.fr.md](CONTRIBUTING.fr.md).

## Remerciements

Un sincère remerciement à **[Fabian Schmidt](https://github.com/Fabian-Schmidt)**, dont remarquable le travail sur l'original  [esphome-truma_inetbox](https://github.com/Fabian-Schmidt/esphome-truma_inetbox)-Repository referenciel qui a initiallement tout redu possible. Ce projet est un fork de son travail et sans son dévouement et son expertise, rien de tout cela n’aurait été possible. Merci, Fabian !

Ce projet s’appuie également sur l’incroyable travail préparatoire de la [WomoLIN-Projekts](https://github.com/muccc/WomoLIN) et [Daniel Fetts inetbox.py](https://github.com/danielfett/inetbox.py) puis [mc0110s inetbox2mqtt](https://github.com/mc0110/inetbox2mqtt) leurs recherches sur le protocole, leurs fichiers journaux et leur documentation ont été d'une valeur inestimable..

---

## Ce que ce Fork ajoute

Ce Fork étend le composant d'origine avec plusieurs fonctionnalités éprouvées sur le terrain développées en fonctionnement quotidien dans un camping-car avec un Truma Combi 6DE et une carte ESP32-S3. La configuration complète et fonctionnelle se trouve dans [`ESP32-S3_truma_6DE_Diesel_example.yaml`](ESP32-S3_truma_6DE_Diesel_example.yaml).

### TPMS — Surveillance de la pression des pneus via proxy Bluetooth

L'ESP32 agit également comme un récepteur Bluetooth Low Energy (BLE) pour les capteurs TPMS disponibles dans le commerce, éliminant ainsi le besoin d'une passerelle séparée. Quatre capteurs sont surveillés simultanément (VL, VR, HR, HL), chacun signalant :

- Pression des pneus en bar
- Température des pneus en °C
- Tension de la batterie du capteur en V

L'intégration utilise `esp32_ble_tracker` avec un scan passif et parse les parametre specific constructeur directement dans une fonction Lambda en C++. Les douze entités de capteurs apparaissent automatiquement dans Home Assistant en tant que capteurs de diagnostic.

Pour adapter cela à vos propres capteurs, les quatre adresses MAC dans le `on_ble_advertise` Remplacez les blocs par vos propres adresses de capteur TPMS. La logique de décodage (décalage d'impression, mise à l'échelle) peut necessité d'être ajustée en fonction de la marque du capteur.

> le balayage BLE et le bus Truma LIN fonctionnent en parallèle sur la même puce. Sur un ESP32-S3 avec PSRAM OctalSPI, la pile BLE peut être déchargée sur la PSRAM, réduisant ainsi considérablement le risque de conflits de mémoire. La PSRAM incluse `sdkconfig_options` dans `ESP32-S3_truma_6DE_Diesel_example.yaml` sont déjà configurés en conséquence.

> **Variante TPMS modulaire :** [@kamahat](https://github.com/kamahat) a extrait la logique TPMS dans un fichier `tpms.yaml` autonome et une fonction auxiliaire C++. Si vous préférez une structure modulaire, consultez son [fork](https://github.com/kamahat/esphome-truma).

### Diesel-„Ramonage" / Residus de Combustion

Si un Truma Combi fonctionne au diesel pendant une longue période, des dépôts de suie peuvent s'accumuler sur ou dans la zone du brûleur (matériau fritté en acier inoxydable) et sur la fenetre. Les precautions recommandés :

- Effectuer un cycle de ramoage mensuel (automatisé par cette configuration, voir ci-dessous).
- Utilisez du carburant diesel de la plus haute qualité possible ou ajoutez un additif de carburant augmentant l'indice de cétane au réservoir — un indice de cétane plus élevé conduit à une combustion plus propre et réduit les dépôts.

Le script intégré `script_diesel_decoking` automatise de processus :

1. Passe le mix énergétique au diesel
2. Régle le radiateur d'appoint sur le mode 30 °C / HIGH pendant 45 minutes
3. Éteint le chauffage après
4. Après l'arrêt, le Truma Combi démarre automatiquement un processus de post-incandescence de la bougie de préchauffage pour brûler tous les résidus sur son filtre — ceci se produit indépendamment de la fonction de décokage et fait partie intégrante du processus d'arrêt interne du Truma
5. (Ouvrez portes et fenêtres ;-) )

Deux boutons sont fournis dans Home Assistant :

| bouton | fonction |
|---|---|
| Start Diesel De-coking | Démarre le cycle de ramonage de 45 minutes |
| Abort Diesel De-coking | Annule le cycle et éteint le chauffage |

Un capteur de modèle (Diesel De-coking Remaining Time, unité : min) affiche le temps restant et est visible dans le tableau de bord Home Assistant et l'interface utilisateur Web intégrée.

### LED RVB embarqué — indicateur d'état visuel (ESP32-S3)

Le [Waveshare ESP32-S3-DEV-KIT-N8R8](https://www.waveshare.com/wiki/ESP32-S3-DEV-KIT-N8R8) et de nombreuses autres cartes de développement ESP32-S3 disposent d'une LED RVB WS2812 intégrée (GPIO38) qui peut être utilisée comme indicateur d'état visuel pour le bus LIN sans matériel supplémentaire.

L'exemple de configuration utilise cette LED avec deux signaux :

| Couleur | Signification |
|---|---|
| Flashs verts (toutes les 2 s, 500 ms) | CP Plus connecté — bus LIN actif |
| Flashs bleus (300 ms) | Commande TX envoyée au radiateur |
| Off | CP Plus non connecté |

**Avantages par rapport à un écran Home Assistant pur :**

- Retour visuel instantané directement sur l'appareil, sans application ni tableau de bord
- Détectable si l'ESP communique avant que le WiFi ou le HA ne soit disponible
- Utile pour la mise en service initiale et le dépannage sur site

**Remarque sur le délai d'attente de 90 secondes :** Le `CP_PLUS_CONNECTED`-Rapports de capteurs `false` seulement 90 secondes après le dernier paquet LIN reçu. Par conséquent, si le CP Plus est physiquement déconnecté, la LED reste verte jusqu'à 90 secondes — c'est le comportement désigné du protocole de bus LIN, qui est destiné à tolérer de courtes interruptions de connexion.

L'implémentation utilise deux globaux ESPHome (`led_color`, `led_ticks`) et un intervalle de 100 ms comme pilote LED, afin que d'autres intervalles (affichage en direct) et actions de commutation (commandes TX) puissent contrôler la LED en définissant simplement ces variables.

### Réglage fin, surveillance et stabilité

L'exemple de configuration contient un certain nombre de paramètres éprouvés en production qui ne sont pas inclus dans l'exemple de base :

Résilience WiFi — Un intervalle de 5 minutes vérifie la perte de connexion et effectue une reconnexion douce (`wifi.disable` → Retard → `wifi.enable`), sans redémarrer l'ESP.. `reboot_timeout` est allumé `0s` réglé pour éviter les redémarrages inattendus pendant les cycles de chauffage.

Optimisation RF WiFi — La puissance de transmission est activée `17 dB` éfinir et le mode d'économie d'énergie sur `light` ensemble, pour une connexion fiable dans une carrosserie de véhicule métallique.

Diagnostic du système — Les capteurs suivants sont toujours disponibles dans Home Assistant :

| capteur | description |
|---|---|
| TR ESP32 Temperature | Température interne de la puce |
| TR WiFi Signal dB | RSSI brut en dBm (mis à jour toutes les 60 s) |
| TR WiFi Signal | RSSI mappé à 0–100 % pour un tableau simple |
| Uptime Sensor | Temps écoulé depuis le dernier redémarrage |

Synchronisation de l'heure Home Assistant — L'horloge ESP est maintenue synchronisée via la plate-forme horaire Home Assistant, ce qui est nécessaire au bon fonctionnement des actions de la minuterie.

Interface utilisateur Web intégrée — Un serveur Web local fonctionne sur le port 80 (ESPHome Web Server v3) avec `include_internal: true`, afin que toutes les entités, y compris les données de diagnostic internes, soient visibles directement dans le navigateur sans avoir besoin de Home Assistant.

Modèles d'interrupteurs — Des interrupteurs marche/arrêt prêts à l'emploi pour le radiateur, la chaudière à eau et la minuterie intégrée sont inclus, simplifiant l'automatisation et l'intégration du tableau de bord.

Bouton reboot — Un bouton de redémarrage ESP en un clic est disponible dans Home Assistant pour la maintenance à distance.

---

## Exemples de configurations

Ce référentiel fournit quatre configurations d'échantillons prêtes à l'emploi pour la famille de radiateurs Truma Combi.
Tous utilisent le framework ESP-IDF et s'approvisionnent en composants directement à partir de ce référentiel.
Nécessite ESPHome >= 2026.3.0.

**Modèles pris en charge**

| Modèle          | Performance      | Combustible       | Électrique (230 V) |
| --------------- | ---------------- | -------------------- | --------------- |
| Combi 4         | ~4 kW            | Gaz (propane/butane) | ❌              |
| Combi 6         | ~6 kW            | Gaz (propane/butane) | ❌              |
| Combi 4E        | ~4 kW (+Hybride) | Gas                  | ✅              |
| Combi 6E        | ~6 kW (+Hybride) | Gas                  | ✅              |
| Combi Diesel 4  | ~4 kW            | Diesel               | ❌              |
| Combi Diesel 6  | ~6 kW            | Diesel               | ❌              |
| Combi Diesel 4E | ~4 kW (+Hybride) | Diesel               | ✅              |
| Combi Diesel 6E | ~6 kW (+Hybride) | Diesel               | ✅              |

> **Remarque sur la compatibilité :** Développé et testé avec un Truma Combi 6DE (construit en 2018, brûleur Eberspächer). Il n'est pas certain que d'autres modèles, et notamment les nouvelles générations de diesel, soient également compatibles avec un brûleur développé par Truma elle-même (sans Eberspächer). Vos commentaires à ce sujet sont les bienvenu — Merci [GitHub Issue](https://github.com/havanti/esphome-truma/issues) öffnen.

### Étape 1: Choisissez la variante du mix énergétique

Choisissez la variante appropriée en fonction de l'équipement du véhicule. Les **E-Varianten** (4E / 6E / DE) prennent en charge en outre le fonctionnement électrique — les entités correspondantes (`HEATER_ELECTRICITY`, `ELECTRIC_POWER_LEVEL`) sont déjà inclus dans les deux exemples de configurations.

| Variante | ESP32 | ESP32-S3 |
|---|---|---|
| **Gaz** | [`ESP32_truma_4-6_Gas_example.yaml`](ESP32_truma_4-6_Gas_example.yaml) | [`ESP32-S3_truma_4-6_Gas_example.yaml`](ESP32-S3_truma_4-6_Gas_example.yaml) |
| **Diesel** | [`ESP32_truma_6DE_Diesel_example.yaml`](ESP32_truma_6DE_Diesel_example.yaml) | [`ESP32-S3_truma_6DE_Diesel_example.yaml`](ESP32-S3_truma_6DE_Diesel_example.yaml) |

La variante détermine quelles entités ESPHome sont activées :

| | Variante Gaz | Variante Diesel |
|---|---|---|
| Capteur binaire (carburant) | `HEATER_GAS` | `HEATER_DIESEL` |
| Selecteur Mix-Energie | `HEATER_ENERGY_MIX_GAS` | `HEATER_ENERGY_MIX_DIESEL` |
| Diesel „décokage ou combustion résiduelle (ESP32-S3 uniquement) | N/A | inclus |

> Remarque : seulement **UN** Choix-Mix-Energie-Mix par configuration — gaz ou diesel, pas les deux en même temps.

### Étape 2: Choisissez la variante matérielle

| caractéristique | ESP32 | ESP32-S3 |
|---|---|---|
| Micro CPU | ESP32 (classique, Rev ≥ 3) | ESP32-S3 |
| Carte | `esp32dev` | `esp32-s3-devkitc-1` |
| PSRAM | 	non utilisé | OctalSPI-PSRAM activé (N16R8, 8 MB) |
| BLE-Stack | dans la RAM interne | externalisé vers PSRAM |
| Pin LIN UART TX | GPIO17 | GPIO18 (évite les conflits de broches PSRAM) |
| Pin LIN UART RX | GPIO16 | GPIO8 (évite les conflits de broches PSRAM) |
| Revision mi de la puce | facultatif  (`CONFIG_ESP32_REV_MIN`, commenté) | aucune restriction |
| LED RVB embarquée | Absent | WS2812, GPIO38, Idicateur d'etat du bus LIN |
| Diesel-„Ramonage" ou combustion résidus | Absent | inclus (variante diesel uniquement) |
| Niveau de journalisation | `DEBUG` | `DEBUG` |

Utilisez la variante ESP32 s'il existe un ESP32 standard (WROOM-32, DevKit, etc.) sans PSRAM. Décommenter `CONFIG_ESP32_REV_MIN: "3"` et `version: recommended` peut réduire la taille binaire sur les anciens toolchain.
Utilisez la variante ESP32-S3 si un module ESP32-S3 avec OctalSPI PSRAM (par exemple N16R8) est présent. 
La configuration PSRAM (mode OCT, 80 MHz) est requise pour cette variante de module. 
Les broches UART ont été éloignées des GPIO16/17, qui sont réservées à la PSRAM sur les cartes S3.

### Exigences

Utiliser toutes les configurations `secrets.yaml` pour les données d'accès Wi-Fi. A `secrets.yaml` créer dans le même répertoire avec :

```yaml
wifi_WOMO_WLAN_ssid: "MobileSSID"
wifi_WOMO_password: "MotDePasseMobile"
wifi_Home_ssid: "SSIDMaison"
wifi_Home_password: "MotDePasseMaison"
api_encryption_key: ""
```

L' `api`-La clé de chiffrement peut être laissée vide pour une utilisation locale ou remplie avec une clé Base64 de 32 octets générée par ESPHome.

### OTA (Miase à jour Over-the-Air)

Toutes les configurations d'échantillons contiennent le Block `ota` ; bloc qui permet les mises à jour du firmware directement via Wi-Fi — sans avoir à connecter physiquement l'ESP32 :

```yaml
ota:
  platform: esphome
  password: "12345678901234567890123456789012"
```

**Important:** Le mot de passe dans les fichiers d'exemple est un espace réservé. Remplacez-le par votre propre mot de passe long avant utilisation et conservez-le en toute sécurité. Si vous oubliez le mot de passe, vous devrez flasher à nouveau l'ESP32 via USB.

### Exemple minimal

```yaml
esphome:
  name: "esphome-truma"

external_components:
  - source:
      type: git
      url: https://github.com/havanti/esphome-truma.git
    components: [truma_inetbox, uart]
    refresh: 0s

esp32:
  board: esp32dev
  framework:
    type: esp-idf
    version: recommended

uart:
  - id: lin_uart_bus
    tx_pin: 17
    rx_pin: 16
    baud_rate: 9600
    stop_bits: 2

truma_inetbox:
  uart_id: lin_uart_bus
  lin_checksum: VERSION_2

binary_sensor:
  - platform: truma_inetbox
    name: "CP Plus alive"
    type: CP_PLUS_CONNECTED

sensor:
  - platform: truma_inetbox
    name: "Current Room Temperature"
    type: CURRENT_ROOM_TEMPERATURE
  - platform: truma_inetbox
    name: "Current Water Temperature"
    type: CURRENT_WATER_TEMPERATURE
```

## Composants ESPHome

Ce projet contient les composants ESPHome suivants :

- `truma_inetbox` avec les paramètres suivants :
  - `cs_pin` (facultatif) si la broche de la puce du pilote LIN est connectée.
  - `fault_pin` (facultatif) si la broche de la puce du pilote LIN est connectée.
  - `on_heater_message` (facultatif) [ESPHome-Trigger](https://esphome.io/guides/automations.html) lorsqu'un message est reçu du CP Plus.

Nécessite ESPHome 2026.3.0 ou supérieur.

### Capteur binaire

Les capteurs binaires sont en lecture seule.

```yaml
binary_sensor:
  - platform: truma_inetbox
    name: "CP Plus alive"
    type: CP_PLUS_CONNECTED
```

Ce qui suit `type` les valeurs sont disponibles :

- `CP_PLUS_CONNECTED`
- `HEATER_ROOM`
- `HEATER_WATER`
- `HEATER_GAS`
- `HEATER_DIESEL`
- `HEATER_MIX_1`
- `HEATER_MIX_2`
- `HEATER_ELECTRICITY`
- `HEATER_HAS_ERROR`
- `TIMER_ACTIVE`
- `TIMER_ROOM`
- `TIMER_WATER`

### Climate

Les composantes climatiques favorisent la lecture et l’écriture.

```yaml
climate:
  - platform: truma_inetbox
    name: "Truma Room"
    type: ROOM
  - platform: truma_inetbox
    name: "Truma Water"
    type: WATER
```

Ce qui suit `type` les valeurs sont disponibles :

- `ROOM`
- `WATER`

### Number

Les composants numériques prennent en charge la lecture et l’écriture.

```yaml
number:
  - platform: truma_inetbox
    name: "Target Room Temperature"
    type: TARGET_ROOM_TEMPERATURE
```

Ce qui suit `type` les valeurs sont disponibles :

- `TARGET_ROOM_TEMPERATURE`
- `TARGET_WATER_TEMPERATURE`
- `ELECTRIC_POWER_LEVEL`
- `AIRCON_MANUAL_TEMPERATURE`

### Select

Certains composants prennent en charge la lecture et l'écriture.

```yaml
select:
  - platform: truma_inetbox
    name: "Fan Mode"
    type: HEATER_FAN_MODE_COMBI
```

Ce qui suit `type` les valeurs sont disponibles :

- `HEATER_FAN_MODE_COMBI`
- `HEATER_FAN_MODE_VARIO_HEAT`
- `HEATER_ENERGY_MIX_GAS`
- `HEATER_ENERGY_MIX_DIESEL`

### Sensor

Les capteurs sont en lecture seule.

```yaml
sensor:
  - platform: truma_inetbox
    name: "Current Room Temperature"
    type: CURRENT_ROOM_TEMPERATURE
```

Ce qui suit `type` les valeurs sont disponibles :

- `CURRENT_ROOM_TEMPERATURE`
- `CURRENT_WATER_TEMPERATURE`
- `TARGET_ROOM_TEMPERATURE`
- `TARGET_WATER_TEMPERATURE`
- `HEATING_MODE`
- `ELECTRIC_POWER_LEVEL`
- `ENERGY_MIX`
- `OPERATING_STATUS`
- `HEATER_ERROR_CODE`

### Actions

Les [actions ESPHome](https://esphome.io/guides/automations.html#actions) suivantes sont disponibles :

- `truma_inetbox.heater.set_target_room_temperature`
  - `temperature` - Température entre 5 °C et 30 °C. En dessous de 5 °C, le chauffage est désactivé.
  - `heating_mode` - Optionnel : définir le mode de chauffage : `"OFF"`, `ECO`, `HIGH`, `BOOST`.
- `truma_inetbox.heater.set_target_water_temperature`
  - `temperature` - Température de l'eau sous forme de nombre : `0`, `40`, `60`, `80`.
- `truma_inetbox.heater.set_target_water_temperature_enum`
  - `temperature` - Température de l'eau sous forme de texte : `"OFF"`, `ECO`, `HIGH`, `BOOST`.
- `truma_inetbox.heater.set_electric_power_level`
  - `watt` - Définir le niveau électrique : `0`, `900`, `1800`.
- `truma_inetbox.heater.set_energy_mix`
  - `energy_mix` - Définir le mix énergétique : `GAS`, `MIX`, `ELECTRICITY`.
  - `watt` - Optionnel : définir le niveau électrique : `0`, `900`, `1800`.
- `truma_inetbox.aircon.manual.set_target_temperature`
  - `temperature` - Température entre 16 °C et 31 °C. En dessous de 16 °C, la climatisation est désactivée.
- `truma_inetbox.timer.disable` - Désactiver la configuration du minuteur.
- `truma_inetbox.timer.activate` - Définir une nouvelle configuration du minuteur.
  - `start` - Heure de début.
  - `stop` - Heure de fin.
  - `room_temperature` - Température entre 5 °C et 30 °C.
  - `heating_mode` - Optionnel : mode de chauffage : `"OFF"`, `ECO`, `HIGH`, `BOOST`.
  - `water_temperature` - Optionnel : température de l'eau sous forme de nombre : `0`, `40`, `60`, `80`.
  - `energy_mix` - Optionnel : mix énergétique : `GAS`, `MIX`, `ELECTRICITY`.
  - `watt` - Optionnel : niveau électrique : `0`, `900`, `1800`.
- `truma_inetbox.clock.set` - Mettre à jour le CP Plus depuis l'ESP. Une autre [source de temps](https://esphome.io/#time-components) doit être configurée, par exemple Home Assistant Time, GPS ou DS1307 RTC.

## Commentaires et tests

Quiconque essaie ce composant recevra des commentaires !

Veuillez tester avec votre propre configuration et faites-nous savoir comment cela fonctionne — si tout fonctionne correctement ou si vous rencontrez des problèmes. Simplement un problème sur [GitHub](https://github.com/havanti/esphome-truma/issues) ouvrir avec les résultats, les rapports de bugs ou les suggestions d'amélioration. Chaque rapport contribue à rendre ce projet meilleur pour tout le monde.

---

## Marque

TRUMA est une marque déposée de Truma Gerätetechnik GmbH & Co. KG, basée à Putzbrunn. Ce projet est une initiative open source indépendante et communautaire et n'a aucune affiliation, n'est ni recommandé ni soutenu par Truma Gerätetechnik GmbH & Co. KG. L'utilisation du nom „Truma" dans ce référentiel est uniquement à des fins d'identification technique et de description de compatibilité.

## Avis de non-responsabilité

L'utilisation de ce projet est entièrement volontaire et à vos propres risques.

Ce logiciel est fourni „tel quel" sans aucune garantie, expresse ou implicite. L'auteur(s) n'assume aucune responsabilité pour tout dommage causé aux personnes, aux biens, aux véhicules, aux équipements de chauffage ou à d'autres actifs résultant de l'utilisation, de la mauvaise utilisation ou de la non-utilisabilité de ce logiciel ou des configurations décrites ici. Cela inclut, sans toutefois s'y limiter, les dommages résultant d'une configuration défectueuse, d'un comportement inattendu de l'appareil, d'erreurs logicielles ou de pannes matérielles.

Avant d’utiliser toute automatisation qui contrôle un chauffage au gaz ou au diesel, il est important de s’assurer que l’appareil en question est parfaitement compris et que toutes les règles de sécurité applicables sont respectées. Testez toujours les nouvelles configurations sous supervision.