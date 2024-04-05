#include <WiFi.h>
#include <WiFiClient.h>
#include "FS.h"
#include <LittleFS.h>

// #define MAX_LINE_LENGTH (64)
#define FORMAT_LITTLEFS_IF_FAILED true

const char ssid[] = "ternary_q";
const char pass[] = "55555555";
const char *ip = "192.168.43.29";
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

bool frees = false;

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

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\r\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}

void appendFile(fs::FS &fs, const char * path, const int message){
    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    file.print(message);
    file.close();
}
void appendFile(fs::FS &fs, const char * path, const char* message){
    // Serial.printf("Appending to file: %s\r\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("- failed to open file for appending");
        return;
    }
    file.print(message);
    file.close();
}

std::string readFile(fs::FS &fs, const char * path, int begin, int end){

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return "";
    }
    std::string result ="";
    // Serial.println("- read from file:");
    file.seek(begin);
    for(int i =begin;i<end;i++){
        if (file.available())
          result +=(char)file.read();
        else
          break;
        // Serial.println(file.position());
    }
    // Serial.println(result.c_str());
    file.close();
    return result;
}

int sizeFile(fs::FS &fs, const char * path){

    File file = fs.open(path);
    int size = file.size();
    file.close();
    return size;
}

void IRAM_ATTR onTimer(){
  // message_t message;
  if (!choice){
    buffer[pointer] = analogRead(34);
    
    // buffer[pointer] &= 0x0FFF;
  }
  else{
    buffera[pointer] = analogRead(34);
    // buffera[pointer] &= 0x0FFF;
  }
  ind++;
  pointer++;
    if (pointer == buff_size) {
      pointer = 0;
      choice = !choice;
      frees = true;
      xTaskCreatePinnedToCore(toFile, "file", 10000, NULL, 1, NULL, 1);
    }
    // if ((ind == 10000 || ind == 20000) ){
    //   xTaskCreatePinnedToCore(send, "send", 10000, NULL, 2, NULL, 1);
    // }
  else if (ind > 10000 && !frees){
    
    xTaskCreatePinnedToCore(send, "send", 10000, NULL, 2, NULL, 1);
    timerDetachInterrupt(My_timer);
  }
}

void toFile(void*params){
  std::string packet = "";
  for (int j = 0; j < 1000; j++) {
        // Serial.println(buffer[i]);
        // client.printf("%d ", buffer[i]);
        if (choice){
          packet += std::to_string(buffer[j]);
        }
        else{
          packet += std::to_string(buffera[j]);
        }
        packet += " ";

    }
    // Serial.println(packet.c_str());
  appendFile(LittleFS, "/data.txt", packet.c_str());
  frees = false;
  Serial.print(".");
  vTaskDelete(NULL);
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

void listen_for_rec(void *params) {
  while (true) {
    WiFiClient cl = server.available();
    if (cl) {
      while (cl.connected()) {
        Serial.print("&");
        if (cl.available()) {
          String cmd = cl.readString();
          Serial.printf("CMD: %s\n", cmd.c_str());
          if (cmd == "REC") {
            client.connect(ip, port);
//            while (!client.connected()){
//              Serial.print(".");
//              delay(100);
//              client.connect(ip, port);
//            }
            
            frees = false;
            xTaskCreatePinnedToCore(setupTimer, "time", 10000, NULL, 1, NULL, 1);
          }
        }
        delay(10);
      }
    }
    delay(150);
  }
}

void send(void*params){

  if (client.connected()){
    delay(75);
    Serial.println();
    std::string packet = readFile(LittleFS, "/data.txt", 0,5);
    client.printf(packet.c_str());
    delay(200);
    // Serial.println(packet.c_str());
    packet = "";
    int size = (sizeFile(LittleFS, "/data.txt")/1000)+1;
    for (int j = 0; j < size; j++) {
        // Serial.println(buffer[i]);
        // client.printf("%d ", buffer[i]);
        packet = readFile(LittleFS, "/data.txt", 5+1000*j,5+1000*(j+1));
        client.printf(packet.c_str());
        delay(250);
        // Serial.println(packet.c_str());
    }
      client.printf("S");
      
      
      // if (k==0){
      //   k+=1;
      //   writeFile(LittleFS, "/data.txt", packet.c_str());
      // }else{
      //   appendFile(LittleFS, "/data.txt", packet.c_str());
      // }
    // Serial.println(packet.c_str());

    // String response = client.readString();
    // if (response == "") {
    //   return;
    // }
    // Serial.printf("Server response: %s\n", response.c_str());


    }else
      Serial.print("#");
    deleteFile(LittleFS, "/data.txt");
    writeFile(LittleFS, "/data.txt", "");
//      client.connect(ip, port);
    if (packet1_num==31){
      packet1_num = 0;
      client.stop();
      Serial.println("Disconnected");
      std::string fill = readFile(LittleFS, "/data.txt", 0,100);
      Serial.println(fill.c_str());

    }
    
  vTaskDelete(NULL);
}




void setup() {
  Serial.begin(115200);
  delay(1000);

#ifdef TWOPART
    if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED, "/lfs2", 5, "part2")){
    Serial.println("part2 Mount Failed");
    return;
    }
    appendFile(LittleFS, "/hello0.txt", "World0!\r\n");
    readFile(LittleFS, "/hello0.txt");
    LittleFS.end();

    Serial.println( "Done with part2, work with the first lfs partition..." );
#endif

    if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LittleFS Mount Failed");
        return;
    }
  
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
//  client.connect(ip, port);
//  while (!client.connected()){
//    Serial.print(".");
//    delay(100);
//    client.connect(ip, port);
//  }

  // if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
  //       Serial.println("LittleFS Mount Failed");
  //   }

//  xTaskCreatePinnedToCore(setupTimer, "time", 10000, NULL, 1, NULL, 1);
  deleteFile(LittleFS, "/data.txt");
  writeFile(LittleFS, "/data.txt", "WAV 0");
  xTaskCreatePinnedToCore(listen_for_rec, "listen_rec", 10000, NULL, 1, &handler_rec, 1);
  // xTaskCreatePinnedToCore(recieve, "recieve", 10000, NULL, 1, NULL, 0);
}
void loop() {
 delay(100);
}
