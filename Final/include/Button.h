#ifndef BUTTON_H
#define BUTTON_H

class Button {
    private: 
        int x, y, width, height;
        bool isPressed, rounded, lastState;
        TaskData task;
    public:
        Button(int x, int y, int width, int height, TaskData task, bool rounded = false);
        void draw();
        bool isPressed();
};
#endif