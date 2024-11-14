// Inspired by CameraWebServerExample

#define CAMERA_MODEL_AI_THINKER
#define CAMERA_MODEL_ESP32_CAM_BOARD

#include "esp_camera.h"
#include "camera_pins.h"
#include <base64.hpp> // I use this library for encoding in base64. https://github.com/Densaugeo/base64_arduino/


camera_fb_t * fb = NULL;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size   = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;  // for streaming
  config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location  = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count     = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    // Best option for face detection/recognition, TODO: Should tweak to adapt to computer vision
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);        // flip it back
    s->set_brightness(s, 1);   // up the brightness just a bit
    s->set_saturation(s, -2);  // lower the saturation
    // TODO: remove saturation
  }
  // drop down frame size for higher initial frame rate
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

  // Take Picture with Camera
  fb = esp_camera_fb_get();
  Serial.println((long long unsigned int)fb);
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
}



void loop() {

  size_t len = 0;
  unsigned char* bmp;
  if (frame2bmp(fb, &bmp, &len)) {
    if (Serial.availableForWrite()) {
      Serial.print("\nimg:");
      int sent = Serial.write(bmp, len);
      Serial.print("\nend");
      if (sent != len) {
        Serial.printf("Couldn't send entire image data ! ");
      }
    } else {
      Serial.println("Couldn't write to the Serial port !");
    }
  }
  else {
    Serial.printf("Error while transforming frame to bmp. Length: %d, pointer: %p \n", len, bmp);
  }

  free(bmp);
  esp_camera_fb_return(fb);

  delay(500000);
}
