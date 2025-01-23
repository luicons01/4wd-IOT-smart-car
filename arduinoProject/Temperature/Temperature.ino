#include <ArduinoMqttClient.h>
#include <WiFiNINA.h> // Se usi una board con WiFi, rimuovilo per un simulatore
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <dht.h>

#define DHTPIN 13 // Pin del sensore DHT11
dht DHT;       // Crea un'istanza dell'oggetto DHT

WiFiClient wifiClient;  // Usa un client WiFi o Ethernet
MqttClient mqttClient(wifiClient);

LiquidCrystal_I2C lcd(0x27, 16, 2); // L'indirizzo I2C è solitamente 0x27 o 0x3F

const char broker[] = "broker.emqx.io"; //broker del laboratorio
int port = 1883;
const char topic1[] = "IOTlab/temp";
const char topic2[] = "IOTlab/humidity";
const char* ssid = "WiFi-LabIoT";
const char* password = "s1jzsjkw5b";

void setup() {
  lcd.init();                // Inizializza l'LCD
  lcd.backlight();          // Accende la retroilluminazione
  Serial.begin(9600);       // Inizializza il monitor seriale

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.println("Attempting to connect to WiFi...");
  }

  Serial.println("Connected to WiFi!");

  Serial.print("Attempting to connect to the MQTT broker on localhost: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
      Serial.println("MQTT connection failed!");
      while (1);
  }
  Serial.println("Connected to MQTT broker on localhost");

} 

void loop() {
  mqttClient.poll(); // Gestisce keep-alive con il broker

  lcd.setCursor(0, 0);
  lcd.print("Wait for new");
  lcd.setCursor(0, 1);
  lcd.print("data...");  
  lcd.setCursor(0, 1);
  //lcd.print("dati...");
  int measure = DHT.read11(DHTPIN); // Legge i dati dal DHT11
  delay(3000);
  //Serial.print("Measure value: ");
  //Serial.println(measure); // Stampa il valore di measure

  int temp = DHT.temperature; // Legge temperatura dal sensore
  int umid = DHT.humidity;    // Legge umidità dal sensore

  // Se la lettura è valida
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temp);
    lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("Umidita: ");
    lcd.print(umid);
    lcd.print(" %");

    delay(6000); // Aspetta 2 secondi prima di pulire lo schermo
    lcd.clear(); // Pulisci lo schermo

    // Stampa nel monitor seriale
    Serial.print("Publishing temperature: ");
    Serial.println(temp);
    mqttClient.beginMessage(topic1);
    mqttClient.print(temp);
    mqttClient.endMessage();

    Serial.print("Publishing humidity: ");
    Serial.println(umid);
    mqttClient.beginMessage(topic2);
    mqttClient.print(umid);
    mqttClient.endMessage();
 
  delay(2000); // Aspetta 2 secondi prima di fare una nuova lettura
}
