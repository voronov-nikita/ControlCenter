#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>

#define LED_PIN D4
#define NUM_LEDS 300
#define BRIGHTNESS 64          // Начальная яркость
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

const char* ssid = "UpRA";
const char* password = "passw0rd";

ESP8266WebServer server(80);

// Глобальные переменные
uint8_t mode = 0;
CRGB color1 = CRGB::Red;
CRGB color2 = CRGB::Blue;
uint8_t speed = 15;
uint8_t waveLength = 50;
uint8_t brightness = BRIGHTNESS;  // ✅ Новая переменная яркости

void setup() {
  Serial.begin(115200);
  Serial.println("🚀 Запуск LED контроллера...");
  
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);
  FastLED.clear();
  FastLED.show();
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("📶 IP адрес: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();
  
  Serial.println("🌐 Веб-сервер запущен!");
}

void loop() {
  server.handleClient();
  
  static uint32_t lastUpdate = 0;
  if (millis() - lastUpdate > (60 - speed) * 2) {
    updateLeds();
    FastLED.setBrightness(brightness);  // ✅ Обновляем яркость каждый кадр
    FastLED.show();
    lastUpdate = millis();
  }
}

void updateLeds() {
  static uint16_t phase = 0;
  static uint8_t fadePos = 0;
  phase += (speed + 1);
  fadePos += (speed + 1);
  
  switch (mode) {
    case 0: { // 📦 Статичный цвет
      fill_solid(leds, NUM_LEDS, color1);
      break;
    }
    
    case 1: { // 🎭 Переливание (все диоды)
      if (fadePos >= 255) fadePos = 0;
      CRGB blended = blend(color1, color2, fadePos);
      fill_solid(leds, NUM_LEDS, blended);
      break;
    }
    
    case 2: { // 🌊 Волна
      for (int i = 0; i < NUM_LEDS; i++) {
        uint16_t pos = (phase + (i * 256 / waveLength)) % 256;
        leds[i] = blend(color1, color2, sin8(pos));
      }
      break;
    }
    
    case 3: { // 🔥 Огонь
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(random8(20, 60), 255, random8(100, 255));
      }
      break;
    }
    
    case 4: { // 🌈 Радуга бегущая
      fill_rainbow(leds, NUM_LEDS, phase, 7);
      break;
    }
    
    case 5: { // ✨ Конфетти
      fadeToBlackBy(leds, NUM_LEDS, 20);
      int pos = random16(NUM_LEDS);
      leds[pos] += CHSV(phase + random8(), 255, 255);
      break;
    }
  }
}

