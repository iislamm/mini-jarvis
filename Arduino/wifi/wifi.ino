/*
  This code is written to be uploaded to ESP8266 microcontroller (NodeMCU)
  The microcontroller connects to a wifi network and sends and recieve requests from the api that we've created for this project
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define READ_NUM 3 // how many types of data will be sent from arduino

/* WIFI CREDENTIALS */
#define SSID ""  //WIFI SSID
#define PASSWORD "" // WIFI PASSWORD

StaticJsonDocument<200> data; // create a json document (just a variable) to save the data in it and be able to map throw it easily
 
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(SSID, PASSWORD);     //Connect to the WIFI network
 
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
 
//  connection successful
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(SSID);
}
 

void loop() {
  int temp;
  boolean LDRStatus;
  int motionState;
  // get the data from the api
  getData();
  // read the data sent from the arduino
  for (int i = 0; i < READ_NUM; i++) {
    if(Serial.available()) {
      // read the flag sent from arduino 
      char action = Serial.read();
      delay(100);

      if (action == 't') { // a new temperature is sent
        temp = Serial.read();
        delay(100);
      }
      else if (action == 'l'){ // a new outside lighting status sent
        LDRStatus = Serial.read();
        delay(100);
      }
      else if (action == 'm') { // a person has left or entered
        motionState = Serial.read();
        delay(100);        
      }
  
      if (action == 't') {
        // send the new temperature to the API
        Serial.println("Temp");
        sendTemp(temp);
        delay(100);
      } else if (action == 'l') {
        // send the new outside lighting status to the API to change the current lighting value
        Serial.println("LDR");
        sendLDR(LDRStatus);
        delay(100);
      }
      else if (action == 'm') {
        // send the API that someone left or entered // the motionState tells wether someone entered or left
        Serial.println("Motion");
        sendMotion(motionState);
        delay(100);
      }
    }
  }
  
  
  delay(5000);
}

/*
  This fucntion gets the current settings from the API and send it to the arduino
*/

void getData() {
  HTTPClient http;    //Declare object of class HTTPClient
  
  http.begin("http://us-central1-mini-jarvis-66d33.cloudfunctions.net/getSettings");  //Specify request destination
  int httpCode = http.GET(); //Send the request
  if (httpCode > 0) { //If the request is successfull
  
    String payload = http.getString();   //Get the request response payload // The response is sent in a JSON format but the http.getString converts it to a string
    
    deserializeJson(data, payload); // convert the recieved data back to JSON and store it in the data JSON document

    int lighting = data["lighting"]; // store the lighting value
    Serial.write('l'); // 'l' is the code for arduino to understand that the following value is the new lighting value
    delay(100);
    Serial.write(lighting); // send the new lighting value to the arduino

    delay(100);
    
    char doorOpenState = data["doorOpenState"]; // store the open state of the door
    Serial.write('d'); // 'd' is the code for arduino to understand that the following value is the new door open state
    delay(100);
    Serial.write(doorOpenState); // send the door open state
    
    delay(100);
    
    char hangingsOpenState = data["hangingsOpenState"];
    Serial.write('h'); // 'h' is the code for arduino to understand that the following value is the new hangings open state
    delay(100);
    Serial.write(hangingsOpenState); // send the new haningings open state
  }
  
  http.end(); // END THE CONNECTION
}


/*
  This function send the new temperature value to the API to update it
*/
void sendTemp(int temp) { // temp is the new temperature value
  String tempString = String(temp); // convert the new temperature value to string to be able to send it
  HTTPClient http;
  http.begin("http://us-central1-mini-jarvis-66d33.cloudfunctions.net/setWeather"); // begin a connection with the API
  http.addHeader("Content-Type", "text/plain"); // set the content type of the sent data
  int code = http.POST(tempString); // send a POST request containing the data
  http.end(); // end the connection
}

/*
 * This function send the new state of the lighting to the API to update the lighting value
*/

void sendLDR(int state) { // state paremter contains the commnand to update the lighting eighter to switch it off or on
  String reqString = String(state); // convert the state to string to be able to send it
  HTTPClient http;
  http.begin("http ://us-central1-mini-jarvis-66d33.cloudfunctions.net/swithcLights"); // begin the connection to the API
  http.addHeader("Content-Type", "text/plain"); // set the content type of the data that will be sent
  int code = http.POST(reqString); // send a POST request containing the commant
  http.end(); // end the connection
}

void sendMotion(int state) { // the state parameter contains the state of the motion, either someine entered or left // 1 entered; 2 left
  String stateString = String(state); // conver the state to string to be able to send it
  HTTPClient http;
  http.begin("http://us-central1-mini-jarvis-66d33.cloudfunctions.net/motionDetected"); // connect to the API
  http.addHeader("Content-Type", "text/plain"); // set the content type of the  data that will be sent
  int code = http.POST(stateString); // send a POST request containing the state
  http.end(); // end the connection
}
