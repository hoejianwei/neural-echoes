/*
 * Arduino Uno R4 WiFi - Dual UDP Sender
 * Fixed IP Address Syntax
 */

#include <SPI.h>
#include <MFRC522.h>
#include <WiFiS3.h>
#include <WiFiUdp.h>

// --- WIFI CONFIGURATION ---
const char ssid[] = "TP-Link_E41B";      
const char pass[] = "03876794";   

// --- UDP CONFIGURATION ---
// IMPORTANT: Use COMMAS (,) not DOTS (.) between numbers
IPAddress remoteIp(192, 168, 0, 107); 

// Define the two destination ports
unsigned int qlabPort = 53000;      // QLab listens here
unsigned int serverPort = 4000;     // Your Python script listens here

// Local port for the Arduino
unsigned int localPort = 8888;      

// --- HARDWARE CONFIGURATION ---
#define SS_PIN  10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);
WiFiUDP Udp;

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  // 1. Initialize Hardware
  SPI.begin();
  mfrc522.PCD_Init();

  // 2. Connect to WiFi
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected to network");
  Serial.print("My IP address: ");
  Serial.println(WiFi.localIP());

  // 3. Start UDP Engine
  Udp.begin(localPort); 
  
  Serial.println("System Ready. Scan a card...");
}

void loop() {
  // --- RFID DETECTION ---
  if ( ! mfrc522.PICC_IsNewCardPresent()) { return; }
  if ( ! mfrc522.PICC_ReadCardSerial()) { return; }

  // --- CARD FOUND ---
  Serial.println("Card Detected! Firing triggers...");

  // --- 1. SEND TO QLAB (Port 53000) ---
  Udp.beginPacket(remoteIp, qlabPort);
  Udp.write("/cue/NOTED/start");
  Udp.endPacket();
  Serial.println(" -> Sent OSC to QLab (53000)");

  // --- 2. SEND TO SERVER (Port 4000) ---
  Udp.beginPacket(remoteIp, serverPort);
  Udp.write("4");
  Udp.endPacket();
  Serial.println(" -> Sent '4' to Server (4000)");

  // --- RESET RFID STATE ---
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  // Debounce
  delay(300);
}