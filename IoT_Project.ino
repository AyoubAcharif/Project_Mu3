#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>

#define SS_PIN 10
#define RST_PIN 9
#define REMOTE_PIN 4

const int triggerPin = 8;
const int pinRelais = 7;

int state = 0;
bool lockOpen = false;  // Nouvelle variable pour suivre l'état de la serrure
bool accessPreviouslyGranted = false;  // Pour suivre l'état précédent

MFRC522 rfid(SS_PIN, RST_PIN);
byte authorizedUID[][4] = { {0x63, 0x44, 0x8E, 0x97},
                            {0x13, 0xEE, 0x6D, 0x97} };

void setup()
{
  Serial.begin(9600);
  pinMode(triggerPin, OUTPUT);
  pinMode(REMOTE_PIN, INPUT);  // Configurer REMOTE_PIN comme entrée
  pinMode(pinRelais, OUTPUT);
  digitalWrite(triggerPin, LOW);
  digitalWrite(pinRelais, LOW);

  SPI.begin();
  rfid.PCD_Init();
}

void loop()
{
  bool accessGranted = false;

  // Vérifier l'accès par carte RFID
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
  {
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

    for (int i = 0; i < sizeof(authorizedUID) / sizeof(authorizedUID[0]); i++)
    {
      if (compareUID(nuidPICC, authorizedUID[i]))
      {
        accessGranted = true;
        break;
      }
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  // Vérifier l'accès distant via REMOTE_PIN
  if (digitalRead(REMOTE_PIN) == HIGH)
  {
    Serial.println("Commande à distance reçue");
    accessGranted = true;
  }

  if (accessGranted)
  {
    if (!accessPreviouslyGranted)
    {
      Serial.println("Accès autorisé");
      digitalWrite(triggerPin, HIGH);
      digitalWrite(pinRelais, HIGH);
      lockOpen = true;  // La serrure est ouverte
      delay(30000);  // Attendre 30 secondes
      digitalWrite(triggerPin, LOW);
      digitalWrite(pinRelais, LOW);
      lockOpen = false;  // La serrure est maintenant fermée
      Serial.println("La serrure est fermée après le délai");
    }
    accessPreviouslyGranted = true;
  }
  else
  {
    if (accessPreviouslyGranted)
    {
      Serial.println("Accès refusé");
    }
    digitalWrite(triggerPin, LOW);
    digitalWrite(pinRelais, LOW);
    state = digitalRead(triggerPin);
    lockOpen = false;  // La serrure est fermée
    accessPreviouslyGranted = false;
  }

  // Attendre 1 seconde avant de vérifier à nouveau
  delay(1000);
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
