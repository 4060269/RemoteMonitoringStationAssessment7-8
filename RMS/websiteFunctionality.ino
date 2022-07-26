const char* PARAM_INPUT_1 = "tempThreshold";

void routesConfiguration() {

  server.onNotFound([](AsyncWebServerRequest * request) {
    if (request->url().endsWith(F(".jpg"))) {
      // Extract the filename that was attempted
      int fnsstart = request->url().lastIndexOf('/');
      String fn = request->url().substring(fnsstart);
      // Load the image from SPIFFS and send to the browser.
      request->send(SPIFFS, fn, "image/jpeg", true);
    } else {
      request->send(SPIFFS, "/404.html");
    }
  });

  // Duplicated serving of index.html route, so the IP can be entered directly to browser
  // No Authentication
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    logEvent("New connection to root: /");
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // Example of linking to an external file
  server.on("/arduino.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/arduino.css", "text/css");
  });

  // Example of a 'standard' route
  // Landing page to introduce all users to the interface
  // No Authentication
  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    // When the user tries to access the website, show index.html
    // Allow the client to download specific resources using HTTP_GET; "/index.html"
    // Web server continuously listens, AsyncWebServerRequest wraps the new client
    logEvent("New connection to root: /");
    // Log for debug/analysis
    request->send(SPIFFS, "/index.html", "text/html");
    // In the made request, send back the html that can be found in SPIFFS
  });

  // Example of a route with additional authentication (popup in browser)
  // And uses the processor function.
  server.on("/guestDashboard.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(guest_http_username, guest_http_password)) {
      logEvent("Admin Dashboard Access Attempt Failed");
      return request->requestAuthentication();
    }
    logEvent("Guest Dashboard accessed");
    request->send(SPIFFS, "/guestDashboard.html", "text/html", false, processor);
    // The user is not allowed to download files to ensure secure practices
    // Run processor to check and modify certain aspects of request
    // Specifying user for example
    // || !request->authenticate(admin_http_username, admin_http_password)
  });

  server.on("/adminDashboard.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(admin_http_username, admin_http_password)) {
      logEvent("Admin Dashboard Access Attempt Failed");
      return request->requestAuthentication();
    }
    logEvent("Admin Dashboard Successfully Accessed");
    request->send(SPIFFS, "/adminDashboard.html", "text/html", false, processor);
    // The admin is now allowed to download files
    // Run processor to check and modify certain aspects of request
    // Specifying user for examples
  });


  // Example of route with authentication, and use of processor
  // Also demonstrates how to have arduino functionality included (turn LED on)
  server.on("/LEDOn", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(guest_http_username, guest_http_password)) {
      logEvent("LED On: Failed via Guest Dashboard");
      return request->requestAuthentication();
    }
    LEDOn = true;
    logEvent("LED On: Success via Guest Dashboard");
    request->send(SPIFFS, "/guestDashboard.html", "text/html", false, processor);
  });


  server.on("/LEDOff", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(guest_http_username, guest_http_password)) {
      logEvent("LED Off: Failed via Guest Dashboard");
      return request->requestAuthentication();
    }

    LEDOn = false;
    logEvent("LED Off: Success via Guest Dashboard");
    request->send(SPIFFS, "/guestDashboard.html", "text/html", false, processor);
  });


  // Example of route which sets file to download - 'true' in send() command.
  server.on("/logOutput", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(admin_http_username, admin_http_password)) {
      logEvent("Log Event Downloaded: Failed via Admin Dashboard");
      return request->requestAuthentication();
    }
    logEvent("Log Event Downloaded: Success via Admin Dashboard");
    request->send(SPIFFS, "/logEvents.csv", "text/html", true);
  });

  server.on("/SafeLock",  HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(guest_http_username, guest_http_password)) {
      logEvent("Safe Locked: Failed via Guest Dashboard");
      return request->requestAuthentication();
    }
    safeLocked = true;
    logEvent("Safe Locked: Success via Guest Dashboard");
    request->send(SPIFFS, "/guestDashboard.html", "text/html", false, processor);
  });

  server.on("/SafeUnlock",  HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(guest_http_username, guest_http_password)) {
      logEvent("Safe Unlocked: Failed via Guest Dashboard");
      return request->requestAuthentication();
    }
    safeLocked = false;
    logEvent("Safe Unlocked: Success via Guest Dashboard");
    request->send(SPIFFS, "/guestDashboard.html", "text/html", false, processor);
  });

  server.on("/adminSafeLock",  HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(admin_http_username, admin_http_password)) {
      logEvent("Safe Locked: Failed via Administrator Dashboard");
      return request->requestAuthentication();
    }
    safeLocked = true;
    logEvent("Safe Locked: Success via Administrator Dashboard");
    request->send(SPIFFS, "/adminDashboard.html", "text/html", false, processor);
  });

  server.on("/adminSafeUnlock",  HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(admin_http_username, admin_http_password)) {
      logEvent("Safe Unlocked: Failed via Administrator Dashboard");
      return request->requestAuthentication();
    }
    safeLocked = false;
    logEvent("Safe Unlocked: Success via Administrator Dashboard");
    request->send(SPIFFS, "/adminDashboard.html", "text/html", false, processor);
  });

  server.on("/FanOn",  HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(guest_http_username, guest_http_password)) {
      logEvent("Fan Manual Control On: Failed via Guest Dashboard");
      return request->requestAuthentication();
    }
    fanEnabled = true;
    logEvent("Fan Manual Control On: Success via Guest Dashboard");
    request->send(SPIFFS, "/guestDashboard.html", "text/html", false, processor);
  });

  server.on("/FanOff",  HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(guest_http_username, guest_http_password)) {
      logEvent("Fan Manual Control Off: Failed via Guest Dashboard");
      return request->requestAuthentication();
    }
    fanEnabled = false;
    logEvent("Fan Manual Control Off: Success via Guest Dashboard");
    request->send(SPIFFS, "/guestDashboard.html", "text/html", false, processor);
  });

  server.on("/FanControl",  HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(guest_http_username, guest_http_password)) {
      logEvent("Fan Manual Control On: Failed via Guest Dashboard");
      return request->requestAuthentication();
    }
    autoFanEnabled = !autoFanEnabled;
    logEvent("Fan Manual Control On: Success via Guest Dashboard");
    request->send(SPIFFS, "/guestDashboard.html", "text/html", false, processor);
  });

  server.on("/adminSetTemperatureThreshold", HTTP_GET,  [](AsyncWebServerRequest * request) {
    int newThreshold;
    if (request->hasParam(PARAM_INPUT_1)) {
      fanTemperatureThreshold = request->getParam(PARAM_INPUT_1)->value().toFloat();
      String logMessage = "Temperature Threshold: Success via Admin Dashboard" + String(fanTemperatureThreshold);
      logEvent(logMessage);
    }
    request->send(SPIFFS, "/adminDashboard.html", "text/html", false, processor);
  });
}

