#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "SensorQMI8658.hpp"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include "lv_conf.h"
#include "demos/lv_demos.h"
#include "pins_arduino.h"

#define USE_WIRE

#ifndef SENSOR_SDA
#define SENSOR_SDA 33
#endif

#ifndef SENSOR_SCL
#define SENSOR_SCL 32
#endif

#ifndef SENSOR_IRQ
#define SENSOR_IRQ 35
#endif

#define IMU_CS 5

#define EXAMPLE_LVGL_TICK_PERIOD_MS 2

/* Change to your screen resolution */
static const uint16_t screenWidth = 240;
static const uint16_t screenHeight = 280;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 10];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */

SensorQMI8658 qmi;

IMUdata acc;
IMUdata gyr;

lv_obj_t *label;  // Global label object
lv_obj_t *chart;  // Global chart object
lv_chart_series_t *acc_series_x;  // Acceleration X series
lv_chart_series_t *acc_series_y;  // Acceleration Y series
lv_chart_series_t *acc_series_z;  // Acceleration Z series

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char *buf) {
  Serial.printf(buf);
  Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp_drv);
}

void example_increase_lvgl_tick(void *arg) {
  /* Tell LVGL how many milliseconds has elapsed */
  lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

static uint8_t count = 0;
void example_increase_reboot(void *arg) {
  count++;
  if (count == 30) {
    esp_restart();
  }
}

void setup() {
  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  Serial.println(LVGL_Arduino);
  Serial.println("I am LVGL_Arduino");

  lv_init();

#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  tft.begin();        /* TFT init */
  tft.setRotation(0); /* Landscape orientation, flipped */

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 10);

  /* Initialize the display */
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /* Change the following line to your display resolution */
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  const esp_timer_create_args_t lvgl_tick_timer_args = {
    .callback = &example_increase_lvgl_tick,
    .name = "lvgl_tick"
  };

  const esp_timer_create_args_t reboot_timer_args = {
    .callback = &example_increase_reboot,
    .name = "reboot"
  };

  esp_timer_handle_t lvgl_tick_timer = NULL;
  esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
  esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000);

  label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "Initializing...");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  /* Create chart */
  chart = lv_chart_create(lv_scr_act());
  lv_obj_set_size(chart, 240, 280);
  lv_obj_align(chart, LV_ALIGN_CENTER, 0, 0);
  lv_chart_set_type(chart, LV_CHART_TYPE_LINE); /* Set the type to line */
  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -3, 3); /* Set the range of y axis */
  lv_chart_set_point_count(chart, 20); /* Set the number of data points */
  acc_series_x = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
  acc_series_y = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);
  acc_series_z = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);

  Serial.println("Setup done");

#ifdef USE_WIRE
  // Using WIRE
  if (!qmi.begin(Wire, QMI8658_L_SLAVE_ADDRESS, SENSOR_SDA, SENSOR_SCL)) {
    printf("Failed to find QMI8658 - check your wiring!\n");
    while (1) {
      delay(1000);
    }
  }
#else
  if (!qmi.begin(IMU_CS)) {
    printf("Failed to find QMI8658 - check your wiring!\n");
    while (1) {
      delay(1000);
    }
  }
#endif

  /* Get chip id */
  printf("Device ID: 0x%X\n", qmi.getChipID());

  qmi.configAccelerometer(
    SensorQMI8658::ACC_RANGE_4G,
    SensorQMI8658::ACC_ODR_1000Hz,
    SensorQMI8658::LPF_MODE_0,
    true);

  qmi.configGyroscope(
    SensorQMI8658::GYR_RANGE_64DPS,
    SensorQMI8658::GYR_ODR_896_8Hz,
    SensorQMI8658::LPF_MODE_3,
    true);

  qmi.enableGyroscope();
  qmi.enableAccelerometer();

  qmi.dumpCtrlRegister();

  printf("Read data now...\n");
}

void loop() {
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);

  if (qmi.getDataReady()) {
    if (qmi.getAccelerometer(acc.x, acc.y, acc.z)) {
      printf("%.2f, %.2f, %.2f\n", acc.x, acc.y, acc.z);

      // Update chart with new accelerometer data
      lv_chart_set_next_value(chart, acc_series_x, acc.x);
      lv_chart_set_next_value(chart, acc_series_y, acc.y);
      lv_chart_set_next_value(chart, acc_series_z, acc.z);
    }

    if (qmi.getGyroscope(gyr.x, gyr.y, gyr.z)) {
      printf("%.2f, %.2f, %.2f\n", gyr.x, gyr.y, gyr.z);
    }
  }
  delay(20); // Increase the frequency of data polling
}
