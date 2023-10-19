// GPS Datalogger Arduino Firmware v1.0
//  for RTK-GPS Datalogger, A.S. 2023
//  (Seeed XIAO RP2040 + Sparkfun ZED-F9P breakout)
// By Robert Mones

#define PIN_SD_MOSI 3
#define PIN_SD_MISO 4
#define PIN_SD_SCK  2
#define PIN_SD_SS   1

#include <SPI.h>
#include <RP2040_SD.h>

#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>

// Uncomment the following line to enable debug mode
//  In debug mode, some testing data is printed to the USB serial port
//  and the system does not start until something is sent to it via the
//  USB serial port. The device will still print error messages via the
//  USB serial port regardless of this setting.
// #define DEBUG

// Size of the data buffer (in bytes)
#define BUFSIZE (1024*32)
// Minimum amount of data to copy from the buffer to the SD card
#define WRITESIZE (1024*1)

// Hardware UART interface to communicate with the GPS module
SerialPIO gps_uart(28, 29);
int fixtype = 0;

// NeoPixel object to control the onboard RGB LED
Adafruit_NeoPixel rgb(1, 12, NEO_GRB + NEO_KHZ800);
uint32_t ledtime = 0;
bool ledstate = 0;

// Display object to communicate with the SSD1306 display over I2C
Adafruit_SSD1306 disp(128, 32, &Wire, -1);

// Data buffers and their respective sizes
size_t bufct = 0;
size_t bufcpyct = 0;
size_t bufctmax = 0;
uint8_t buf[BUFSIZE];
uint8_t bufcpy[BUFSIZE];

// File object which is being written to on the SD card
char fname[30];
File datafile;

// Flag which is set if an error occurs
volatile bool error_flag = false;
// Helper function which flashes the RGB led red, prints an error message to the display
//  (if msg != NULL), and prints an error message to the USB serial port (if dbgmsg != NULL)
void error(char *msg, char *dbgmsg) {
  error_flag = true;
  delay(1000);
  rgb.clear();
  rgb.setPixelColor(0, rgb.Color(127,0,0));
  rgb.show();

  if(dbgmsg != NULL) {
    Serial.println(dbgmsg);
  }

  if(msg != NULL) {
    disp.clearDisplay();
    disp.setTextSize(2);
    disp.setCursor(0,0);
    disp.write(msg);
    disp.display();
  }

  while(true) {
    rgb.clear();
    rgb.setPixelColor(0, rgb.Color(127,0,0));
    rgb.show();
    delay(250);
    rgb.clear();
    rgb.setPixelColor(0, rgb.Color(0,0,0));
    rgb.show();
    delay(250);
  }
}