String getDateTime() {
  DateTime rightNow = rtc.now();
  char csvReadableDate[25];
  sprintf(csvReadableDate, "%02d:%02d:%02d %02d/%02d/%02d",  rightNow.hour(), rightNow.minute(), rightNow.second(), rightNow.day(), rightNow.month(), rightNow.year());
  return csvReadableDate;
}
// Find and store the current date & time for logged events as a string

String processor(const String & var) {
  /*
     Updates the HTML by replacing set variables with return value from here.
     For example:
     in HTML file include %VARIABLEVALUE%.
     In this function, have:
      if (var=="VARIABLEVALUE") { return "5";}
  */

  if (var == "DATETIME") {
    String datetime = getDateTime();
    return datetime;
  }
  // Reading current date and time from sensor to display onto website

  if (var == "TEMPERATURE") {
    return String(tempsensor.readTempC());
  }
  // Reading current temperature from sensor to display onto website

  if (var == "FANCONTROL") {
    if (autoFanEnabled) {
      return "Automatic";
    } else {
      return "Manual";
    }
  }

  if (var == "INVFANCONTROL") {
    if (autoFanEnabled) {
      return "Manual";
    } else {
      return "Automatic";
    }
  }
  // Determining manual or automatic fan control when a user tries to change it through the website

  if (var == "CURRENTTHRESHOLD") {
    return String(fanTemperatureThreshold);
  }

  return String();
  // Default "catch" which will return nothing in case the HTML has no variable to replace.
}
