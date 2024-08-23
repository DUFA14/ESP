#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <EEPROM.h>
const char* hostname = "12345678";
const char* defaultAPSSID = "ESP8266_Config";
const char* defaultAPPassword = "12345678";
const char* FIRMWARE_VERSION = "1";
const int buttonPin = D0;  // Пин кнопки
ESP8266WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp1.ntp-servers.net", 3600, 60000);

struct Settings {
  char ssid[64];
  char password[64];
IPAddress local_ip;
IPAddress gateway;
IPAddress subnet;
int timezoneOffset;
} settings;

//Функция для получения времени
String getCurrentTime() {
  timeClient.setTimeOffset(settings.timezoneOffset);  // Устанавливаем смещение часового пояса
  timeClient.update();  // Обновляем время из NTP
  return timeClient.getFormattedTime();  // Возвращаем форматированное время
}

// Функция для очистки EEPROM
void clearEEPROM() {
  Serial.println("Clearing EEPROM...");
  EEPROM.begin(512);
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.end();
  Serial.println("EEPROM cleared.");
}
// Функция для загрузки настроек из EEPROM
void loadSettings() {
  Serial.println("Loading settings from EEPROM...");
  EEPROM.begin(512);
  EEPROM.get(0, settings);
  EEPROM.end();
  Serial.println("Settings loaded:");

}

// Функция для сохранения настроек в EEPROM
void saveSettings() {
  Serial.println("Saving settings to EEPROM...");
  EEPROM.begin(512);
  EEPROM.put(0, settings);
  EEPROM.end();
  Serial.println("Settings saved.");
}

