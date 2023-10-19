// GPS Datalogger Arduino Firmware v0.1
//  for RTK-GPS Datalogger, A.S. 2023
//  (Seeed XIAO RP2040 + Sparkfun ZED-F9P breakout)
// By Robert Mones

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>

#define PIN_SD_MOSI 3
#define PIN_SD_MISO 4
#define PIN_SD_SCK  2
#define PIN_SD_SS   1

#include <SPI.h>
#include <RP2040_SD.h>

#define BUFSIZE (1024*16)
#define WRITESIZE (1024*1)

SerialPIO gps_uart(28, 29);

Adafruit_SSD1306 disp(128, 32, &Wire, -1);

size_t bufct = 0;
size_t bufcpyct = 0;
size_t bufctmax = 0;
uint8_t buf[BUFSIZE];
uint8_t bufcpy[BUFSIZE];

char fname[30];
File datafile;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting GPS Datalogger");

  if(!disp.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("FAILED TO INIT DISPLAY");
    while(true);
  }
  disp.clearDisplay();
  disp.setTextSize(2);
  disp.setTextColor(SSD1306_WHITE);
  disp.display();

  memset(buf, 0, BUFSIZE);
  memset(bufcpy, 0, BUFSIZE);
  Serial.println("Buffers initialized");

  SPI.setRX(PIN_SD_MISO);
  SPI.setCS(PIN_SD_SS);
  SPI.setSCK(PIN_SD_SCK);
  SPI.setTX(PIN_SD_MOSI);
  if (!SD.begin(PIN_SD_SS)) {
    Serial.println("FAILED TO INIT SD CARD");
    while(true);
  }
  for (uint32_t i = 0; i < 10000; i++) {
    sprintf(fname, "DATA%04d.BIN", i);
    if (!SD.exists(fname)) break;
  }
  if (SD.exists(fname)) SD.remove(fname);
  datafile = SD.open(fname, FILE_WRITE);
  if (!datafile) {
    Serial.print("FAILED TO OPEN FILE ");
    Serial.println(fname);
  }
  Serial.print("uSD Card initialized -- writing to ");
  Serial.println(fname);
  delay(1000);

  rp2040.fifo.push(1);
  rp2040.fifo.pop();
}

void loop() {
  // Check buffer usage
  if (bufct > WRITESIZE) {
    if(bufct > bufctmax) bufctmax = bufct;
    // Ask for custody of buf from core1 and proceed on positive response
    rp2040.fifo.push_nb(1);
    if (rp2040.fifo.pop() == 1) {
      // Copy buf to bufcpy
      memcpy(bufcpy, buf, bufct);
      bufcpyct = bufct;
      bufct = 0;
      // Finish copy by giving custody of buf back to core1
      rp2040.fifo.push(0);
      delay(100);

      // Write copied data to SD card
      datafile.write(bufcpy, bufcpyct);
      datafile.flush();

      Serial.print("Wrote ");
      Serial.print(bufcpyct);
      Serial.println(" bytes");
    }
  }

  // Write to display
  disp.clearDisplay();
  disp.setCursor(0,0);
  char printbuf[11];
  sprintf(printbuf, "FILE: %c%c%c%c", fname[4], fname[5], fname[6], fname[7]);
  disp.write(printbuf);
  disp.write("\n");
  sprintf(printbuf, "%5ldbytes", bufctmax);
  disp.write(printbuf);
  disp.display();
}

void setup1() {
  gps_uart.begin(460800);
  rp2040.fifo.pop();
  rp2040.fifo.push(1);
}

void loop1() {
  while (gps_uart.available()) {
    buf[bufct] = gps_uart.read();
    bufct = (bufct + 1) % BUFSIZE;
  }
  uint32_t coremsg;
  // Check if a request for buf custody is present
  if (rp2040.fifo.pop_nb(&coremsg) && coremsg == 1) {
    // Respond giving custody to core0
    rp2040.fifo.push_nb(1);
    // Wait for custody back from core1
    uint32_t t1 = micros();
    while (rp2040.fifo.pop() != 0);
    uint32_t t2 = micros();
    Serial.print("Waited ");
    Serial.print(t2 - t1);
    Serial.println(" us");
  }
}
