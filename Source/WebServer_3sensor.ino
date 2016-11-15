/*
  Web Server

 A web server program that shows the value of the analog and digital input pins
 using an Arduino Ethernet shield.
//The program shows the reading of pressure and temperature sensors from analog port and flow meter from digital port 
// the data is taken from sensors and send to Ethernet server 
 */

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
////////////////////////for flow sensor
volatile int flow_frequency; // Measures flow sensor pulses
unsigned int l_hour; // Calculated litres/hour
//unsigned char flowsensor = 4; // Sensor Input
const int flowsensor = 2;
unsigned long currentTime;
unsigned long cloopTime;
void flow () // Interrupt function
{
   flow_frequency++;
}


float readPressure(int pin)/////for pressure sensor
{
    int pressureValue = analogRead(pin);
    float pressure=((pressureValue/1024.0)+0.095)/0.000009;
    return pressure;
}
////////////////////////flow sensor end//////////////////////////////////////////////////

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

IPAddress ip(10, 4, 0, 219);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
  // Open serial communications and wait for port to open:
   ///////////for flow sensor /////////////////////////////
   digitalWrite(flowsensor, HIGH); // Optional Internal Pull-Up
   attachInterrupt(0, flow, RISING); // Setup Interrupt
   sei(); // Enable interrupts
   currentTime = millis();
   cloopTime = currentTime;
///////////for flow sensor end/////////////////////
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        
        char c = client.read();
        
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin
          //for (int analogChannel = 0; analogChannel < 1; analogChannel++) {
            /////////////////////////temp sensor//////////////////////////////////////////
            int analogChannel = 0;//taking analog port '0'
            int sensorReading = analogRead(analogChannel);//reading temperature sensor value 
            client.print("analog input ");// printing the data in ethernet server
            client.print(analogChannel);
            client.print(" for temperature ");
            float mv = ( sensorReading/1024.0)*5000; 
            float cel = mv/10;
            float farh = (cel*9)/5 + 32;
            client.print(cel);
            client.print("*C");
            client.print("\n");
            client.println("<br />");
            Serial.print(cel);
            Serial.print("*C"); 
            
           /////////////////////////pressure sensor//////////////////////////////////////////
          
            analogChannel = 1;
            float pressure = readPressure(analogChannel);//reading analog port '1'
            float millibars = pressure/100;
            client.print("analog input ");//printing the value
            client.print(analogChannel);
            client.println();
            client.print("Pressure = ");
            client.print(pressure);
            client.println(" pascals");
            client.print("Pressure = ");
            client.print(millibars);
            client.println(" millibars");
            
            Serial.print("Pressure = ");
            Serial.print(pressure);
            Serial.println(" pascals");
            client.print("\n");
            client.println("<br />");
            /////////////////////////water flow sensor//////////////////////////////////////////
            pinMode(flowsensor, INPUT);//define as input pin
            currentTime = millis();
            // Every second, calculate and print litres/hour
            if(currentTime >= (cloopTime + 1000))
            {
              cloopTime = currentTime; // Updates cloopTime
              // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
              l_hour = (flow_frequency * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
              flow_frequency = 0; // Reset Counter
              client.print("Digital input 2");
            
              client.print(" Flow is ");
              client.print(l_hour, DEC); // Print litres/hour
              client.println(" L/hour");
               client.print("\n");
               client.println("<br />");
              Serial.print(" Flow is ");
              Serial.print(l_hour, DEC); // Print litres/hour
              Serial.println(" L/hour"); 
           
            }
            
            /////////////////////////////////////////////////////////////////////////////
            delay(1000);
          client.println("<br />");
          //}
          client.println("</html>");
          break;
        }
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') 
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

