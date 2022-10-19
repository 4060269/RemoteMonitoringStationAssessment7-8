void routesConfiguration() {

  server.onNotFound([](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/404.html");
  });

  // Duplicated serving of index.html route, so the IP can be entered directly to browser
  // No Authentication
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    logEvent("route: /");
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
    logEvent("route: /");
    // Log for debug/analysis
    request->send(SPIFFS, "/index.html", "text/html");
    // In the made request, send back the html that can be found in SPIFFS
  });

  // Example of a route with additional authentication (popup in browser)
  // And uses the processor function.
  server.on("/dashboard.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    logEvent("Dashboard");
    request->send(SPIFFS, "/dashboard.html", "text/html", false, processor);
    // The user is not allowed to download files to ensure secure practices
    // Run processor to check and modify certain aspects of request
    // Specifying user for example
  });


  // Example of route with authentication, and use of processor
  // Also demonstrates how to have arduino functionality included (turn LED on)
  server.on("/LEDOn", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    LEDOn = true;
    logEvent("LED turned on from website");
    request->send(SPIFFS, "/dashboard.html", "text/html", false, processor);
  });


  server.on("/LEDOff", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    LEDOn = false;
    logEvent("LED turned off from website");
    request->send(SPIFFS, "/dashboard.html", "text/html", false, processor);
  });


  // Example of route which sets file to download - 'true' in send() command.
  server.on("/logOutput", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    logEvent("Log Event Downloaded");
    request->send(SPIFFS, "/logEvents.csv", "text/html", true);
  });

  server.on("/SafeLock",  HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    safeLocked = true;
    logEvent("Safe Locked via Website");
    request->send(SPIFFS, "/dashboard.html", "text/html", false, processor);
  });

  server.on("/SafeUnlock",  HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    safeLocked = false;
    logEvent("Safe Unlocked via Website");
    request->send(SPIFFS, "/dashboard.html", "text/html", false, processor);
  });

  server.on("/FanOn",  HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    fanEnabled = true;
    logEvent("Fan Manual Control: On");
    request->send(SPIFFS, "/dashboard.html", "text/html", false, processor);
  });

  server.on("/FanOff",  HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    fanEnabled = false;
    logEvent("Fan Manual Control: Off");
    request->send(SPIFFS, "/dashboard.html", "text/html", false, processor);
  });

  server.on("/FanControl",  HTTP_GET, [](AsyncWebServerRequest * request) {
    if (!request->authenticate(http_username, http_password))
      return request->requestAuthentication();
    autoFanEnabled = !autoFanEnabled;
    logEvent("Fan Manual Control: On");
    request->send(SPIFFS, "/dashboard.html", "text/html", false, processor);
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
    if (automaticFan) {
      return "Automatic";
    } else {
      return "Manual";
    }
  }

  if (var == "INVFANCONTROL") {
    if (autoFanEnabled) {
      return "  ";
    } else {
      return "Automatic";
    }
  }
  // Determining manual or automatic fan control when a user tries to change it through the website

  return String();
  // Default "catch" which will return nothing in case the HTML has no variable to replace.
}
