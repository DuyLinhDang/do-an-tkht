#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <EEPROM.h>
ESP8266WebServer webServer(80);
//==========AP info=======================//
char *ssid_ap = "ESP_SEVER";
char *pass_ap = "12345678";
IPAddress ip_ap(192, 168, 1, 1);
IPAddress gateway_ap(192, 168, 1, 1);
IPAddress subnet_ap(255, 255, 255, 0);
String ssid;
String pass;
String mqttServer;
String mqttPort;
String mqttUsername;
String mqttPassword;
// ......................setup mqtt.............................................................
#define mqtt_server "project02.cloud.shiftr.io"
#define mqtt_topic_pub_pin "pin"
#define mqtt_topic_pub_dd "dd"
#define mqtt_topic_pub_on_off "on_off"
#define mqtt_topic_sub "power"
#define mqtt_user "project02"
#define mqtt_pwd "k5lPJcglnvDYsxNq"
#define power D4
#define anl A0
#define on_off D3
const uint16_t mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);
void setup_wifi();
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
boolean read_EEPROM();
boolean select_mqtt();
void clear_EEPROM();
void restart_esp();
void write_EEPROM();
void checkConnection();
void mainpage();
void write_mqtt_server();
ICACHE_RAM_ATTR void ISR();
boolean read_mqttSever_EEPROM(String &sv,String &pt,String &un,String &pw);
void clearMqttServer();
long lastMsg = 0;
char msg[50];
int value = 0;
int count;
float value_anl_pin;
float value_anl_dd;
int value_on_off;
const char MainPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>D.a</title>
</head>
<style>
    body {
        text-align: center;
        font-size: 130%;
        background-color: rgb(245, 245, 228);
    }
    
    button {
        padding: 10px 10px;
        margin-top: 20px;
        margin-bottom: 10px;
        margin-right: 10px;
        font-size: 85%;
    }
    
    input {
        width: 250px;
        height: 25px;
        font-size: 18px;
        margin: 7px 7px;
        background-color: rgb(245, 245, 242);
    }
    
    .all {
        width: 1440px;
        margin: auto;
        padding-top: 50px;
    }
    
    .dang-nhap-wifi {
        width: 50%;
        float: left;
    }
    
    .mqtt {
        width: 50%;
        float: left;
    }
    
    h1 {
        color: brown;
    }
    
    h2 {
        margin-top: -20px;
        font-size: 15px;
    }
    
    .button-connect,
    .button-save {
        background-color: rgb(79, 243, 142);
    }
    
    .button-clear-eeprom,
    .button-clear-mqtt {
        background-color: rgb(194, 21, 21);
    }
    
    .button-restart {
        background-color: rgb(112, 59, 209);
    }
</style>

