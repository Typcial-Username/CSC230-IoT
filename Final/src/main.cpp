// Main Imports
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Arduino_Json.h>
#include <vector>
#include <WebServer.h>
#include <SPIFFS.h>

// #ifndef FIREBASE_SSE_TIMEOUT_MS
// #define FIREBASE_SSE_TIMEOUT_MS 40000
// #endif

// #ifndef ENABLE_ID_TOKEN
// #define ENABLE_ID_TOKEN
// #endif

// Firebase Imports
#include <FirebaseClient.h>
#include <FirebaseJson.h>

#include <M5Core2.h>

// Local Imports
#include "secrets.h"
#include "Types.h"
#include "TaskButton.h"

// Global variables
using AsyncClient = AsyncClientClass;

WiFiClientSecure sslClient;

AsyncClient aClient(sslClient);
UserAuth userAuth(FIREBASE_AUTH, FIREBASE_EMAIL, FIREBASE_PASSWORD);

FirebaseApp app;

FirebaseClient firebaseClient;
RealtimeDatabase database;

AsyncResult databaseResult;

std::vector<TaskData> tasks;

TaskData testTask;
TaskButton exButton;

// Prototypes
void processData(AsyncResult &aResult);
void getTasks();
void checkOffTask(TaskData task);

void handleRoot();
void handleGetTasks();
void handlePostTask();
void handleDeleteTask();

bool testTaskSent = false;

WebServer server(80);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }

  Serial.println("Initializing..");

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  // Setup WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  if (!WiFi.softAP("TODO_AP", "")) {
    Serial.println("Failed to create Access Point");
  }

  Serial.println();

  IPAddress localIP = WiFi.localIP();
  Serial.print("Connected to WiFi. IP Address: ");
  Serial.println(localIP);
  Serial.println();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/tasks", HTTP_GET, handleGetTasks);
  server.on("/api/tasks", HTTP_POST, handlePostTask);
  server.on("/api/tasks", HTTP_DELETE, handleDeleteTask);

  server.begin();
  IPAddress serverIP = WiFi.softAPIP();
  Serial.print("HTTP server started on http://");
  Serial.print(serverIP);
  Serial.println(":80");

  sslClient.setInsecure(); // Disable SSL certificate verification for simplicity

  // Initialize Firebase
  initializeApp(aClient, app, getAuth(userAuth), processData);
  app.getApp<RealtimeDatabase>(database);
  database.url(FIREBASE_URL);

  // Send a test task to Firebase
  testTask = {2, "Test Task 2", "2023-12-31", 1, false};

  M5.begin();
  M5.Lcd.setTextSize(2);

  exButton = TaskButton(100, 100, 200, 50, testTask, true);

  Serial.println("Initialized!");
}

unsigned long authStart = millis();

void loop() {
  app.loop();
  server.handleClient();

  // if (!userAuth.isInitialized() || !app.ready()) {
  //   Serial.println("Waiting for Firebase App to be ready...");
  //   // delay(1000);
  //   return;
  // }

  if (!app.ready())
  {
    if (millis() - authStart > 10000)
    {
      Serial.println("Firebase App not ready after 10 seconds.");
      authStart = millis(); // Reset the timer
    } else {
      Serial.print("Waiting for auth... elapsed: ");
      Serial.print((millis() - authStart) / 1000);
      Serial.println(" s");
    }

    delay(1000);
    return;
  }

  if (!testTaskSent)
  {
    FirebaseJson taskJson;
    taskJson.set("id", testTask.id);
    taskJson.set("taskName", testTask.taskName);
    taskJson.set("dueDate", testTask.dueDate);
    taskJson.set("priority", testTask.priority);
    taskJson.set("completed", testTask.completed);
  
    String taskPath = "/Tasks/";
    taskPath += String(testTask.id);
  
    database.set(aClient, taskPath, object_t (taskJson.raw()));
    Serial.println(taskJson.raw());

    testTaskSent = true;
  }

  delay(2500);

  // Get tasks from Firebase
  getTasks();

  if (exButton.isPressed()) {
    exButton.setFill(GREEN);
  }

  if (aClient.lastError().code() == 0)
   {
    Serial.println("Value set successfully.");
   } else 
   {
    Serial.print("Error setting value: ");
    Serial.print(aClient.lastError().code());
    Serial.println(aClient.lastError().message());
   }

   delay(5000);
}

