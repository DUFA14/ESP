void handleRoot() {
    Serial.println("Serving main menu...");
    String html = "";
    generateHeader(html, "Главное меню");
    
    html += "<button onclick='location.href=\"/otaUpdatePage\"'>Обновление прошивки</button>";
    html += "<button onclick='location.href=\"/settings\"'>Настройки</button>";
    html += "<button onclick='location.href=\"/info\"'>Информация</button>";
    html += "<button onclick='location.href=\"/console\"'>Консоль</button>";
    html += "<button class='red' onclick='location.href=\"/reboot\"'>Перезагрузка</button>";

    generateFooter(html);
    server.send(200, "text/html", html);
}

void handleSettingsPage() {
    Serial.println("Serving settings page...");

    char local_ip[32];
    char gateway[32];
    char subnet[32];
    char timezone[8];

    snprintf(local_ip, sizeof(local_ip), "%d.%d.%d.%d", settings.local_ip[0], settings.local_ip[1], settings.local_ip[2], settings.local_ip[3]);
    snprintf(gateway, sizeof(gateway), "%d.%d.%d.%d", settings.gateway[0], settings.gateway[1], settings.gateway[2], settings.gateway[3]);
    snprintf(subnet, sizeof(subnet), "%d.%d.%d.%d", settings.subnet[0], settings.subnet[1], settings.subnet[2], settings.subnet[3]);
    snprintf(timezone, sizeof(timezone), "%d", settings.timezoneOffset / 3600);

    String html = "";
    generateHeader(html, "Настройки");

    html += "<form action=\"/saveSettings\" method=\"post\">";
    html += "SSID:<br><input type=\"text\" name=\"ssid\" value=\"" + String(settings.ssid) + "\"><br>";
    html += "Пароль:<br><input type=\"password\" name=\"password\" value=\"" + String(settings.password) + "\"><br>";
    html += "IP Адрес:<br><input type=\"text\" name=\"local_ip\" value=\"" + String(local_ip) + "\"><br>";
    html += "Шлюз:<br><input type=\"text\" name=\"gateway\" value=\"" + String(gateway) + "\"><br>";
    html += "Маска подсети:<br><input type=\"text\" name=\"subnet\" value=\"" + String(subnet) + "\"><br>";
    html += "Смещение часового пояса (в часах):<br><input type=\"number\" step=\"1\" name=\"timezone\" value=\"" + String(timezone) + "\"><br>";
    html += "<button type=\"submit\">Сохранить настройки</button>";
    html += "</form>";
    html += "<button onclick='location.href=\"/\"'>Главное меню</button>";
    generateFooter(html);
    server.send(200, "text/html", html);
}

void handleInfoPage() {
    Serial.println("Serving information page...");
    String html = "";
    generateHeader(html, "Информация о системе");
    char hostname[16];
    html += "<p>Имя хоста: " + String(hostname) + "</p>";
    html += "<p>SSID: " + String(WiFi.SSID()) + "</p>";
    html += "<p>IP: " + WiFi.localIP().toString() + "</p>";
    html += "<p>Свободная память(IRAM): " + String(ESP.getFreeHeap() / 1024) + " Кбайт</p>";
    html += "<p>Свободная память: " + String(ESP.getFreeSketchSpace() / 1024) + " Кбайт</p>";
    html += "<p>Время работы: " + String(millis() / 1000) + " секунд</p>";
    html += "<p>Версия прошивки: " + String(FIRMWARE_VERSION) + "</p>";
    html += "<p>Скорость процессора: " + String(ESP.getCpuFreqMHz()) + " МГц</p>";
    html += "<button onclick='location.href=\"/\"'>Главное меню</button>";

    generateFooter(html);
    server.send(200, "text/html", html);
}

void handleUpdatePage() {
    Serial.println("Serving OTA update page...");
    String html = "";
    generateHeader(html, "Обновление прошивки");

  html += "<div class='container'>";
  html += "<h2>Обновление через веб-сервер</h2>";
  html += "<form action=\"/otaUpdate\" method=\"get\">";
  html += "Ссылка OTA:<br><input type=\"text\" name=\"url\" value=\"http://TeriaHost.ru/firmware.bin\"><br>";
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


    generateFooter(html);
    server.send(200, "text/html", html);
}


void handleRebootPage() {
    Serial.println("Rebooting device...");

    String html = "";
    generateHeader(html, "Перезагрузка");

    html += "<h2>Устройство перезагружается...</h2>";
    html += "<p>Пожалуйста, подождите несколько секунд.</p>";

    generateFooter(html);
    server.send(200, "text/html", html);

    delay(3000);  // Задержка перед перезагрузкой, чтобы страница успела отобразиться
    ESP.restart();
}

void handleConsolePage() {
    Serial.println("Serving console page...");

    String html = "";
    generateHeader(html, "Консоль");

    html += "<div id='console' style='text-align:left; background-color:#000; color:#0f0; height:400px; overflow:auto; padding:10px;'></div>";
    html += "<script>";
    html += "var ws = new WebSocket('ws://' + window.location.hostname + ':81/');";
    html += "ws.onmessage = function(event) { document.getElementById('console').innerHTML += event.data + '<br>'; };";
    html += "</script>";
    html += "<button onclick='location.href=\"/\"'>Главное меню</button>";

    generateFooter(html);
    server.send(200, "text/html", html);
}

