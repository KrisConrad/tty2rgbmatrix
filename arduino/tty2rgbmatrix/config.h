
/* ----------------- RGB MATRIX CONFIG ----------------- */

const int panelResX = 64;        // Number of pixels wide of each INDIVIDUAL panel module.
const int panelResY = 64;        // Number of pixels tall of each INDIVIDUAL panel module.
const int panels_in_X_chain = 4; // Total number of panels in X
const int panels_in_Y_chain = 1; // Total number of panels in Y

const int totalWidth  = panelResX * panels_in_X_chain;  //used in span function
const int totalHeight = panelResY * panels_in_Y_chain;  //used in span function

/* 
 *  May or may not need to be changed to 'false' depending on your matrix
 * Example of what needing it looks like:
 * https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA/issues/134#issuecomment-866367216 
*/
bool CLKPHASE = false;

// Some panels may need the green and blue pins swapped
bool SWAP_GREEN_BLUE = false;

/* 
 *  The driver chip used in your matrix panels
 *  
 *  Options:
 *  HUB75_I2S_CFG::SHIFTREG // default value
 *  HUB75_I2S_CFG::FM6124
 *  HUB75_I2S_CFG::FM6126A
 *  HUB75_I2S_CFG::ICN2038S
 *  HUB75_I2S_CFG::MBI5124
 *  HUB75_I2S_CFG::SM5266P
*/
#define DRIVER HUB75_I2S_CFG::SHIFTREG


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
