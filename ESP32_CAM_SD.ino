//========================================Including the libraries
#include "esp_camera.h"        //--> Camera ESP
#include "Arduino.h"
#include "FS.h"                //--> SD Card ESP32
#include "SD_MMC.h"            //--> SD Card ESP32
#include "SPI.h"
#include "soc/soc.h"           //--> Disable brownour problems
#include "soc/rtc_cntl_reg.h"  //--> Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            //--> read and write from flash memory
//========================================

//========================================define the number of bytes you want to access
#define EEPROM_SIZE 1
//========================================

//========================================Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
//========================================

int pictureNumber = 0; //--> Variable to hold photo naming sequence data from EEPROM.

//________________________________________________________________________________void setup()
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //--> disable brownout detector
 
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  //Serial.println();

  //----------------------------------------Camera configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
   
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  //----------------------------------------
  
  //----------------------------------------Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  //----------------------------------------

  //----------------------------------------Start accessing and checking SD card
 
  Serial.println("Starting SD Card");
  delay(250);
  
  //******4-bit mode
//  Serial.println("Start accessing SD Card 4-bit mode");
//  if(!SD_MMC.begin()){
//    Serial.println("SD Card Mount Failed");
//    return;
//  }
//  Serial.println("Started accessing SD Card 4-bit mode successfully");
  //******

  
  //******1-bit mode
  pinMode(13, INPUT_PULLUP); //--> This is done to resolve an "error" in 1-bit mode when SD_MMC.begin("/sdcard", true). 
  
  Serial.println("Start accessing SD Card 1-bit mode");
  if(!SD_MMC.begin("/sdcard", true)){
    Serial.println("SD Card Mount Failed");
    return;
  }
  Serial.println("Started accessing SD Card 1-bit mode successfully");

  pinMode(13, INPUT_PULLDOWN);
  //******
  //----------------------------------------

  //----------------------------------------Checking SD card type
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }
  //----------------------------------------

  //----------------------------------------Turning on the LED Flash on the ESP32 Cam Board
  // The line of code to turn on the LED Flash is placed here, because the next line of code is the process of taking photos or pictures by the ESP32 Cam.
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  delay(1000);
  //----------------------------------------

}

void loop() {
  
  camera_fb_t * fb = NULL;
  
  //----------------------------------------Take Picture with Camera
  if(digitalRead(13) == 0){
    digitalWrite(4, HIGH);
    delay(100);
  //----------------------------------------Take Picture with Camera
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  //----------------------------------------

  digitalWrite(4, LOW); //--> Turn off the LED Flash because the process of taking pictures and saving images to the SD card is complete.
  //----------------------------------------initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  delay(50);
  pictureNumber = EEPROM.read(0) + 1;
  delay(50);
  //----------------------------------------

  //----------------------------------------Save images to MicroSD Card.
  String path = "/picture" + String(pictureNumber) +".jpg"; //--> Path where new picture will be saved in SD Card

  fs::FS &fs = SD_MMC; 
  Serial.printf("Picture file name: %s\n", path.c_str());
  
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", path.c_str());
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
  }
  file.close();
  //----------------------------------------
}
}
