#include <esp_now.h>
#include <WiFi.h>

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
int i = 1;
byte flag;

typedef struct Data
{
  int node;
  float x;
  float y;
} Data;

Data Accel_XY_1[2000];
Data Accel_XY_2[2000];
Data Accel_XY_3[2000];
Data Accel_XY_4[1100];

Data *Qptr_1 = Accel_XY_1;
Data *Qptr_2 = Accel_XY_2;
Data *Qptr_3 = Accel_XY_3;
Data *Qptr_4 = Accel_XY_4;

Data *Prev_1 = Accel_XY_1;
Data *Prev_2 = Accel_XY_2;
Data *Prev_3 = Accel_XY_3;
Data *Prev_4 = Accel_XY_4;

Data *A_Limit1 = Accel_XY_1;
Data *A_Limit2 = Accel_XY_2;
Data *A_Limit3 = Accel_XY_3;
Data *A_Limit4 = Accel_XY_4;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  return;
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)
{
  flag = 1;
  switch (*incomingData)
  {
    case 1:
      memcpy(Qptr_1, incomingData, sizeof(Data));
      ++Qptr_1;
      if (Qptr_1 > A_Limit1)
        Qptr_1 = Accel_XY_1;
      break;

    case 2:
      memcpy(Qptr_2, incomingData, sizeof(Data));
      ++Qptr_2;
      if (Qptr_2 > A_Limit2)
        Qptr_2 = Accel_XY_2;
      break;

    case 3:
      memcpy(Qptr_3, incomingData, sizeof(Data));
      ++Qptr_3;
      if (Qptr_3 > A_Limit3)
        Qptr_3 = Accel_XY_3;
      break;

    case 4:
      memcpy(Qptr_4, incomingData, sizeof(Data));
      ++Qptr_4;
      if (Qptr_4 > Accel_XY_4)
        Qptr_4 = Accel_XY_4;
      break;
  }
}

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);

  pinMode(5, OUTPUT);

  disableCore0WDT();
  disableCore1WDT();
}

void loop()
{
  while (1) {
    digitalWrite(5, LOW);
    if (i <= 4)
    {
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &i, sizeof(i));
      delayMicroseconds(800);
      ++i;
    }
    else
      i = 1;

    if (flag)
    {
      Serial.print(Prev_1->x, 3);
      Serial.print(" ");
      Serial.print(Prev_1->y, 3);
      Serial.print(" ");
      Serial.print(Prev_2->x, 3);
      Serial.print(" ");
      Serial.print(Prev_2->y, 3);
      Serial.print(" ");
      Serial.print(Prev_3->x, 3);
      Serial.print(" ");
      Serial.print(Prev_3->y, 3);
      Serial.print(" ");
      Serial.print(Prev_4->x, 3);
      Serial.print(" ");
      Serial.println(Prev_4->y, 3);

      Prev_1++;
      Prev_2++;
      Prev_3++;
      Prev_4++;
      flag = 0;
    }

    if (Prev_1 > A_Limit1)
      Prev_1 = Accel_XY_1;

    if (Prev_2 > A_Limit2)
      Prev_2 = Accel_XY_2;

    if (Prev_3 > A_Limit3)
      Prev_3 = Accel_XY_3;

    if (Prev_4 > A_Limit4)
      Prev_4 = Accel_XY_4;
    digitalWrite(5, HIGH);
  };
}
