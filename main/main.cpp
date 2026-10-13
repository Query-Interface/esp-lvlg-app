/*
MIT License

Copyright (c) 2022 Sukesh Ashok Kumar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

static const char *TAG = "ESP32-TUX";
#include "main.hpp"
#include "driver/i2s_std.h"
#include "audio_player.h"
#include <stdio.h>

static void set_timezone()
{
    // Update local timezone
    //setenv("TZ", cfg->TimeZone, 1);
    setenv("TZ", CONFIG_TIMEZONE_STRING, 1);
    tzset();    
}

// GEt time from internal RTC and update date/time of the clock
static void update_datetime_ui()
{
    // If we are on another screen where lbl_time is not valid
    //if (!lv_obj_is_valid(lbl_time)) return;

    // date/time format reference => https://cplusplus.com/reference/ctime/strftime/
    time_t now;
    struct tm datetimeinfo;
    time(&now);
    localtime_r(&now, &datetimeinfo);

    // tm_year will be (1970 - 1900) = 70, if not set
    if (datetimeinfo.tm_year < 100) // Time travel not supported :P
    {
        // ESP_LOGD(TAG, "Date/time not set yet [%d]",datetimeinfo.tm_year);    
        return;
    }

    // Send update time to UI with payload using lv_msg
    lv_msg_send(MSG_TIME_CHANGED, &datetimeinfo);
}

static const char* get_id_string(esp_event_base_t base, int32_t id) {
    //if (base == TUX_EVENTS) {
        switch(id) {
            case TUX_EVENT_DATETIME_SET:
                return "TUX_EVENT_DATETIME_SET";
            case TUX_EVENT_OTA_STARTED:
                return "TUX_EVENT_OTA_STARTED";
            case TUX_EVENT_OTA_IN_PROGRESS:
                return "TUX_EVENT_OTA_IN_PROGRESS";
            case TUX_EVENT_OTA_ROLLBACK:
                return "TUX_EVENT_OTA_ROLLBACK";
            case TUX_EVENT_OTA_COMPLETED:
                return "TUX_EVENT_OTA_COMPLETED";
            case TUX_EVENT_OTA_FAILED:
                return "TUX_EVENT_OTA_FAILED";
            case TUX_EVENT_OTA_ABORTED:
                return "TUX_EVENT_OTA_ABORTED";
            case TUX_EVENT_WEATHER_UPDATED:
                return "TUX_EVENT_WEATHER_UPDATED";
            case TUX_EVENT_THEME_CHANGED:
                return "TUX_EVENT_THEME_CHANGED";
            default:
                return "TUX_EVENT_UNKNOWN";        
        }
    //} 
}

// tux event handler
static void tux_event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    ESP_LOGD(TAG, "tux_event_handler => %s:%s", event_base, get_id_string(event_base, event_id));
    if (event_base != TUX_EVENTS) return;   // bye bye - me not invited :(

    // Handle TUX_EVENTS
    if (event_id == TUX_EVENT_DATETIME_SET) {

        set_timezone();

        // update clock
        update_datetime_ui();

        // Enable timer after the date/time is set.
        lv_timer_ready(timer_weather); 

    } else if (event_id == TUX_EVENT_OTA_STARTED) {
        // OTA Started
        char buffer[150] = {0};
        snprintf(buffer,sizeof(buffer),"OTA: %s",(char*)event_data);
        lv_msg_send(MSG_OTA_STATUS,buffer);

    } else if (event_id == TUX_EVENT_OTA_IN_PROGRESS) {
        // OTA In Progress - progressbar?
        char buffer[150] = {0};
        int bytes_read = (*(int *)event_data)/1024;
        snprintf(buffer,sizeof(buffer),"OTA: Data read : %dkb", bytes_read);
        lv_msg_send(MSG_OTA_STATUS,buffer);

    } else if (event_id == TUX_EVENT_OTA_ROLLBACK) {
        // OTA Rollback - god knows why!
        char buffer[150] = {0};
        snprintf(buffer,sizeof(buffer),"OTA: %s", (char*)event_data);
        lv_msg_send(MSG_OTA_STATUS,buffer);

    } else if (event_id == TUX_EVENT_OTA_COMPLETED) {
        // OTA Completed - YAY! Success
        char buffer[150] = {0};
        snprintf(buffer,sizeof(buffer),"OTA: %s", (char*)event_data);
        lv_msg_send(MSG_OTA_STATUS,buffer);

        // wait before reboot
        vTaskDelay(3000 / portTICK_PERIOD_MS);

    } else if (event_id == TUX_EVENT_OTA_ABORTED) {
        // OTA Aborted - Not a good day for updates
        char buffer[150] = {0};
        snprintf(buffer,sizeof(buffer),"OTA: %s", (char*)event_data);
        lv_msg_send(MSG_OTA_STATUS,buffer);

    } else if (event_id == TUX_EVENT_OTA_FAILED) {
        // OTA Failed - huh! - maybe in red color?
        char buffer[150] = {0};
        snprintf(buffer,sizeof(buffer),"OTA: %s", (char*)event_data);
        lv_msg_send(MSG_OTA_STATUS,buffer);

    } else if (event_id == TUX_EVENT_WEATHER_UPDATED) {
        // Weather updates - summer?

    } else if (event_id == TUX_EVENT_THEME_CHANGED) {
        // Theme changes - time to play with dark & light
    }
}                          

// Wifi & IP related event handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    //ESP_LOGD(TAG, "%s:%s: wifi_event_handler", event_base, get_id_string(event_base, event_id));
    if (event_base == WIFI_EVENT  && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        is_wifi_connected = true;
        if (timer_datetime) lv_timer_ready(timer_datetime);   // start timer (may not exist yet)

        // After OTA device restart, RTC will have time but not timezone
        set_timezone();

        // Not a warning but just for highlight
        ESP_LOGW(TAG,"WIFI_EVENT_STA_CONNECTED");
        lv_msg_send(MSG_WIFI_CONNECTED,NULL);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        is_wifi_connected = false;        
        lv_timer_pause(timer_datetime);   // stop/pause timer

        ESP_LOGW(TAG,"WIFI_EVENT_STA_DISCONNECTED");
        lv_msg_send(MSG_WIFI_DISCONNECTED,NULL);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        is_wifi_connected = true;
        if (timer_datetime) lv_timer_ready(timer_datetime);   // start timer (may not exist yet)

        ESP_LOGW(TAG,"IP_EVENT_STA_GOT_IP");
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;

        snprintf(ip_payload,sizeof(ip_payload),"%d.%d.%d.%d", IP2STR(&event->ip_info.ip));
        
        // We got IP, lets update time from SNTP. RTC keeps time unless powered off
        xTaskCreate(configure_time, "config_time", 1024*4, NULL, 3, NULL);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP)
    {
        is_wifi_connected = false;
        ESP_LOGW(TAG,"IP_EVENT_STA_LOST_IP");
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_START) {
        ESP_LOGW(TAG,"WIFI_PROV_START");
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_CRED_RECV) {
        ESP_LOGW(TAG,"WIFI_PROV_CRED_RECV");
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_CRED_FAIL) {
        ESP_LOGW(TAG,"WIFI_PROV_CRED_FAIL");
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_CRED_SUCCESS) {
        ESP_LOGW(TAG,"WIFI_PROV_CRED_SUCCESS");
        // FIXME Refresh IP details once provision is successfull
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_END) {
        ESP_LOGW(TAG,"WIFI_PROV_END");
    }
    else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_SHOWQR) {
        ESP_LOGW(TAG,"WIFI_PROV_SHOWQR");
        strcpy(qr_payload,(char*)event_data);   // Add qr payload to the variable
    }
}

static void heap_monitor_task(void *arg)
  {
      while (true) {
          vTaskDelay(pdMS_TO_TICKS(5000));   // wait 5 s
          ESP_LOGI(TAG, "Internal free: %u, largest: %u, min ever: %u",
                   heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                   heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL),
                   heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL));
      }
  }

static i2s_chan_handle_t tx;
static int volume = 10;

extern "C" void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_DEBUG);      // enable DEBUG logs for this App
    //esp_log_level_set("SettingsConfig", ESP_LOG_DEBUG);    
    esp_log_level_set("wifi", ESP_LOG_WARN);    // enable WARN logs from WiFi stack

    // Print device info
    ESP_LOGE(TAG,"\n%s",device_info().c_str());

    //Initialize NVS
    esp_err_t err = nvs_flash_init();

    // NVS partition contains new data format or unable to access
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err); 

    // Init SPIFF - needed for lvgl images
    init_spiff();

//********************** CONFIG HELPER TESTING STARTS

     //cfg = new SettingsConfig("/sdcard/settings.json");    // yet to test
   /*
    cfg = new SettingsConfig("/spiffs/settings.json");
    // Save settings
    cfg->save_config();   // save default loaded settings
    // Load values
    cfg->load_config();
    // Change device name
    cfg->DeviceName = "ESP32-TUX";
    cfg->WeatherAPIkey = CONFIG_WEATHER_API_KEY;
    cfg->WeatherLocation = CONFIG_WEATHER_LOCATION;
    cfg->WeatherProvider = CONFIG_WEATHER_OWM_URL;
    // Change brightness
    cfg->Brightness=250;
    // Save settings again
    cfg->save_config();
    cfg->load_config();
*/
//******************************************** 
    // = new OpenWeatherMap();
