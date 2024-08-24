void generateHeader(String &html, String title) {
    html += "<html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>" + title + "</title>";
    html += "<style>";
    html += "body { background-color: #282c34; color: #ffffff; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; padding: 0; display: flex; flex-direction: column; align-items: center; justify-content: center; min-height: 100vh; }";
    html += "h1, h2 { margin: 0.5em 0; }";
    html += ".container { width: 100%; max-width: 600px; padding: 20px; box-sizing: border-box; }";
    html += "button { width: 100%; padding: 15px; margin: 10px 0; font-size: 18px; border: none; border-radius: 5px; background-color: #61dafb; color: #000000; cursor: pointer; transition: background-color 0.3s; }";
    html += "button.red { background-color: #ff4b4b; color: #ffffff; }";
    html += "button:hover { background-color: #21a1f1; }";
    html += "button.red:hover { background-color: #e04343; }";
    html += "input { width: 100%; padding: 12px; margin: 10px 0; border-radius: 5px; border: 1px solid #ccc; font-size: 16px; }";
    html += ".footer { margin-top: auto; padding: 10px 0; text-align: center; font-size: 14px; color: #999999; }";
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

    html += "</head><body onload='updateTime()'>";
    html += "<div class='container'>";
    html += "<h1>" + title + "</h1>";
    html += "<div class='time'>Текущее время: <span id='time'>" + getCurrentTime() + "</span></div>";
}


void generateFooter(String &html) {
    html += "</div><div class='footer'>© 2024 ООО Предприятие №8</div>";
    html += "</body></html>";
}