#include <LovyanGFX.hpp>

//#define TOUCH_ENABLED
//#define SD_ENABLED
#define SHARED_SPI

#define SPI_MODE 0

#define TFT_MOSI    11
#define TFT_MISO    13
#define TFT_SCLK    12
#define TFT_DC      4
#define TFT_CS      6
#define TFT_RST     5
#define TFT_BCK_LT   7

#define TFT_RTOUCH_CS    -1

// Portrait
#define TFT_WIDTH   320 //updated
#define TFT_HEIGHT  480 //updated

// CF: https://github.com/lovyan03/LovyanGFX/issues/513

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ST7796  _panel_instance;  // ST7796S
  lgfx::Bus_SPI _bus_instance;
  lgfx::Light_PWM     _light_instance;
  lgfx::Touch_FT5x06 _touch_instance; // OK for capacitive touch panel with controller FT6336

public:
  LGFX(void)
  {
    {
      // set up bus control.
      auto cfg = _bus_instance.config(); // gets the structure for bus settings.

      // SPI bus settings
      cfg.spi_host = SPI2_HOST; // Select SPI to use ESP32-S2,C3: SPI2_HOST or SPI3_HOST / ESP32: VSPI_HOST or HSPI_HOST

      //* Due to the ESP-IDF upgrade, the description of VSPI_HOST , HSPI_HOST will be deprecated, so if you get an error, use SPI2_HOST , SPI3_HOST instead.
      cfg.spi_mode = SPI_MODE;          // Set SPI communication mode (0-3)
      cfg.freq_write = 40*1000*1000; // SPI clock on transmission (up to 80MHz, rounded to 80MHz divided by integer)
      cfg.freq_read = 20*1000*1000;  // SPI clock on reception
      cfg.spi_3wire = false;      // Set true when receiving on the MOSI pin
      cfg.use_lock = true;       // set true if transaction lock is used

      //  * With the ESP-IDF version upgrade, SPI_DMA_CH_AUTO (automatic setting) of DMA channels is recommended.
      cfg.dma_channel = SPI_DMA_CH_AUTO; // Set DMA channel to be used (0=DMA not used / 1=1ch / 2=ch / SPI_DMA_CH_AUTO=Auto setting)

      cfg.pin_sclk = TFT_SCLK; // Set SCLK pin number for SPI
      cfg.pin_mosi = TFT_MOSI; // Set SPI MOSI pin number

      // When using the SPI bus, which is common to the SD card, be sure to set MISO without omitting it.
      cfg.pin_miso = TFT_MISO; // Set THE MSO pin number of spi (-1 = disable)
      cfg.pin_dc = TFT_DC;    // Set THE D/C pin number of SPI (-1 = disable)

      _bus_instance.config(cfg);              // reflects the setting value on the bus.
      _panel_instance.setBus(&_bus_instance); // Set the bus to the panel.
    }

    {
      auto cfg = _panel_instance.config();    

      cfg.pin_cs           =    TFT_CS;
      cfg.pin_rst          =    TFT_RST;
      cfg.pin_busy         =    -1;

      cfg.panel_width      =   TFT_WIDTH;
      cfg.panel_height     =   TFT_HEIGHT;
      cfg.offset_x         =     0;
      cfg.offset_y         =     0;
      cfg.offset_rotation  =     0;
      cfg.dummy_read_pixel =     8;
      cfg.dummy_read_bits  =     1;
      cfg.readable         =  true;
      cfg.invert           = true;
      cfg.rgb_order        = false;
      cfg.dlen_16bit       = false;
      cfg.bus_shared       = false;
   
      auto lightCfg = _light_instance.config();    
      lightCfg.pin_bl = TFT_BCK_LT;   //updated  - pin back light          
      lightCfg.invert = false;          
      //lightCfg.freq   = 44100;    // commented to get default value      
      //lightCfg.pwm_channel = 7;          

      _light_instance.config(lightCfg);
      _panel_instance.setLight(&_light_instance);
     
      _panel_instance.config(cfg);
    }

#ifdef TOUCH_ENABLED
    {
      auto cfg = _touch_instance.config();

      cfg.x_min      = 0;
      cfg.x_max      = 319;  //updated
      cfg.y_min      = 0;  
      cfg.y_max      = 479;  //updated
      cfg.pin_int    = 7;  
      cfg.bus_shared = true;
      cfg.offset_rotation = 0;

      cfg.i2c_port = 1;
      cfg.i2c_addr = 0x38;
      cfg.pin_sda  = 6;  
      cfg.pin_scl  = 5;  
      cfg.freq = 400000;  

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);  
    }
#endif

    setPanel(&_panel_instance);
  }
};