//********************** CONFIG HELPER TESTING ENDS

ESP_LOGI(TAG, "Starting LCD Initialization...");
    lcd.init();         // Initialize LovyanGFX
    lcd.initDMA();      // Init DMA
    lv_init();          // Initialize lvgl

    

    if (lv_display_init() != ESP_OK) // Configure LVGL
    {
        ESP_LOGE(TAG, "LVGL setup failed!!!");
    }

    // /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Register for event handler for Wi-Fi, IP related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
    
    /* Events related to provisioning */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    // TUX EVENTS
    ESP_ERROR_CHECK(esp_event_handler_instance_register(TUX_EVENTS, ESP_EVENT_ANY_ID, tux_event_handler, NULL, NULL));

    #ifdef SD_SUPPORTED
    // Initializing SDSPI 
    //lvgl_acquire();
    if (init_sdspi() == ESP_OK) // SD SPI
    {
        is_sdcard_enabled = true;
        lv_print_readme_txt("S:/test.txt");   // SDCARD
/*
        // Diagnostic code for SD-Card
        char letters[16];
        lv_fs_get_letters(letters);
        ESP_LOGI(TAG, "LVGL drives: %s", letters);
        ESP_LOGI(TAG, "S: ready: %d", lv_fs_is_ready('S'));

        lv_fs_file_t f;
        lv_fs_res_t res = lv_fs_open(&f, "S:/test.txt", LV_FS_MODE_RD);
        if (res == LV_FS_RES_OK) {
            char buf[64] = {0};
            uint32_t n;
            lv_fs_read(&f, buf, sizeof(buf) - 1, &n);
            ESP_LOGI(TAG, "Read %lu bytes: %s", n, buf);
            lv_fs_close(&f);
        } else {
            ESP_LOGE(TAG, "lv_fs_open failed: %d", res);
        }
        */
    }
    //lvgl_release();
    #endif
    // LV_FS integration & print readme.txt from the root for testing
    ESP_LOGI(TAG, "reading readme");
    lv_print_readme_txt("F:/readme.txt");   // SPIFF / FAT

 