<body>
    <div class="all">
        <div class="dang-nhap-wifi">
            <h1>Đăng nhập WIFI</h1>
            <div>Tên Wifi</div>
            <div><input type="text" id="ssid"></div>
            <div>Mật khẩu</div>
            <div><input type="text" id="pass"></div>
            <div>
                <button class="button-connect" onclick="writeEEPROM()">Connect WIFI</button>
                <button class="button-clear-eeprom" onclick="clearEEPROM()">Clear WIFI</button>
                <button class="button-restart" onclick="restartesp()">Restart ESP</button>
            </div>
            <div id="reponsetext"></div>
        </div>
        <div class="mqtt">
            <h1>Đăng nhập MQTT</h1>
            <h2>(Nếu chưa có tài khoản có thể bỏ trống)</h2>
            <div>Host</div>
            <div><input type="text" id="mqttServer"></div>
            <div>MQTT Port</div>
            <div><input type="text" id="mqttPort"></div>
            <div>Username</div>
            <div><input type="text" id="mqttUsername"></div>
            <div>Password</div>
            <div><input type="text" id="mqttPassword"></div>
            <div>
                <button class="button-save" onclick="writeMqttServer()">Save MQTT</button>
                <button class="button-clear-mqtt" onclick="clearMqttServer()">Clear MQTT</button>
            </div>
        </div>
        <div id="reponsetext"></div>
    </div>
    <!-- .................................JavaScript...............................  -->
    <script>
        // tạo đối tương object
        var xhttp = new XMLHttpRequest();

        function writeEEPROM() {
            var ssid = document.getElementById("ssid").value;
            var pass = document.getElementById("pass").value;
            xhttp.open("GET", "/writeEEPROM?ssid=" + ssid + "&pass=" + pass, true);
            xhttp.onreadystatechange = process; //nhận reponse 
            xhttp.send();
        }
        function writeMqttServer() {
            var mqttServer = document.getElementById("mqttServer").value;
            var mqttPort = document.getElementById("mqttPort").value;
            var mqttUsername = document.getElementById("mqttUsername").value;
            var mqttPassword = document.getElementById("mqttPassword").value;
            xhttp.open("GET", "/writeMqttSever?mqttServer=" + mqttServer + "&mqttPort=" + mqttPort + "&mqttUsername=" + mqttUsername + "&mqttPassword=" + mqttPassword, true);
            xhttp.onreadystatechange = process; //nhận reponse 
            xhttp.send();
        }

        function clearEEPROM() {
            xhttp.open("GET", "/clearEEPROM", true);
            xhttp.onreadystatechange = process; //nhận reponse 
            xhttp.send();
        }
        function restartesp(){
          xhttp.open("GET","/restartesp",true);
          xhttp.onreadystatechange = process;//nhận reponse 
          xhttp.send();
        }
        function clearMqttServer(){
          xhttp.open("GET","/clearMqttServer",true);
          xhttp.onreadystatechange = process;//nhận reponse 
          xhttp.send();
        }
        function process() {
            if (xhttp.readyState == 4 && xhttp.status == 200) {
                //------Updat data sử dụng javascript----------
                ketqua = xhttp.responseText;
                document.getElementById("reponsetext").innerHTML = ketqua;
            }
        }
    </script>
</body>

