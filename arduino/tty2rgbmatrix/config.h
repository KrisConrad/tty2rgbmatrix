#define FILESYSTEM SD

MatrixPanel_I2S_DMA *dma_display = nullptr;

/* 
 *  May or may not need to be changed to 'false' depending on your matrix
 * Example of what needing it looks like:
 * https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA/issues/134#issuecomment-866367216 
*/
bool CLKPHASE = true;

// If you're using panels based on the FM6126A chip, change this value to 'true'
bool IS_FM6126A = false;

// Some panels made need the green and blue pins swapped
bool SWAP_GREEN_BLUE = false;


/* ----------------- RGB MATRIX CONFIG ----------------- */
// more panel setup is found in the void setup() function!

const int panelResX = 64;        // Number of pixels wide of each INDIVIDUAL panel module.
const int panelResY = 32;        // Number of pixels tall of each INDIVIDUAL panel module.
const int panels_in_X_chain = 2; // Total number of panels in X
const int panels_in_Y_chain = 1; // Total number of panels in Y

const int totalWidth  = panelResX * panels_in_X_chain;  //used in span function
const int totalHeight = panelResY * panels_in_Y_chain;  //used in span function


/* ----------------- Micro SD Card Module Pinout ----------------- */
// these pins below are known to work with this config on esp32 trinity boards by brian lough
//sd  to tri
//3v3 to 3v3
//gnd to gnd
//clk to 33
//do  to 32
//di  to sda
//cs  to scl

/* 
 *  Pin setup for Adafruit MicroSD Card Breakout Board+ 
 *  https://www.adafruit.com/product/254 
 */

#define HSPI_MISO 21     //trinity pin labeled SDA
#define HSPI_MOSI 32 
#define HSPI_SCLK 22     //trinity pin labeled SCL
#define HSPI_CS 33
#define PIN_E 18         // The trinity connects GPIO 18 to the typical pin E.
