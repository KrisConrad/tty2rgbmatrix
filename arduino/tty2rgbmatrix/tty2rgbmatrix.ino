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
String newCORE = "MENU";         // Buffer String for Text change detection

void setup() {
  Serial.begin(115200);
  Serial.println();

  initializeDisplay();
  mountSDCard();

  // initialize gif object
  gif.begin(LITTLE_ENDIAN_PIXELS);

  runStartupPattern();

  // setup initial core to default to menu
  currentCORE = "NULL";
  coreSelected(newCORE);
} /* void setup() */


void loop() {
  readNewCore();
  coreSelected(newCORE);
  delay(500);

} /* void loop() */

void mountSDCard() {
  //Mount SD Card, display some info and list files in /gifs folder to serial output
  Serial.println("Micro SD Card Mounting...");

  spi = new SPIClass(HSPI);
  spi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_CS);

  SD.begin(HSPI_CS, *spi);
  delay(500);

  if (!SD.begin(HSPI_CS, *spi)) {
    displayText("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    displayText("No SD card attached");
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

  mxconfig.driver = DRIVER;

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(128); //0-255
  dma_display->clearScreen();
}

void runStartupPattern() {
  //screen startup test (watch for dead or misfiring pixels)

  /* ----------------- Colors ----------------- */
  uint16_t myBLACK = dma_display->color565(0, 0, 0);
  uint16_t myWHITE = dma_display->color565(128, 128, 128);
  uint16_t myRED = dma_display->color565(128, 0, 0);
  uint16_t myGREEN = dma_display->color565(0, 128, 0);
  uint16_t myBLUE = dma_display->color565(0, 0, 128);
  
  dma_display->fillScreen(myRED);
  delay(1000);
  dma_display->fillScreen(myGREEN);
  delay(1000);
  dma_display->fillScreen(myBLUE);
  delay(1000);
  dma_display->fillScreen(myWHITE);
  delay(1000);

  displayText("MiSTer FPGA");
  delay(1000);

  dma_display->fillScreen(myBLACK);
}

String animatedPath = "";
String staticPath = "";
bool isAnimated = false;
bool hasStarted = false;

void coreSelected(String newCore) {
  // Proceed only if it's not the first transmission
  if (newCore == currentCORE || newCore.endsWith("QWERTZ")) return;
  
  Serial.printf("CORE is %s \n", newCore);

  if (newCore != currentCORE) {
    dma_display->clearScreen();
    hasStarted = false;
    
    String letter = newCore.substring(0, 1);
    letter.toUpperCase();
    String fileName = newCore + ".gif";
    
    animatedPath =  "/animated/" + letter + "/" + fileName;
    staticPath = "/static/" + letter + "/" + fileName;
  }
  
  currentCORE = newCore;

  if (hasStarted && !isAnimated) return;
  hasStarted = true;

  char animatedFilePath[256];
  animatedPath.toCharArray(animatedFilePath, animatedPath.length()+1);

  char staticFilePath[256];
  staticPath.toCharArray(staticFilePath, staticPath.length()+1);
  
  if (SD.exists(animatedFilePath)) {
    isAnimated = true;
    showGIF(animatedFilePath, true);
  } else if (SD.exists(staticFilePath)) {
    isAnimated = false;
    showGIF(staticFilePath, false);
  } else if (newCore != DEFAULT_CORE) {
    Serial.printf("IMAGE FILE FOR %s NOT FOUND!\n", newCore);
    newCORE = DEFAULT_CORE;
    coreSelected(DEFAULT_CORE);
  } else {
    isAnimated = false;
    Serial.printf("IMAGE FILE FOR %s NOT FOUND!\n", newCore);
    displayText("Default gif was not found");
    delay(3000);
  }
}

void displayText(String text) {
    dma_display->clearScreen();
    dma_display->setTextSize(2);
    dma_display->setCursor(0, 0);
    dma_display->print(text);
    Serial.println(text);
}

void readNewCore() {
  if (Serial.available()) {
  // Read string from serial until NewLine "\n" (from MiSTer's echo command) is detected or timeout (1000ms) happens.
    newCORE = Serial.readStringUntil('\n');
  }
}

void showGIF(char *name, bool animated) {
  unsigned long start_tick = millis();
  
  if (gif.open(name, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw)) {
    int canvasWidth = gif.getCanvasWidth();
    int canvasHeight = gif.getCanvasHeight();
    
    xPos = max(0, (totalWidth - canvasWidth) / 2);
    yPos = max(0, (totalHeight - canvasHeight) / 2);

    Serial.flush();
    
    if (animated)  {
      Serial.println("animated gif flag found, playing whole gif");
      bool running = true;
      while(running) {
        while(gif.playFrame(true, NULL)) {
         //keep on playing unless...
          readNewCore();
          if (newCORE != currentCORE) {
            running = false;
            break;
          }
       }
      }
    } else  {
      Serial.println("static gif flag found, playing 1st frame of gif");
      gif.playFrame(true, NULL);
      gif.reset();
    }
  }
  Serial.println("closing gif file");
  gif.close();

} /* ShowGIF() */
