#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>

#define SS_PIN 10
#define RST_PIN 9

const int triggerPin = 8;
const int pinRelais = 7;

int state = 0;
bool lockOpen = false;  // Nouvelle variable pour suivre l'état de la serrure

MFRC522 rfid(SS_PIN, RST_PIN);
byte authorizedUID[][2] = { {0x63, 0x44, 0x8E, 0x97},
                            {0x13, 0xEE, 0x6D, 0x97} };

void setup()
{
  Serial.begin(9600);
  pinMode(triggerPin, OUTPUT);
  pinMode(pinRelais, OUTPUT);
  digitalWrite(triggerPin, LOW);
  digitalWrite(pinRelais, LOW);

  SPI.begin();
  rfid.PCD_Init();
}

void loop()
{
  if (!rfid.PICC_IsNewCardPresent())
    return;

  if (!rfid.PICC_ReadCardSerial())
    return;

  byte nuidPICC[4];
  for (byte i = 0; i < 4; i++)
  {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }

  Serial.println("Un badge est détecté");
  Serial.println(" L'UID du tag est:");
  for (byte i = 0; i < 4; i++)
  {
    Serial.print(nuidPICC[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  bool accessGranted = false;
  for (int i = 0; i < sizeof(authorizedUID) / sizeof(authorizedUID[0]); i++)
  {
    if (compareUID(nuidPICC, authorizedUID[i]))
    {
      accessGranted = true;
      break;
    }
  }

  if (accessGranted)
  {
    Serial.println("Accès autorisé");
    digitalWrite(triggerPin, HIGH);
    delay(100);  // Attendre 100 millisecondes
    digitalWrite(triggerPin, LOW);
    state = digitalRead(triggerPin);
    digitalWrite(pinRelais, HIGH);
    lockOpen = true;  // La serrure est ouverte
    Serial.println(state);
  }
  else
  {
    Serial.println("Accès refusé");
    digitalWrite(triggerPin, LOW);
    digitalWrite(pinRelais, LOW);
    state = digitalRead(triggerPin);
    lockOpen = false;  // La serrure est fermée
    Serial.println(state);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  // Ajouter la logique pour refermer la serrure après un certain temps
  if (lockOpen)
  {
    delay(15000);  // Attendre 5 secondes (ajustez selon vos besoins)
    digitalWrite(pinRelais, LOW);
    lockOpen = false;  // La serrure est maintenant fermée
  }
}

bool compareUID(byte uid1[], byte uid2[])
{
  for (int i = 0; i < 4; i++)
  {
    if (uid1[i] != uid2[i])
    {
      return false;
    }
  }
  return true;
}
