// SD card Working / enable it below
#define BD_ER_TFT035M
#define TOUCH_ENABLED
#define SD_SUPPORTED

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// SD CARD - SPI
#define SDSPI_HOST_ID SPI2_HOST
#define SD_MISO       GPIO_NUM_13
#define SD_MOSI       GPIO_NUM_11
#define SD_SCLK       GPIO_NUM_12
#define SD_CS         GPIO_NUM_10

#define LCD_RESET     15
#define LCD_CS        16
#define LCD_WR        17
#define LCD_RD        18
#define LCD_DC_RS     21
#define LCD_BL        9

// GPIO_NUM_37 is SPIDQS (Data Strobe) for the octal PSRAM on ESP32-S3.
// Configuring it as a regular GPIO breaks the PSRAM interface -> cache fault.
// Use -1 to disable INT (FT5x06 polling mode) and leave GPIO37 as SPIDQS.
#define TOUCH_INT   GPIO_NUM_40  // was GPIO_NUM_37 - conflicts with ESP32-S3 SPIDQS/PSRAM. // => LCD PIN 39
#define TOUCH_SDA   GPIO_NUM_38
#define TOUCH_SCL   GPIO_NUM_39
#define TOUCH_RST   -1 //GPIO_NUM_45

// Portrait
#define TFT_WIDTH   320
#define TFT_HEIGHT  480

class LGFX : public lgfx::LGFX_Device
{
  lgfx::Panel_ILI9488  _panel_instance;
  lgfx::Bus_Parallel8 _bus_instance;
  lgfx::Light_PWM     _light_instance;
  lgfx::Touch_FT5x06  _touch_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.freq_write = 40000000;    
      cfg.pin_wr = LCD_WR; //47            
      cfg.pin_rd = LCD_RD;     //-1        
      cfg.pin_rs = LCD_DC_RS; //0;              

      // LCD data interface, 8bit MCU (8080)
      cfg.pin_d0 = 1;              
      cfg.pin_d1 = 2;             
      cfg.pin_d2 = 3;              
      cfg.pin_d3 = 4;              
      cfg.pin_d4 = 5; //18;             
      cfg.pin_d5 = 6; //17;             
      cfg.pin_d6 = 7; //16;             
      cfg.pin_d7 = 8; //15;             

      _bus_instance.config(cfg);   
      _panel_instance.setBus(&_bus_instance);      
    }

    { 
      auto cfg = _panel_instance.config();    

      cfg.pin_cs           =    LCD_CS; //-1;  
      cfg.pin_rst          =    LCD_RESET;  
      cfg.pin_busy         =    -1; 

      cfg.panel_width      =   TFT_WIDTH;
      cfg.panel_height     =   TFT_HEIGHT;
      cfg.offset_x         =     0;
      cfg.offset_y         =     0;
      cfg.offset_rotation  =     0;
      cfg.dummy_read_pixel =     8;
      cfg.dummy_read_bits  =     1;
      cfg.readable         =  false;
      cfg.invert           = true;
      cfg.rgb_order        = false;
      cfg.dlen_16bit       = false;
      cfg.bus_shared       = false;

      _panel_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();    

      cfg.pin_bl = LCD_BL; //45;              
      cfg.invert = false;           
      cfg.freq   = 44100;           
      cfg.pwm_channel = 7;          

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);  
    }
#ifdef TOUCH_ENABLED
    { 
      auto cfg = _touch_instance.config();

      cfg.x_min      = 0;
      cfg.x_max      = TFT_WIDTH - 1;
      cfg.y_min      = 0;  
      cfg.y_max      = TFT_HEIGHT - 1;
      cfg.pin_int    = TOUCH_INT;
      cfg.pin_rst    = TOUCH_RST;
      cfg.bus_shared = false;
      cfg.offset_rotation = 0;

      cfg.i2c_port = I2C_NUM_0;
      cfg.i2c_addr = 0x38;
      cfg.pin_sda  = TOUCH_SDA;  // => LCD PIN 31
      cfg.pin_scl  = TOUCH_SCL;  // => LCD PIN 30
      cfg.freq = 400000; //400000;

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }
#endif

    setPanel(&_panel_instance); 
  }
};
