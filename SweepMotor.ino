#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

// Wi-Fi 設置
const char* ssid = "Pixel 6 pro";     // 替換為你的 Wi-Fi 名稱
const char* password = "99887766"; // 替換為你的 Wi-Fi 密碼

// 建立網頁伺服器
ESP8266WebServer server(80);

// 伺服馬達和 ESC 物件
Servo steeringServo;
Servo esc;

// 超音波模組腳位
const int trigPin = 5; // D1 (GPIO 5)
const int echoPin = 0; // D3 (GPIO 0)

// 儲存距離的變數
float distance = 0;

void setup() {
  Serial.begin(115200);
  analogWriteFreq(50);

  // 初始化超音波模組
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // 初始化伺服馬達和 ESC
  steeringServo.attach(4, 1000, 2000);
  esc.attach(12, 1000, 2000);
  steeringServo.write(90);
  esc.writeMicroseconds(1500);
  delay(2000);

  // 伺服馬達測試
  Serial.println("開始執行馬達方向測試...");
  steeringServo.write(45);
  Serial.println("左轉 45°");
  delay(2000);
  steeringServo.write(90);
  Serial.println("回正 90°");
  delay(2000);
  steeringServo.write(135);
  Serial.println("右轉 135°");
  delay(2000);
  steeringServo.write(90);
  Serial.println("回正 90°");
  delay(1000);
  Serial.println("測試完成");

  // Wi-Fi 連接
  WiFi.begin(ssid, password);
  Serial.print("正在連接到 Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi 連接成功！");
  Serial.print("IP 地址：");
  Serial.println(WiFi.localIP());

  // 設定網頁伺服器路由
  server.on("/", handleRoot);
  server.on("/forward", goForward);
  server.on("/backward", goBackward);
  server.on("/left", turnLeft);
  server.on("/right", turnRight);
  server.on("/hardleft", turnHardLeft);
  server.on("/hardright", turnHardRight);
  server.on("/stop", stopMoving);
  server.on("/distance", getDistance); // 新增路由用於獲取距離
  server.begin();
  Serial.println("網頁伺服器已啟動");
}

void loop() {
  server.handleClient();
  measureDistance(); // 持續測量距離
}

// 測量距離的函數
void measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // 計算距離（單位：cm）

  Serial.print("距離：");
  Serial.print(distance);
  Serial.println(" cm");
}

// 提供距離數據的路由
void getDistance() {
  String json = "{\"distance\":";
  json += distance;
  json += "}";
  server.send(200, "application/json", json);
}