void setup() {
  // Init debug USB serial port
  Serial.begin(115200);
  #ifdef DEBUG
  while(!Serial.available());
  while(Serial.available()) Serial.read();
  Serial.println("Starting GPS Datalogger");
  #endif

  // Init RGB led
  rgb.begin();
  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);
  rgb.clear();
  rgb.setPixelColor(0, rgb.Color(31,31,0));
  rgb.show();

  // Init display
  if(!disp.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    error(NULL, "FAILED TO INIT DISPLAY");
  }
  disp.clearDisplay();
  disp.setTextSize(2);
  disp.setTextColor(SSD1306_WHITE);
  disp.display();

  memset(buf, 0, BUFSIZE);
  memset(bufcpy, 0, BUFSIZE);

  // Init SD card
  SPI.setRX(PIN_SD_MISO);
  SPI.setCS(PIN_SD_SS);
  SPI.setSCK(PIN_SD_SCK);
  SPI.setTX(PIN_SD_MOSI);
  if (!SD.begin(PIN_SD_SS)) {
    error("NO SD CARD", "FAILED TO INIT SD CARD");
  }
  // Search for filename
  for (uint32_t i = 0; i < 10000; i++) {
    sprintf(fname, "DATA%04d.BIN", i);
    if (!SD.exists(fname)) break;
  }
  // Create the new file and overwrite if existing, but this won't happen
  //  until file cap is reached
  if (SD.exists(fname)) SD.remove(fname);
  datafile = SD.open(fname, FILE_WRITE);
  if (!datafile) {
    char printbuf[40];
    sprintf(printbuf, "FAILED TO OPEN FILE %s", fname);
    error("FAILED TO OPEN FILE", printbuf);
  }
  #ifdef DEBUG
  Serial.print("uSD Card initialized -- writing to ");
  Serial.println(fname);
  #endif
  // Delay to let everything power up
  delay(1000);

  // Tell other core to begin loop and wait for it to be ready
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

      // Write copied data to SD card
      if(datafile.write(bufcpy, bufcpyct) < bufcpyct) {
        error("FILE WRITE  FAILED", "FAILED TO WRITE TO FILE");
      }
      datafile.flush();

      #ifdef DEBUG
      Serial.print("Wrote ");
      Serial.print(bufcpyct);
      Serial.println(" bytes");
      #endif

      // Parse GNGGA NMEA packet to get utc time, fix type, and number of satellites
      // Start from end of data being written to only consider latest packet
      size_t nmea_start = 0;
      size_t nmea_end = 0;
      size_t i = bufcpyct - 1;
      for(; i >= 4; i--) {
        // Look for end of NMEA packet
        if(bufcpy[i-4] == '*' &&
            ((bufcpy[i-3] >= '0' && bufcpy[i-3] <= '9') || (bufcpy[i-3] >= 'A' && bufcpy[i-3] <= 'F')) &&
            ((bufcpy[i-2] >= '0' && bufcpy[i-2] <= '9') || (bufcpy[i-2] >= 'A' && bufcpy[i-2] <= 'F')) &&
            bufcpy[i-1] == '\r' && bufcpy[i] == '\n') {
          nmea_end = i-1; // Exclusive end of NMEA string
          break;
        }
      }
      for(; i >= 1; i--) {
        // Look for start of NMEA packet
        if(strncmp("$GNGGA", (char*)(bufcpy+i-1), 6) == 0) {
          nmea_start = i-1; // Inclusive start of GNGGA NMEA string
          break;
        }
      }
      size_t nmea_len = nmea_end - nmea_start;
      if(nmea_len >= 6 && nmea_len <= 82) {
        // Found valid GNGGA NMEA packet
        char gnggabuf[100];
        memcpy(gnggabuf, bufcpy+nmea_start, nmea_len);
        // Need to separate commas and replace with spaces to support easy sscanf parsing
        for(size_t j = 1; j < nmea_len; j++) {
          if(gnggabuf[j-1] == ',' && gnggabuf[j] == ',') {
            memmove(gnggabuf+j+1, gnggabuf+j, nmea_len-j);
            gnggabuf[j] = 'x';
            nmea_len++;
          }
        }
        for(size_t j = 0; j < nmea_len; j++) {
          if(gnggabuf[j] == ',') gnggabuf[j] = ' ';
        }
        gnggabuf[nmea_len] = '\0';

        #ifdef DEBUG
        Serial.print("Extracted packet '");
        Serial.print(gnggabuf);
        Serial.println("'");
        #endif

        // Parse relevant information from packet
        char utcbuf[10];
        int tmpfixtype = -1;
        int numsats = -1;
        int parsect = sscanf(gnggabuf, "$GNGGA %s %*s %*s %*s %*s %d %d %*s %*s %*s %*s %*s *%*s", utcbuf, &tmpfixtype, &numsats);
        utcbuf[9] = '\0';
        
        #ifdef DEBUG
        Serial.print("Parsed (");
        Serial.print(parsect);
        Serial.print(") ");
        Serial.print(utcbuf);
        Serial.print(" time, ");
        Serial.print(fixtype);
        Serial.print(" fix, ");
        Serial.print(numsats);
        Serial.println(" satellites");
        #endif

        // If parse succeeded, proceed with storing fix type in global variable and
        //  write to display
        if(parsect == 3) {
          fixtype = tmpfixtype;
          // Write to display
          disp.clearDisplay();
          disp.setTextSize(2);
          disp.setCursor(0,0);
          char printbuf[11];
          sprintf(printbuf, "#%c%c%c%c  %s", fname[4], fname[5], fname[6], fname[7], (fixtype > 0) ? "FIX" : "---");
          disp.write(printbuf);
          disp.write("\n");
          int utchr = -1;
          int utcmin = -1;
          int utcsec = -1;
          sscanf(utcbuf, "%02d%02d%02d.%*02d", &utchr, &utcmin, &utcsec);
          sprintf(printbuf, "%02d:%02d:%02d     Sats:%02d", utchr, utcmin, utcsec, numsats);
          disp.setTextSize(1);
          disp.write("\n");
          disp.write(printbuf);
          disp.display();
        }
      }
      
      delay(10);
    }
  }

  // Flash RGB LED blue if no fix and green if fix
  uint32_t t = millis();
  if(t - ledtime > 500) {
    ledtime = t;
    ledstate = !ledstate;
    
    rgb.clear();
    rgb.setPixelColor(0, ledstate ? (fixtype > 0 ? rgb.Color(0,31,0) : rgb.Color(0,0,63)) : rgb.Color(0,0,0));
    rgb.show();
  }

  // Halt while error_flag is set to true
  while(error_flag);
}

void setup1() {
  // Open the hardware UART port to the GPS module
  gps_uart.begin(460800);

  // Wait for the other core to be ready and tell it this core is ready
  rp2040.fifo.pop();
  rp2040.fifo.push(1);
}

void loop1() {
  // Read the data from the GPS module into the buffer
  while (gps_uart.available()) {
    buf[bufct] = gps_uart.read();
    bufct++;
    if(bufct >= BUFSIZE) {
      error("BUFFER    OVERFLOW", "BUFFER OVERFLOW");
    }
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
    #ifdef DEBUG
    Serial.print("Waited ");
    Serial.print(t2 - t1);
    Serial.println(" us");
    #endif
  }

  // Halt while error_flag is set to true
  while(error_flag);
}