void handleRoot() {
  String page = F("<!DOCTYPE html><html><head>");
  page += F("<meta charset='UTF-8'>");
  page += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  page += F("<title>🌈 LED Контроллер v3.0</title>");
  page += F("<style>");
  page += F("body{font-family:'Segoe UI',Arial,sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);color:white;padding:20px;margin:0;min-height:100vh;}");
  page += F(".card{background:rgba(255,255,255,0.1);backdrop-filter:blur(10px);border-radius:20px;padding:25px;margin:10px 0;box-shadow:0 8px 32px rgba(0,0,0,0.3);}");
  page += F("input,select{width:100%;margin:8px 0;padding:12px;border:none;border-radius:10px;background:rgba(255,255,255,0.2);color:white;font-size:16px;box-sizing:border-box;}");
  page += F("input[type=range]{height:8px;} button{background:linear-gradient(45deg,#ff6b6b,#feca57);color:white;padding:15px;font-size:18px;border:none;border-radius:12px;width:100%;font-weight:bold;cursor:pointer;transition:transform 0.2s;}");
  page += F("button:hover{transform:scale(1.02);} h1{text-align:center;font-size:28px;} h2{font-size:22px;margin:0 0 15px 0;} label{display:block;margin:10px 0;} output{font-size:18px;font-weight:bold;}");
  page += F("</style></head><body>");
  
  page += F("<div class='card'><h1>🎨 LED Контроллер v3.0</h1>");
  page += F("<p style='text-align:center;color:#ffd700;font-size:16px;'>📶 IP: ") + WiFi.localIP().toString() + F("</p></div>");
  
  page += F("<form action='/set' method='GET'>");
  
  // 🎯 Режимы
  page += F("<div class='card'><h2>🎯 Режим работы</h2>");
  page += F("<select name='mode'>");
  page += String(F("<option value='0'")) + (mode == 0 ? " selected" : "") + F(">📦 Статичный цвет</option>");
  page += String(F("<option value='1'")) + (mode == 1 ? " selected" : "") + F(">🎭 Переливание</option>");
  page += String(F("<option value='2'")) + (mode == 2 ? " selected" : "") + F(">🌊 Волна</option>");
  page += String(F("<option value='3'")) + (mode == 3 ? " selected" : "") + F(">🔥 Огонь</option>");
  page += String(F("<option value='4'")) + (mode == 4 ? " selected" : "") + F(">🌈 Радуга</option>");
  page += String(F("<option value='5'")) + (mode == 5 ? " selected" : "") + F(">✨ Конфетти</option>");
  page += F("</select></div>");
  
  // 🎨 Цвета
  char buf1[7]; sprintf(buf1, "%02X%02X%02X", color1.r, color1.g, color1.b);
  char buf2[7]; sprintf(buf2, "%02X%02X%02X", color2.r, color2.g, color2.b);
  
  page += F("<div class='card'><h2>🎨 Цвета</h2>");
  page += F("<label>Цвет 1: <input type='color' name='c1' value='#") + String(buf1) + F("'></label>");
  page += F("<label>Цвет 2: <input type='color' name='c2' value='#") + String(buf2) + F("'></label>");
  page += F("</div>");
  
  // ⚡ Скорость
  page += F("<div class='card'><h2>⚡ Скорость</h2>");
  page += F("<label>Скорость (1-50): ");
  page += F("<input type='range' name='speed' min='1' max='50' value='") + String(speed) + 
          F("' oninput='speedOut.value=this.value'> <output id='speedOut'>") + String(speed) + F("</output></label>");
  page += F("</div>");
  
  // 🌊 Длина волны (только для волны)
  if (mode == 2) {
    page += F("<div class='card'><h2>🌊 Длина волны</h2>");
    page += F("<label>Длина (10-100): ");
    page += F("<input type='range' name='wave' min='10' max='100' value='") + String(waveLength) + 
            F("' oninput='waveOut.value=this.value'> <output id='waveOut'>") + String(waveLength) + F("</output></label>");
    page += F("</div>");
  }
  
  // 💡 Яркость ✅ НОВЫЙ ПОЛЗУНОК!
  page += F("<div class='card'><h2>💡 Яркость</h2>");
  page += F("<label>Уровень (1-255): ");
  page += F("<input type='range' name='brightness' min='1' max='255' value='") + String(brightness) + 
          F("' oninput='brightOut.value=this.value'> <output id='brightOut'>") + String(brightness) + F("</output></label>");
  page += F("</div>");
  
  // 🚀 Кнопка
  page += F("<button type='submit'>🚀 Применить настройки</button>");
  page += F("</form>");
  
  // ℹ️ Описание
  page += F("<div class='card' style='margin-top:20px;'>");
  page += F("<h2>ℹ️ Описание режимов</h2>");
  page += F("<p><strong>📦 Статичный:</strong> все диоды одного цвета</p>");
  page += F("<p><strong>🎭 Переливание:</strong> плавный переход между цветами</p>");
  page += F("<p><strong>🌊 Волна:</strong> бегущая волна цветов по ленте</p>");
  page += F("<p><strong>🔥 Огонь:</strong> реалистичное пламя</p>");
  page += F("<p><strong>🌈 Радуга:</strong> классический радужный эффект</p>");
  page += F("<p><strong>✨ Конфетти:</strong> случайные яркие пиксели</p>");
  page += F("</div></body></html>");
  
  server.send(200, "text/html; charset=UTF-8", page);
}

void handleSet() {
  if (server.hasArg("mode")) mode = constrain(server.arg("mode").toInt(), 0, 5);
  
  // Цвета
  if (server.hasArg("c1")) {
    String hex1 = server.arg("c1"); 
    if (hex1.startsWith("#")) hex1 = hex1.substring(1);
    if (hex1.length() == 6) {
      long num = strtol(hex1.c_str(), NULL, 16);
      color1 = CRGB((num >> 16) & 0xFF, (num >> 8) & 0xFF, num & 0xFF);
    }
  }
  
  if (server.hasArg("c2")) {
    String hex2 = server.arg("c2"); 
    if (hex2.startsWith("#")) hex2 = hex2.substring(1);
    if (hex2.length() == 6) {
      long num = strtol(hex2.c_str(), NULL, 16);
      color2 = CRGB((num >> 16) & 0xFF, (num >> 8) & 0xFF, num & 0xFF);
    }
  }
  
  if (server.hasArg("speed")) speed = constrain(server.arg("speed").toInt(), 1, 50);
  if (server.hasArg("wave")) waveLength = constrain(server.arg("wave").toInt(), 10, 100);
  
  // ✅ Яркость!
  if (server.hasArg("brightness")) {
    brightness = constrain(server.arg("brightness").toInt(), 1, 255);
    FastLED.setBrightness(brightness);
  }
  
  server.sendHeader("Location", "/");
  server.send(302);
}
