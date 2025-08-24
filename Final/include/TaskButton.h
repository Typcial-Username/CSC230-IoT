#include "Types.h"

#ifndef TASK_BUTTON_H
#define TASK_BUTTON_H

class TaskButton {
    private: 
        int x, y, width, height;
        bool rounded, lastState;
        TaskData task;
    public:
        TaskButton();
        TaskButton(int x, int y, int width, int height, TaskData task, bool rounded = false);
        void draw();
        bool isPressed();
        TaskData getTask() { return task; }
        void setFill(int color);

};
#endif