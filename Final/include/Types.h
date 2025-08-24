#ifndef TASK_TYPES_H
#define TASK_TYPES_H

#include <string>

struct TaskData {
  int id;
  std::string taskName;
  std::string dueDate;
  int priority;
  bool completed;
};

#endif