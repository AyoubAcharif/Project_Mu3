#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char *ssid = "AMN-TL";
const char *password = "Yaman12345";
const int triggerPin = 5;                     // Remplacez par le numéro de la broche que vous utilisez
const char *nodeRedServer = "192.168.129.5";  // Adresse IP de votre Raspberry Pi
const int nodeRedPort = 1880;                 // Port utilisé par Node-RED
const String endpoint = "/updatevalue";

int previousPinState = -1; // Initialisez à une valeur différente de 0 et 1

void setup() {
  Serial.begin(115200);

  // Connexion au réseau Wi-Fi
  Serial.println();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }
  Serial.println("");
  Serial.println("Connecté au réseau WiFi");
}

void loop() {
  int pinState = digitalRead(triggerPin);

  // Si l'état actuel est différent de l'état précédent, afficher le message et mettre à jour l'état précédent
  if (pinState != previousPinState) {
    Serial.println("Changement d'état de la broche : " + String(pinState));

    // Construire l'URL du serveur Node-RED
    String url = "http://" + String(nodeRedServer) + ":" + String(nodeRedPort) + endpoint;

    // Configurer la connexion HTTP avec le client WiFi
    WiFiClient client;
    HTTPClient http;
    http.begin(client, url);

    // Ajouter les données à la requête HTTP
    http.addHeader("Content-Type", "application/json"); // Utiliser le type de contenu JSON
    String postData = "{\"value\":" + String(pinState) + "}"; // Construire la chaîne JSON

    // Envoyer la requête HTTP POST
    http.POST(postData);

    // Libérer les ressources de la connexion HTTP
    http.end();

    // Mettre à jour l'état précédent
    previousPinState = pinState;
  }

 // delay(500); // Attendre 1 seconde avant la prochaine vérification
}
