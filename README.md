# Système de Détection Précoce d'Incendie

Ce projet présente un système de détection précoce d'incendie basé sur un microcontrôleur ESP32, un capteur de gaz de la série MQ, des indicateurs visuels et sonores (LEDs, buzzer), ainsi qu'une connexion à la plateforme IoT de Youpilab pour le suivi en temps réel et la notification de l'état d'alerte.

## Table des matières
1. Fonctionnalités
2. Matériel Requis
3. Schéma de Câblage (Brochage)
4. Configuration et Déploiement
5. Fonctionnement du Code
6. API Youpilab IoT
7. Améliorations Futures

---

## 1. Fonctionnalités

- **Surveillance locale continue** : Mesure constante de la concentration de gaz ou de fumée dans l'air ambiant.
- **Double alerte visuelle et sonore** :
  - **État normal** : La LED verte s'allume, indiquant qu'aucune anomalie n'est détectée.
  - **État d'alerte** : Dès que la mesure franchit le seuil critique défini, la LED rouge s'allume et le buzzer s'active pour avertir localement d'un danger imminent.
- **Transmission intelligente des données (IoT)** :
  - Envoi périodique des mesures (toutes les 15 secondes) à l'API Youpilab pour un suivi historique stable.
  - Envoi instantané dès qu'un changement d'état d'alerte survient (passage de "Sûr" à "Danger" ou inversement) afin de garantir une réactivité maximale.
- **Optimisation réseau** : Encodage correct des paramètres de requête HTTP pour assurer la conformité des transmissions réseau.

---

## 2. Matériel Requis

- **Microcontrôleur** : ESP32 (avec support WiFi)
- **Capteur de Gaz** : Capteur MQ (par exemple, MQ-2 pour le GPL, le propane, le méthane, l'hydrogène, l'alcool, la fumée)
- **Signaux Visuels** :
  - 1x LED Verte
  - 1x LED Rouge
  - 2x Résistances d'adaptation (par exemple, 220 Ohms)
- **Signal Sonore** : 1x Buzzer actif ou passif (compatible tension de commande ESP32)
- **Divers** : Breadboard, câbles de pontage (jumpers), câble de programmation USB-C ou Micro-USB.

---

## 3. Schéma de Câblage (Brochage)

Le microcontrôleur ESP32 utilise les broches suivantes pour interfacer les différents composants :

| Composant | Broche ESP32 | Type de Signal | Rôle |
| :--- | :---: | :---: | :--- |
| **Capteur MQ** | GPIO 34 | Entrée Analogique | Mesure de la concentration de gaz/fumée (ADC1_CH6) |
| **LED Rouge** | GPIO 26 | Sortie Numérique | Indicateur d'alerte active (Danger) |
| **LED Verte** | GPIO 27 | Sortie Numérique | Indicateur d'absence d'alerte (Sécurité) |
| **Buzzer** | GPIO 25 | Sortie Numérique | Alerte sonore active |

---

## 4. Configuration et Déploiement

### Prérequis logiciels
1. Installer l'environnement de développement **Arduino IDE**.
2. Configurer le support des cartes ESP32 dans l'Arduino IDE via le gestionnaire de cartes en ajoutant l'URL correspondante dans les préférences :
   `https://dl.espressif.com/dl/package_esp32_index.json`
3. Installer le paquet de cartes **esp32** par Espressif Systems.
4. Les bibliothèques nécessaires (`WiFi.h` et `HTTPClient.h`) sont intégrées par défaut dans le noyau ESP32 pour Arduino IDE.

### Configuration du code
Avant d'injecter le programme sur l'ESP32, modifiez les paramètres de connexion et d'API dans le fichier `code.ino` :

```cpp
// Paramètres WiFi
const char* ssid = "VOTRE_NOM_DE_RESEAU_WIFI";
const char* password = "VOTRE_MOT_DE_PASSE_WIFI";

// Paramètres de l'API Youpilab
const String APP_ID  = "VOTRE_APP_ID";
const String APP_KEY = "VOTRE_APP_KEY";
```

Vous pouvez également modifier le seuil de déclenchement de l'alerte (valeur de 0 à 4095) selon la sensibilité souhaitée de votre capteur MQ :
```cpp
const int SEUIL_ALERTE = 700; // Ajuster selon les conditions environnementales
```

---

## 5. Fonctionnement du Code

Le script s'articule autour de trois sections principales :

### Configuration initiale (`setup`)
- Initialisation du port série à une vitesse de 115200 bauds pour le débogage.
- Définition des modes de broches (Entrées / Sorties).
- Extinction globale de tous les actionneurs (LEDs et buzzer) par sécurité au démarrage.
- Connexion réseau via la fonction `WiFi.begin()`. Le système attend que la connexion soit établie avant de poursuivre.

### Boucle principale (`loop`)
- Acquisition de la valeur analogique lue sur la broche du capteur MQ.
- Comparaison de cette valeur avec le seuil `SEUIL_ALERTE` pour déterminer l'état d'alerte.
- Activation immédiate des actionneurs matériels (allumage de la LED Rouge et du Buzzer si alerte, sinon allumage de la LED Verte).
- Analyse temporelle et événementielle : les données sont envoyées à la plateforme Youpilab si l'un des critères est satisfait :
  1. La durée écoulée depuis le dernier envoi dépasse `INTERVALLE_ENVOI` (fixé à 15 secondes).
  2. Un changement brusque de l'état d'alerte s'est produit depuis le dernier cycle (permet de signaler un départ d'incendie sans attendre le délai de 15 secondes).

### Envoi des données (`sendData`)
- Vérification préalable de l'état de la connexion WiFi.
- Encodage URL de la chaîne de texte décrivant l'alerte (remplacement des espaces par `%20`).
- Construction d'une requête HTTP GET vers l'API Youpilab contenant l'identifiant de l'application, la clé d'accès, la valeur mesurée du capteur et le statut encodé.
- Envoi de la requête et traitement de la réponse retournée par le serveur pour confirmation dans la console série.

---

## 6. API Youpilab IoT

La communication vers la plateforme s'effectue via l'adresse suivante :
`https://iot.youpilab.com/api/data/send`

Le service attend quatre paramètres clés par requête GET :
- `APP_ID` : L'identifiant de l'application cliente sur Youpilab.
- `APP_KEY` : La clé d'authentification associée.
- `gaz` : La valeur entière analogique lue (0-4095).
- `alerte` : Le texte d'état (par exemple, "Alerte%20incendie" ou "Pas%20d'alerte").

---

## 7. Améliorations Futures

Pour étendre les capacités de cette solution, les pistes suivantes peuvent être envisagées :
- **Intégration d'un capteur de température** : Ajouter un capteur de type DHT22 ou DS18B20 pour corréler la détection de gaz avec une augmentation rapide de la température.
- **Notifications multi-canaux** : Configurer des notifications instantanées complémentaires par email ou vers une application de messagerie externe (comme Telegram ou WhatsApp).
- **Mode économie d'énergie** : Implémenter l'utilisation du mode d'endormissement profond (Deep Sleep) pour un fonctionnement sur batterie, tout en réveillant l'ESP32 par interruption sur franchissement de seuil analogique.
- **Boîtier de protection** : Concevoir et imprimer en 3D un boîtier hermétique laissant passer l'air pour protéger les composants électroniques de la poussière et de l'humidité.
