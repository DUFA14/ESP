#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WebSocketsServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <EEPROM.h>

const char* defaultAPSSID = "ESP8266_Config";
const char* defaultAPPassword = "12345678";
const char* FIRMWARE_VERSION = "1";
const int buttonPin = D0;  // Пин кнопки

ESP8266WebServer server(80);
WebSocketsServer webSocket(81); 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.cloudflare.com", 0, 60000);
IPAddress dns(1, 1, 1, 1);

struct Settings {
  char ssid[128];
  char password[128];
IPAddress local_ip;
IPAddress gateway;
IPAddress subnet;
int timezoneOffset;
} settings;


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_TEXT) {
        // Обработка текста от клиента
        Serial.printf("Received text: %s\n", payload);
    }
}

void sendToConsole(String message) {
    webSocket.broadcastTXT(message);
}


//Функция для получения времени
String getCurrentTime() {
  timeClient.setTimeOffset(settings.timezoneOffset);  // Устанавливаем смещение часового пояса
  timeClient.update();  // Обновляем время из NTP
  return timeClient.getFormattedTime();  // Возвращаем форматированное время
}

// Функция для очистки EEPROM
void clearEEPROM() {
  Serial.println("Clearing EEPROM...");
  EEPROM.begin(1024);
  for (int i = 0; i < 1024; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.end();
  Serial.println("EEPROM cleared.");
}
// Функция для загрузки настроек из EEPROM
void loadSettings() {
  Serial.println("Loading settings from EEPROM...");
  EEPROM.begin(1024);
  EEPROM.get(0, settings);
  EEPROM.end();
  Serial.println("Settings loaded:");

}

// Функция для сохранения настроек в EEPROM
void saveSettings() {
  Serial.println("Saving settings to EEPROM...");
  EEPROM.begin(1024);
  EEPROM.put(0, settings);
  EEPROM.end();
  Serial.println("Settings saved.");
}




// Функция для сохранения настроек
void handleSaveSettings() {
    Serial.println("Saving settings from web interface...");

    // Используем статические массивы вместо String
    char ssid[128];
    char password[128];
    char local_ip_str[128];
    char gateway_str[128];
    char subnet_str[128];
    
    server.arg("ssid").toCharArray(ssid, sizeof(ssid));
    server.arg("password").toCharArray(password, sizeof(password));
    server.arg("local_ip").toCharArray(local_ip_str, sizeof(local_ip_str));
    server.arg("gateway").toCharArray(gateway_str, sizeof(gateway_str));
    server.arg("subnet").toCharArray(subnet_str, sizeof(subnet_str));

    // Проверка длины строк
    if (strlen(ssid) >= sizeof(settings.ssid)) {
        server.send(400, "text/plain", "SSID слишком длинный!");
        return;
    }

    if (strlen(password) >= sizeof(settings.password)) {
        server.send(400, "text/plain", "Пароль слишком длинный!");
        return;
    }
    
    // Копируем строки в структуру настроек
    strncpy(settings.ssid, ssid, sizeof(settings.ssid));
    strncpy(settings.password, password, sizeof(settings.password));

    // Проверка и преобразование IP-адресов
    if (!settings.local_ip.fromString(local_ip_str) || 
        !settings.gateway.fromString(gateway_str) || 
        !settings.subnet.fromString(subnet_str)) {
        server.send(400, "text/plain", "Неверный IP адрес!");
        return;
    }

    // Сохранение смещения времени
    settings.timezoneOffset = server.arg("timezone").toFloat() * 3600;

    // Сохранение настроек и перезагрузка
    saveSettings();

    server.send(200, "text/html", "<html><body><h2>Настройки сохранены! Перезагрузка...</h2></body> </html>" "<meta charset='UTF-8'>");
    delay(2000);
    Serial.println("Disconnecting from WiFi and applying new settings...");
    WiFi.disconnect();
    WiFi.config(settings.local_ip, settings.gateway, settings.subnet, dns);
    WiFi.begin(settings.ssid, settings.password);

    delay(1000);
    ESP.restart();
}

    

// OTA обновление по URL
void handleOTAUpdate() {
  String url = server.arg("url");
  Serial.println("Starting OTA update with URL: " + url);
  if (url.length() > 0) {
    WiFiClient client;
    t_httpUpdate_return ret = ESPhttpUpdate.update(client, url);
     
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.println("OTA Update Failed: " + String(ESPhttpUpdate.getLastErrorString()));
        server.send(500, "text/plain", "error: "  + String(ESPhttpUpdate.getLastErrorString()));
        break;
      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("OTA Update: No updates available");
        server.send(500, "text/html","<html><body><h2>Нет доступных обновлений</h2></body> </html>" "<meta charset='UTF-8'>");
        break;
      case HTTP_UPDATE_OK:
        Serial.println("OTA Update: Success!");
        server.send(200, "text/html", "<html><body><h2>Обновление успешно завершено! Перезагрузка...</h2></body> </html>" "<meta charset='UTF-8'>");
        break;
    }              
  } else {
    Serial.println("OTA Update: URL not provided");
    server.send(400, "text/plain", "Требуется URL!" "<meta charset='UTF-8'>");
  }
}

// Обработка загрузки файла прошивки
void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.setDebugOutput(true);
    Serial.printf("Upload: START, filename: %s\n", upload.filename.c_str());
    uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    if (!Update.begin(maxSketchSpace)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Serial.printf("Upload: WRITE, Bytes: %u\n", upload.currentSize);
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Serial.printf("Upload: END, Size: %u\nПерезагрузка...\n", upload.totalSize);
      server.send(200, "text/html", "<html><body><h2>Обновление завершено. </h2></body> </html>" "<meta charset='UTF-8'>");
      delay(2000);
      ESP.restart();  
    } else {
      Update.printError(Serial);
      server.send(500, "text/html", "<html><body><h2>Ошибка при завершении обновления. </h2></body> </html>" "<meta charset='UTF-8'>");
    }
  }
}


// Обр врем
void handleGetTime() {
  server.send(200, "text/plain", getCurrentTime());
}




void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  loadSettings();  // Загрузка настроек из EEPROM

    // Загрузка веб сокета
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket server started");


  // Если SSID не задан, то запускаем точку доступа
  if (String(settings.ssid) == "") {
    Serial.println("SSID not found, starting Access Point...");
    WiFi.softAP(defaultAPSSID, defaultAPPassword);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  } else {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(settings.ssid, settings.password);
    WiFi.config(settings.local_ip, settings.gateway, settings.subnet, dns);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  server.on("/", handleRoot);
  server.on("/otaUpdatePage", handleUpdatePage);
  server.on("/settings", handleSettingsPage);
  server.on("/saveSettings", handleSaveSettings);
  server.on("/update", HTTP_POST, [](){}, handleFileUpload);
  server.on("/otaUpdate", handleOTAUpdate);
  server.on("/info", handleInfoPage);
  server.on("/console", handleConsolePage);
  server.on("/reboot", handleRebootPage);
  server.on("/getTime", handleGetTime);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {

  
  // Проверка состояния кнопки
  if (digitalRead(buttonPin) == LOW) {  
    delay(200);  // Антидребезг
    if (digitalRead(buttonPin) == HIGH) {  
      clearEEPROM();  // Очистка EEPROM
      ESP.restart();  // Перезагрузка после очистки EEPROM
    }
  }
   
  if (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  server.handleClient();
}
