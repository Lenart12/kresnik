#include <KeyboardInput.h>
#include <util.h>

static const uint8_t backspace = 0x08;
static const uint8_t shift = 0x0F;

const KeyboardButton KeyboardInput::_keyboard[51] = {
    KeyboardButton(10, 40, '1', '!'),
    KeyboardButton(35, 40, '2', '@'),
    KeyboardButton(60, 40, '3', '#'),
    KeyboardButton(85, 40, '4', '$'),
    KeyboardButton(110, 40, '5', '%'),
    KeyboardButton(135, 40, '6', '^'),
    KeyboardButton(160, 40, '7', '&'),
    KeyboardButton(185, 40, '8', '*'),
    KeyboardButton(210, 40, '9', '('),
    KeyboardButton(235, 40, '0', ')'),
    KeyboardButton(260, 40, '-', '_'),
    KeyboardButton(285, 40, '=', '+'),
    KeyboardButton(10, 80, 'q', 'Q'),
    KeyboardButton(35, 80, 'w', 'W'),
    KeyboardButton(60, 80, 'e', 'E'),
    KeyboardButton(85, 80, 'r', 'R'),
    KeyboardButton(110, 80, 't', 'T'),
    KeyboardButton(135, 80, 'y', 'Y'),
    KeyboardButton(160, 80, 'u', 'U'),
    KeyboardButton(185, 80, 'i', 'I'),
    KeyboardButton(210, 80, 'o', 'O'),
    KeyboardButton(235, 80, 'p', 'P'),
    KeyboardButton(260, 80, '[', '{'),
    KeyboardButton(285, 80, ']', '}'),
    KeyboardButton(10, 120, 'a', 'A'),
    KeyboardButton(35, 120, 's', 'S'),
    KeyboardButton(60, 120, 'd', 'D'),
    KeyboardButton(85, 120, 'f', 'F'),
    KeyboardButton(110, 120, 'g', 'G'),
    KeyboardButton(135, 120, 'h', 'H'),
    KeyboardButton(160, 120, 'j', 'J'),
    KeyboardButton(185, 120, 'k', 'K'),
    KeyboardButton(210, 120, 'l', 'L'),
    KeyboardButton(235, 120, ';', ':'),
    KeyboardButton(260, 120, '\'', '"'),
    KeyboardButton(285, 120, '\\', '|'),
    KeyboardButton(10, 160, '\\', '|'),
    KeyboardButton(35, 160, 'z', 'Z'),
    KeyboardButton(60, 160, 'x', 'X'),
    KeyboardButton(85, 160, 'c', 'C'),
    KeyboardButton(110, 160, 'v', 'V'),
    KeyboardButton(135, 160, 'b', 'B'),
    KeyboardButton(160, 160, 'n', 'N'),
    KeyboardButton(185, 160, 'm', 'M'),
    KeyboardButton(210, 160, ',', '<'),
    KeyboardButton(235, 160, '.', '>'),
    KeyboardButton(260, 160, '/', '?'),
    KeyboardButton(10, 200, shift, shift, 64, 30),
    KeyboardButton(79, 200, ' ', ' ', 128, 30),
    KeyboardButton(212, 200, backspace, backspace, 50, 30),
    KeyboardButton(267, 200, '\n', '\n', 45, 30)
};

String KeyboardInput::input(const char* placeholder){
    _input = "";
    _placeholder = placeholder;
    _shift = false;
    bool done, redraw = true;

    while(true){
        if(redraw){
            draw();
        }
        _buttons->update();
        redraw = update(done);
        if(done) break;
    }
    Sprintln("KeyboardInput("+_input+')');
    return _input;
}

void KeyboardInput::draw(){
    _display->fillScreen(ILI9341_WHITE);
    _display->setTextSize(1);

    _display->drawRect(5, 5, W - 10, H - 10, ILI9341_BLACK);
    _display->drawRect(8, 8, W - 16, 20, ILI9341_BLACK);

    _display->setTextColor((_input != "") ? ILI9341_BLACK : fromRGB(50, 50, 50));
    printAlignText(*_display, (_input != "") ? _input : _placeholder, Align::left, Align::center, 12, 10, W-18, 20);

    _display->setTextColor(ILI9341_BLACK);

    for(uint8_t i = 0; i < 47; i++){
        _display->drawRect(_keyboard[i].x, _keyboard[i].y, _keyboard[i].w, _keyboard[i].h, ILI9341_BLACK);
        printAlignText(*_display, (_shift) ? (String)_keyboard[i].c_shift : (String)_keyboard[i].c,
                       Align::center, Align::center,
                       _keyboard[i].x, _keyboard[i].y,
                       _keyboard[i].w, _keyboard[i].h);
    }
    _display->fillRect(_keyboard[47].x, _keyboard[47].y, _keyboard[47].w, _keyboard[47].h, BUTTON_0_COLOR);
    _display->setTextColor(~BUTTON_0_COLOR);
    printAlignText(*_display, "SHIFT", Align::center, Align::center, _keyboard[47].x, _keyboard[47].y, _keyboard[47].w, _keyboard[47].h);

    _display->drawRect(_keyboard[48].x, _keyboard[48].y, _keyboard[48].w, _keyboard[48].h, ILI9341_BLACK);

    _display->fillRect(_keyboard[49].x, _keyboard[49].y, _keyboard[49].w, _keyboard[49].h, BUTTON_2_COLOR);
    _display->setTextColor(~BUTTON_2_COLOR);
    printAlignText(*_display, "DEL", Align::center, Align::center, _keyboard[49].x, _keyboard[49].y, _keyboard[49].w, _keyboard[49].h);

    _display->fillRect(_keyboard[50].x, _keyboard[50].y, _keyboard[50].w, _keyboard[50].h, BUTTON_3_COLOR);
    _display->setTextColor(~BUTTON_3_COLOR);
    printAlignText(*_display, "OK", Align::center, Align::center, _keyboard[50].x, _keyboard[50].y, _keyboard[50].w, _keyboard[50].h);

    _display->setTextColor(ILI9341_BLACK);
}

bool KeyboardInput::update(bool &done){
    done = false;
    if(_ts->touched()){
        TS_Point p;
        while(_ts->touched()) p = _ts->getPoint(); 

        p.x = map(p.x, 240, 3820, 0, W);
        p.y = map(p.y, 430, 3840, 0, H);

        for(uint8_t i = 0; i < 51; i++){
            if(
                p.x >= _keyboard[i].x && p.x <= _keyboard[i].x + _keyboard[i].w &&
                p.y >= _keyboard[i].y && p.y <= _keyboard[i].y + _keyboard[i].h
            ){
                switch(i){
                    case 47: _shift = !_shift; break;
                    case 49: if (_input.length() > 0) _input = _input.substring(0, _input.length() - 1); break;
                    case 50: done = true; return false;
                    default: _input += (_shift) ? _keyboard[i].c_shift : _keyboard[i].c; _shift = false;
                }
                return true;
            }
        }
    }
    else if(_buttons->wasPressed()){
        if(_buttons->wasPressed(0)) _shift = !_shift;
        else if(_buttons->wasPressed(1)) {_input += ' '; _shift = false;}
        else if(_buttons->wasPressed(2) && _input.length() > 0) _input = _input.substring(0, _input.length() - 1);
        else if(_buttons->wasPressed(3)) done = true;
        return true;
    }
    return false;
}