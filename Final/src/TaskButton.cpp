#include <M5Core2.h>
#include "Types.h"
#include "TaskButton.h"

int padding = 10;
TaskButton::TaskButton() : x(0), y(0), width(0), height(0), lastState(false), task(), rounded(false) {
    draw();
}

TaskButton::TaskButton(int x, int y, int width, int height, TaskData task, bool rounded) 
    : x(x), y(y), width(width), height(height), lastState(false), task(task), rounded(rounded) {
        draw();
    }

void TaskButton::draw() {
    
    setFill(WHITE);

    M5.Lcd.setTextColor(BLACK, WHITE);

    // Top left of the button
    M5.Lcd.setCursor(x + padding, y + padding);
    M5.Lcd.setTextSize(1.5);
    M5.Lcd.println(task.priority);

    // M5.Lcd.setTextDatum(TR_DATUM);
    // Top right of the button
    M5.Lcd.setCursor(x + width - padding, y + padding);

    M5.Lcd.println(task.dueDate.c_str());

    // Center of the button
    // M5.Lcd.setTextDatum(MC_DATUM);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(x + width / 2 - (task.taskName.length() * 8) / 2, (y + height / 2) - 8); // 8 half of char height
    // M5.Lcd.setTextColor(BLACK, WHITE);
    M5.Lcd.println(task.taskName.c_str());
}

void TaskButton::setFill(int color) {
    if (rounded) {
        M5.Lcd.fillRoundRect(x, y, width, height, 10, color);
    } else {
        M5.Lcd.fillRect(x, y, width, height, color);
    }
}

bool TaskButton::isPressed() {
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