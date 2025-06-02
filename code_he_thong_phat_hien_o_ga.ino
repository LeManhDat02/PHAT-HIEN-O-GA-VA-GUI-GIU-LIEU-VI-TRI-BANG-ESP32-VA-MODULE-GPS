#include <HardwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Thông tin WiFi
const char* ssid = "Madrid";
const char* password = "123456788";

// Cấu hình SIM
#define MCU_SIM_BAUDRATE 115200
#define MCU_SIM_TX_PIN 21
#define MCU_SIM_RX_PIN 20
#define MCU_SIM_EN_PIN 2  // GPIO2 bật nguồn SIM

#define SENSOR 1

String Web_App_URL = "https://script.google.com/macros/s/AKfycbwYV6aF26tfzWvX089BTRvCSfGGIdXp3de52sqj2mYsbOtZdXhXsD6UeWPx3UvduyE/exec";

HardwareSerial simSerial(0);

// Biến lưu GPS
String gpsData = "";
bool gpsReady = false;

void sim_at_wait() {
  delay(100);
  while (simSerial.available()) {
    char c = simSerial.read();
    // Serial.write(c);  // In ra debug
    gpsData += c;  // Lưu dữ liệu GPS
  }
}

bool sim_at_cmd(String cmd, int timeout = 1000) {
  // Serial.print("Gửi lệnh: ");
  // Serial.println(cmd);
  simSerial.println(cmd);

  unsigned long start = millis();
  gpsData = "";
  while (millis() - start < timeout) {
    sim_at_wait();
    if (gpsData.indexOf("OK") != -1 || gpsData.indexOf("ERROR") != -1 || gpsData.indexOf("+QGPSLOC:") != -1) {
      break;
    }
    delay(50);
  }

  return gpsData.indexOf("OK") != -1;
}

void setup() {
  pinMode(MCU_SIM_EN_PIN, OUTPUT);
  digitalWrite(MCU_SIM_EN_PIN, HIGH);  // Bật nguồn SIM module
  delay(8000);

  Serial.begin(115200);
  delay(1000);

  simSerial.begin(MCU_SIM_BAUDRATE, SERIAL_8N1, MCU_SIM_RX_PIN, MCU_SIM_TX_PIN);

  Serial.println("Kết nối WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi đã kết nối!");

  Serial.println("Kiểm tra SIM...");
  sim_at_cmd("AT");
  sim_at_cmd("AT+CPIN?");
  sim_at_cmd("AT+CSQ");

  Serial.println("Bật GPS...");
  sim_at_cmd("AT+QGPS=1", 3000);

  pinMode(SENSOR, INPUT);
}

void loop() {
  int in = digitalRead(SENSOR);
  Serial.println("cảm biến: " + String(in));
  String gps = get_gps();
  Serial.println("*************************************** ĐỊA CHỈ ***************************************\n \n"
                 + gps + "\n\n******************************************************************************************\n ");

  if (in == 1) {
    send_ggsheet(gps);
  }
  delay(100);
}

String get_gps() {
  String location = getGPSLocation();

  if (location.length() > 5) {
    int commaPos = location.indexOf(',');
    String lat = location.substring(0, commaPos);
    String lon = location.substring(commaPos + 1);

    lat = convertToDecimalString(lat);
    lon = convertToDecimalString(lon);

    String address = getAddressFromCoordinates(lat, lon);
    if (address.length() > 0) {
      return address;
    } else {
      return "Không lấy được địa chỉ từ tọa độ GPS.";
    }
  } else {
    return "Chưa lấy được tọa độ GPS, thử lại...";
  }
}

String convertToDecimalString(String coord) {
  // Lấy ký tự hướng (N/S/E/W)
  char direction = coord.charAt(coord.length() - 1);
  coord.remove(coord.length() - 1);  // Xóa ký tự hướng

  // Chuyển sang float
  float val = coord.toFloat();

  // Lấy phần độ và phút
  int degrees = int(val / 100);
  float minutes = val - (degrees * 100);

  // Tính độ thập phân
  float decimalDegrees = degrees + (minutes / 60.0);

  // Đảo dấu nếu là 'S' hoặc 'W'
  if (direction == 'S' || direction == 'W') {
    decimalDegrees *= -1;
  }

  // Trả về dạng chuỗi (6 chữ số thập phân)
  return String(decimalDegrees, 6);
}

String getGPSLocation() {
  sim_at_cmd("AT+QGPSLOC=0", 2000);  // Lấy vị trí GPS

  // Ví dụ dữ liệu trả về: +QGPSLOC: 123456.0,21.028511,105.804817,11.0,0.0,0.0,1,7,0.9,7.4,5.0,1.1,11,3
  // Ta cần lấy hai số sau dấu phẩy đầu tiên là lat và lon

  int startIndex = gpsData.indexOf("+QGPSLOC:");
  if (startIndex == -1) return "";

  String loc = gpsData.substring(startIndex);
  int firstComma = loc.indexOf(',');
  int secondComma = loc.indexOf(',', firstComma + 1);
  int thirdComma = loc.indexOf(',', secondComma + 1);

  if (firstComma == -1 || secondComma == -1 || thirdComma == -1) return "";

  String latStr = loc.substring(firstComma + 1, secondComma);
  String lonStr = loc.substring(secondComma + 1, thirdComma);

  return latStr + "," + lonStr;
}

String getAddressFromCoordinates(String lat, String lon) {
  HTTPClient http;

  String url = "https://nominatim.openstreetmap.org/reverse?format=json&lat=" + lat + "&lon=" + lon + "&zoom=18&addressdetails=1";

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      const char* display_name = doc["display_name"];
      http.end();
      return String(display_name);
    } else {
      Serial.print("JSON parse error: ");
      Serial.println(error.c_str());
      http.end();
      return "";
    }
  } else {
    Serial.print("HTTP request failed, error: ");
    Serial.println(http.errorToString(httpCode).c_str());
    http.end();
    return "";
  }
}

void send_ggsheet(String address) {
  HTTPClient http;
  String fullUrl = Web_App_URL + "?sts=write&address=" + urlEncodeUTF8(address);

  http.begin(fullUrl);
  int httpCode = http.GET();

  Serial.print("HTTP Response code: ");
  Serial.println(httpCode);

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Response:");
    Serial.println(payload);
  } else {
    Serial.println("Error on HTTP request");
  }
  http.end();
}

String urlEncodeUTF8(const String& msg) {
  String encodedMsg = "";
  for (unsigned int i = 0; i < msg.length(); i++) {
    char c = msg.charAt(i);
    if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
      encodedMsg += c;
    } else if (c == ' ') {
      encodedMsg += "%20";
    } else {
      encodedMsg += '%';
      String temp = String(c, HEX);
      temp.toUpperCase();
      if (temp.length() == 1) encodedMsg += "0";  // để chắc chắn đủ 2 ký tự hex
      encodedMsg += temp;
    }
  }
  return encodedMsg;
}
