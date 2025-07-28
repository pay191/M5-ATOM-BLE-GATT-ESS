#include <M5AtomS3.h>        // M5 Library for the Atom S3
#include <M5UnitENV.h>       // M5 ENV unit sensor driver (SHT4x and BMP280)
#include <BLEDevice.h>       // Core BLE functionality for ESP32
#include <BLEServer.h>       // BLE server class
#include <BLEUtils.h>        // Utility functions for BLE
#include <BLE2902.h>         // BLE descriptor for enabling notifications

// BLE GATT UUIDs for Environmental Sensing Service and its various attributes
#define SERVICE_UUID_ENV           BLEUUID((uint16_t)0x181A)  // Environmental Sensing Service
#define CHAR_UUID_TEMPERATURE      BLEUUID((uint16_t)0x2A6E)
#define CHAR_UUID_HUMIDITY         BLEUUID((uint16_t)0x2A6F)
#define CHAR_UUID_PRESSURE         BLEUUID((uint16_t)0x2A6D)
#define CHAR_UUID_ELEVATION        BLEUUID((uint16_t)0x2A6C)

BLECharacteristic* pTempChar;
BLECharacteristic* pHumChar;
BLECharacteristic* pPressChar;
BLECharacteristic* pElevChar;

// Create ENV sensor objects to hold ENV.IV Sensor data
SHT4X sht4;   // High-precision temperature/humidity sensor (SHT40)
BMP280 bmp;   // Pressure/temp sensor (not used in this sketch, but available)

float tempCorrectionOffset = -0.5; // We find this sensor runs a little hotter than reality
bool bleClientConnected = false;  // Track connection status

// Custom BLE callback handler for connect/disconnect
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    Serial.println("✅ BLE client connected");
    bleClientConnected = true;

    // Optional: Change LED to indicate active connection
    AtomS3.dis.drawpix(0x00FFFF); // Cyan for connected
    AtomS3.update();
  }

  void onDisconnect(BLEServer* pServer) override {
    Serial.println("❌ BLE client disconnected");
    bleClientConnected = false;

    // Optional: Change LED to disconnected state
    AtomS3.dis.drawpix(0xFF00FF); // Magenta for disconnected
    AtomS3.update();

    // Restart advertising to allow re-connection
    pServer->getAdvertising()->start();
  }
};

void setup() {

  delay(100);

  // Initialize M5 Atom S3 Lite hardware
  AtomS3.begin(true);  
  AtomS3.dis.setBrightness(100);
  Serial.begin(115200);

  // Show white LED during boot
  AtomS3.dis.drawpix(0xFFFFFF); 
  AtomS3.update();
  delay(1000);

  // Set up I2C on Port A (SDA=GPIO 2, SCL=GPIO 1)
  Wire.begin(2, 1, 400000);  // Use 400kHz for fast sensor communication

  // Initialize SHT4x sensor
  if (!sht4.begin(&Wire, SHT40_I2C_ADDR_44, 2, 1, 400000U)) {
    Serial.println("Couldn't find SHT40 Temperature Sensor on Port A");
    AtomS3.dis.drawpix(0xFF0000); // Red = error
    AtomS3.update();
    while (1) delay(1);  // Halt if sensor not found
  }

  sht4.setPrecision(SHT4X_HIGH_PRECISION);
  sht4.setHeater(SHT4X_NO_HEATER);

  // Initialize SHT4x sensor
  if (!bmp.begin(&Wire, BMP280_I2C_ADDR, 2, 1, 400000U)) {
      Serial.println("Couldn't find BMP280");
      AtomS3.dis.drawpix(0xFF0000); // Red = error
      AtomS3.update();
      while (1) delay(1);
  }


  // Initialize BLE
  String deviceName = generateDeviceName();
  Serial.println("BLE Device Name: " + deviceName);
  BLEDevice::init(deviceName.c_str());

  BLEDevice::init(generateDeviceName().c_str());
  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID_ENV);

  // Temperature
  pTempChar = pService->createCharacteristic(CHAR_UUID_TEMPERATURE, BLECharacteristic::PROPERTY_NOTIFY);
  pTempChar->addDescriptor(new BLE2902());

  // Humidity
  pHumChar = pService->createCharacteristic(CHAR_UUID_HUMIDITY, BLECharacteristic::PROPERTY_NOTIFY);
  pHumChar->addDescriptor(new BLE2902());

  // Pressure
  pPressChar = pService->createCharacteristic(CHAR_UUID_PRESSURE, BLECharacteristic::PROPERTY_NOTIFY);
  pPressChar->addDescriptor(new BLE2902());

  // Elevation
  pElevChar = pService->createCharacteristic(CHAR_UUID_ELEVATION, BLECharacteristic::PROPERTY_NOTIFY);
  pElevChar->addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID_ENV);
  pAdvertising->start();
  Serial.println("📡 BLE Environmental Sensing ready");

}

