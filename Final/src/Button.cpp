#include <M5Core2.h>
#include "Types.h"
#include "Button.h"

int padding = 10;
Button::Button(int x, int y, int width, int height, TaskData task, bool rounded = false) 
    : x(x), y(y), width(width), height(height), lastState(false), rounded(rounded), task(task) {}

void Button::draw() {
    if (rounded) {
        M5.Lcd.fillRoundRect(x, y, width, height, 10, WHITE);
    } else {
        M5.Lcd.fillRect(x, y, width, height, WHITE);
    }

    // Top left of the button
    M5.Lcd.setCursor(x + padding, y + padding);
    M5.Lcd.println(task.priority);

    // Top right of the button
    M5.Lcd.setCursor(x + width - padding, y + padding);
    M5.Lcd.println(task.dueDate.c_str());

    // Center of the button
    M5.Lcd.setCursor(x + width / 2 - (task.taskName.length() & 6) / 2, y + height / 2 - 8); // 8 half of char height
    M5.Lcd.println(task.taskName.c_str());
}

bool Button::isPressed() {
    if (!M5.Touch.ispressed()) {
        lastState = false;
        return false;
    }

    auto point = M5.Touch.getPressPoint();

    bool pressed = (point.x >= x && point.x <= x + width &&
                    point.y >= y && point.y <= y + height);

    if (pressed && !lastState) {
        lastState = true;
        return true;
    } else if (!pressed) {
        lastState = false;
    }

    return false;
};