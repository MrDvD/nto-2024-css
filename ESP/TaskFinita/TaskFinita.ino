#include <WiFi.h>
#include <WiFiClient.h>
#include <iostream>

// #define MAX_LINE_LENGTH (64)
#define FORMAT_LITTLEFS_IF_FAILED true

const char ssid[] = "ternary_q";
const char pass[] = "55555555";
const char *ip = "192.168.43.130";
int port = 7001;

const int buff_size = 1000;
int frames = 10;


int ind=0;

int buffer[buff_size];
int buffera[buff_size];
bool choice = 0;
int pointer = 0;

int freq = 133;
int k =0;

// TaskHandle_t task_send, task_rot;
hw_timer_t *My_timer = NULL;
TaskHandle_t handler_rec;

int packet1_num = 1;

WiFiClient client;
WiFiServer server(7020);

// void appendFile(fs::FS &fs, const char * path, const char * message){
//     Serial.printf("Appending to file: %s\r\n", path);

//     File file = fs.open(path, FILE_APPEND);
//     if(!file){
//         Serial.println("- failed to open file for appending");
//         return;
//     }
//     if(file.print(message)){
//         Serial.println("- message appended");
//     } else {
//         Serial.println("- append failed");
//     }
//     file.close();
// }

// void readFile(fs::FS &fs, const char * path){
//     Serial.printf("Reading file: %s\r\n", path);

//     File file = fs.open(path);
//     if(!file || file.isDirectory()){
//         Serial.println("- failed to open file for reading");
//         return;
//     }

//     Serial.println("- read from file:");
//     while(file.available()){
//         Serial.write(file.read());
//     }
//     file.close();
// }

// void writeFile(fs::FS &fs, const char * path, const char * message){
//     Serial.printf("Writing file: %s\r\n", path);

//     File file = fs.open(path, FILE_WRITE);
//     if(!file){
//         Serial.println("- failed to open file for writing");
//         return;
//     }
//     if(file.print(message)){
//         Serial.println("- file written");
//     } else {
//         Serial.println("- write failed");
//     }
//     file.close();
// }

void listen_for_rec(void *params) {
  while (true) {
    WiFiClient cl = server.available();
    if (cl) {
      Serial.println("connection success");
      while (cl.connected()) {
        if (cl.available()) {
          String cmd = cl.readString();
          Serial.printf("CMD: %s\n", cmd.c_str());
          if (cmd == "REC") {
            Serial.println("Must start rec");
          }
        }
        delay(10);
      }
    }
    delay(150);
  }
}

void ss(void*params){

  if (client.connected()){
    delay(75);
    std::string packet = "WAV ";
    // client.printf(packet.c_str());
    packet +="0 30 ";
    packet += std::to_string(packet1_num);
    packet += " 1000S";
    client.printf(packet.c_str());
    delay(75);
    // Serial.println(packet.c_str());
    packet = "";
    for (int j = 0; j < 1000; j++) {
        // Serial.println(buffer[i]);
        // client.printf("%d ", buffer[i]);
        if (choice)
          packet += std::to_string(buffer[j]);
        else
          packet += std::to_string(buffera[j]);

        packet += " ";

    }
      client.printf(packet.c_str());
      
      // if (k==0){
      //   k+=1;
      //   writeFile(LittleFS, "/data.txt", packet.c_str());
      // }else{
      //   appendFile(LittleFS, "/data.txt", packet.c_str());
      // }
      packet = "";
    // Serial.println(packet.c_str());

    // String response = client.readString();
    // if (response == "") {
    //   return;
    // }
    // Serial.printf("Server response: %s\n", response.c_str());

    packet1_num++;
    Serial.println(packet1_num);
    }else
      Serial.print("#");
      // client.connect(ip, port);
    if (packet1_num==31){
      client.stop();
      Serial.println("Disconnected");
    }
    
  vTaskDelete(NULL);
}




void IRAM_ATTR onTimer(){
  // message_t message;
  if (!choice){
    buffer[pointer] = analogRead(35);
    // buffer[pointer] &= 0x0FFF;
  }
  else{
    buffera[pointer] = analogRead(35);
    // buffera[pointer] &= 0x0FFF;
  }
  ind++;
  pointer++;
    if (pointer == buff_size) {
      pointer = 0;
      choice = !choice;
      xTaskCreatePinnedToCore(ss, "send", 10000, NULL, 1, NULL, 1);
      // int ret = xQueueSend(QueueHandle, (void*) &message, 0);

    }
  if (ind>30000){
    
    timerDetachInterrupt(My_timer);
  }
  // if (pointer == 7999){
  //   pointer = 0;
  //   xTaskCreate(sendTask, "send", 10000, (void*) &buffer, 1, &task_rot);
  // }
  
  // if(QueueHandle != NULL && uxQueueSpacesAvailable(QueueHandle) > 0){
  //   message.number = counter;
  //   int ret = xQueueSend(QueueHandle, (void*) &message, 0);
  // }
}


void setupTimer(void* params){
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 1000, true);
  timerAlarmEnable(My_timer);
  vTaskDelete(NULL);
}

void setup() {
  Serial.begin(115200);

  // QueueHandle = xQueueCreate(QueueElementSize, sizeof(message_t));
  // if(QueueHandle == NULL){
  //   Serial.println("Queue could not be created. Halt.");
  //   delay(1000);
  //   ESP.restart(); // Halt at this point as is not possible to continue
  // }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("[WIFI_FAIL]");
    ESP.restart();
  } else {
    Serial.println("[WIFI_SUCCESS]");
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  }
  
  server.begin();
  client.connect(ip, port);
  while (!client.connected()){
    Serial.print(".");
    delay(100);
    // client.connect(ip, port);
  }

  // if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
  //       Serial.println("LittleFS Mount Failed");
  //   }

  xTaskCreatePinnedToCore(setupTimer, "time", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(listen_for_rec, "listen_rec", 10000, NULL, 1, &handler_rec, 1);
  // xTaskCreatePinnedToCore(recieve, "recieve", 10000, NULL, 1, NULL, 0);
}
void loop() {
 delay(100);
}