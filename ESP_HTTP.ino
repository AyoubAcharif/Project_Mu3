#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// Informations de connexion Wi-Fi
const char* ssid = "xxx";
const char* password = "xxx";

// Déclaration du serveur HTTP
ESP8266WebServer server(80);

// Déclaration des pins
const int triggerPin = 5; // Pin utilisée pour détecter l'état (trigger)
const int pin4 = 4;       // Pin à contrôler (la même que triggerPin pour simplification, sinon utilisez une autre pin)

// Informations du serveur Node-RED
const char* nodeRedServer = "192.168.129.4";
const int nodeRedPort = 1880;
const String endpoint = "/updatevalue";

// État précédent de la pin pour détection de changement
int previousPinState = -1; 

void setup() {
  // Initialisation de la communication série
  Serial.begin(115200);

  // Configuration de la pin 5 comme sortie
  pinMode(pin4, OUTPUT);
  digitalWrite(pin4, LOW); // Initialisation de la pin à l'état bas

  // Configuration Wi-Fi avec IP statique
  IPAddress ip(192, 168, 129, 65);
  IPAddress dns(192, 168, 128, 1);
  IPAddress gateway(192, 168, 128, 1);
  IPAddress subnet(255, 255, 254, 0);
  WiFi.config(ip, dns, gateway, subnet);

  // Connexion au réseau Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connexion au Wi-Fi...");
  }
  Serial.println("Connecté au réseau Wi-Fi");
  Serial.println(WiFi.localIP());

  // Définir la route pour changer l'état de la pin via HTTP POST
  server.on("/set_pin", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      // Parse JSON from the request body
      String body = server.arg("plain");
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, body);

      // Extract the "value" field
      int value = doc["value"].as<int>();
      Serial.println(value);

      // Control the pin based on the value
      if (value == 1) {
        digitalWrite(pin4, HIGH);  // Met la pin 5 à l'état haut
        server.send(200, "text/plain", "Pin 5 HIGH");
        delay(1000);
        digitalWrite(pin4, LOW); 
        
      } else {
        digitalWrite(pin4, LOW);  // Met la pin 5 à l'état bas
        server.send(200, "text/plain", "Pin 5 LOW");
      }
    } else {
      server.send(400, "text/plain", "Bad Request");
    }
  });

  // Démarre le serveur
  server.begin();
  Serial.println("Serveur HTTP démarré");
}

void loop() {
  // Traiter les requêtes des clients HTTP
  server.handleClient();

  // Lire l'état de la pin de déclenchement
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
    Serial.println(postData);

    // Envoyer la requête HTTP POST
    http.POST(postData);

    // Libérer les ressources de la connexion HTTP
    http.end();

    // Mettre à jour l'état précédent
    previousPinState = pinState;
  }
}
