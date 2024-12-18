// Load Wi-Fi library
#include <WiFi.h>
#include <SD.h>

// Replace with your network credentials
const char* ssid = "AulaMultiversaEIE";
const char* password = "multiversa22";
// const char* ssid = "Gabriel";
// const char* password = "hola.1234";
// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;
#define CS_PIN 5  // Asegúrate de conectar este pin al CS del adaptador
#define LED 21 //PIN PARA EL LED
#define LED1 4 //LED PARA ERRORES
#define BOTON 22 //Pin para el boton
// #define MAX_BUFFER_SIZE 25024  // Tamaño máximo del búfer

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// byte fileBuffer[MAX_BUFFER_SIZE];
// size_t bufferSize = 0;
String fileName = ""; // Variable para almacenar el nombre del archivo detectado

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(BOTON, INPUT_PULLUP);
 
   if (!SD.begin(CS_PIN)) {
    Serial.println("Error al inicializar la tarjeta SD.");
    return;
  }
  Serial.println("Tarjeta SD inicializada correctamente.");

  // Detectar el primer archivo válido en la SD
  File root = SD.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("Error al abrir el directorio raíz.");
    return;
  }

  while (true) { // Iterar hasta encontrar un archivo válido
    File file = root.openNextFile();
    if (!file) {
      Serial.println("No se encontraron archivos válidos en la tarjeta SD.");
      break;
    }

    // Filtrar directorios y archivos del sistema
    if (file.isDirectory()) {
      Serial.print("Directorio ignorado: ");
      Serial.println(file.name());
      continue;
    }

    if (String(file.name()) == "System Volume Information") {
      Serial.print("Archivo del sistema ignorado: ");
      Serial.println(file.name());
      continue;
    }

    // Si es un archivo válido
    fileName = "/" + String(file.name());
    Serial.print("Archivo válido detectado: ");
    Serial.println(fileName);

    break; // Salir del bucle después de encontrar el primer archivo válido
  }


  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  digitalWrite(LED1,HIGH);
}

bool Detectar = false;
bool Presion = false;

void loop() {
 
  if (Detectar == false){
    Presion = !digitalRead(BOTON);
    if (Presion == false){
      Detectar = false;
    } else{
      Detectar = true;
    }
  }else {
    Presion = true;
  }
  
  if (Presion == true){
    digitalWrite(LED,HIGH);
    handle_client();
  }
}

void handle_client () {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          if (currentLine.length() == 0) {
            // HTTP headers
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/octet-stream");
            client.println("Content-Disposition: attachment; filename=\"file.bin\"");
            client.println("Connection: close");
            client.println();

            // Send binary file content
            sendBinaryFile(client);

            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    header = "";  // Clear the header variable
    client.stop();  // Close the connection
    Serial.println("Client disconnected.");
    Serial.println("");
  }

}

void sendBinaryFile(WiFiClient& client) {
    File file = SD.open(fileName, FILE_READ); // Abre el archivo para lectura
    if (!file) {
        Serial.println("Error al abrir el archivo.");
        return;
    }

    const size_t CHUNK_SIZE = 1024; // Tamaño de bloque
    byte buffer[CHUNK_SIZE];

    // Leer y enviar por bloques
    while (file.available()) {
        size_t bytesRead = file.read(buffer, CHUNK_SIZE); // Leer bloque
        client.write(buffer, bytesRead);                 // Enviar bloque
    }

    file.close(); // Cierra el archivo al terminar
    Serial.println("Archivo enviado exitosamente.");
}