// Главная страница
void handleRoot() {
  Serial.println("Serving main menu...");
  String html = "<html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<style>";
  html += "body { text-align: center; background-color: #f0f0f0; color: #333; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; }";
  html += ".container { padding: 20px; max-width: 400px; margin: auto; background-color: #fff; border-radius: 8px; box-shadow: 0px 0px 15px rgba(0, 0, 0, 0.1); }";
  html += "h2 { color: #0056b3; }";
  html += "button { width: 100%; padding: 10px; margin: 10px 0; font-size: 18px; border: none; color: white; cursor: pointer; border-radius: 4px; }";
  html += "button.update { background-color: #28a745; }";
  html += "button.settings { background-color: #17a2b8; }";
  html += "button.info { background-color: #ffc107; color: #333; }";
  html += "button.reboot { background-color: #dc3545; }";
  html += "</style>";
 html += "<script>";
  
  // JavaScript для обновления времени
  html += "function updateTime() {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (xhr.readyState == 4 && xhr.status == 200) {";
  html += "      document.getElementById('time').innerHTML = xhr.responseText;";
  html += "    }";
  html += "  };";
  html += "  xhr.open('GET', '/getTime', true);";
  html += "  xhr.send();";
  html += "}";
  html += "setInterval(updateTime, 1000);";  // Обновление каждую секунду
  html += "</script>";
  html += "</head><body>";

  // Добавляем текущую дату и время в шапку страницы с id для обновления
  html += "<div class='time'>Текущее время: <span id='time'>" + getCurrentTime() + "</span></div>";

  html += "<div class='container'>";
  html += "<h2>Главное меню</h2>";
  html += "<button class='update' onclick='location.href=\"/otaUpdatePage\"'>Обновление прошивки</button>";
  html += "<button class='settings' onclick='location.href=\"/settings\"'>Настройки</button>";
  html += "<button class='info' onclick='location.href=\"/info\"'>Информация</button>";
  html += "<button class='reboot' onclick='location.href=\"/reboot\"'>Перезагрузка</button>";
  html += "</div>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// Страница обновления прошивки
void handleUpdatePage() {
  Serial.println("Serving OTA update page...");
  String html = "<html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<style>";
  html += "body { text-align: center; background-color: #f0f0f0; color: #333; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; }";
  html += ".container { padding: 20px; max-width: 400px; margin: auto; background-color: #fff; border-radius: 8px; box-shadow: 0px 0px 15px rgba(0, 0, 0, 0.1); }";
  html += "h2 { color: #0056b3; }";
  html += "input, button { width: 100%; padding: 10px; margin: 10px 0; font-size: 16px; border-radius: 4px; border: 1px solid #ccc; }";
  html += "button { background-color: #28a745; color: white; border: none; cursor: pointer; }";
  html += "button:hover { background-color: #218838; }";
  html += "</style>";
 html += "<script>";
  
  // JavaScript для обновления времени
  html += "function updateTime() {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (xhr.readyState == 4 && xhr.status == 200) {";
  html += "      document.getElementById('time').innerHTML = xhr.responseText;";
  html += "    }";
  html += "  };";
  html += "  xhr.open('GET', '/getTime', true);";
  html += "  xhr.send();";
  html += "}";
  html += "setInterval(updateTime, 1000);";  // Обновление каждую секунду
  html += "</script>";
  html += "</head><body>";

  // Добавляем текущую дату и время в шапку страницы с id для обновления
  html += "<div class='time'>Текущее время: <span id='time'>" + getCurrentTime() + "</span></div>";
 
  html += "<div class='container'>";
  html += "<h2>Обновление через веб-сервер</h2>";
  html += "<form action=\"/otaUpdate\" method=\"get\">";
  html += "Ссылка OTA:<br><input type=\"text\" name=\"url\" value=\"http://example.com/firmware.bin\"><br>";
  html += "<button type=\"submit\">Начать обновление</button>";
  html += "</form>";
  html += "<h2>Обновление путем загрузки файла</h2>";
  html += "<form method=\"POST\" action=\"/update\" enctype=\"multipart/form-data\">";
  html += "<input type=\"file\" name=\"update\"><br>";
  html += "<button type=\"submit\">Начать обновление</button>";
  html += "</form>";
  html += "<br><button onclick='location.href=\"/\"'>Главное меню</button>";
  html += "</div>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

// Страница настроек
void handleSettingsPage() {
    Serial.println("Serving settings page...");

    // Создание буферов для строк IP-адресов
    char local_ip[32];
    char gateway[32];
    char subnet[32];
    char timezone[8];  // Буфер для смещения времени

    // Преобразование IP-адресов в строки
    snprintf(local_ip, sizeof(local_ip), "%d.%d.%d.%d", settings.local_ip[0], settings.local_ip[1], settings.local_ip[2], settings.local_ip[3]);
    snprintf(gateway, sizeof(gateway), "%d.%d.%d.%d", settings.gateway[0], settings.gateway[1], settings.gateway[2], settings.gateway[3]);
    snprintf(subnet, sizeof(subnet), "%d.%d.%d.%d", settings.subnet[0], settings.subnet[1], settings.subnet[2], settings.subnet[3]);
    snprintf(timezone, sizeof(timezone), "%d", settings.timezoneOffset / 3600);
 
     String html = "<html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<style>";
  html += "body { text-align: center; background-color: #f0f0f0; color: #333; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; }";
  html += ".container { padding: 20px; max-width: 400px; margin: auto; background-color: #fff; border-radius: 8px; box-shadow: 0px 0px 15px rgba(0, 0, 0, 0.1); }";
  html += "h2 { color: #0056b3; }";
  html += "input, button { width: 100%; padding: 10px; margin: 10px 0; font-size: 16px; border-radius: 4px; border: 1px solid #ccc; }";
  html += "button { background-color: #17a2b8; color: white; border: none; cursor: pointer; }";
  html += "button:hover { background-color: #138496; }";
  html += "</style>";
 html += "<script>";
  
  // JavaScript для обновления времени
  html += "function updateTime() {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (xhr.readyState == 4 && xhr.status == 200) {";
  html += "      document.getElementById('time').innerHTML = xhr.responseText;";
  html += "    }";
  html += "  };";
  html += "  xhr.open('GET', '/getTime', true);";
  html += "  xhr.send();";
  html += "}";
  html += "setInterval(updateTime, 1000);";  // Обновление каждую секунду
  html += "</script>";
  html += "</head><body>";

  // Добавляем текущую дату и время в шапку страницы с id для обновления
  html += "<div class='time'>Текущее время: <span id='time'>" + getCurrentTime() + "</span></div>";

  html += "<div class='container'>";
  html += "<h2>Настройки WiFi и Сети</h2>";
  html += "<form action=\"/saveSettings\" method=\"post\">";
    html += "SSID:<br><input type=\"text\" name=\"ssid\" value=\"" + String(settings.ssid) + "\"><br>";
    html += "Пароль:<br><input type=\"password\" name=\"password\" value=\"" + String(settings.password) + "\"><br>";
    html += "IP Адрес:<br><input type=\"text\" name=\"local_ip\" value=\"" + String(local_ip) + "\"><br>";
    html += "Шлюз:<br><input type=\"text\" name=\"gateway\" value=\"" + String(gateway) + "\"><br>";
    html += "Маска подсети:<br><input type=\"text\" name=\"subnet\" value=\"" + String(subnet) + "\"><br>";
    html += "Смещение часового пояса (в часах):<br><input type=\"number\" step=\"0.5\" name=\"timezone\" value=\"" + String(timezone)+ "\"><br>";
    html += "<button type=\"submit\">Сохранить настройки</button>";
    html += "</form>";
    html += "<br><button onclick='location.href=\"/\"'>Главное меню</button>";
    html += "</div>";
    html += "</body></html>";

    server.send(200, "text/html", html);
}


// Функция для сохранения настроек
void handleSaveSettings() {
    Serial.println("Saving settings from web interface...");

    // Используем статические массивы вместо String
    char ssid[64];
    char password[64];
    char local_ip_str[16];
    char gateway_str[16];
    char subnet_str[16];
    
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

    Serial.println("Disconnecting from WiFi and applying new settings...");
    WiFi.disconnect();
    WiFi.config(settings.local_ip, settings.gateway, settings.subnet);
    WiFi.begin(settings.ssid, settings.password);

    server.send(200, "text/html", "<html><body><h2>Настройки сохранены! Перезагрузка...</h2></body></html>");
    delay(2000);
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
        server.send(500, "text/plain", "Обновление не удалось: " + String(ESPhttpUpdate.getLastErrorString()));
        break;
      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("OTA Update: No updates available");
        server.send(500, "text/plain", "Нет доступных обновлений");
        break;
      case HTTP_UPDATE_OK:
        Serial.println("OTA Update: Success!");
        server.send(200, "text/plain", "Обновление успешно завершено!");
        break;
    }
  } else {
    Serial.println("OTA Update: URL not provided");
    server.send(400, "text/plain", "Требуется URL!");
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
      server.send(200, "text/plain", "Обновление завершено.");
      delay(1000);
      ESP.restart();
    } else {
      Update.printError(Serial);
      server.send(500, "text/plain", "Ошибка при завершении обновления.");
    }
  }
}

