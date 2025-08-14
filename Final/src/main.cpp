// Main Imports
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Arduino_Json.h>
#include <vector>


#ifndef FIREBASE_SSE_TIMEOUT_MS
#define FIREBASE_SSE_TIMEOUT_MS 40000
#endif

#ifndef ENABLE_ID_TOKEN
#define ENABLE_ID_TOKEN
#endif

// Firebase Imports
#include <FirebaseClient.h>
#include <FirebaseJson.h>

#include <M5Core2.h>

// Local Imports
#include "secrets.h"
#include "Types.h"
#include "Button.h"

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

// Prototypes
void processData(AsyncResult &aResult);
void getTasks();
void checkOffTask(TaskData task);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }

  Serial.println("Initializing..");

  // Setup WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected to WiFi. IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  sslClient.setInsecure(); // Disable SSL certificate verification for simplicity

  // Initialize Firebase
  initializeApp(aClient, app, getAuth(userAuth), processData);
  app.getApp<RealtimeDatabase>(database);
  database.url(FIREBASE_URL);

  M5.begin();
  M5.Lcd.setTextSize(2);



  Serial.println("Initialized!");
}

unsigned long authStart = millis();

void loop() {
  app.loop();

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

  // Send a test task to Firebase
  TaskData testTask = {1, "Test Task", "2023-12-31", 1, false};

  FirebaseJson taskJson;
  taskJson.set("id", testTask.id);
  taskJson.set("taskName", testTask.taskName);
  taskJson.set("dueDate", testTask.dueDate);
  taskJson.set("priority", testTask.priority);
  taskJson.set("completed", testTask.completed);

  String taskPath = "Task";
  taskPath += String(testTask.id);
  
  database.set(aClient, taskPath, taskJson);

  // Get tasks from Firebase
  getTasks();

  if (aClient.lastError().code() == 0)
   {
    Serial.println("Value set successfully.");
   } else 
   {
    Serial.print("Error setting value: ");
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

    FirebaseJson jsonData(aResult.c_str());

    size_t len = jsonData.iteratorBegin();
    
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

        taskJson.get(taskData, "id", id);
        taskJson.get(taskData, "taskName", taskName);
        taskJson.get(taskData, "dueDate", dueDate);
        taskJson.get(taskData, "priority", priority);
        taskJson.get(taskData, "completed", completed);

        TaskData task = {
          .id = id,
          .taskName = taskName.c_str(),
          .dueDate = dueDate.c_str(),
          .priority = priority,
          .completed = completed
        };

        tasks.push_back(task);
      }

      jsonData.iteratorEnd();

      Serial.printf("Key: %s, Value: %s\n", key.c_str(), value.c_str());
    }
  }

}

void getTasks() {
  database.get(aClient, "/tasks");
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

  String path = "/tasks/";
  path += String(task.id);
  database.set(aClient, path, jsonData);
}
