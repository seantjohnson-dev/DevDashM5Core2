#include "SensorDashboard.h"
#include <M5Core2.h>
#include <lvgl.h>

// --- Simple “card” style helpers ---
static lv_style_t style_card;
static lv_style_t style_title;
static lv_style_t style_value;

static lv_obj_t* lbl_accel;
static lv_obj_t* lbl_gyro;
static lv_obj_t* lbl_imu_temp;

static lv_obj_t* lbl_axp_temp;
static lv_obj_t* lbl_bat_v;
static lv_obj_t* lbl_bat_i;
static lv_obj_t* lbl_bat_p;

static lv_obj_t* lbl_touch;

static void make_styles() {
  lv_style_init(&style_card);
  lv_style_set_radius(&style_card, 16);
  lv_style_set_pad_all(&style_card, 12);
  lv_style_set_pad_row(&style_card, 6);
  lv_style_set_pad_column(&style_card, 8);
  lv_style_set_bg_opa(&style_card, LV_OPA_100);
  lv_style_set_bg_color(&style_card, lv_color_white());
  lv_style_set_border_color(&style_card, lv_palette_main(LV_PALETTE_GREY));
  lv_style_set_border_width(&style_card, 1);
  lv_style_set_shadow_width(&style_card, 10);
  lv_style_set_shadow_ofs_y(&style_card, 4);

  lv_style_init(&style_title);
  lv_style_set_text_font(&style_title, &lv_font_montserrat_18);
  lv_style_set_text_color(&style_title, lv_palette_darken(LV_PALETTE_GREY, 2));

  lv_style_init(&style_value);
  lv_style_set_text_font(&style_value, &lv_font_montserrat_18);
  lv_style_set_text_color(&style_value, lv_color_black());
}

static lv_obj_t* make_card(lv_obj_t* parent, const char* title) {
  lv_obj_t* card = lv_obj_create(parent);
  lv_obj_add_style(card, &style_card, 0);
  lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_width(card, LV_PCT(100));

  lv_obj_t* lbl = lv_label_create(card);
  lv_obj_add_style(lbl, &style_title, 0);
  lv_label_set_text(lbl, title);

  return card;
}

static lv_obj_t* make_value_label(lv_obj_t* parent, const char* initial) {
  lv_obj_t* lab = lv_label_create(parent);
  lv_obj_add_style(lab, &style_value, 0);
  lv_label_set_text(lab, initial);
  return lab;
}

// --- Timer to pull sensor data and update the labels ---
static void sensor_timer_cb(lv_timer_t* t) {
  LV_UNUSED(t);

  // IMU
  float ax = 0, ay = 0, az = 0;
  M5.IMU.getAccelData(&ax, &ay, &az);
  lv_label_set_text_fmt(lbl_accel, "Accel:  X=%0.2f  Y=%0.2f  Z=%0.2f", ax, ay, az);

  float gx = 0, gy = 0, gz = 0;
  M5.IMU.getGyroData(&gx, &gy, &gz);
  lv_label_set_text_fmt(lbl_gyro,  "Gyro:   X=%0.2f  Y=%0.2f  Z=%0.2f", gx, gy, gz);

  float temp = 0;
  M5.IMU.getTempData(&temp);
  lv_label_set_text_fmt(lbl_imu_temp, "IMU Temp: %0.2f °C", temp);

  // Power & Battery (AXP192)
  float axpTemp = M5.Axp.GetTempInAXP192();
  lv_label_set_text_fmt(lbl_axp_temp, "Power Temp: %0.2f °C", axpTemp);

  float batV = M5.Axp.GetBatVoltage();
  lv_label_set_text_fmt(lbl_bat_v, "Battery Voltage: %0.2f V", batV);

  float batI = M5.Axp.GetBatCurrent();
  lv_label_set_text_fmt(lbl_bat_i, "Battery Current: %0.2f mA", batI);

  float batP = M5.Axp.GetBatPower();
  lv_label_set_text_fmt(lbl_bat_p, "Battery Power: %0.2f mW", batP);

  // Touch
  TouchPoint_t p = M5.Touch.getPressPoint();
  // If not pressed, M5Core2 typically returns (-1, -1); display “—” nicely
  if (p.x >= 0 && p.y >= 0) {
    lv_label_set_text_fmt(lbl_touch, "Touch:  X=%d  Y=%d", p.x, p.y);
  } else {
    lv_label_set_text(lbl_touch, "Touch:  —");
  }
}

