/*
 * Blink
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */

#include "Arduino.h"

#include <SPI.h>
//#include <FS.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "LITTLEFS.h"

#define WIRE Wire
// Задаем переменные
static int cnt_сheck = 0, cnt_сheck_old = 0, cnt_сheck_save = 0;
int cnt_cur = 0, cnt_save = 0;
int plus_old = 0, plus_cur = 1;
int minu_old = 0, minu_cur = 1;
int save_old = 0, save_cur = 1;

const char *save_path = "/f.txt";
File save_file;

unsigned long save_pres = 0, save_rlse = 0, save_drtn = 0;
unsigned long save_p_pres = 0, save_p_rlse = 0, save_p_drtn = 0;
// отмечаем пины для кнопок
static int btn_plus = 12;
static int btn_minu = 13;
static int btn_save = 14;

/*#define BUTTON_A 12
#define BUTTON_B 13
#define BUTTON_C 14*/

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &WIRE);

void setup()
{
  // initialize LED digital pin as an output.
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(btn_plus, INPUT_PULLUP);
  pinMode(btn_minu, INPUT_PULLUP);
  pinMode(btn_save, INPUT_PULLUP);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  // display.display();
  // delay(1000);

  // text display tests
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Starting system");
  display.display();

  display.setTextColor(SSD1306_WHITE);

  digitalWrite(LED_BUILTIN, HIGH);
  // инициируем файловую систему
  if (LittleFS.begin())
  {
    Serial.println("LittleFS Initialize....ok");
  }
  else
  {
    Serial.println("LittleFS Initialization...failed");
  }

  if (!LittleFS.exists(save_path))
  {
    if (LittleFS.format())
    {
      Serial.println("LittleFS Format....ok");
    }
    else
    {
      Serial.println("LittleFS Format....failed");
    }
  }
  else
  {
    Serial.println("LittleFS File exist....ok");
  }
  save_file = LittleFS.open(save_path, "r");
  if (!save_file)
  {
    Serial.println("LittleFS file not create");
  }
  else
  {
    cnt_сheck = save_file.readString().toInt();
    Serial.println("LittleFS file read....ok");
    Serial.print("Read value = ");
    Serial.println(cnt_сheck);
    cnt_сheck_save = cnt_сheck;
  }
  save_file.close();
  // Clear the buffer.
  display.clearDisplay();
  display.display();
  display.setTextSize(5);
  display.setCursor(0, 0);
  display.println(cnt_сheck);
  display.display(); // actually display all of the above
}
// Мигаем светодиодом о действии
void ledBlink()
{
  // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED_BUILTIN, HIGH);
  // wait for a second
  delay(500);
  // turn the LED off by making the voltage LOW
  digitalWrite(LED_BUILTIN, LOW);
  // wait for a second
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
  // wait for a second
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
}

// Сохраняем значение в файл
void saveValues(int save_value)
{
  Serial.print("LittleFS save value = ");
  Serial.println(save_value);
  save_file = LittleFS.open(save_path, "a+");
  if (!save_file)
  {
    Serial.println("LittleFS file not create");
  }
  save_file.truncate(0);
  save_file.println(save_value);
  save_file.close();
}

void loop()
{
  cnt_cur = 0;
  plus_cur = !digitalRead(btn_plus);
  minu_cur = !digitalRead(btn_minu);
  save_cur = !digitalRead(btn_save);
  // Нажали и отпустили кнопку увелечения счетчика
  if (plus_cur == 0 && plus_old == 1)
  {
    Serial.println("plus");
    cnt_cur = 1;
  }
  // Нажали и отпустили кнопку уменьшения счетчика
  if (minu_cur == 0 && minu_old == 1)
  {
    Serial.println("minus");
    cnt_cur = -1;
  }
  if (save_old == 0 && save_cur == 1)
  {
    // нажали кнопку сохранения
    save_pres = millis();
  }
  if (save_cur == 1 && save_old == 1)
  {
    save_rlse = millis();
    save_drtn = save_rlse - save_pres;
    if (save_drtn > 3000)
    {
      if (cnt_сheck != cnt_сheck_save)
      {
        saveValues(cnt_сheck);
        cnt_сheck_save = cnt_сheck;
        ledBlink();
      }
      // save_pres = 0;
    }
    if (save_drtn > 15000)
    {
      if (cnt_сheck != 0)
      {
        saveValues(0);
        cnt_сheck_save = 0;
        cnt_сheck = 0;
        ledBlink();
      }
    }
  }
  plus_old = plus_cur;
  minu_old = minu_cur;
  save_old = save_cur;
  cnt_сheck += cnt_cur;

  if (cnt_сheck != cnt_сheck_old)
  {
    display.clearDisplay();
    display.display();

    display.setCursor(0, 0);
    display.println(cnt_сheck);
    display.display();
    cnt_сheck_old = cnt_сheck;

    save_p_pres = 0;
  }
  else if (save_p_pres == 0 && cnt_сheck != cnt_сheck_save)
  {
    save_p_pres = millis();
  }

  if (save_p_pres != 0)
  {
    save_p_rlse = millis();
    save_p_drtn = save_p_rlse - save_p_pres;
    if (save_p_drtn > 15000)
    {
      if (cnt_сheck != cnt_сheck_save)
      {
        saveValues(cnt_сheck);
        cnt_сheck_save = cnt_сheck;
        ledBlink();
        save_p_pres = 0;
      }
    }
  }

  if (cnt_сheck == cnt_сheck_save)
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
