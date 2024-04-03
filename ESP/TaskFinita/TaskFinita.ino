#include <WiFi.h>
#include <WiFiClient.h>
#include <iostream>

// #define MAX_LINE_LENGTH (64)
#define FORMAT_LITTLEFS_IF_FAILED true

const char ssid[] = "Redmi Note 10S";
const char pass[] = "11111111";
const char *ip = "192.168.123.176";
int port = 7001;

const int buff_size = 1000;
int frames = 10;

int boba[30][1000];

int ind=0;

int buffer[buff_size];
int buffera[buff_size];
bool choice = 0;
int pointer = 0;

int freq = 133;
int k =0;

// TaskHandle_t task_send, task_rot;
hw_timer_t *My_timer = NULL;

int packet1_num = 0;



WiFiServer server(80);


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

void ss(void*params){
  WiFiClient client;
  client.connect(ip, port);
  if (client.connected()){
    std::string packet = "WAV ";
    packet += std::to_string(packet1_num);
    packet += " ";
    // packet += std::to_string(j);
    packet += std::to_string(frames);
    packet += " ";
    packet += std::to_string(buff_size);
    packet += " ";
    packet += std::to_string(freq);
    client.printf(packet.c_str());
    packet = "";
    for (int j = 0; j < 10; j++) {
      for (int i = 100*j; i < 100*(j+1) ; i++){
        // Serial.println(buffer[i]);
        // client.printf("%d ", buffer[i]);
        if (choice)
          packet += std::to_string(buffer[i]);
        else
          packet += std::to_string(buffera[i]);
        if (i != buff_size - 1) {
          packet += " ";
        }
        delay(1);
      }
      client.printf(packet.c_str());
      // if (k==0){
      //   k+=1;
      //   writeFile(LittleFS, "/data.txt", packet.c_str());
      // }else{
      //   appendFile(LittleFS, "/data.txt", packet.c_str());
      // }
      packet = "";
    }
    // Serial.println(packet.c_str());

    // String response = client.readString();
    // if (response == "") {
    //   return;
    // }
    // Serial.printf("Server response: %s\n", response.c_str());
    client.stop();
    Serial.println("Disconnected");
    packet1_num++;
    Serial.println(packet1_num);
  }else
    Serial.println("Not able to connect");
  vTaskDelete(NULL);
}




void IRAM_ATTR onTimer(){
  // message_t message;
  if (!choice){
    buffer[pointer] = analogRead(34);
    buffer[pointer] &= 0x0FFF;
  }
  else{
    buffera[pointer] = analogRead(34);
    buffera[pointer] &= 0x0FFF;
  }
  ind++;
  pointer++;
    if(pointer == buff_size){
      pointer = 0;
      choice = !choice;
      xTaskCreatePinnedToCore(ss, "send", 10000, NULL, 1, NULL, 1);
      // int ret = xQueueSend(QueueHandle, (void*) &message, 0);

    }
  if (ind==30000){
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

  // if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
  //       Serial.println("LittleFS Mount Failed");
  //   }

  xTaskCreatePinnedToCore(setupTimer, "time", 10000, NULL, 1, NULL, 1);
  // xTaskCreatePinnedToCore(recieve, "recieve", 10000, NULL, 1, NULL, 0);



}
void loop() {
 delay(100);
}