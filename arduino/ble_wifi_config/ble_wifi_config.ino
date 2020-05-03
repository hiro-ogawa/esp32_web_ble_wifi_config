#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <WiFi.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_1 "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_2 "beb5483f-36e1-4688-b7f5-ea07361b26a8"
#define BLE_DEVICE_NAME  "MyESP32"

#define WIFI_MAX_COUNTER  30

std::string wifi_ssid;
std::string wifi_password;
std::string wifi_mask("this is the wifi mask.");

bool wifi_entered;

class MyCallbacks1: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      wifi_ssid = pCharacteristic->getValue();
      Serial.print("SSID:");
      Serial.println(wifi_ssid.c_str());
    }
};

class MyCallbacks2: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      wifi_password = pCharacteristic->getValue();
//      Serial.print("Password:");
//      Serial.println(wifi_password.c_str());

      wifi_entered = true;
    }
};

IPAddress wifi_connect_with_ble(std::string ssid, std::string password){
  wifi_ssid = ssid;
  wifi_password = password;

  BLEDevice::init(BLE_DEVICE_NAME);
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic1 = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_1,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  BLECharacteristic *pCharacteristic2 = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_2,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic1->setCallbacks(new MyCallbacks1());
  pCharacteristic1->setValue("Input SSID");
  pCharacteristic2->setCallbacks(new MyCallbacks2());
  pCharacteristic2->setValue("Input Password");

  BLEAdvertising *pAdvertising = pServer->getAdvertising();

  int wifi_counter;
  do{
    wifi_counter = WIFI_MAX_COUNTER;
    wifi_entered = false;

    if(wifi_ssid == "" || wifi_password == "" ){
      Serial.println("");
      Serial.println("Waiting Wifi password with BLE...");
      pService->start();
      pAdvertising->start();
      while(!wifi_entered){
        delay(1000);
        Serial.print(".");
      }
      pAdvertising->stop();
      pService->stop();

      Serial.println("");
    }

    for( int i = 0 ; i < wifi_password.length() && i < wifi_mask.length() ; i++ )
      wifi_password[i] ^= wifi_mask[i];

    Serial.println("Connecting to Wifi AP...");
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");

      wifi_counter--;
      if( wifi_counter <= 0 )
        break;
    }
    wifi_password = "";
  }while( wifi_counter <= 0 );

  Serial.println("");

  return WiFi.localIP();
}

void setup() {
  Serial.begin(115200);

  IPAddress ipaddress = wifi_connect_with_ble("", "");
  Serial.print("Connected : ");
  Serial.println(ipaddress);
}


void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("loop() called");
  delay(2000);
}