</html>
)=====";
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(power, OUTPUT);
  pinMode(on_off, INPUT_PULLUP); //pull-up button
  attachInterrupt(on_off, ISR, FALLING);
  digitalWrite(power, HIGH);
  EEPROM.begin(512);
  delay(10);
  if (read_EEPROM())
  {
    Serial.print("checkconnect");
    checkConnection();
    if (true)
    {
      if (read_mqttSever_EEPROM(mqttServer,mqttPort,mqttUsername,mqttPassword))
      {
        // mqttServer = mqttServer.c_str();
        // mqttPort = mqttPort.c_str();
        // mqttUsername = mqttUsername.c_str();
        // mqttPassword = mqttPassword.c_str();
        Serial.println(mqttServer);
        Serial.println(mqttPort);
        Serial.println(mqttUsername);
        Serial.println(mqttPassword);
        // mqttServer = "";
        // //const char mqttServer0  "aaaaaaa";
        // for (int i = 0; i < 30; i++)
        // {
        //   mqttServer += char(EEPROM.read(i+40));
        //   // Serial.print(z);
        // }
        // Serial.print("---");
        // Serial.print(mqttServer);
        // Serial.print("----");
        Serial.print("mqtt nhap vao");
        client.setServer(mqttServer.c_str(), mqttPort.toInt());
      }
      // int f = mqttPort.toInt();
      else{
        Serial.print("mqtt mac dinh");
        client.setServer(mqtt_server, mqtt_port);
      }
      client.setCallback(callback);
      Serial.print("success");
      client.subscribe(mqtt_topic_sub);
    }
  }
  if (!read_EEPROM())
  {
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(ip_ap, gateway_ap, subnet_ap);
    WiFi.softAP(ssid_ap, pass_ap);
    Serial.println("Soft Access Point mode!");
    Serial.print("Please connect to ");
    Serial.println(ssid_ap);
    Serial.print("Web Server IP Address: ");
    Serial.println(ip_ap);
    // ..............................................
    webServer.on("/", mainpage);
    webServer.on("/writeEEPROM", write_EEPROM);
    webServer.on("/clearEEPROM", clear_EEPROM);
    webServer.on("/restartesp", restart_esp);
    webServer.on("/writeMqttSever", write_mqtt_server);
    webServer.on("/clearMqttServer",clearMqttServer);
    webServer.begin();
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (EEPROM.read(0) == 0)
    while (true)
    {
      webServer.handleClient();
    }

  while (EEPROM.read(0) != 0)
  {
    if (!client.connected())
    {
      reconnect();
    }
    long now = millis();
    if (now - lastMsg > 5000)
    {
      lastMsg = now;
      value_anl_pin = analogRead(anl);
      //value_anl_pin = value_anl_pin / 20;
      //value1 = (String)value_anl_pin;
      value_anl_dd = 1.5 * value_anl_pin;
      client.publish(mqtt_topic_pub_pin, String(value_anl_pin).c_str());
      delay(50);
      Serial.println(value_anl_dd);
      Serial.println(value_on_off);
      client.publish(mqtt_topic_pub_dd, String(value_anl_dd).c_str());
      delay(50);
      client.publish(mqtt_topic_pub_on_off, String(value_on_off).c_str());
      delay(50);
      client.subscribe("power");
      
    }
    client.loop();
  }
}
// ................................chuong trinh con........................
ICACHE_RAM_ATTR void ISR(){
  value_on_off++;
}
void mainpage()
{
  String s = FPSTR(MainPage);
  webServer.send(200, "text/html", s);
}
//---------------------------------------***-------------------
boolean read_mqttSever_EEPROM(String &sv,String &pt,String &un,String &pw){
  if (EEPROM.read(40) != 0)
  {
    sv = "";
    pt = "";
    un = "";
    pw = "";
    for (int i = 0; i < 30; ++i)
    {
      sv += char(EEPROM.read(i+40));
    }
    for (int i = 0; i < 20; ++i)
    {
      pt += char(EEPROM.read(i+70));
    }
    // Serial.print("Port: ");
    // Serial.println(mqttPort);
    for (int i = 0; i < 25; ++i)
    {
      un += char(EEPROM.read(i+100));
    }
    // Serial.print("mqttUsername ");
    // Serial.println(mqttUsername);
    for (int i = 0; i < 25; ++i)
    {
      pw += char(EEPROM.read(i+125));
    }
    // Serial.print("mqttPassword ");
    // Serial.println(mqttPassword);
    // mqttServer = mqttServer.c_str();
    // mqttPort = mqttPort.c_str();
    // mqttUsername = mqttUsername.c_str();
    // mqttPassword = mqttPassword.c_str();
    return true;
  }
  else{
    return false;
  }
}
//----------------------------------------------
boolean read_EEPROM()
{
  Serial.println("Reading EEPROM...");
  if (EEPROM.read(0) != 0)
  {
    ssid = "";
    pass = "";
    for (int i = 0; i < 20; ++i)
    {
      ssid += char(EEPROM.read(i));
    }
    for (int i = 0; i < 20; ++i)
    {
      pass += char(EEPROM.read(i+20));
    }
    ssid = ssid.c_str();
    pass = pass.c_str();
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("PASSWORD: ");
    Serial.println(pass);
    
    return true;
  }
  else
  {
    Serial.println("Data wifi not found!");
    return false;
  }
}
//--------------------------*********--------------------------
void clear_EEPROM()
{
  Serial.println("Clear EEPROM!");
  for (int i = 0; i < 40; ++i)
  {
    EEPROM.write(i, 0);
    delay(10);
  }
  EEPROM.commit();
  String s = "Đã xóa bộ nhớ EEPROM";
  webServer.send(200, "text/html", s);
}
//-----------------------****-------------------
void clearMqttServer(){
  for (int i = 0; i < 110; i++)
  {
    EEPROM.write(i+40,0);
  }
  EEPROM.commit();
  String s = "Đã lưu clear mqtt";
  webServer.send(200, "text/html", s);
}
//--------------------------*********-------------
void restart_esp()
{
  ESP.restart();
  String s = "Đã khởi động lại ESP8266";
  webServer.send(200, "text/html", s);
}
//--------------------------*********-------------
void write_EEPROM()
{
  ssid = webServer.arg("ssid");
  pass = webServer.arg("pass");
  Serial.println("Clear EEPROM!");
  for (int i = 0; i < 40; ++i)
  {
    EEPROM.write(i, 0);
    delay(10);
  }
  for (int i = 0; i < ssid.length(); ++i)
  {
    EEPROM.write(i, ssid[i]);
  }
  for (int i = 0; i < pass.length(); ++i)
  {
    EEPROM.write(20 + i, pass[i]);
  }
  EEPROM.commit();
  Serial.println("Writed to EEPROM!");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("PASS: ");
  Serial.println(pass);
  String s = "Đã lưu thông tin wifi";
  webServer.send(200, "text/html", s);
}
//--------------------------*********--------------------------
void write_mqtt_server()
{
  mqttServer = webServer.arg("mqttServer");
  mqttPort = webServer.arg("mqttPort");
  mqttUsername = webServer.arg("mqttUsername");
  mqttPassword = webServer.arg("mqttPassword");
  Serial.println("Clear EEPROM!");
  for (int i = 40; i < 150; ++i)
  {
    EEPROM.write(i, 0);
    delay(10);
  }
  for (int i = 0; i < mqttServer.length(); ++i)
  {
    EEPROM.write(40+i, mqttServer[i]);
  }
  for (int i = 0; i < mqttPort.length(); ++i)
  {
    EEPROM.write(i+70, mqttPort[i]);
  }
  for (int i = 0; i < mqttUsername.length(); ++i)
  {
    EEPROM.write(i+100, mqttUsername[i]);
  }
  for (int i = 0; i < mqttPassword.length(); ++i)
  {
    EEPROM.write(i+125, mqttPassword[i]);
  }
  EEPROM.commit();
  Serial.println("Writed to EEPROM!");
  Serial.print("mqttServer ");
  Serial.println(mqttServer);
  Serial.print("mqttport ");
  Serial.println(mqttPort);
  Serial.print("mqttUsername ");
  Serial.println(mqttUsername);
  Serial.print("mqttPassword ");
  Serial.println(mqttPassword);
  String s = "Đã lưu thông tin mqtt";
  webServer.send(200, "text/html", s);
}
//......................................................
//---------------SETUP WIFI------------------------------
void checkConnection()
{
  Serial.println();
  Serial.print("Check connecting to ");
  Serial.println(ssid);
  WiFi.disconnect();
  WiFi.begin(ssid, pass);
  count = 0;
  while (count < 50)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println();
      Serial.print("Connected to ");
      Serial.println(ssid);
      Serial.print("Web Server IP Address: ");
      Serial.println(WiFi.localIP());
      return;
    }
    delay(200);
    Serial.print(".");
    count++;
  }
  if (count == 50)
  {
    for (int i = 0; i < 40; ++i)
    {
      EEPROM.write(i, 0);
      delay(10);
    }
    EEPROM.commit();
  }
}
// ....................................................................................
// Hàm call back để nhận dữ liệu
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    char receivedChar = (char)payload[i];
    Serial.println(receivedChar);
    if (receivedChar == '1')
    {
      // Kiểm tra nếu tin nhận được là 1 thì bật LED và ngược lại
      digitalWrite(power, 0);
      Serial.println("ON");
    }
    if (receivedChar == '0')
    {
      digitalWrite(power, 1);
      Serial.println("OFF");
    }
  }
  Serial.println();
}
//--------------------------*********--------------------------
boolean select_mqtt(){
  if (EEPROM.read(40) !=0)
  {
    Serial.print("mqtt nhap vao");
    return client.connect("ESP8266Client", mqttUsername.c_str(), mqttPassword.c_str());
  }
  else{
    Serial.print("mqtt mat dinh");
    return client.connect("ESP8266Client", mqtt_user, mqtt_pwd);
  }
  
}
// Hàm reconnect thực hiện kết nối lại khi mất kết nối với MQTT Broker
void reconnect(){
  // Chờ tới khi kết nối
  int timer = 0;
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Thực hiện kết nối với mqtt user và pass
    if (select_mqtt())
    {
      Serial.println("connected");
      // Khi kết nối sẽ publish thông báo
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Đợi 5s
      delay(5000);
      timer++;
      if (timer == 7)
      {
        clearMqttServer();
        ESP.restart();
      }
    }
  }
}
