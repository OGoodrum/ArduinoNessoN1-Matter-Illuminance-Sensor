#include <Arduino_Nesso_N1.h>
#include <Matter.h>
#include <Wire.h>
#include <BH1750.h>
#include <esp_matter.h>
// Include WiFi.h only if you plan to use Matter over Wi-Fi


// --- Transport Layer Configuration ---
// To use Matter over Thread, include the three defines below.
// To use Matter over Wi-Fi, comment out or remove these three defines.
#define CONFIG_ENABLE_CHIPOBLE 1        // Enables BLE for commissioning
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD 1 // Enables the Thread stack
#define CHIP_DEVICE_CONFIG_ENABLE_WIFI 0   // CRITICAL: Disables the Wi-Fi stack
// -------------------------------------

// --- For Matter over Wi-Fi only ---
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
  #include <WiFi.h>
  const char* ssid = "IoTFactory";
  const char* password = "only4iot";
#endif
// ------------------------------------

NessoDisplay display; // Create a display instance
BH1750 lightMeter;// Light Sensor

// Create an On/Off Light Endpoint
MatterOnOffLight OnOffLight;
// esp-matter illuminance endpoint handles
uint16_t illuminance_endpoint_id = 0;


// This callback function is executed when a Matter controller sends a command
bool setLightOnOff(bool state) {
  Serial.printf("Received Matter command: Light %s\r\n", state ? "ON" : "OFF");
  
  // Control the built-in LED (inverted logic: LOW is ON)
  digitalWrite(LED_BUILTIN, state ? LOW : HIGH);

  display.fillScreen(TFT_BLACK);
  display.drawString(state ? "ON" : "OFF", display.width() / 2, display.height() / 2);
  
  return true; // Return true to confirm the command was successful
}

void displaySetup(){
  display.begin();
  display.setRotation(1); // Set to landscape mode

  // Set text properties
  display.setTextDatum(MC_DATUM); // Middle-Center datum for text alignment
  display.setTextColor(TFT_WHITE, TFT_BLACK); // White text, black background
  display.setTextSize(10);

  // Clear the screen and draw the string
  display.fillScreen(TFT_BLACK);
}

void sensorSetup(){
  Wire.begin();
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("BH1750 initialized.");
  } else {
    Serial.println("BH1750 not found! Check wiring.");
  }
}

void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Start with the LED off
  Serial.begin(115200);
  displaySetup();
  sensorSetup();

  // initialize the pushbutton pin as an input:
  pinMode(KEY1, INPUT);

  delay(1000);

  while(!Serial);

  // --- For Matter over Wi-Fi only ---
  #if (!CHIP_DEVICE_CONFIG_ENABLE_THREAD)
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println(" Connected");
  #endif
  // ------------------------------------

  // Initialize the OnOffLight endpoint with an initial state of OFF
  OnOffLight.begin(false);
  // Attach the callback function to handle state changes
  OnOffLight.onChange(setLightOnOff);
  
  // Create illuminance endpoint via esp-matter API
  esp_matter::node_t *node = esp_matter::node::get();
  esp_matter::endpoint::light_sensor::config_t endpoint_config;
  esp_matter::endpoint_t *ep = esp_matter::endpoint::light_sensor::create(node, &endpoint_config, ENDPOINT_FLAG_NONE, NULL);
  illuminance_endpoint_id = esp_matter::endpoint::get_id(ep);

  // Start the Matter service.
  Matter.begin();
  

  
  
  // If the device was already commissioned, sync its LED state on boot
  if (Matter.isDeviceCommissioned()) {
    Serial.println("Matter Node is already commissioned. Ready for use.");
    setLightOnOff(OnOffLight.getOnOff());
  }
}

void loop() {
  if (digitalRead(KEY1) == LOW) {
    Serial.println("Factory resetting Matter...");
    Matter.decommission(); // clears all commissioning data
  }


  // This block periodically prints the pairing information until the device is commissioned
  if (!Matter.isDeviceCommissioned()) {
    static unsigned long lastPrintTime = 0;
    if (millis() - lastPrintTime > 5000) {
      display.drawString("Pair", display.width() / 2, display.height() / 2);
      Serial.println("\n----------------------------------------------------");
      Serial.println("Matter Node not commissioned. Waiting for pairing...");
      Serial.printf("Pairing QR code: %s\r\n", Matter.getOnboardingQRCodeUrl().c_str());
      Serial.printf("Manual pairing code: %s\r\n", Matter.getManualPairingCode().c_str());
      Serial.println("----------------------------------------------------");
      lastPrintTime = millis();
    }
  }

  if (lightMeter.measurementReady()) {
    float lux = lightMeter.readLightLevel();
    Serial.printf("Light Sensor: %.1f lux\n", lux);
    

    // Matter illuminance = 10000 * log10(lux) + 1 (per spec)
    uint16_t matter_lux = (lux > 0) ? (uint16_t)(10000.0f * log10f(lux) + 1) : 0;

    esp_matter_attr_val_t val = esp_matter_uint16(matter_lux);
    esp_matter::attribute::update(illuminance_endpoint_id,
      chip::app::Clusters::IlluminanceMeasurement::Id,
      chip::app::Clusters::IlluminanceMeasurement::Attributes::MeasuredValue::Id,
      &val);
  }
  

  delay(5000);
}