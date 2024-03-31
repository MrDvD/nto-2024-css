#include <WiFi.h>
#include <WiFiClient.h>

#define MAX_LINE_LENGTH (64)

const char ssid[] = "YOTA-6203";
const char pass[] = "55EC0657";
const char *ip = "10.0.0.100";
int port = 7001;

QueueHandle_t QueueHandle;
const int QueueElementSize = 10;

typedef struct{
  char line[MAX_LINE_LENGTH];
  uint8_t line_length;
} message_t;

WiFiServer server(80);

void recieve(void* params){
  message_t message;
  while(1){
    if (QueueHandle != NULL){
      int ret = xQueueReceive(QueueHandle, &message, portMAX_DELAY);
      if(ret == pdPASS){
        // The message was successfully received - send it back to Serial port and "Echo: "
        Serial.printf("Echo line of size %d: \"%s\"\n", message.line_length, message.line);
        // The item is queued by copy, not by reference, so lets free the buffer after use.
      }else if(ret == pdFALSE){
        Serial.println("The `TaskWriteToSerial` was unable to receive data from the Queue");
      }
    }
    delay(10);
  }
}

void send(void* params){
  message_t message;
  while(1){
    WiFiClient client;
    client.connect(ip, port);
    if (client.connected()){
      client.printf("[TEST] test");
      String response = client.readString();
      if (response == "") {
        return;
      }
      Serial.printf("Server response: %s\n", response.c_str());
      client.stop();
      Serial.println("Disconnected");
      message.line_length = response.length();
      if( message.line_length> 0){
        if(QueueHandle != NULL && uxQueueSpacesAvailable(QueueHandle) > 0){
          if (message.line_length < MAX_LINE_LENGTH){
            const char* ff =response.c_str();
            
            for (int i = 0; i<message.line_length; i++){
              message.line[i]=ff[i];
            }
            // message.line = gg;
            message.line[message.line_length] = 0;
          }
          int ret = xQueueSend(QueueHandle, (void*) &message, 0);
          if(ret == pdTRUE){
            Serial.println("Sent to queue");
          }else if(ret == errQUEUE_FULL){
            Serial.println("Failed to send to queue");
          }
        }
      }else
        delay(200);
    }else Serial.println("Not able to connect");
    
    delay(100);
  }
}

void setup() {
  Serial.begin(115200);

  QueueHandle = xQueueCreate(QueueElementSize, sizeof(message_t));
  if(QueueHandle == NULL){
    Serial.println("Queue could not be created. Halt.");
    delay(1000);
    ESP.restart(); // Halt at this point as is not possible to continue
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("[WIFI_FAIL]");
    ESP.restart();
  } else {
    Serial.println("[WIFI_SUCCESS]");
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  }

  xTaskCreatePinnedToCore(send, "send", 10000, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(recieve, "recieve", 10000, NULL, 1, NULL, 0);

}
void loop() {
 delay(100);
}