// --- Public setup you can call from your app ---
void create_sensor_dashboard_screen() {
  make_styles();

  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_100, 0);
  lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_all(scr, 12, 0);
  lv_obj_set_style_pad_row(scr, 12, 0);

  // Header
  lv_obj_t* header = lv_label_create(scr);
  lv_obj_add_style(header, &style_title, 0);
  lv_obj_set_style_text_font(header, &lv_font_montserrat_22, 0);
  lv_label_set_text(header, "Sensor Dashboard");

  // Card 1: IMU
  {
    lv_obj_t* card = make_card(scr, "IMU");
    lbl_accel   = make_value_label(card, "Accel:  X=0.00  Y=0.00  Z=0.00");
    lbl_gyro    = make_value_label(card, "Gyro:   X=0.00  Y=0.00  Z=0.00");
    lbl_imu_temp= make_value_label(card, "IMU Temp: 0.00 °C");
  }

  // Card 2: Power & Battery
  {
    lv_obj_t* card = make_card(scr, "Power & Battery");
    lbl_axp_temp = make_value_label(card, "Power Temp: 0.00 °C");
    lbl_bat_v    = make_value_label(card, "Battery Voltage: 0.00 V");
    lbl_bat_i    = make_value_label(card, "Battery Current: 0.00 mA");
    lbl_bat_p    = make_value_label(card, "Battery Power: 0.00 mW");
  }

  // Card 3: Touch
  {
    lv_obj_t* card = make_card(scr, "Touch");
    lbl_touch = make_value_label(card, "Touch:  —");
  }

  lv_scr_load(scr);

  // Update 10ms like your sketch, but 50–100ms is usually plenty smooth:
  lv_timer_create(sensor_timer_cb, 100, NULL);
}

bool SensorDashboard::begin() {
    // M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.IMU.Init();
    return true;
}

void SensorDashboard::loop() {
    float ax = 0, ay = 0, az = 0;
    M5.IMU.getAccelData(&ax, &ay, &az);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("Accel: X=%.2f, Y=%.2f, Z=%.2f\n", ax, ay, az);

    float gx = 0, gy = 0, gz = 0;
    M5.IMU.getGyroData(&gx, &gy, &gz);
    M5.Lcd.setCursor(0, 20);
    M5.Lcd.printf("Gyro: X=%.2f, Y=%.2f, Z=%.2f\n", gx, gy, gz);

    float temp = 0;
    M5.IMU.getTempData(&temp);
    M5.Lcd.setCursor(0, 40);
    M5.Lcd.printf("Temp: %.2f C\n", temp);

    float powTemp = M5.Axp.GetTempInAXP192();
    M5.Lcd.setCursor(0, 120);
    M5.Lcd.printf("Power Temp: %.2f C\n", powTemp);

    float batVolt = M5.Axp.GetBatVoltage();
    M5.Lcd.setCursor(0, 60);
    M5.Lcd.printf("Battery Voltage: %.2f V\n", batVolt);

    float batCurr = M5.Axp.GetBatCurrent();
    M5.Lcd.setCursor(0, 80);
    M5.Lcd.printf("Battery Current: %.2f mA\n", batCurr);

    float batPower = M5.Axp.GetBatPower();
    M5.Lcd.setCursor(0, 100);
    M5.Lcd.printf("Battery Power: %.2f mW\n", batPower);

    TouchPoint_t coordinate;
    coordinate = M5.Touch.getPressPoint();
    M5.Lcd.setCursor(0, 120);
    M5.Lcd.printf("Touch: X=%d, Y=%d\n", coordinate.x, coordinate.y);

    delay(10);
}

void SensorDashboard::destroy() {}