///////////////////////////////////////////////////////////////
/* This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or* (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  You can download the latest version of this code from:
  https://github.com/h3llb3nt/tty2rgbmatrix
*///////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
/* tty2rgbmatrix sdcard edition 2022/09/18
   loads gif files from an sdcard and play them on an rgb matrix based on serial input from MiSTer fpga
   those code is know to work with arduino IDE 1.8.19 with the ESP32 package version 2.0.4 installed */
//////////////////////////////////////////////////////////////

// versions of the libraries used are commented below
// use other versions at your own peril
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>  //v2.0.7 by mrfaptastic verifed to work

#include "SD.h"                               //v1.2.4
#include "SPI.h"
#include "config.h"
#include "gifs.h"

AnimatedGIF gif;
SPIClass *spi = NULL;
String currentCORE = "";         // Buffer String for Text change detection
char newCOREArray[30]="";        // Array of char needed for some functions, see below "newCORE.toCharArray"

/* ----------------- Colors ----------------- */
uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);


void setup() {
  Serial.begin(115200);
  Serial.println();

  mountSDCard();

  // initialize gif object
  gif.begin(LITTLE_ENDIAN_PIXELS);

  initializeDisplay();

  runStartupPattern();

  gif.begin(LITTLE_ENDIAN_PIXELS);

  // setup initial core to default to menu
  currentCORE = "NULL";
  coreSelected("MENU");
} /* void setup() */


void loop() {
  String newCore = readNewCore();

   delay(10);
   Serial.printf("%s is oldcore, %s is newcore\n", String(currentCORE), String(newCore));

  coreSelected(newCore);

} /* void loop() */

void mountSDCard() {
  //Mount SD Card, display some info and list files in /gifs folder to serial output
  Serial.println("Micro SD Card Mounting...");

  spi = new SPIClass(HSPI);
  spi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_CS);

  SD.begin(HSPI_CS, *spi);
  delay(500);

  if (!SD.begin(HSPI_CS, *spi)) {
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");

  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  Serial.println();
}

void initializeDisplay() {
  // start display
  HUB75_I2S_CFG mxconfig(
    panelResX,           // module width
    panelResY,           // module height
    panels_in_X_chain    // Chain length
  );

  // If you are using a 64x64 matrix you need to pass a value for the E pin
  if (panelResX >= 64 && panelResY >= 64) {
     mxconfig.gpio.e = PIN_E;
  }

  mxconfig.clkphase = CLKPHASE;

  if (SWAP_GREEN_BLUE) {
    mxconfig.gpio.b1 = 26;
    mxconfig.gpio.b2 = 12;

    mxconfig.gpio.g1 = 27;
    mxconfig.gpio.g2 = 13;
  }

  if (IS_FM6126A) { 
    mxconfig.driver = HUB75_I2S_CFG::FM6126A;
  }

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(128); //0-255
  dma_display->clearScreen();
}

void runStartupPattern() {
  //screen startup test (watch for dead or misfiring pixels)
  dma_display->fillScreen(myBLACK);
  delay(500);
  dma_display->fillScreen(myRED);
  delay(500);
  dma_display->fillScreen(myGREEN);
  delay(500);
  dma_display->fillScreen(myBLUE);
  delay(500);
  dma_display->fillScreen(myWHITE);
  delay(500);
  dma_display->clearScreen();
  delay(500);

  dma_display->setCursor(0, 0);
  dma_display->println("MiSTer FPGA");
  delay(1000);

  dma_display->fillScreen(myBLACK);
}

void coreSelected(String newCore) {

  // Proceed only if Core Name changed and it's not th efirst transmission
  if (newCore != currentCORE && !newCore.endsWith("QWERTZ")) {
    Serial.printf("Running a check because %s is oldcore, %s is newcore\n", String(currentCORE), String(newCore));

    String letter = newCore.substring(0, 1);
    letter.toUpperCase();
    String fileName = newCore + ".gif";
    
    String animatedPath =  "/animated/" + letter + "/" + fileName;
    char animatedFilePath[256];
    animatedPath.toCharArray(animatedFilePath, animatedPath.length()+1);

    String staticPath = "/static/" + letter + "/" + fileName;
    char staticFilePath[256];
    staticPath.toCharArray(staticFilePath, staticPath.length()+1);

    if (SD.exists(animatedFilePath)) {
      showGIF(animatedFilePath, true);
    } else if (SD.exists(staticFilePath)) {
      showGIF(staticFilePath, false);
    } else {
      Serial.printf("IMAGE FILE FOR %s NOT FOUND!\n", newCore);
      dma_display->clearScreen();
      dma_display->setCursor(0, 0);
      dma_display->print(newCore);
      dma_display->println(" not found");
      delay(3000);
    }

  } // end newCORE!=currentCORE

  currentCORE = newCore;
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

String readNewCore() {
  if (Serial.available()) {
  // Read string from serial until NewLine "\n" (from MiSTer's echo command) is detected or timeout (1000ms) happens.
    return Serial.readStringUntil('\n');
  } else {
    return currentCORE;
  }
}

void showGIF(char *name, bool animated) {
  unsigned long start_tick = millis();
  
  if (gif.open(name, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw)) {
    int canvasWidth = gif.getCanvasWidth();
    int canvasHeight = gif.getCanvasHeight();
    
    x_offset = max(0, (MATRIX_WIDTH - canvasWidth) / 2);
    y_offset = max(0, (MATRIX_HEIGHT - canvasHeight) / 2);

    Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", canvasWidth, canvasHeight);
    Serial.flush();
    
    if (animated)  {
      Serial.println("animated gif flag found, playing whole gif");
      while(gif.playFrame(true, NULL)) {
        //keep on playing unless...
        if (readNewCore() != currentCORE) break;
      }
    } else  {
      Serial.println("static gif flag found, playing 1st frame of gif");
      while (!gif.playFrame(true, NULL)) { // leaving this break in here incase i need it for interrupting the the current playing gif in a future rev
        
        // play first frame of non-animated gif and wait 10 seconds  
        if ( (millis() - start_tick) > 10000) break;
        Serial.print(".");
        if (readNewCore() != currentCORE) break;
        gif.reset();
      }
    }
    Serial.println("closing gif file");
    gif.close();
  }

} /* ShowGIF() */