/* Push LVGL/UI to its own UI task later*/
ESP_LOGI(TAG, "Displaying splashscreen...");
    // Splash screen
    lvgl_acquire();
    create_splash_screen();
    lvgl_release();

    // Main UI
    lvgl_acquire();
    lv_setup_styles();    
    show_ui();
    lvgl_release();
/* Push these to its own UI task later*/

#ifdef SD_SUPPORTED
    // Icon status color update
    lv_msg_send(MSG_SDCARD_STATUS,&is_sdcard_enabled);
#endif

    ESP_LOGI(TAG, "Internal free: %u, largest block: %u",
         heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
         heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    // Wifi Provision and connection.
    // Use idf.py menuconfig to configure
    // Use SoftAP only / BLE has some issues
    // Tuning PSRAM options visible only in IDF5, so will wait till then for BLE.
    // WiFi starts first: its static buffers must come from internal RAM.
    s_wifi_init_done = xSemaphoreCreateBinary();
    xTaskCreate(provision_wifi, "wifi_prov", 1024*8, NULL, 3, NULL);
    if (xSemaphoreTake(s_wifi_init_done, pdMS_TO_TICKS(5000)) != pdTRUE) {
        ESP_LOGW(TAG, "WiFi init not done after 5s, starting audio anyway");
    }

    audio_start();
    xTaskCreate(heap_monitor_task, "heap_mon", 3072, NULL, 1, NULL);
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());

    // Date/Time update timer - once per sec
    timer_datetime = lv_timer_create(timer_datetime_callback, 1000,  NULL);
    //lv_timer_pause(timer_datetime); // enable only when wifi is connected

    // Weather update timer - Once per min (60*1000) or maybe once in 10 mins (10*60*1000)
   // removed: timer_weather = lv_timer_create(timer_weather_callback, WEATHER_UPDATE_INTERVAL,  NULL);

    //lv_timer_set_repeat_count(timer_weather,1);
    //lv_timer_pause(timer_weather);  // enable after wifi is connected

    // Subscribe to page change events in the UI
    /* SPELLING MISTAKE IN API BUG => https://github.com/lvgl/lvgl/issues/3822 */
    lv_msg_subsribe(MSG_PAGE_HOME, tux_ui_change_cb, NULL);
    lv_msg_subsribe(MSG_PAGE_REMOTE, tux_ui_change_cb, NULL);
    lv_msg_subsribe(MSG_PAGE_SETTINGS, tux_ui_change_cb, NULL);
    lv_msg_subsribe(MSG_PAGE_OTA, tux_ui_change_cb, NULL);
    lv_msg_subsribe(MSG_OTA_INITIATE, tux_ui_change_cb, NULL);    // Initiate OTA
}

