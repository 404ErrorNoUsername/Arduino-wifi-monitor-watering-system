#include <WiFiS3.h>

char ssid[] = "Your_wifi_name"; //Enter your wifi name
char pass[] = "password";       //Enter your wifi password

WiFiServer server(80);

int IN1 = 2;                    //output going from arduino d2 to relay in1
int IN2 = 3;                    //output going from arduino d3 to relay in2
int Pin1 = A0;                  //sensor 1 signal pin to A0 on arduino
int Pin2 = A1;                  //sensor 2 signal pin to A1 on arduino
float value1 = 0;
float value2 = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(Pin1, INPUT);
  pinMode(Pin2, INPUT);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
  delay(500);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client.");
    String request = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;

        if (request.endsWith("\r\n\r\n")) {
          break;
        }
      }
    }

    // Read sensors
    value1 = analogRead(Pin1);
    value2 = analogRead(Pin2);

    // Control outputs based on sensors
    digitalWrite(IN1, value1 > 750 ? LOW : HIGH);
    digitalWrite(IN2, value2 > 750 ? LOW : HIGH);

    // Handle AJAX requests
    if (request.indexOf("GET /kick") >= 0) {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      delay(500); //time interval for manual watering.
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, HIGH);

      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println("Connection: close");
      client.println();
      client.println("Kicked!");
    } else if (request.indexOf("GET /data") >= 0) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.print("{\"sensor1\":");
      client.print(value1);
      client.print(",\"sensor2\":");
      client.print(value2);
      client.println("}");
    } else {
      // Serve HTML page
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println("Connection: close");
      client.println();
      client.println("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
      client.println("<title>Moisture Monitor</title></head><body>");
      client.println("<h1>Real-Time Moisture Monitor</h1>");
      client.println("<p>Sensor 1: <span id='s1'>...</span></p>");
      client.println("<p>Sensor 2: <span id='s2'>...</span></p>");
      client.println("<button onclick=\"kickOutputs()\">Manual Water</button>");
      client.println("<script>");
      client.println("function updateData() {");
      client.println("  fetch('/data').then(res => res.json()).then(data => {");
      client.println("    document.getElementById('s1').textContent = data.sensor1;");
      client.println("    document.getElementById('s2').textContent = data.sensor2;");
      client.println("  });");
      client.println("}");
      client.println("function kickOutputs() {");
      client.println("  fetch('/kick').then(res => res.text()).then(msg => {");
      client.println("  });");
      client.println("}");
      client.println("setInterval(updateData, 1000);"); //sets how often the web page updates the sensor values
      client.println("</script>");
      client.println("</body></html>");
    }

    delay(1);
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

