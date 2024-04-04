#include <WiFi.h>
#include <WiFiClient.h>
#include "EEPROM.h"

// #define MAX_LINE_LENGTH (64)
// #define FORMAT_LITTLEFS_IF_FAILED true

const char ssid[] = "ternary_q";
const char pass[] = "55555555";
const char *ip = "192.168.43.102";
int port = 7001;

const int buff_size = 1000;
int frames = 10;
int address = 0;

struct MyStruct  {
  int buffer[buff_size];
};

int ind=0;

int buffer[buff_size];

bool choice = 0;
int pointer = 0;

int freq = 133;
int k =0;

// TaskHandle_t task_send, task_rot;
hw_timer_t *My_timer = NULL;
TaskHandle_t handler_rec;

int packet1_num = 1;

// WiFiClient client;
// WiFiServer server(7020);

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

// void listen_for_rec(void *params) {
//   while (true) {
//     WiFiClient cl = server.available();
//     if (cl) {
//       while (cl.connected()) {
//         Serial.print("&");
//         if (cl.available()) {
//           String cmd = cl.readString();
//           Serial.printf("CMD: %s\n", cmd.c_str());
//           if (cmd == "REC") {
//             client.connect(ip, port);
//             while (!client.connected()){
//               Serial.print(".");
//               delay(100);
//               client.connect(ip, port);
//             }
//             xTaskCreatePinnedToCore(setupTimer, "time", 10000, NULL, 1, NULL, 1);
//           }
//         }
//         delay(10);
//       }
//     }
//     delay(150);
//   }
// }

void send(void*params){

  // if (client.connected()){
    if (1){
    delay(75);
    std::string packet = "WAV ";
    // client.printf(packet.c_str());
    packet += "1 30 ";
    packet += std::to_string(packet1_num);
    packet += " 1000S";
    Serial.println(packet.c_str());
    delay(75);
    // Serial.println(packet.c_str());
    packet = "";
    for (int i = 0; i<30; i++){
      // int buffera[buff_size];
      // EEPROM.get(address, buffera);
      // address += sizeof(buffer);
      for (int j = 0; j < 1000; j++) {
          // Serial.println(buffer[i]);
          // client.printf("%d ", buffer[i]);
          int val;
          EEPROM.get(address, val);
          packet += std::to_string(val);
          address += sizeof(int);
          packet += " ";

      }
        Serial.println(packet.c_str());
        
        // if (k==0){
        //   k+=1;
        //   writeFile(LittleFS, "/data.txt", packet.c_str());
        // }else{
        //   appendFile(LittleFS, "/data.txt", packet.c_str());
        // }
        packet = "";
        delay(100);
    }
    // Serial.println(packet.c_str());

    // String response = client.readString();
    // if (response == "") {
    //   return;
    // }
    // Serial.printf("Server response: %s\n", response.c_str());

    packet1_num++;
    Serial.println(packet1_num);}
    // }else
    //   Serial.print("#");
    //   client.connect(ip, port);
    if (packet1_num==31){
      packet1_num = 0;
      // client.stop();
      Serial.println("Disconnected");
    }
    
  vTaskDelete(NULL);
}

void writeIntArrayIntoEEPROM(int address, int numbers[], int arraySize)
{
  int addressIndex = address;
  for (int i = 0; i < arraySize; i++) 
  {
    EEPROM.write(addressIndex, numbers[i] >> 8);
    EEPROM.write(addressIndex + 1, numbers[i] & 0xFF);
    addressIndex += 2;
  }
}

void readIntArrayFromEEPROM(int address, int numbers[], int arraySize)
{
  int addressIndex = address;
  for (int i = 0; i < arraySize; i++)
  {
    numbers[i] = (EEPROM.read(addressIndex) << 8) + EEPROM.read(addressIndex + 1);
    addressIndex += 2;
  }
}

void IRAM_ATTR onTimer(){
  // message_t message;
    
    // buffer[pointer] = analogRead(35);
    buffer[pointer] = analogRead(35);
    // EEPROM.put(address, gigga);
    
    // address += sizeof(int);
    // buffer[pointer] &= 0x0FFF;

    
  ind++;
  pointer++;
    if (pointer == buff_size) {
      pointer = 0;
      choice = !choice;
      
      
      Serial.println("send");
      writeIntArrayIntoEEPROM(address, buffer, 1000)
      address += sizeof(buffer);
      // eeprom_write_block((void*)&myStruct, address, sizeof(myStruct));
      address += sizeof(buffer);
      // xTaskCreatePinnedToCore(send, "send", 10000, NULL, 1, NULL, 1);
    }
  if (ind > 30000){
    address = 0;
    EEPROM.commit();
    xTaskCreatePinnedToCore(send, "send", 10000, NULL, 1, NULL, 1);
    timerDetachInterrupt(My_timer);
  }
}



void setupTimer(void* params){
  Serial.println("Timer config");
  ind = 0, pointer = 0, packet1_num = 1;
  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 1000, true);
  timerAlarmEnable(My_timer);
  Serial.println("Timer cfg_end");
  vTaskDelete(NULL);
}

void setup() {
  Serial.begin(115200);
  // if (!EEPROM.begin(10000)) {
  //   Serial.println("Failed to initialise EEPROM");
  //   Serial.println("Restarting...");
  //   delay(1000);
  //   ESP.restart();
  // }

  // QueueHandle = xQueueCreate(QueueElementSize, sizeof(message_t));
  // if(QueueHandle == NULL){
  //   Serial.println("Queue could not be created. Halt.");
  //   delay(1000);
  //   ESP.restart(); // Halt at this point as is not possible to continue
  // }

  // WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, pass);
  // if (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //   Serial.println("[WIFI_FAIL]");
  //   ESP.restart();
  // } else {
  //   Serial.println("[WIFI_SUCCESS]");
  //   Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  // }
  
  // server.begin();
//  client.connect(ip, port);
//  while (!client.connected()){
//    Serial.print(".");
//    delay(100);
//    client.connect(ip, port);
//  }

  // if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
  //       Serial.println("LittleFS Mount Failed");
  //   }

 xTaskCreatePinnedToCore(setupTimer, "time", 10000, NULL, 1, NULL, 1);
  // xTaskCreatePinnedToCore(listen_for_rec, "listen_rec", 10000, NULL, 1, &handler_rec, 1);
  // xTaskCreatePinnedToCore(recieve, "recieve", 10000, NULL, 1, NULL, 0);
}
void loop() {
 delay(100);
}