void processData(AsyncResult &aResult)
{
  if (!aResult.isResult()) return;

  if (aResult.isEvent()) Serial.printf("Event received\n\t UID: %s\nMessage: %s\nCode: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());

  if (aResult.isDebug())
    Serial.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());

  if (aResult.isError())
    Serial.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());

  if (aResult.available()) 
  {
    Serial.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());

    String jsonResult = aResult.c_str();
    FirebaseJson jsonData(jsonResult);

    if (jsonResult.equals("null") || jsonResult.equals("") || jsonResult.equals("{}")) {
      Serial.println("No valid JSON data received");
      return;
    }

    size_t len = jsonData.iteratorBegin();
    // tasks.clear();

    Serial.printf("Number of tasks: %d\n", len);
    
    String key, value;
    int type;

    tasks.clear();

    FirebaseJson taskJson;
    FirebaseJsonData taskData;

    for (size_t i = 0; i < len; i++) {
      if (jsonData.iteratorGet(i, type, key, value) == 0) {

        taskJson.setJsonData(value);

        int id;
        String taskName, dueDate;
        int priority;
        bool completed;

        TaskData task = {};

        if (taskJson.get(taskData, "id", id)) {
          task.id = id;
        }
        if (taskJson.get(taskData, "taskName", taskName)) {
          task.taskName = taskName.c_str();
        }
        if (taskJson.get(taskData, "dueDate", dueDate)) {
          task.dueDate = dueDate.c_str();
        }
        if (taskJson.get(taskData, "priority", priority)) {
          task.priority = priority;
        }
        String completedStr;
        if (taskJson.get(taskData, "completed", completedStr)) {
          task.completed = (completedStr.equals("true") || completedStr.equals("1")) || completedStr == "true" ? true : false;
        }

        // TaskData task = {
        //   .id = id,
        //   .taskName = taskName.c_str(),
        //   .dueDate = dueDate.c_str(),
        //   .priority = priority,
        //   .completed = completed
        // };

        tasks.push_back(task);
        Serial.printf("Recieved %d tasks\n", tasks.size());
      }
    }

    jsonData.iteratorEnd();

    Serial.printf("Key: %s, Value: %s\n", key.c_str(), value.c_str());
  }

}

void getTasks() {
  Serial.println("Fetching tasks from Firebase...");
  database.get(aClient, "/Tasks/");
  Serial.println("Fetched tasks from Firebase...");
  // Serial.println(databaseResult.c_str());
}

void checkOffTask(TaskData task) {
  // Mark the task as completed
  task.completed = true;

  FirebaseJson jsonData;

  jsonData.set("id", task.id);
  jsonData.set("taskName", task.taskName);
  jsonData.set("dueDate", task.dueDate);
  jsonData.set("priority", task.priority);
  jsonData.set("completed", task.completed);

  String path = "/Tasks/";
  path += String(task.id);
  
  String jsonStr;
  jsonData.toString(jsonStr);

  if (!database.set(aClient, path, object_t(jsonStr))) {
    Serial.print("Error updating task in Firebase");
    Serial.print(aClient.lastError().code());
    Serial.println(aClient.lastError().message());
  }
}

void handleRoot() {
  File file = SPIFFS.open("/index.html", "r");
  if (file) {
    server.streamFile(file, "text/html");
    file.close();
  } 
  else {
    server.send(404, "text/plain", "File not found");
  }
}

// Format the tasks for JSON
void handleGetTasks() {
  Serial.println("Handling GET tasks request");

  getTasks();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  if (tasks.size() == 0) {
    Serial.println("No tasks in list, adding a default task");
    
    // Create a default task for testing
    TaskData defaultTask = { 1, "Default Task", "2025-08-23", 2, false };

    // Add to tasks vector
    tasks.push_back(defaultTask);
    
    // Also add to Firebase
    FirebaseJson json;
    json.set("id", 1);
    json.set("taskName", "Default Task");
    json.set("dueDate", "2025-08-23");
    json.set("priority", 2);
    json.set("completed", false);
    
    database.set(aClient, "/Tasks/1", object_t(json.raw()));
  }

  String json = "[";
  for (size_t i = 0; i < tasks.size(); i++) {
    json += "{";
    json += "\"id\":";
    json += String(tasks[i].id);
    json += ",";

    json += "\"taskName\":\"";
    json += tasks[i].taskName.c_str();
    json += "\",";

    json += "\"dueDate\":\"";
    json += tasks[i].dueDate.c_str();
    json += "\",";

    json += "\"priority\":";
    json += String(tasks[i].priority);
    json += ",";

    json += "\"completed\":";
    json += String(tasks[i].completed ? "true" : "false");

    json += "}";

    if (i < tasks.size() - 1) json += ",";
  }

  json += "]";
  server.send(200, "application/json", json);
  Serial.print("Sent tasks JSON:");
  Serial.println(json);

  getTasks(); // Refresh tasks after sending

}

void handlePostTask() {
  server.send(200, "text/plain", "Post task endpoint not implemented yet");
}

void handleDeleteTask() {
  // Get params
  if (!server.hasArg("id")) {
    server.send(400, "text/plain", "Missing id parameter");
    return;
  }

  int id = server.arg("id").toInt();

  // Find and remove task from vector
  auto it = std::remove_if(tasks.begin(), tasks.end(), [id](const TaskData &task) {
    return task.id == id;
  });

  tasks.erase(it, tasks.end());

  // Remove from Firebase
  String path = "/Tasks/";
  path += String(id);
}