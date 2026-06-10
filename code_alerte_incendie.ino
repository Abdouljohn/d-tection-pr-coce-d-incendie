#include <WiFi.h>
#include <HTTPClient.h>

// ================= PINS =================
#define MQ_PIN 34        // Broche analogique capteur MQ
#define LED_ROUGE 26     // LED Danger
#define LED_VERTE 27     // LED OK
#define BUZZER_PIN 25    // Broche du Buzzer

// ================= PARAMÈTRES =================
const int SEUIL_ALERTE = 700;        // Seuil d'activation de l'alerte (0 à 4095)
const long INTERVALLE_ENVOI = 15000;  // Envoi régulier toutes les 15 secondes si tout est stable

// ================= WIFI =================
const char* ssid = "youpilab_fibre 2G";
const char* password = "Washingi_loV3_yl2025Fibre#Cit1";

// ================= API YOUPILAB =================
const String BASE_URL = "https://iot.youpilab.com/api";
const String APP_ID  = "deta6b3c";
const String APP_KEY = "7998275e";

// Variables de contrôle de temps et d'état
unsigned long previousMillis = 0;
bool dernierEtatAlerte = false;

void setup() {
  Serial.begin(115200);

  // Configuration des broches
  pinMode(MQ_PIN, INPUT);
  pinMode(LED_ROUGE, OUTPUT);
  pinMode(LED_VERTE, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Tout éteindre au démarrage
  digitalWrite(LED_ROUGE, LOW);
  digitalWrite(LED_VERTE, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Connexion WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connexion au réseau WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Connecté avec succès !");
}

void loop() {
  // Lecture du capteur
  int valeurGaz = analogRead(MQ_PIN);
  
  // Vérification du seuil de danger
  bool alerteActive = (valeurGaz >= SEUIL_ALERTE);

  // Gestion des LEDs et du Buzzer
  if (alerteActive) {
    digitalWrite(LED_ROUGE, HIGH);
    digitalWrite(LED_VERTE, LOW);
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(LED_ROUGE, LOW);
    digitalWrite(LED_VERTE, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
  }

  unsigned long currentMillis = millis();
  
  // Déclenchement de l'envoi
  if (currentMillis - previousMillis >= INTERVALLE_ENVOI || alerteActive != dernierEtatAlerte) {
    previousMillis = currentMillis;
    dernierEtatAlerte = alerteActive;

    Serial.print("[INFO] Valeur MQ: ");
    Serial.print(valeurGaz);
    Serial.print(" | Danger : ");
    Serial.println(alerteActive ? "OUI 🔥" : "NON 🟢");

    // Définition du texte personnalisé selon la situation
    String texteAlerte = alerteActive ? "Alerte incendie" : "Pas d'alerte";

    // Envoi à la plateforme Youpilab avec le texte (String)
    sendData(valeurGaz, texteAlerte);
  }

  delay(500);
}

// ================= FONCTION D'ENVOI API MODIFIÉE =================
void sendData(int gaz, String alerte) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[Erreur] WiFi déconnecté.");
    return;
  }

  HTTPClient http;

  // IMPORTANT : On remplace informatiquement les espaces par "%20" 
  // pour que l'URL reste valide lors du transfert HTTP.
  String alerteEncodee = alerte;
  alerteEncodee.replace(" ", "%20");

  String url =
    BASE_URL +
    "/data/send?APP_ID=" + APP_ID +
    "&APP_KEY=" + APP_KEY +
    "&gaz=" + String(gaz) +
    "&alerte=" + alerteEncodee; // Envoi de la chaîne encodée

  http.begin(url);

  int httpCode = http.GET();

  Serial.print("[HTTP GET] Code de retour : ");
  Serial.println(httpCode);

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.print("[REPONSE YOUPILAB] : ");
    Serial.println(payload);
  } else {
    Serial.print("[HTTP] Échec de la requête, erreur : ");
    Serial.println(http.errorToString(httpCode).c_str());
  }

  http.end();
}