// 修改網頁內容，顯示距離和警告方塊
void handleRoot() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta charset='UTF-8'>";
  html += "<title>ESP8266 車輛控制</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f0f0f0; position: relative; }";
  html += "h1 { color: #333; }";
  html += ".button { display: inline-block; padding: 15px 30px; margin: 10px; font-size: 18px; cursor: pointer; border-radius: 5px; }";
  html += ".forward { background-color: #4CAF50; color: white; }";
  html += ".backward { background-color: #f44336; color: white; }";
  html += ".left { background-color: #2196F3; color: white; }";
  html += ".right { background-color: #FFC107; color: white; }";
  html += ".hardleft { background-color: #0288D1; color: white; }";
  html += ".hardright { background-color: #FFA000; color: white; }";
  html += ".stop { background-color: #555; color: white; }";
  html += ".back { background-color: #888; color: white; }"; // 新增返回按鍵的樣式
  html += ".control-pad { display: grid; grid-template-columns: repeat(3, 100px); gap: 10px; justify-content: center; margin: 20px; }";
  html += "#status { font-size: 20px; color: #333; margin-top: 20px; }";
  html += "#distance { font-size: 18px; color: #333; margin-top: 10px; }";
  html += "#warning { position: absolute; top: 10px; left: 10px; width: 50px; height: 50px; border-radius: 5px; }";
  html += "#warning-text { position: absolute; top: 10px; left: 70px; font-size: 16px; color: #333; }";
  html += "</style></head>";
  html += "<body>";
  html += "<div id='warning'></div>";
  html += "<div id='warning-text'>距離正常</div>";
  html += "<h1>ESP8266 車輛控制</h1>";
  html += "<div class='control-pad'>";
  html += "<div></div>";
  html += "<button class='button forward' onclick='sendCommand(\"forward\")'>前進</button>";
  html += "<div></div>";
  html += "<button class='button left' onclick='sendCommand(\"left\")'>左轉</button>";
  html += "<button class='button stop' onclick='sendCommand(\"stop\")'>停止</button>";
  html += "<button class='button right' onclick='sendCommand(\"right\")'>右轉</button>";
  html += "<div></div>";
  html += "<button class='button backward' onclick='sendCommand(\"backward\")'>後退</button>";
  html += "<div></div>";
  html += "</div>";
  html += "<p><button class='button hardleft' onclick='sendCommand(\"hardleft\")'>大左轉</button>";
  html += "<button class='button hardright' onclick='sendCommand(\"hardright\")'>大右轉</button>";
  html += "<button class='button back' onclick='goBack()'>返回</button></p>"; // 新增返回按鍵
  html += "<p id='status'>狀態：待機</p>";
  html += "<p id='distance'>距離：載入中...</p>";
  html += "<script>";
  html += "function sendCommand(action) {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/' + action, true);";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (xhr.readyState == 4 && xhr.status == 200) {";
  html += "      document.getElementById('status').innerText = '狀態：' + (action === 'forward' ? '前進' : action === 'backward' ? '後退' : action === 'left' ? '左轉' : action === 'right' ? '右轉' : action === 'hardleft' ? '大左轉' : action === 'hardright' ? '大右轉' : '停止');";
  html += "    } else if (xhr.readyState == 4) {";
  html += "      console.log('錯誤: ' + xhr.status);";
  html += "    }";
  html += "  };";
  html += "  xhr.send();";
  html += "}";
  html += "function goBack() {";
  html += "  window.close();"; // 關閉當前窗口
  html += "}";
  html += "function updateDistance() {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.open('GET', '/distance', true);";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (xhr.readyState == 4 && xhr.status == 200) {";
  html += "      var data = JSON.parse(xhr.responseText);";
  html += "      document.getElementById('distance').innerText = '距離：' + data.distance.toFixed(1) + ' cm';";
  html += "      var warningBox = document.getElementById('warning');";
  html += "      var warningText = document.getElementById('warning-text');";
  html += "      if (data.distance < 20) {";
  html += "        warningBox.style.backgroundColor = 'red';";
  html += "        warningText.innerText = '警告：距離過近！';";
  html += "      } else if (data.distance >= 20 && data.distance <= 40) {";
  html += "        warningBox.style.backgroundColor = 'yellow';";
  html += "        warningText.innerText = '注意：距離較近';";
  html += "      } else {";
  html += "        warningBox.style.backgroundColor = 'green';";
  html += "        warningText.innerText = '距離正常';";
  html += "      }";
  html += "    }";
  html += "  };";
  html += "  xhr.send();";
  html += "}";
  html += "setInterval(updateDistance, 1000);";
  html += "updateDistance();";
  html += "</script>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// 前進（維持0.5秒）
void goForward() {
  Serial.println("執行動作: 前進");
  esc.writeMicroseconds(1550);
  Serial.println("前進: 伺服角度=90, ESC=1550μs");
  server.send(200, "text/plain", "OK");
  delay(500);
  esc.writeMicroseconds(1500);
}

// 後退（維持0.5秒）
void goBackward() {
  Serial.println("執行動作: 後退");
  esc.writeMicroseconds(1400);
  Serial.println("後退: 伺服角度=90, ESC=1450μs");
  server.send(200, "text/plain", "OK");
  delay(500);
  esc.writeMicroseconds(1500);
}

// 左轉
void turnLeft() {
  Serial.println("執行動作: 左轉");
  esc.writeMicroseconds(1500);
  delay(500);
  steeringServo.write(120);
  delay(1000);
  Serial.println("左轉: 伺服角度=120, ESC=1500μs");
  server.send(200, "text/plain", "OK");
}

// 右轉
void turnRight() {
  Serial.println("執行動作: 右轉");
  esc.writeMicroseconds(1500);
  delay(500);
  steeringServo.write(60);
  delay(1000);
  Serial.println("右轉: 伺服角度=60, ESC=1500μs");
  server.send(200, "text/plain", "OK");
}

// 大左轉
void turnHardLeft() {
  Serial.println("執行動作: 大左轉");
  esc.writeMicroseconds(1500);
  delay(500);
  steeringServo.write(160);
  delay(1000);
  Serial.println("大左轉: 伺服角度=160, ESC=1500μs");
  server.send(200, "text/plain", "OK");
}

// 大右轉
void turnHardRight() {
  Serial.println("執行動作: 大右轉");
  esc.writeMicroseconds(1500);
  delay(500);
  steeringServo.write(20);
  delay(1000);
  Serial.println("大右轉: 伺服角度=20, ESC=1500μs");
  server.send(200, "text/plain", "OK");
}

// 停止
void stopMoving() {
  Serial.println("執行動作: 停止");
  esc.writeMicroseconds(1500);
  steeringServo.write(90);
  delay(1000);
  Serial.println("停止: 伺服角度=90, ESC=1500μs");
  server.send(200, "text/plain", "OK");
}