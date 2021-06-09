/***********************************************************************************************  
 *   Interaction Test for the Control Units of a WPT system  
 *  Features: Communication and cooperation of the control units through ethernet  
 *  Details:
 *    1. the web browser calls one of the units as a master, when 
 *    2. one of the units received the signal as the server,it turns on the red LED as signal, 
 *    3. master unit send slave signals to the neihgbouring units via GET request
 *    4. the other unit received the slave signal and turns on the green LED.
 *    5. when one of them receives the off signal, both LEDs should be turned off.
 *  Version: 1.0.0
 *  Last Modified: 12.10.2020
 *  By: Xiaooyue Zhu
 ***********************************************************************************************/

 
#include <SPI.h>
#include <Ethernet2.h>

// definition the enable/disable ports for Ethernet Shield WIZnet5500 and SD Card
#define CS1 10 // D10: CS1(Chip Selection) for W5500
#define RST1 11 // D11: RST(Reset) for W5500 
#define CS2 4 // D4: CS2 pin for SD-Card

// PWM outputs on D6 and D12 
#define Master 6
#define Slave 12

// red LED signs for the master and green for slave
#define LED_Red 3
#define LED_Green 2

byte mac[] = { 0x54, 0x55, 0x58, 0x10, 0x00, 0x03 }; //ethernet shield mac address
byte ip[] = { 192, 168, 1, 173 }; // arduino IP in lan slave 05
byte gateway[] = { 192, 168, 1, 1 }; // internet access via router
byte subnet[] = { 255, 255, 255, 0 }; //subnet mask

EthernetServer server(80); 

String readString;       //store the information from a client

void Mater_PWM_Act(){
    TCCR4B = _BV(CS40);       // CS4<3:0>:Clock Select bits. clock @ F_CPU / 1
    TCCR4C = _BV(COM4D0)| _BV(PWM4D) ; //non-inverting PWM
    TCCR4D = 0;
    TCCR4E = _BV(OC4OE1)|_BV(OC4OE0);      
    OCR4C  = 159; 
    OCR4D  = 79; 
}

void setup() 
{
  pinMode(CS1, OUTPUT);
  pinMode(RST1, OUTPUT);
  pinMode(CS2, OUTPUT);

  pinMode(Master, OUTPUT);
  pinMode(Slave, OUTPUT);

  pinMode(LED_Red, OUTPUT);    // LED_Red, Sign for Master
  pinMode(LED_Green, OUTPUT);  // LED_Green, Sign for Slave

  digitalWrite(CS1, HIGH);     // CS1 for Ethernet Shield on
  digitalWrite(CS2, LOW);      // CS2 for SD Card off
  digitalWrite(RST1, HIGH);    // Reset off
  
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  server.begin();
  
  Serial.begin(9600);

  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  //PLLFRQ |= (1 << PLLTM0);  PPL Clock
  TCCR4A = 0;
  TCCR4B = 0;   // CS4<3:0>:Clock Select bits. clock @ F_CPU / 1
  TCCR4C = 0; //non-inverting PWM on OC1B
  TCCR4D = 0;
  TCCR4D = 0;
}


void loop()
{   
  // listen for incoming clients
  EthernetClient client = server.available(); //create a client when the connection is built 
  if (client) 
  {
    Serial.println("client connected");
    // Make a HTTP request:
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        if (readString.length() < 100) 
        {
          readString += c;                     //store characters to string
          Serial.print(c);
        }
        Serial.write(c);
        // HTTP request has ended          
        if (c == '\n') 
        {
          // you're starting a new line
          Serial.println(readString);
          if(readString.indexOf('?') >=0)        //don't send new page
          { 
            client.println("HTTP/1.1 204 Xiaoyue");
            client.println();
            client.println(); 
          }
          else
          {
            client.println("HTTP/1.1 200 OK");    //send new page
            client.println("Content-Type: text/html");
            client.println();
            client.println("<HTML>");
            client.println("<HEAD>");
            client.println("<TITLE>Arduino GET test page</TITLE>");
            client.println("</HEAD>");
            client.println("<BODY>");
            client.println("<H1>Logic Test:A3~A5</H1>");
            //A1
            client.println("<a href='http://192.168.1.171:80/?master' target='inlineframe'>A1 Master</a>");
            client.println("<a href='http://192.168.1.171:80/?slave' target='inlineframe'>A1 Slave</a>");
            client.println("<a href='http://192.168.1.171:80/?off' target='inlineframe'>A1 OFF</a>");
            //A3
            client.println("<br><br><a href='http://192.168.1.173:80/?master' target='inlineframe'>A3 Master</a>");
            client.println("<a href='http://192.168.1.173:80/?slave' target='inlineframe'>A3 Slave</a>");
            client.println("<a href='http://192.168.1.173:80/?off' target='inlineframe'>A3 OFF</a>");
            //A4
            client.println("<br><br><a href='http://192.168.1.174:80/?master' target='inlineframe'>A4 Master</a>");
            client.println("<a href='http://192.168.1.174:80/?slave' target='inlineframe'>A4 Slave</a>");
            client.println("<a href='http://192.168.1.174:80/?off' target='inlineframe'>A4 OFF</a>");
            //A5
            client.println("<br><br><a href='http://192.168.1.175:80/?master' target='inlineframe'>A5 Master</a>");
            client.println("<a href='http://192.168.1.175:80/?slave' target='inlineframe'>A5 Slave</a>");
            client.println("<a href='http://192.168.1.175:80/?off' target='inlineframe'>A5 OFF</a>");
            client.println("<IFRAME name=inlineframe style='display:none'>");         
            client.println("</IFRAME>");
            client.println("</BODY>");
            client.println("</HTML>");
          }
          delay(1);
          //stopping client
          client.stop();
          if(readString.indexOf("master") >0)
          {
            digitalWrite(LED_Red,HIGH);          // lighted on as symbol for master
            Serial.println("master");
            TCCR4B = _BV(CS40);                  // CS4<3:0>:Clock Select bits. clock @ F_CPU / 1
            TCCR4C = _BV(COM4D0)| _BV(PWM4D) ;   //non-inverting PWM
            TCCR4D = 0;
            TCCR4E = _BV(OC4OE1)|_BV(OC4OE0);      
            OCR4C  = 159; 
            OCR4D  = 79; 
            IPAddress server1(192, 168, 1, 171 );
            EthernetClient client;
            if (client.connect(server1, 80)) 
            {
              Serial.println("connected");
              // Make a HTTP request:
              client.println("GET /?slave HTTP/1.1");
              //client.println("Host: 192.168.1.174");
              //client.println("Connection: close");
              client.println();
              client.stop();
              delay(1);
             }
            IPAddress server2(192, 168, 1, 174 );
            if (client.connect(server2, 80)) 
            {
              Serial.println("connected");
              // Make a HTTP request:
              client.println("GET /?slave HTTP/1.1");
              //client.println("Host: 192.168.1.174");
              //client.println("Connection: close");
              client.println();
              delay(1);
             } 
          }
          if(readString.indexOf("off") >0)
          {
            TCCR4C = 0;
            digitalWrite(LED_Red,LOW);
            digitalWrite(LED_Green,LOW);          // lighted on as symbol for master
            Serial.println("off");
          }        
          if(readString.indexOf("slave") >0)
          {
            TCCR4C = 0;
            digitalWrite(LED_Green,HIGH);
            digitalWrite(LED_Red,LOW);           // wait for a second// blink as symbol for slave
            Serial.println("slave");
          }
          readString="";                 
        }
      } 
    }
  }
}
