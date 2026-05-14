#include <ESP8266AVRISP.h>
#include <command.h>

#include <Arduino.h> 
#include <ESP8266WiFi.h> 
#include <ESP8266WebServer.h> 
#include <ESP8266mDNS.h> 
#include <ESP8266HTTPClient.h> 
#include <WiFiClientSecure.h> 
#include <LittleFS.h> 
#include <WiFiManager.h> 
#include "AudioGeneratorMP3.h" 
#include "AudioOutputI2S.h" 
#include "AudioFileSourceLittleFS.h" 
 
ESP8266WebServer server(80); 
bool playRequested = false; 
String textToSpeak = ""; 
String currentAccent = "en-us"; 
float currentVol= 1.5f; 
 
AudioGeneratorMP3 *mp3; 
AudioFileSourceLittleFS *file; 
AudioOutputI2S *out; 
 
// ======================================================================= 
// HTML & CSS FOR THE DASHBOARD 
// ======================================================================= 
const char INDEX_HTML[] PROGMEM = R"rawliteral( 
<!DOCTYPE html> 
<html> 
<head> 
  <meta name="viewport" content="width=device-width, initial-scale=1"> 
  <title>text to speech</title> 
  <style> 
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
background-color: #121212; color: #ffffff; display: flex; flex-direction: 
column; align-items: center; justify-content: center; height: 100vh; 
margin: 0; } 
    .container { background-color: #1e1e1e; padding: 30px; border-radius: 
12px; box-shadow: 0 8px 16px rgba(0,0,0,0.5); text-align: center; 
max-width: 400px; width: 90%; } 
    h2 { margin-top: 0; color: #4CAF50; text-transform: lowercase; } 
    textarea { width: 100%; height: 100px; padding: 12px; border-radius: 
8px; border: 1px solid #333; background-color: #2c2c2c; color: white; 
font-size: 16px; resize: none; box-sizing: border-box; margin-bottom: 5px; 
outline: none; } 
    .char-counter { text-align: right; font-size: 12px; color: #888888; 
margin-bottom: 15px; } 
    .translation-box { background-color: #2a2a2a; border-left: 4px solid 
#2196F3; padding: 10px 12px; margin-bottom: 20px; border-radius: 4px; 
font-size: 15px; text-align: left; display: none; color: #e0e0e0; } 
    select { width: 100%; padding: 12px; border-radius: 8px; border: 1px 
solid #333; background-color: #2c2c2c; color: white; font-size: 16px; 
margin-bottom: 20px; outline: none; cursor: pointer; } 
    button { background-color: #4CAF50; color: white; border: none; 
padding: 12px 24px; font-size: 16px; border-radius: 8px; cursor: pointer; 
width: 100%; transition: 0.3s; } 
    button:hover { background-color: #45a049; } 
    #status { margin-top: 15px; font-size: 14px; color: #aaaaaa; height: 
20px; } 
    .footer { margin-top: 25px; font-size: 10px; color: #555555; } 
  </style> 
</head> 
<body> 
  <div class="container"> 
    <h2>text to speech</h2> 
    
    <textarea id="textInput" placeholder="type what you want to say" 
maxlength="200" oninput="updateCounter()"></textarea> 
    <div class="char-counter" id="charCount">0 / 200</div> 
    
    <div id="transBox" class="translation-box"></div> 
    
    <select id="langInput"> 
      <option value="en-us">US English</option> 
      <option value="en-gb">British English</option> 
      <option value="en-in">Indian English</option> 
      <option value="hi">Translate to Hindi</option> 
      <option value="ja">Translate to Japanese</option> 
      <option value="ko">Translate to Korean</option> 
      <option value="zh">Translate to Chinese</option> 
      <option value="ru">Translate to Russian</option> 
      <option value="es">Translate to Spanish</option> 
      <option value="fr">Translate to French</option> 
      <option value="de">Translate to German</option> 
      <option value="it">Translate to Italian</option> 
      <option value="ar">Translate to Arabic</option> 
    </select> 
    
  
      <label for="speed">Volume:</label> 
      <input type="range" id="volume" name="speed" min="0" max="3" 
value="1.5" step="0.1" style="margin-bottom: 20px;"> 
    
      <button onclick="processText()">Speak!</button> 
    <div id="status">Ready</div> 
    
    <div class="footer">&copy; 2026-27 ME . all right reserved</div> 
  </div> 
 
  <script> 
    function updateCounter() { 
      var text = document.getElementById('textInput').value; 
      document.getElementById('charCount').innerText = text.length + " / 
200"; 
    } 
 
    function processText() { 
      var text = document.getElementById('textInput').value; 
      var targetLang = document.getElementById('langInput').value; 
      if(!text) return; 
 
      var statusDiv = document.getElementById('status'); 
      var tBox = document.getElementById('transBox'); 
      
      if (targetLang.startsWith("en-")) { 
        tBox.style.display = 'none'; 
        sendToESP(text, targetLang, statusDiv); 
      } else { 
        statusDiv.innerHTML = "Translating..."; 
        statusDiv.style.color = "#2196F3"; 
 
        var translateUrl = 
"https://translate.googleapis.com/translate_a/single?client=gtx&sl=auto" + 
targetLang + "&dt=t&q=" + encodeURIComponent(text); 
        
        fetch(translateUrl) 
          .then(response => response.json()) 
          .then(data => { 
            var translatedText = data[0][0][0]; 
            tBox.style.display = 'block'; 
            tBox.innerHTML = "<b>Translation:</b> " + translatedText; 
            sendToESP(translatedText, targetLang, statusDiv); 
          }) 
          .catch(error => { 
            statusDiv.innerHTML = "Translation failed."; 
            statusDiv.style.color = "#f44336"; 
          }); 
      } 
    } 
 
    function getVolume() { 
      return document.getElementById('volume').value; 
    } 
 
    function sendToESP(finalText, langCode, statusDiv) { 
      statusDiv.innerHTML = "Sending to Device..."; 
      statusDiv.style.color = "#ffeb3b"; 
 
      fetch('/speak', { 
        method: 'POST', 
        headers: { 'Content-Type': 'application/x-www-form-urlencoded' }, 
        body: 'text=' + encodeURIComponent(finalText) + '&accent=' + 
encodeURIComponent(langCode) + '&volume=' + 
encodeURIComponent(getVolume()) 
      }) 
      .then(response => { 
        if(response.ok) { 
          statusDiv.innerHTML = "Downloading & Playing..."; 
          statusDiv.style.color = "#4CAF50"; 
          setTimeout(() => { statusDiv.innerHTML = "Ready"; 
statusDiv.style.color = "#aaaaaa"; }, 4000); 
        } else { 
          statusDiv.innerHTML = "Error sending to device."; 
          statusDiv.style.color = "#f44336"; 
        } 
      }); 
    } 
  </script> 
</body> 
</html> 
)rawliteral"; 
 
// ======================================================================= 
// URL ENCODER 
// ======================================================================= 
String urlEncode(String str) { 
  String encodedString = ""; 
  char c; 
  char code0; 
  char code1; 
  for (int i = 0; i < str.length(); i++) { 
    c = str.charAt(i); 
    if (isalnum(c)) { 
      encodedString += c; 
    } else if (c == ' ') { 
      encodedString += '+'; 
    } else { 
      code1 = (c & 0xf) + '0'; 
      if ((c & 0xf) > 9) code1 = (c & 0xf) - 10 + 'A'; 
      c = (c >> 4) & 0xf; 
      code0 = c + '0'; 
      if (c > 9) code0 = c - 10 + 'A'; 
      encodedString += '%'; 
      encodedString += code0; 
      encodedString += code1; 
    } 
  } 
  return encodedString; 
} 
 
// ======================================================================= 
// AUDIO FUNCTIONS 
// ======================================================================= 
void downloadTTS(String text, String accent) { 
  Serial.println("Connecting to Google TTS..."); 
  
  // 1. Sanitize the string 
  text.replace("\r\n", " "); 
  text.replace("\n", " ");   
  text.replace("\r", " ");   
  text.trim(); 
 
  // 2. Encode for web 
  String safeText = urlEncode(text); 
  
  String tts_url = 
"https://translate.googleapis.com/translate_tts?ie=UTF-8&q=" + safeText + 
accent + "&client=gtx"; 
 
  yield(); 
  delay(50); 
 
  WiFiClientSecure client; 
  client.setInsecure(); 
  
 
  HTTPClient http; 
  http.begin(client, tts_url); 
  
  int httpCode = http.GET(); 
  
  if (httpCode > 0 && httpCode < 400) { 
    Serial.println("Downloading MP3 to LittleFS..."); 
    File f = LittleFS.open("/tts.mp3", "w"); 
    if (f) { 
      http.writeToStream(&f); 
      f.close(); 
      Serial.println("Download complete."); 
    } else { 
      Serial.println("Failed to open LittleFS file for writing!"); 
    } 
  } else { 
    Serial.printf("HTTP GET failed, error: %s (Code: %d)\n", 
http.errorToString(httpCode).c_str(), httpCode); 
  } 
  http.end(); 
} 
 
void playAudio(const float &volume) { 
  file = new AudioFileSourceLittleFS("/tts.mp3"); 
  out = new AudioOutputI2S(); 
  out->SetGain(volume); 
  mp3 = new AudioGeneratorMP3(); 
  Serial.println("Starting TTS playback..."); 
  mp3->begin(file, out); 
} 
 
// ======================================================================= 
// WEB SERVER ROUTING 
// ======================================================================= 
void handleRoot() { 
  server.send(200, "text/html", INDEX_HTML); 
} 
 
void handleSpeak() { 
  if (server.hasArg("text")) { 
    textToSpeak = server.arg("text"); 
    if(server.hasArg("volume")) 
    { 
      currentVol =server.arg("volume").toFloat(); 
    } 
    else 
    { 
      currentVol = 1.5f; 
    } 
    
    if (server.hasArg("accent")) { 
      currentAccent = server.arg("accent"); 
    } else { 
      currentAccent = "en-us"; 
    } 
    
    server.send(200, "text/plain", "OK"); 
    playRequested = true; 
  } else { 
    server.send(400, "text/plain", "Bad Request"); 
  } 
} 
 
// ======================================================================= 
// SETUP & LOOP 
// ======================================================================= 
void setup() { 
  Serial.begin(115200); 
  delay(1000); 
  
  if (!LittleFS.begin()) { 
    Serial.println("\nLittleFS mount failed. Formatting..."); 
    LittleFS.format(); 
    LittleFS.begin(); 
  } 
 
  WiFi.mode(WIFI_STA); 
  WiFiManager wifiManager; 
  Serial.println("\nStarting WiFiManager..."); 
  
  if (!wifiManager.autoConnect("TTS_Setup", "TTSadmin@123")) { 
    Serial.println("Failed to connect and hit timeout"); 
    delay(3000); 
    ESP.restart(); // Reset and try again if it completely fails 
  } 
  
  WiFi.mode(WIFI_STA); 
  Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString()); 
  WiFi.setSleepMode(WIFI_NONE_SLEEP); 
  // ---------------------------- 
  delay(1000); 
  // Start mDNS 
  if (!MDNS.begin("TTS")) { 
    Serial.println("Error setting up MDNS responder!"); 
  } else { 
    Serial.println("mDNS started! You can now access: http://esp.local"); 
  } 
 
 
  server.on("/", HTTP_GET, handleRoot); 
  server.on("/speak", HTTP_POST, handleSpeak); 
  server.begin(); 
  Serial.println("HTTP server started"); 
 
 
  String myIP = WiFi.localIP().toString(); 
  myIP.replace(".", " dot "); 
  
 
  textToSpeak = "System ready. My I P address is " + myIP; 
  currentAccent = "en-us"; 
  playRequested = true; 
} 
 
void loop() { 
  server.handleClient(); 
  MDNS.update(); 
 
  if (playRequested) { 
    playRequested = false; 
    
    if (mp3 && mp3->isRunning()) { 
      mp3->stop(); 
    } 
    
    downloadTTS(textToSpeak, currentAccent); 
    playAudio(currentVol); 
  } 
 
  if (mp3 && mp3->isRunning()) { 
    if (!mp3->loop()) { 
      mp3->stop(); 
      Serial.println("Playback finished."); 
      
      if (file) { 
        file->close();   
        delete file;     
        file = nullptr;   
      } 
      
      LittleFS.remove("/tts.mp3");
       } 
  } 
}