static void timer_datetime_callback(lv_timer_t * timer)
{
    // Battery icon animation
    if (battery_value>100) battery_value=0;
    battery_value+=10;
    
    lv_msg_send(MSG_BATTERY_STATUS,&battery_value);
    update_datetime_ui();
}

static void timer_weather_callback(lv_timer_t * timer)
{
    ESP_LOGW(TAG,"Disabled weather update for now!!!");
        return;
    if (cfg->WeatherAPIkey.empty()) {   // If API key not defined skip weather update
        ESP_LOGW(TAG,"Weather API Key not set");
        return;
    }

    // Update weather and trigger UI update
    owm->request_weather_update();
    lv_msg_send(MSG_WEATHER_CHANGED, owm);
}

// Callback to notify App UI change
static void tux_ui_change_cb(void * s, lv_msg_t *m)
{
    LV_UNUSED(s);
    unsigned int page_id = lv_msg_get_id(m);
    const char * msg_payload = (const char *)lv_msg_get_payload(m);
    const char * msg_data = (const char *)lv_msg_get_user_data(m);

    //ESP_LOGW(TAG,"[%" PRIu32 "] page event triggered",page_id);

    switch (page_id)
    {
        case MSG_PAGE_HOME:
            // Update date/time and current weather
            // Date/time is updated every second anyway
            //lv_msg_send(MSG_WEATHER_CHANGED, owm);
            break;
        case MSG_PAGE_REMOTE:
            // Trigger loading buttons data
            break;
        case MSG_PAGE_SETTINGS:
            if (!is_wifi_connected)  {// Provisioning mode
                lv_msg_send(MSG_WIFI_PROV_MODE, qr_payload);
                //lv_msg_send(MSG_QRCODE_CHANGED, qr_payload);
            } else {
                lv_msg_send(MSG_WIFI_CONNECTED, ip_payload);
            }
            break;
        case MSG_PAGE_OTA:
            // Update firmware current version info
            lv_msg_send(MSG_DEVICE_INFO,device_info().c_str());
            break;
        case MSG_OTA_INITIATE:
            // OTA update from button trigger
            xTaskCreate(run_ota_task, "run_ota_task", 1024 * 8, NULL, 5, NULL);
            break;
    }
}

static string device_info()
{
    std::string s_chip_info = "";

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);

    // CPU Speed - 80Mhz / 160 Mhz / 240Mhz
    rtc_cpu_freq_config_t conf;
    rtc_clk_cpu_freq_get_config(&conf);

    multi_heap_info_t info;    
	heap_caps_get_info(&info, MALLOC_CAP_SPIRAM);
    float psramsize = (info.total_free_bytes + info.total_allocated_bytes) / (1024.0 * 1024.0);

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;
    
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        s_chip_info += fmt::format("Firmware Ver : {}\n",running_app_info.version);
        s_chip_info += fmt::format("Project Name : {}\n",running_app_info.project_name);
        // running_app_info.time
        // running_app_info.date
    }
    s_chip_info += fmt::format("IDF Version  : {}\n\n",esp_get_idf_version());

    s_chip_info += fmt::format("Controller   : {} Rev.{}\n",CONFIG_IDF_TARGET,chip_info.revision);  
    //s_chip_info += fmt::format("\nModel         : {}",chip_info.model); // esp_chip_model_t type
    s_chip_info += fmt::format("CPU Cores    : {}\n", (chip_info.cores==2)? "Dual Core" : "Single Core");
    s_chip_info += fmt::format("CPU Speed    : {}Mhz\n",conf.freq_mhz);
    if(esp_flash_get_size(NULL, &flash_size) == ESP_OK) {
    s_chip_info += fmt::format("Flash Size   : {}MB {}\n",flash_size / (1024 * 1024),
                                            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "[embedded]" : "[external]");
    }
    s_chip_info += fmt::format("PSRAM Size   : {}MB {}\n",static_cast<int>(round(psramsize)),
                                            (chip_info.features & CHIP_FEATURE_EMB_PSRAM) ? "[embedded]" : "[external]");

    s_chip_info += fmt::format("Connectivity : {}{}{}\n",(chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "2.4GHz WIFI" : "NA",
                                                    (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
                                                    (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    //s_chip_info += fmt::format("\nIEEE 802.15.4 : {}",string((chip_info.features & CHIP_FEATURE_IEEE802154) ? "YES" : "NA"));

    //ESP_LOGE(TAG,"\n%s",device_info().c_str());
    return s_chip_info;
}


static esp_err_t i2s_init(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;   // send silence when no data, avoids buzzing
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx, NULL));

    i2s_std_config_t cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(44100),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,  // MAX98357A needs no MCLK
            .bclk = GPIO_NUM_14,
            .ws   = GPIO_NUM_47,
            .dout = GPIO_NUM_42,
            .din  = I2S_GPIO_UNUSED,
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx, &cfg));
    return i2s_channel_enable(tx);
}

