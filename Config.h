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