void loop() {

  // Update sensor and send data only if BLE client is connected
  if (bleClientConnected && sht4.update()) {

    float tempC = sht4.cTemp + tempCorrectionOffset;
    float humidity = sht4.humidity;

    bmp.update();
    float pressure = bmp.pressure;           // hPa
    float elevation = bmp.readAltitude();    // meters

    Serial.printf("🌡 Temp: %.2f °C  💧 Humidity: %.1f %%  🧭 Pressure: %.1f hPa  ⛰ Alt: %.1f m\n", 
                  tempC, humidity, pressure, elevation);

    // Send propery encoded values.
    pTempChar->setValue((uint8_t*)encodeIEEE11073Float(tempC).data(), 4);
    pTempChar->notify();

    pHumChar->setValue((uint8_t*)encodeHumidity(humidity).data(), 2);
    pHumChar->notify();

    pPressChar->setValue((uint8_t*)encodePressure(pressure).data(), 4);
    pPressChar->notify();

    pElevChar->setValue((uint8_t*)encodeElevation(elevation).data(), 2);
    pElevChar->notify();

    AtomS3.dis.drawpix(0x00AAFF);
    AtomS3.update();
    delay(500);

  }

  // LED Off for idle state to keep temperatures down.
  AtomS3.dis.drawpix(0x000000);
  AtomS3.update();

  delay(4500);  // Wait between readings
}


//Function to make a unique device name using the chip ID
String generateDeviceName() {
  uint64_t mac = ESP.getEfuseMac();  // Unique per ESP32
  uint32_t suffix = (uint32_t)(mac & 0xFFFFFF);  // Take last 3 bytes for brevity
  char deviceName[32];
  sprintf(deviceName, "HVL-BLE-ESS-%06X", suffix);  // E.g., HVL-BLE-ESS-071717
  return String(deviceName);
}

//Some functions to porpery encode our values - need to encode each characteristic according to the Bluetooth SIG specification for that UUID
std::string encodeIEEE11073Float(float value) {
  uint8_t buffer[4];
  int32_t mantissa = (int32_t)(value * 100);  // 2 decimal places
  int8_t exponent = -2;

  buffer[0] = mantissa & 0xFF;
  buffer[1] = (mantissa >> 8) & 0xFF;
  buffer[2] = (mantissa >> 16) & 0xFF;
  buffer[3] = exponent;

  return std::string((char*)buffer, 4);
}

std::string encodeHumidity(float humidityPercent) {
  uint16_t value = (uint16_t)(humidityPercent * 100.0);  // 0.01% units
  uint8_t buffer[2] = { (uint8_t)(value & 0xFF), (uint8_t)(value >> 8) };
  return std::string((char*)buffer, 2);
}

std::string encodePressure(float hPa) {
  uint32_t pascals = (uint32_t)(hPa * 100.0);  // hPa to Pa
  uint8_t buffer[4] = {
    (uint8_t)(pascals & 0xFF),
    (uint8_t)((pascals >> 8) & 0xFF),
    (uint8_t)((pascals >> 16) & 0xFF),
    (uint8_t)((pascals >> 24) & 0xFF)
  };
  return std::string((char*)buffer, 4);
}

std::string encodeElevation(float meters) {
  int16_t value = (int16_t)meters;
  uint8_t buffer[2] = { (uint8_t)(value & 0xFF), (uint8_t)(value >> 8) };
  return std::string((char*)buffer, 2);
}