// Called by the player when a file's sample rate / channels change
static esp_err_t clk_set_fn(uint32_t rate, uint32_t bits_cfg, i2s_slot_mode_t ch)
{
    i2s_std_clk_config_t clk = I2S_STD_CLK_DEFAULT_CONFIG(rate);
    i2s_std_slot_config_t slot = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG((i2s_data_bit_width_t)bits_cfg, ch);
    slot.slot_mask = I2S_STD_SLOT_BOTH;   // mono files go to both L and R

    i2s_channel_disable(tx);
    i2s_channel_reconfig_std_clock(tx, &clk);
    i2s_channel_reconfig_std_slot(tx, &slot);
    return i2s_channel_enable(tx);
}

// Called with decoded 16-bit PCM; apply software volume here
static esp_err_t write_fn(const void *buf, size_t len, size_t *written, uint32_t timeout_ms)
{
    int16_t *s = (int16_t *)buf;
    for (size_t i = 0; i < len / 2; i++) {
        s[i] = (int32_t)s[i] * volume / 100;
    }
    // The player passes pdMS_TO_TICKS(100) (ticks, = 10 at 100Hz) as "timeout_ms",
    // which makes i2s_channel_write time out and drop samples. Block instead:
    // the I2S DMA clock paces playback.
    return i2s_channel_write(tx, buf, len, written, portMAX_DELAY);
}

static esp_err_t mute_fn(AUDIO_PLAYER_MUTE_SETTING setting)
{
    return ESP_OK;   // no hardware mute wired (see below)
}

static void sd_read_test_task(void *arg);

static void audio_start(void)
{
    i2s_init();
    ESP_LOGI(TAG, "i3s initialized");
    audio_player_config_t config = {
        .mute_fn    = mute_fn,
        .clk_set_fn = clk_set_fn,
        .write_fn   = write_fn,
        .priority   = 5,
        .coreID     = 0,   // keep audio away from the LVGL core
    };
    ESP_ERROR_CHECK(audio_player_new(config));
    ESP_LOGI(TAG, "player created");

    FILE *f = fopen("/sdcard/test.mp3", "rb");
    if (f) {
        ESP_LOGI(TAG, "start playing");
        audio_player_play(f);   // the player closes the file when done
    } else {
        ESP_LOGE("audio", "Cannot open test.mp3");
    }

    //xTaskCreate(sd_read_test_task, "sd_test", 4096, NULL, 5, NULL);
}

// SD read test in its own task so app_main isn't blocked
static void sd_read_test_task(void *arg)
{
    FILE *f = fopen("/sdcard/test.mp3", "rb");
      if (!f) { ESP_LOGE(TAG, "open failed"); vTaskDelete(NULL); }

      static uint8_t buf[4096];
      size_t total = 0;
      int64_t t0 = esp_timer_get_time();
      ESP_LOGI(TAG, "reading test.mp3 file...");
      while (true) {
          size_t n = fread(buf, 1, sizeof(buf), f);
          total += n;
          if (n < sizeof(buf)) {
              ESP_LOGW(TAG, "stop at offset %u after %lld ms (%s)", total,
                       (esp_timer_get_time() - t0) / 1000,
                       ferror(f) ? "READ ERROR" : "EOF");
              break;
          }
          vTaskDelay(pdMS_TO_TICKS(100));   // ~40 KB/s, about the MP3 read rate
      }
      fclose(f);
      vTaskDelete(NULL);
}