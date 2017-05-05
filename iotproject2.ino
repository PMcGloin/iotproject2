#include <Bridge.h>
#include <Temboo.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include "TembooAccount.h"

BridgeServer server;
const String CLIENT_ID = "1027676406829-jioum6bp454oomn41vlevv2mlfkm7m8p.apps.googleusercontent.com";
const String CLIENT_SECRET = "_viNzWO7a5xeNuwM6B9pBufv";
const String REFRESH_TOKEN = "1/K5Q3tTEO9DmVSG88IyoBHh78TwjkiQGo5tCcjZif3Nw";
const String SPREADSHEET_ID = "1U6WkNuvD1rrQgrVizimstRyrXRz8AVj5IWRV6YODDpc";
String socketStatus = "Socket Status";

void setup() {
  Bridge.begin();  // make contact with the linux processor
  Console.begin();
  delay(1000);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  Bridge.begin();
  digitalWrite(2, HIGH);
  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  // Get clients coming from server
  BridgeClient client = server.accept();

  // There is a new client?
  if (client) {
    // Process request
    process(client);

    // Close connection and free resources.
    client.stop();
  }

  delay(50); // Poll every 50ms
}

void process(BridgeClient client) {
  // read the command
  String command = client.readStringUntil('/');

  // is "digital" command?
  if (command == "digital") {
    digitalCommand(client);
  }

  // is "analog" command?
  if (command == "analog") {
    analogCommand(client);
  }

  // is "mode" command?
  if (command == "mode") {
    modeCommand(client);
  }
}

void digitalCommand(BridgeClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/digital/13/1"
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
    
    // we need a Process object to send a Choreo request to Temboo
    TembooChoreo AppendValuesChoreo;

    // invoke the Temboo client
    // NOTE that the client must be reinvoked and repopulated with
    // appropriate arguments each time its run() method is called.
    AppendValuesChoreo.begin();

    // set Temboo account credentials
    AppendValuesChoreo.setAccountName(TEMBOO_ACCOUNT);
    AppendValuesChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    AppendValuesChoreo.setAppKey(TEMBOO_APP_KEY);

    // identify the Temboo Library choreo to run (Google > Sheets > AppendValues)
    AppendValuesChoreo.setChoreo("/Library/Google/Sheets/AppendValues");

    // set the required Choreo inputs
    // see https://www.temboo.com/library/Library/Google/Sheets/AppendValues/
    // for complete details about the inputs for this Choreo

    // your Google application client ID
    AppendValuesChoreo.addInput("ClientID", CLIENT_ID);
    // your Google application client secret
    AppendValuesChoreo.addInput("ClientSecret", CLIENT_SECRET);
    // your Google OAuth refresh token
    AppendValuesChoreo.addInput("RefreshToken", REFRESH_TOKEN);

    // the ID of the spreadsheet you want to append to
    AppendValuesChoreo.addInput("SpreadsheetID", SPREADSHEET_ID);
     
    // convert the time and sensor values to a comma separated string
    String rowData = "[[\"" +  String(socketStatus) + "\", \"" + (value) + "\"]]";

    // add the RowData input item
    AppendValuesChoreo.addInput("Values", rowData);
    // run the Choreo and wait for the results
    // The return code (returnCode) will indicate success or failure
    unsigned int returnCode = AppendValuesChoreo.run();

    Console.println("Success! Appended " + rowData);
    Console.println("");

    AppendValuesChoreo.close();
   }
    else {
    value = digitalRead(pin);
  }

  // Send feedback to client
  client.print(F("Pin D"));
  client.print(pin);
  client.print(F(" set to "));
  client.println(value);

  // Update datastore key with the current pin value
  String key = "D";
  key += pin;
  Bridge.put(key, String(value));
}

void analogCommand(BridgeClient client) {
  int pin, value;

  // Read pin number
  pin = client.parseInt();

  // If the next character is a '/' it means we have an URL
  // with a value like: "/analog/5/120"
  if (client.read() == '/') {
    // Read value and execute command
    value = client.parseInt();
    analogWrite(pin, value);

    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" set to analog "));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "D";
    key += pin;
    Bridge.put(key, String(value));
  } else {
    // Read analog pin
    value = analogRead(pin);

    // Send feedback to client
    client.print(F("Pin A"));
    client.print(pin);
    client.print(F(" reads analog "));
    client.println(value);

    // Update datastore key with the current pin value
    String key = "A";
    key += pin;
    Bridge.put(key, String(value));
  }
}

void modeCommand(BridgeClient client) {
  int pin;

  // Read pin number
  pin = client.parseInt();

  // If the next character is not a '/' we have a malformed URL
  if (client.read() != '/') {
    client.println(F("error"));
    return;
  }

  String mode = client.readStringUntil('\r');

  if (mode == "input") {
    pinMode(pin, INPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as INPUT!"));
    return;
  }

  if (mode == "output") {
    pinMode(pin, OUTPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as OUTPUT!"));
    return;
  }

  client.print(F("error: invalid mode "));
  client.print(mode);
}



