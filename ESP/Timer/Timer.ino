#include <WiFi.h>
#include <WiFiClient.h>

#define MAX_LINE_LENGTH (64)

const char ssid[] = "YOTA-6203";
const char pass[] = "55EC0657";
const char *ip = "10.0.0.100";
int port = 7001;

QueueHandle_t QueueHandle;
const int QueueElementSize = 10;

hw_timer_t *My_timer = NULL;

typedef struct{
  int number;
} message_t;

WiFiServer server(80);

int counter=0;

String options[3] = {"Test1", "Test2", "Test3"};

void IRAM_ATTR onTimer(){
  counter++;
  message_t message;
  if(QueueHandle != NULL && uxQueueSpacesAvailable(QueueHandle) > 0){
    message.number = counter;
    int ret = xQueueSend(QueueHandle, (void*) &message, 0);
  }
}

void send(void* params){
  message_t message;
  while(1){
    if (QueueHandle != NULL){
      int ret = xQueueReceive(QueueHandle, &message, portMAX_DELAY);
      if(ret == pdPASS){
        WiFiClient client;
        client.connect(ip, port);
        if (client.connected()){
          client.printf(options[message.number%3].c_str());
          client.stop();
          Serial.println(options[message.number%3]);
        }else 
          Serial.println("Not able to connect");
      }
    }
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
  // xTaskCreatePinnedToCore(recieve, "recieve", 10000, NULL, 1, NULL, 0);

  My_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(My_timer, &onTimer, true);
  timerAlarmWrite(My_timer, 1000000, true);
  timerAlarmEnable(My_timer);
}
void loop() {
  delay(200);
}