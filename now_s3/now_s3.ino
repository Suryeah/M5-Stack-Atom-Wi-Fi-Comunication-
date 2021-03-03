#include <esp_now.h>
#include "esp_system.h"
#include "M5Atom.h"
#include <WiFi.h>
#include <cppQueue.h>
#include <filters.h>
#include <FIR.h>

FIR<float, 2> firx;
FIR<float, 2> firy;

bool IMU6886Flag = false;
float accX, accY, accZ;
uint8_t RecvAddress[] = {0x24, 0x0A, 0xC4, 0xB5, 0x6F, 0x58};//84:0D:8E:34:46:60

typedef struct Data
{
  int node;
  float x;
  float y;
} Data;

Data Accel_XY;
int i;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)
{
  i = *incomingData;
}

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Accel_XY.node = 3;

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, RecvAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);

  float coefx[2] = {1., 1.};
  float coefy[2] = {1., 1.};

  firx.setFilterCoeffs(coefx);
  firy.setFilterCoeffs(coefy);

  M5.begin(true, false, true);
  if (!M5.IMU.myInit()) IMU6886Flag = true;
}

void loop()
{
  if (i == 3)
  {
    if (IMU6886Flag == true)
    {
      M5.IMU.getAccelData(&accX, &accY, &accZ); //, &accY, &accZ);
      Accel_XY.x = firx.processReading(accX);
      //    Accel_XY.x = ++z;
      Accel_XY.y = firy.processReading(accY);
      //    Accel_XY.y = ++z;
    }
    esp_err_t result = esp_now_send(RecvAddress, (uint8_t *) &Accel_XY, sizeof(Accel_XY));
    i = 0;
  }
}
