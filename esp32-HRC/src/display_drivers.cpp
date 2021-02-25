#include <display_drivers.h>

void display_init(){
    tft.begin();
	tft.setRotation(TFT_ROTATION);

	pinMode(TFT_LED, OUTPUT);
	digitalWrite(TFT_LED, ON);

	lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.hor_res = W;
	disp_drv.ver_res = H;
	disp_drv.flush_cb = display_flush_cb;
	disp_drv.buffer = &disp_buf;
	lv_disp_drv_register(&disp_drv);

	LV_LOG_TRACE("Display initialized");
}

void display_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p){
    uint16_t c;

	tft.startWrite();
	tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1));
	for (int y = area->y1; y <= area->y2; y++)
	{
		for (int x = area->x1; x <= area->x2; x++)
		{
			c = color_p->full;
			tft.writeColor(c, 1);
			color_p++;
		}
	}
	tft.endWrite();
	lv_disp_flush_ready(disp);
}

void lv_tick_handler(){
    lv_tick_inc(LVGL_TICK_PERIOD);
}