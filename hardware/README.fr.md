# Installation matérielle : Adaptateur LIN-Bus Truma

[🇩🇪 Deutsch](README.md) | [🇬🇧 English](README.en.md) | 🇫🇷 Français

---

## Composants nécessaires

| Composant | Modèle / Remarque |
|---|---|
| Microcontrôleur | Waveshare ESP32-S3-DEV-KIT-N8R8 |
| Transceiver LIN | Module TJA1020 (ex. FST T151) ou WomoLIN Board v2 |
| Convertisseur de tension | DC/DC 12V → 5V (min. 500 mA) |
| Câble de connexion | Prise RJ12 (6P6C) pour le port Truma |
| Fils | Couleurs différentes recommandées (voir code couleur) |

---

## Schéma de câblage

![Schéma de câblage](wiring.png)

Version imprimable : [einbauanleitung_truma_lin_adapter.pdf](einbauanleitung_truma_lin_adapter.pdf)

### Code couleur

| Couleur | Signification |
|---|---|
| Rouge | 12V Plus (+) |
| Bleu | Masse / GND (−) |
| Jaune | Alimentation 5V |
| Vert/Rose | UART TX / RX |
| Orange | Signal LIN (Truma) |

---

## Toutes les connexions en un coup d'œil

| # | De | Vers | Tension |
|---|---|---|---|
| 1 | Alimentation 12V véhicule (+) | Entrée DC/DC (+) | 12V |
| 2 | Alimentation 12V véhicule (−) | Entrée DC/DC (−) | GND |
| 3 | Sortie DC/DC (+) | Broche 5V ESP32-S3 | 5V |
| 4 | Sortie DC/DC (−) | Broche GND ESP32-S3 | GND |
| 5 | ESP32-S3 GPIO18 | TJA1020 TX | Signal 3,3V |
| 6 | TJA1020 RX | ESP32-S3 GPIO8 | Signal 3,3V |
| 7 | ESP32-S3 GND | TJA1020 GND | GND |
| 8 | Alimentation 12V véhicule (+) | Entrée 12V TJA1020 | 12V |
| 9 | TJA1020 LIN | RJ12 Pin 4 | Signal LIN |
| 10 | TJA1020 GND | RJ12 Pin 6 | GND |

> **Remarque :** Le TJA1020 nécessite du 12V directement depuis l'alimentation du
> véhicule (connexion #8) — pas depuis la sortie 5V du convertisseur DC/DC.
> Le libellé exact de l'entrée 12V varie selon le type de carte (ex. FST T151
> ou WomoLIN Board v2).

---

## Ordre d'installation

1. **Convertisseur DC/DC** — câbler l'entrée 12V et la sortie 5V
2. **ESP32-S3** — connecter à la sortie 5V du convertisseur
3. **UART TX/RX** — relier l'ESP32-S3 au TJA1020
4. **Câble RJ12** — brancher sur le port Truma (alimentation coupée)
5. **Alimentation 12V** du TJA1020 — en dernier !

---

## Avertissements importants

> **Ne jamais inverser le Plus et le Moins** — cela endommage immédiatement l'ESP32 et le TJA1020.

> **Le TJA1020 nécessite du 12V directement depuis l'alimentation du véhicule** — pas depuis le convertisseur DC/DC.

> **Effectuer tous les branchements avant de mettre sous tension.**

> **Fusible recommandé :** Fusible 1A sur le fil positif 12V, le plus près possible de la batterie.

---

## Configuration ESPHome

Les exemples de configuration correspondants sont disponibles dans le dépôt :
[github.com/havanti/esphome-truma](https://github.com/havanti/esphome-truma)

Choisir la variante appropriée pour l'ESP32-S3 :

| Chauffage | Fichier |
|---|---|
| Truma Combi 4/6 kW gaz | [`ESP32-S3_truma_4-6_Gas_example.yaml`](../ESP32-S3_truma_4-6_Gas_example.yaml) |
| Truma Combi 6 kW diesel | [`ESP32-S3_truma_6DE_Diesel_example.yaml`](../ESP32-S3_truma_6DE_Diesel_example.yaml) |

> **Version minimale :** ESPHome 2026.3.1 ou version ultérieure requise.

> **CP Plus vs. iNet Box :** Les YAMLs d'exemple utilisent `lin_checksum: VERSION_2`
> (pour CP Plus). En cas d'utilisation d'une ancienne iNet Box, passer à `VERSION_1`
> si nécessaire.
