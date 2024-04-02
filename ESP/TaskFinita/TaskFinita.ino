#include <WiFi.h>
#include <WiFiClient.h>
#include <iostream>

// #define MAX_LINE_LENGTH (64)

const char ssid[] = "Redmi Note 10S";
const char pass[] = "11111111";
const char *ip = "192.168.123.176";
int port = 7001;

const int buff_size = 1000;
int frames = 10;

int buffer[buff_size];
int buffera[buff_size];
bool choice = 0;
int pointer = 0;

// QueueHandle_t QueueHandle;
// const int QueueElementSize = 10;
// TaskHandle_t task_send, task_rot;
hw_timer_t *My_timer = NULL;

int packet1_num = 0;

// typedef struct{
//   int data;
// } message_t;

// struct soundhdr {
//   char  riff[4];        /* "RIFF"                                  */
//   long  flength;        /* file length in bytes                    */
//   char  wave[4];        /* "WAVE"                                  */
//   char  fmt[4];         /* "fmt "                                  */
//   long  chunk_size;     /* size of FMT chunk in bytes (usually 16) */
//   short format_tag;     /* 1=PCM, 257=Mu-Law, 258=A-Law, 259=ADPCM */
//   short num_chans;      /* 1=mono, 2=stereo                        */
//   long  srate;          /* Sampling rate in samples per second     */
//   long  bytes_per_sec;  /* bytes per second = srate*bytes_per_samp */
//   short bytes_per_samp; /* 2=16-bit mono, 4=16-bit stereo          */
//   short bits_per_samp;  /* Number of bits per sample               */
//   char  data[4];        /* "data"                                  */
//   long  dlength;        /* data length in bytes (filelength - 44)  */
// } wavh;

WiFiServer server(80);

// void appender(void*params){
//   while(1){
//     if(QueueHandle != NULL){
//       int ret = xQueueReceive(QueueHandle, &message, portMAX_DELAY);
//       if(ret == pdPASS){
//           // The message was successfully received - send it back to Serial port and "Echo: "
//           buffer[pointer++] = message.data;
//           if (pointer == 7999){
//             pointer = 0;
//           }
//           Serial.printf("Echo line of size %d: \"%s\"\n", message.line_length, message.line);
//           // The item is queued by copy, not by reference, so lets free the buffer after use.
//         }else if(ret == pdFALSE){
//           Serial.println("The `TaskWriteToSerial` was unable to receive data from the Queue");
//         }
      
//     }

//   }
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

// void sendTask(void*params){
//     // int buffe = *((int*)params);
//     message_t message;
    
//     packet1_num++;
//     if(QueueHandle != NULL && uxQueueSpacesAvailable(QueueHandle) > 0){
//       for (int i = 0; i < buff_size; i++){
//         message.data[i] = buffer[i];
//       }
//       int ret = xQueueSend(QueueHandle, (void*) &message, 0);
//     }
//     vTaskDelete(NULL);
// }

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
  pointer++;
    if(pointer == buff_size){
      pointer = 0;
      choice = !choice;
      xTaskCreatePinnedToCore(ss, "send", 10000, NULL, 1, NULL, 1);
      // int ret = xQueueSend(QueueHandle, (void*) &message, 0);

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

// void send(void* params){
//   message_t message;
//   while(1){
//     if(QueueHandle != NULL){
//       int ret = xQueueReceive(QueueHandle, &message, portMAX_DELAY);
//       if(ret == pdPASS){
//         WiFiClient client;
//         client.connect(ip, port);
//         if (client.connected()){
//           client.printf("WAV %d:", packet1_num);
//           for (int i = 0; i < buff_size; i++){
//             client.printf("%d ", message.data[i]);
//           }
//           // String response = client.readString();
//           // if (response == "") {
//           //   return;
//           // }
//           // Serial.printf("Server response: %s\n", response.c_str());
//           client.stop();
//           Serial.println("Disconnected");
//         }else
//           Serial.println("Not able to connect");
//       }else if(ret == pdFALSE){
//         Serial.println("The `TaskWriteToSerial` was unable to receive data from the Queue");
//       }
//     }
//     delay(100);
//   }
// }



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

  // xTaskCreatePinnedToCore(send, "send", 10000, NULL, 1, &task_send, 1);
  // xTaskCreatePinnedToCore(recieve, "recieve", 10000, NULL, 1, NULL, 0);

  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 2000000/buff_size, true);
  timerAlarmEnable(My_timer);

}
void loop() {
 delay(100);
}