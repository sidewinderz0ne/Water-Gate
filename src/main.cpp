#include <Arduino.h>

//--------------SETTINGAN ALAT
//--------------------------------------------------
//SETTING WAKTU
const long alarmTime = 10000; //Setting waktu logging
int buzzerDuration = 1001;   //Mili Detik
int buzzerSound = 2345;      //KHz
long alarm;
//--------------------------------------------------
//SETTING SENSOR
int jarakSensorKeTanah = 240;
int gateClosed = 200;
int gateOpened = 50;
int nilaiMaxGrafik = 200;
//--------------------------------------------------
//SETTING PIN
#define buzzer 30
//sensor air 1
#define trigPin1 22
#define echoPin1 23
//sensor air 2
#define trigPin2 24
#define echoPin2 25
//sensor pintu
#define trigPin3 26
#define echoPin3 27
//relay kontrol pintu
#define relayUp 28
#define relayDn 29
//--------------------------------------------------

#include <Wire.h>

#include <Filter.h>
ExponentialFilter<float> H1(20, 0), H2(20, 0);
int gIn[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int gOu[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int gFI[13] = {62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62};
int gFO[13] = {62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62};

#include <RTClib.h>
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};

#include <SPI.h> // f.k. for Arduino-1.5.2
#define USE_SDFAT
#include <SdFat.h> // Use the SdFat library
SdFatSoftSpi<12, 11, 13> SD; //Bit-Bang on the Shield pins
#define SD_CS 10
#include <Adafruit_GFX.h> // Hardware-specific library
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
/* some RGB color definitions                                                 */
#define Black 0x0000       /*   0,   0,   0 */
#define Navy 0x000F        /*   0,   0, 128 */
#define DarkGreen 0x03E0   /*   0, 128,   0 */
#define DarkCyan 0x03EF    /*   0, 128, 128 */
#define Maroon 0x7800      /* 128,   0,   0 */
#define Purple 0x780F      /* 128,   0, 128 */
#define Olive 0x7BE0       /* 128, 128,   0 */
#define LightGrey 0xC618   /* 192, 192, 192 */
#define DarkGrey 0x7BEF    /* 128, 128, 128 */
#define Blue 0x001F        /*   0,   0, 255 */
#define Green 0x07E0       /*   0, 255,   0 */
#define Cyan 0x07FF        /*   0, 255, 255 */
#define Red 0xF800         /* 255,   0,   0 */
#define Magenta 0xF81F     /* 255,   0, 255 */
#define Yellow 0xFFE0      /* 255, 255,   0 */
#define White 0xFFFF       /* 255, 255, 255 */
#define Orange 0xFD20      /* 255, 165,   0 */
#define GreenYellow 0xAFE5 /* 173, 255,  47 */
#define WarnaBgGrfKanan 0x002E /* 173, 255,  47 */
#define WarnaBgGrfKiri 0x0164 /* 173, 255,  47 */
#define Pink 0xF81F
//#define NAMEMATCH ""         // "" matches any name
#define NAMEMATCH "logo_srs" // *tiger*.bmp
#define PALETTEDEPTH 8       // support 256-colour Palette
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
//char namebuf[32] = "/";   //BMP files in root directory
char namebuf[32] = "/Bitmap/"; //BMP directory e.g. files in /bitmaps/*.bmp
File root;
int pathlen;
File f;

//Function
//------------------------------------------------------------------
uint8_t showBMP(char *nm, int x, int y);
void logging();
//-----------------------------------------------------------------

void buuuzzz(void)
{
  tone(buzzer, buzzerSound);
  delay(buzzerDuration);
  noTone(buzzer);
}

void logging(void)
{
  alarm = alarm + alarmTime;                       //KALIBERASI WAKTU
  long duration3, distance3;                       //VARIABLE SENSOR PINTU WATER GATE
  long duration1, distance1, duration2, distance2; //VIARABEL SENSOR AIR

  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  duration1 = pulseIn(echoPin1, HIGH);
  distance1 = jarakSensorKeTanah - ((duration1 / 2) / 29.1);
  tft.fillRect(0, 129, 160, 41, WarnaBgGrfKiri);
  tft.setFont(&FreeSansBold24pt7b);
  tft.setCursor(10, 166);
  tft.print(distance1);
  tft.print("cm");
  H1.Filter(distance1);

  for (int i = 13; i > 1; i--) //LOOP NGOPY NILAI SEBELUMNYA
  {
    gIn[i] = gIn[i - 1];
  }
  /* gIn[1] = rand() % 200 + 1; */
  gIn[1] = H1.Current(); //MASUKAN HAIL FILTER NILAI PERTAMA

  for (int i = 1; i < 14; i++) //BIKIN GRAFIK
  {
    if (gIn[i] >= nilaiMaxGrafik)
    {
      gFI[i] = 62;
    }
    else if (gIn[i] <= 0)
    {
      gFI[i] = 125;
    }
    else
    {
      int a = 60 * (gIn[i] - nilaiMaxGrafik) / (nilaiMaxGrafik * -1);
      gFI[i] = 62 + a;
      //GANTI RUMUS
    }
    tft.fillRect(10 + (10 * i), 62, 10, 68, WarnaBgGrfKiri);
    tft.fillRect(10 + (10 * i), gFI[i], 3, 3, Yellow);
    if (i != 1)
    {
      tft.drawLine(10 + (10 * i), gFI[i], 10 + (10 * (i - 1)), gFI[(i - 1)], Green);
    }
  }

  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  duration2 = pulseIn(echoPin2, HIGH);
  distance2 = jarakSensorKeTanah - ((duration2 / 2) / 29.1);
  tft.fillRect(160, 129, 320, 41, WarnaBgGrfKanan);
  tft.setCursor(170, 166);
  tft.print(distance2);
  tft.print("cm");
  H2.Filter(distance2);

  for (int i = 13; i > 1; i--) //LOOP NGOPY NILAI SEBELUMNYA
  {
    gOu[i] = gOu[i - 1];
  }

  /* gOu[1] = rand() % 200 + 1; */
  gOu[1] = H2.Current(); //MASUKAN HAIL FILTER NILAI PERTAMA

  for (int i = 1; i < 14; i++) //BIKIN GRAFIK
  {
    if (gOu[i] >= nilaiMaxGrafik)
    {
      gFO[i] = 62;
    }
    else if (gOu[i] <= 0)
    {
      gFO[i] = 125;
    }
    else
    {
      int a = 60 * (gOu[i] - nilaiMaxGrafik) / (nilaiMaxGrafik * -1);
      gFO[i] = 62 + a;
      //GANTI RUMUS
    }
    tft.fillRect(170 + (10 * i), 62, 10, 68, WarnaBgGrfKanan);
    tft.fillRect(170 + (10 * i), gFO[i], 3, 3, Green);
    if (i != 1)
    {
      tft.drawLine(170 + (10 * i), gFO[i], 170 + (10 * (i - 1)), gFO[(i - 1)], Yellow);
    }
  }

  digitalWrite(trigPin3, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin3, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin3, LOW);
  duration3 = pulseIn(echoPin3, HIGH);
  distance3 = (duration3 / 3) / 29.1;

  if (distance1 > distance2) //ATUR PINTU AIR DENGAN 2 RELAY
  {
    digitalWrite(relayUp, LOW);
    digitalWrite(relayDn, HIGH);
  }
  else
  {
    digitalWrite(relayUp, HIGH);
    digitalWrite(relayDn, LOW);
  }

  //PRINT STATUS PINTU KE LCD
  tft.fillRect(205, 205, 220, 50, Black);
  tft.setFont(&FreeSans9pt7b);
  if (distance3 >= gateClosed)
  {
    tft.setCursor(210, 226);
    tft.print("Closed");
  }
  else if (distance3 >= gateOpened && distance3 <= gateClosed)
  {
    tft.setCursor(210, 226);
    tft.print("Mid");
  }
  else if (distance3 <= gateOpened)
  {
    tft.setCursor(210, 226);
    tft.print("Opened");
  }

  //SIMPAN KE MICRO SD CARD
  f = SD.open("hasil.txt", FILE_WRITE);
  if (f)
  {
    f.print(rtc.now().day(), DEC);
    f.print("/");
    f.print(rtc.now().month(), DEC);
    f.print("/");
    f.print(rtc.now().year(), DEC);
    f.print(",");
    f.print(rtc.now().hour(), DEC);
    f.print(":");
    f.print(rtc.now().minute(), DEC);
    f.print(":");
    f.print(rtc.now().second(), DEC);
    f.print(",");
    f.print(H1.Current());
    f.print(",");
    f.print(H2.Current());
    f.print(",");
    f.print(distance3);
    f.print(",");
    f.println();
    f.close(); // close the file
    buuuzzz();    //BUZZ KALO OKE
    Serial.println("sd tulis berhasil");
    tft.fillRect(0, 0, 20, 20, Green);
  }
  else
  {
    //KALO ERROR
    tft.fillRect(0, 0, 20, 20, Red);
  }
}

void setup()
{
  uint16_t ID;
  Serial.begin(9600);

  //SETTING AWAL BUZZER
  pinMode(buzzer, OUTPUT);
  buuuzzz(); //TES BUZZER

  alarm = alarmTime; //SETTING ALARM KE NILAI AWAL

  //SETTING AWAL RTC
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  if (rtc.lostPower())
  {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //SET INPUT OUTPUT PIN
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);
  pinMode(relayUp, OUTPUT);
  pinMode(relayDn, OUTPUT);

  //SETTING AWAL LCD
  Serial.print("Show BMP files on TFT with ID:0x");
  ID = tft.readID();
  Serial.println(ID, HEX);
  if (ID == 0x0D3D3)
    ID = 0x9481;
  tft.begin(ID);
  tft.setRotation(1);

  //SETTING AWAL SD CARD
  bool good = SD.begin(SD_CS);
  if (!good)
  {
    Serial.print(F("cannot start SD"));
    while (1)
      ;
  }
  else
  {
    Serial.println(F("SD is OK!"));
  }
  root = SD.open(namebuf);
  pathlen = strlen(namebuf);
  tft.fillRect(0, 0, 320, 240, Black);
  char name[32] = "/Bitmap/logo1.bmp";
  showBMP(name, 10, 174);

  tft.fillRect(0, 25, 160, 150, WarnaBgGrfKiri);
  tft.fillRect(160, 25, 320, 150, WarnaBgGrfKanan);

  tft.setCursor(38, 15);
  tft.setFont(&FreeSans12pt7b);
  tft.print("Automatic Water Gate");

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(56, 55);
  tft.print("LV IN:");

  tft.setCursor(201, 55);
  tft.print("LV OUT:");

  tft.setFont(&FreeSansBold24pt7b);
  Serial.println("LCD OK");

  //base graph
  tft.fillRect(15, 62, 130, 63, WarnaBgGrfKiri);
  tft.fillRect(175, 62, 130, 63, WarnaBgGrfKanan);
  logging(); //LOGGING AWAL SETUP
}

void loop()
{
  Serial.println(" . ");

  //NGUKUR UNTUK MASUKIN KE FILTER DAN CETAK LCD PER DETIK
  long duration1, distance1, duration2, distance2;
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  duration1 = pulseIn(echoPin1, HIGH);
  distance1 = jarakSensorKeTanah - ((duration1 / 2) / 29.1);
  tft.fillRect(0, 129, 160, 41, 0x0164);
  tft.setFont(&FreeSansBold24pt7b);
  tft.setCursor(10, 166);
  tft.print(distance1);
  tft.print("cm");
  H1.Filter(distance1);
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  duration2 = pulseIn(echoPin2, HIGH);
  distance2 = jarakSensorKeTanah - ((duration2 / 2) / 29.1);
  tft.fillRect(160, 129, 320, 41, 0x002E);
  tft.setCursor(170, 166);
  tft.print(distance2);
  tft.print("cm");
  H2.Filter(distance2);
  //-----------------------------------------------------------------

  //PRINT WAKTU PER DETIK
  DateTime now = rtc.now();
  tft.fillRect(140, 186, 220, 25, 0x0000);
  tft.setCursor(125, 226);
  tft.setFont(&FreeSans9pt7b);
  tft.print("Gate Pos:");
  tft.setCursor(125, 202);
  tft.print(now.year(), DEC);
  tft.print('/');
  tft.print(now.month(), DEC);
  tft.print('/');
  tft.print(now.day(), DEC);
  tft.print(" - ");
  tft.print(now.hour(), DEC);
  tft.print(':');
  tft.print(now.minute(), DEC);
  tft.print(':');
  if (now.second(), DEC < 10)
  {
    tft.print("0");
    tft.print(now.second(), DEC);
  }
  else
  {
    tft.print(now.second(), DEC);
  }
  //----------------------------------------------------------------

  //KALAU ALARM TRIGGER FUNGSINYA LOGGING
  unsigned long currentMillis = millis();
  if (currentMillis >= alarm)
  {
    logging();
  }
  //----------------------------------------------------------------

  delay(900); //DELAY PRINT LCD
}

#define BMPIMAGEOFFSET 54

#define BUFFPIXEL 20

uint16_t read16(File &f)
{
  uint16_t result; // read little-endian
  f.read(&result, sizeof(result));
  return result;
}

uint32_t read32(File &f)
{
  uint32_t result;
  f.read(&result, sizeof(result));
  return result;
}

uint8_t showBMP(char *nm, int x, int y)
{
  File bmpFile;
  int bmpWidth, bmpHeight;         // W+H in pixels
  uint8_t bmpDepth;                // Bit depth (currently must be 24, 16, 8, 4, 1)
  uint32_t bmpImageoffset;         // Start of image data in file
  uint32_t rowSize;                // Not always = bmpWidth; may have padding
  uint8_t sdbuffer[3 * BUFFPIXEL]; // pixel in buffer (R+G+B per pixel)
  uint16_t lcdbuffer[(1 << PALETTEDEPTH) + BUFFPIXEL], *palette = NULL;
  uint8_t bitmask, bitshift;
  boolean flip = true; // BMP is stored bottom-to-top
  int w, h, row, col, lcdbufsiz = (1 << PALETTEDEPTH) + BUFFPIXEL, buffidx;
  uint32_t pos;          // seek position
  boolean is565 = false; //

  uint16_t bmpID;
  uint16_t n; // blocks read
  uint8_t ret;

  if ((x >= tft.width()) || (y >= tft.height()))
    return 1; // off screen

  bmpFile = SD.open(nm);            // Parse BMP header
  bmpID = read16(bmpFile);          // BMP signature
  (void)read32(bmpFile);            // Read & ignore file size
  (void)read32(bmpFile);            // Read & ignore creator bytes
  bmpImageoffset = read32(bmpFile); // Start of image data
  (void)read32(bmpFile);            // Read & ignore DIB header size
  bmpWidth = read32(bmpFile);
  bmpHeight = read32(bmpFile);
  n = read16(bmpFile);        // # planes -- must be '1'
  bmpDepth = read16(bmpFile); // bits per pixel
  pos = read32(bmpFile);      // format
  if (bmpID != 0x4D42)
    ret = 2; // bad ID
  else if (n != 1)
    ret = 3; // too many planes
  else if (pos != 0 && pos != 3)
    ret = 4; // format: 0 = uncompressed, 3 = 565
  else if (bmpDepth < 16 && bmpDepth > PALETTEDEPTH)
    ret = 5; // palette
  else
  {
    bool first = true;
    is565 = (pos == 3); // ?already in 16-bit format
    // BMP rows are padded (if needed) to 4-byte boundary
    rowSize = (bmpWidth * bmpDepth / 8 + 3) & ~3;
    if (bmpHeight < 0)
    { // If negative, image is in top-down order.
      bmpHeight = -bmpHeight;
      flip = false;
    }

    w = bmpWidth;
    h = bmpHeight;
    if ((x + w) >= tft.width()) // Crop area to be loaded
      w = tft.width() - x;
    if ((y + h) >= tft.height()) //
      h = tft.height() - y;

    if (bmpDepth <= PALETTEDEPTH)
    { // these modes have separate palette
      //bmpFile.seek(BMPIMAGEOFFSET); //palette is always @ 54
      bmpFile.seek(bmpImageoffset - (4 << bmpDepth)); //54 for regular, diff for colorsimportant
      bitmask = 0xFF;
      if (bmpDepth < 8)
        bitmask >>= bmpDepth;
      bitshift = 8 - bmpDepth;
      n = 1 << bmpDepth;
      lcdbufsiz -= n;
      palette = lcdbuffer + lcdbufsiz;
      for (col = 0; col < n; col++)
      {
        pos = read32(bmpFile); //map palette to 5-6-5
        palette[col] = ((pos & 0x0000F8) >> 3) | ((pos & 0x00FC00) >> 5) | ((pos & 0xF80000) >> 8);
      }
    }

    tft.setAddrWindow(x, y, x + w - 1, y + h - 1);
    for (row = 0; row < h; row++)
    {
      uint8_t r, g, b;
      int lcdidx, lcdleft;
      if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
        pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
      else // Bitmap is stored top-to-bottom
        pos = bmpImageoffset + row * rowSize;
      if (bmpFile.position() != pos)
      { // Need seek?
        bmpFile.seek(pos);
        buffidx = sizeof(sdbuffer); // Force buffer reload
      }

      for (col = 0; col < w;)
      { //pixels in row
        lcdleft = w - col;
        if (lcdleft > lcdbufsiz)
          lcdleft = lcdbufsiz;
        for (lcdidx = 0; lcdidx < lcdleft; lcdidx++)
        { // buffer at a time
          uint16_t color;
          // Time to read more pixel data?
          if (buffidx >= sizeof(sdbuffer))
          { // Indeed
            bmpFile.read(sdbuffer, sizeof(sdbuffer));
            buffidx = 0; // Set index to beginning
            r = 0;
          }
          switch (bmpDepth)
          { // Convert pixel from BMP to TFT format
          case 24:
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            color = tft.color565(r, g, b);
            break;
          case 16:
            b = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];
            if (is565)
              color = (r << 8) | (b);
            else
              color = (r << 9) | ((b & 0xE0) << 1) | (b & 0x1F);
            break;
          case 1:
          case 4:
          case 8:
            if (r == 0)
              b = sdbuffer[buffidx++], r = 8;
            color = palette[(b >> bitshift) & bitmask];
            r -= bmpDepth;
            b <<= bmpDepth;
            break;
          }
          lcdbuffer[lcdidx] = color;
        }
        tft.pushColors(lcdbuffer, lcdidx, first);
        first = false;
        col += lcdidx;
      }                                                         // end cols
    }                                                           // end rows
    tft.setAddrWindow(0, 0, tft.width() - 1, tft.height() - 1); //restore full screen
    ret = 0;                                                    // good render
  }
  bmpFile.close();
  return (ret);
}