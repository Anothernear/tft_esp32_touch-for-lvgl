/* Configuracion de la libreria tft, lvgl y el driver CST816S*/
#define TFT_HOR_RES   240
#define TFT_VER_RES   240
#define TFT_ROTATION  LV_DISPLAY_ROTATION_0

TFT_eSPI tft = TFT_eSPI();
CST816S touch(21, 22, 4, 19);	// sda, scl, rst, irq


#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];
/* Display flushing */
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)px_map, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);  // Aquí 'disp' debe ser del tipo 'lv_disp_drv_t *'
}
/*Read the touchpad*/
void my_touchpad_read( lv_indev_t * indev_drv , lv_indev_data_t * data )
{
  if (touch.available()) {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = touch.data.x;
    data->point.y = touch.data.y;
    printf("X:%d, Y:%d\n", data->point.x = touch.data.x, data->point.y = touch.data.y);
  }
  else{
    data->state = LV_INDEV_STATE_REL;
    Serial.println("No hay nada pulsado");
  }
}

// Timer handled
#define MY_TICK() millis()
static uint32_t my_tick(void){
    return MY_TICK();
}

/*LVGL&&TFT_INIT_CONF*/
void INIT_LVGL_and_TFT(){
  lv_init();

  // Configuramos el callback cada cierto tiempo.
  lv_tick_set_cb(my_tick);

  /*Initialize the display*/
  lv_display_t * disp_drv;
  /*Change the following line to your display resolution*/
  disp_drv = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
  lv_display_set_rotation(disp_drv, TFT_ROTATION);
  lv_display_set_flush_cb(disp_drv, my_disp_flush);
  lv_display_set_buffers(disp_drv, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

  /*Initialize the (dummy) input device driver*/
  lv_indev_t * indev_drv = lv_indev_create();
  lv_indev_set_type(indev_drv, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  // lv_indev_set_mode(indev_drv, LV_INDEV_MODE_EVENT);
  lv_indev_set_display(indev_drv, disp_drv);
  lv_indev_set_read_cb(indev_drv, my_touchpad_read);
  lv_indev_enable(indev_drv, true);
  
}
/*----------------------------------------------------- Finaliza la configuracion ----------------------------------------*/


/*=======================NTP===============
===========================================*/
bool isTimeSet = false;
ESP32Time rtc;


//>>>>>>>>mode
bool debugging = true;


//========================================//
//==========WiFi configuration AP=========//
//========================================//

String serverName = "ip_del_esp32_cam"; // this value will be changed by the INIT_WiFi function
// Create an AsyncWebServer and DNSServer instances
AsyncWebServer server(80);
DNSServer dns;
// Create an instance of AsyncWiFiManager
AsyncWiFiManager wifiManager(&server, &dns);


bool synchronizeTime() {
  const char* ntpServer = "north-america.pool.ntp.org";
  const long gmtOffset_sec = -6*3600;
  const int daylightOffset_sec = 0;

  if (!isTimeSet) {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    struct tm timeinfo;
    int retryCount = 0;
    const int maxRetries = 3; // Adjust as needed

    while (retryCount < maxRetries && !getLocalTime(&timeinfo)) {
      Serial.println("Failed to get time from NTP server. Retrying...");
      delay(1000); // Wait for 1 second before retrying
      retryCount++;
    }

    if (getLocalTime(&timeinfo)) {
      rtc.setTimeStruct(timeinfo);
      Serial.println(rtc.getTime("%d/%m/%Y"));
      Serial.println(rtc.getTime("%H:%M:%S"));
      if (debugging) {
        Serial.println("$########################$");
        Serial.printf("%d °C", (int)temperatureRead());
        Serial.println("$########################$");
      }
      return true;
    } else {
      Serial.println("Failed to get time from NTP server after multiple retries.");
      // Consider alternative time sources here (if available)
      // ...
      return false;
    }
  }
  return true; // Time already set
}

void INIT_WiFi(){
  const char* ssid = "CORVUS_CONF";
  // Customize the AP's IP address
  wifiManager.setAPStaticIPConfig(IPAddress(192, 168, 1, 1),
                                  IPAddress(192, 168, 1, 1),
                                  IPAddress(255, 255, 255, 0));

  
  
  // Iniciar el portal cautivo
  Serial.println("Iniciando el portal cautivo...");
  if (!wifiManager.autoConnect(ssid)) {
    Serial.println("No se pudo conectar, reiniciando...");
    ESP.restart();  // Reinicia si no logra conectarse
  }
  // Sincronizar la hora solo la primera vez
  if (!synchronizeTime()) { // 5 intentos, 2 segundos entre cada intento
  // Manejar el error: mostrar mensaje al usuario, intentar otra fuente de hora, etc.
    Serial.println("Se acabaron los intentos");
  }

  Serial.println("Conexión establecida!");
  Serial.print("Dirección IP: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  ip[3] = 111;
  serverName = ip.toString();
  Serial.println("Conectado a: " + String(WiFi.SSID()));

  delay(1000);
}
/*--------------------------------------------END_the_configuration_of_WiFi-----------------------------*/
