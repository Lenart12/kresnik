#include <util.h>
#include <defines.h>

void printAlignText(Adafruit_ILI9341 &display, String str, Align halign, Align valign, int16_t x, int16_t y, uint16_t w, uint16_t h){
	int16_t x1, y1;
	uint16_t strw, strh;
	display.getTextBounds(str, x, y, &x1, &y1, &strw, &strh);
    x -= x1 - x;
	if(strw < w) switch(halign){
		case Align::center: x += (w - strw)/2 ;break;
		case Align::left: break;
		case Align::right: x += w - strw; break;
	}
    y -= y1 - y;
	if(strh < h) switch(valign){
		case Align::center: y += (h-strh)/2 ;break;
		case Align::top: break;
		case Align::bottom: y += h - strh; break;
	}

	display.setCursor(x, y);
	display.print(str);
}