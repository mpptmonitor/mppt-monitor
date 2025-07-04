#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "raspberry";      
const char* password = "123123123";

const char* serverName = "https://script.google.com/macros/s/AKfycbwasCvJOhAqiPExo7OV449nylNloNqIzAMHpmj5rmdCsQsv6LA7ds_5eOj-UZT43Z6S5w/exec"; 

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200); // RX2 ke GPIO 16, TX2 ke GPIO 17

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  if (Serial2.available()) {
    String data = Serial2.readStringUntil('\n');
    data.trim();

    if (data.startsWith("[") && data.endsWith("]")) {
      data.remove(0, 1);   // Menghapus karakter '['
      data.remove(data.length() - 1, 1); // Menghapus karakter ']'

      // Memisahkan data berdasarkan tanda koma
      String solar_voltage = getValue(data, ',', 0);
      String bat_voltage = getValue(data, ',', 1);
      String solar_current = getValue(data, ',', 2);
      String bat_current = getValue(data, ',', 3);
      String solar_power = getValue(data, ',', 4);
      String output_power = getValue(data, ',', 5);
      String pwm_percentage = getValue(data, ',', 6);
      String mode_str = getValue(data, ',', 7);
      String load_status = getValue(data, ',', 8);

      // Kirim data ke Google Sheets
      sendToGoogleSheets(solar_voltage, bat_voltage, solar_current, bat_current, solar_power, output_power, pwm_percentage, mode_str, load_status);
    }
  }
}

void sendToGoogleSheets(String solar_voltage, String bat_voltage, String solar_current, String bat_current, String solar_power, String output_power, String pwm_percentage, String mode_str, String load_status) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String httpRequestData = "solar_voltage=" + solar_voltage +
                             "&bat_voltage=" + bat_voltage +
                             "&solar_current=" + solar_current +
                             "&bat_current=" + bat_current +
                             "&solar_power=" + solar_power +
                             "&output_power=" + output_power +
                             "&pwm_percentage=" + pwm_percentage +
                             "&mode_str=" + mode_str +
                             "&load_status=" + load_status;

    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Data sent to Google Sheets. Response:");
      Serial.println(response);
    } else {
      Serial.print("Error sending data: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

String getValue(String data, char separator, int index) {
  int start = 0;
  int end = -1;
  for (int i = 0; i <= index; i++) {
    start = end + 1;
    end = data.indexOf(separator, start);
    if (end == -1) {
      end = data.length();
    }
  }
  return data.substring(start, end);
}