// Страница информации
void handleInfoPage() {
  Serial.println("Serving information page...");
  String html = "<html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { text-align: center; background-color: #f4f4f9; color: #333; font-family: 'Roboto', sans-serif; }";
  html += ".container { max-width: 600px; margin: 50px auto; padding: 20px; background-color: #fff; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); border-radius: 10px; }";
  html += "h2 { color: #fb8c00; }";
  html += ".info-box { text-align: left; padding: 10px; border: 1px solid #ccc; margin-bottom: 10px; background-color: #f9f9f9; }";
  html += ".info-box span { display: block; font-weight: bold; color: #333; }";
  html += "</style>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h2>Системная информация</h2>";

  html += "<div class='info-box'>";
  html += "<span>Имя хоста:</span> " + String(hostname);
  html += "</div>";


  html += "<div class='info-box'>";
  html += "<span>Время работы:</span> " + String(millis() / 1000) + " секунд";
  html += "</div>";

  html += "<div class='info-box'>";
  html += "<span>Версия прошивки:</span> " + String(FIRMWARE_VERSION);
  html += "</div>";

  html += "<div class='info-box'>";
  html += "<span>Скорость процессора:</span> " + String(ESP.getCpuFreqMHz()) + " МГц";
  html += "</div>";

  html += "<br><button onclick='location.href=\"/\"'>Главное меню</button>";
  html += "</div>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}
// Обр врем
void handleGetTime() {
  server.send(200, "text/plain", getCurrentTime());
}


// Перезагрузка устройства
void handleRebootPage() {
  Serial.println("Rebooting...");
  String html = "<html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { text-align: center; background-color: #f4f4f9; color: #333; font-family: 'Roboto', sans-serif; }";
  html += ".container { max-width: 600px; margin: 50px auto; padding: 20px; background-color: #fff; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); border-radius: 10px; }";
  html += "h2 { color: #e53935; }";
  html += "button { width: 100%; padding: 15px; margin: 10px 0; font-size: 18px; border-radius: 5px; border: none; cursor: pointer; background-color: #e53935; color: #fff; }";
  html += "</style>";
  html += "</head><body>";
  html += "<div class='container'>";
  html += "<h2>Перезагрузка устройства...</h2>";
  html += "<br><button onclick='location.href=\"/\"'>Главное меню</button>";
  html += "</div>";
  html += "</body></html>";

  server.send(200, "text/html", html);
  delay(1000);
  ESP.restart();
}


void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  loadSettings();  // Загрузка настроек из EEPROM

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
    WiFi.config(settings.local_ip, settings.gateway, settings.subnet);

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
  server.on("/reboot", handleRebootPage);
  server.on("/getTime", handleGetTime);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

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
}
