/****************************************************************
Hardware Connections:
 
 Uno Pin    CC3000 Board    Function
 
 +5V        VCC or +5V      5V
 GND        GND             GND
 2          INT             Interrupt
 7          EN              WiFi Enable
 10         CS              SPI Chip Select
 11         MOSI            SPI MOSI
 12         MISO            SPI MISO
 13         SCK             SPI Clock
****************************************************************/

#include <SPI.h>
#include <SFE_CC3000.h>
#include <SFE_CC3000_Client.h>

// Pins
#define CC3000_INT      2   // Needs to be an interrupt pin (D2/D3)
#define CC3000_EN       7   // Can be any digital pin
#define CC3000_CS       10  // Preferred is pin 10 on Uno

// Connection info data lengths
#define IP_ADDR_LEN     4   // Length of IP address in bytes

// Constants
char ap_ssid[] = "";                  // SSID of network
char ap_password[] = "";          // Password of network
unsigned int ap_security = WLAN_SEC_WPA2; // Security of network
unsigned int timeout = 30000;             // Milliseconds

//Weather Const


char server[] = "data.sparkfun.com";        // Remote host site
const String publicKey = "";

int dataPosition = 0;

//18 is the max amount of data points
String weatherData[17];

// Global Variables
SFE_CC3000 wifi = SFE_CC3000(CC3000_INT, CC3000_EN, CC3000_CS);
SFE_CC3000_Client client = SFE_CC3000_Client(wifi);

void configureWifi()
{
  // Connect using DHCP
  Serial.print("Connecting to SSID: ");
  Serial.println(ap_ssid);
  while(!wifi.connect(ap_ssid, ap_security, ap_password, timeout)) {
    Serial.println("Error: Could not connect to AP...Reconnecting....");
    delay(100);
  }
}

void initWifi()
{
  int i;
  boolean connectToWifi = true;
  ConnectionInfo connection_info;
  
  
  while(connectToWifi)
  {
    // Initialize CC3000 (configure SPI communications)
    if ( wifi.init() ) {
      Serial.println("CC3000 initialization complete");
      connectToWifi = false;
    } else {
      Serial.println("Something went wrong during CC3000 init!...Trying Agian");
      delay(100);
    }
  }
  
  configureWifi();
  
  // Gather connection details and print IP address
  if ( !wifi.getConnectionInfo(connection_info) ) {
    Serial.println("Error: Could not obtain connection details");
  } else {
    Serial.print("IP Address: ");
    for (i = 0; i < IP_ADDR_LEN; i++) {
      Serial.print(connection_info.ip_address[i]);
      if ( i < IP_ADDR_LEN - 1 ) {
        Serial.print(".");
      }
    }
    Serial.println();
  }
}

void setup() {
  
  int i;
  
  // Initialize Serial port
  Serial.begin(9600);
  Serial.println();
  Serial.println("---------------------------");
  Serial.println("CC3000 - WeatherClient is Online");
  Serial.println("---------------------------");
  
  initWifi();
}

void getWeatherData()
{
  
  // Make a TCP connection to remote host
  Serial.print("Performing HTTP GET of: ");
  Serial.println(server);
  if ( !client.connect(server, 80) ) {
    Serial.println("Error: Could not make a TCP connection");
  }
  
  // Make a HTTP GET request
  client.print("GET /output/");
  client.print(publicKey);
  client.print(".csv?page=1");
  
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(server);
  client.println("Connection: close");
  client.println();
   
  boolean startOfData = false;
  boolean foundNewLine = false;
  char lastChar = '0';
   
  while (client.connected())
  {
    if ( client.available() )
    {
      char c = client.read();
      
      if(startOfData && c == '\n')
      {
        if(foundNewLine)
        {
          break; 
        }
        else
        {
          foundNewLine = true;
        }
      }
      
      if(startOfData && c != '\n')
      {
        //Serial.print(c);
        breakOutData(c);
      }
      
      if(!startOfData && c == '5' && lastChar == '8')
      {
        startOfData = true;
      }
      else
      {  
        lastChar = c;
      }
          
    }      
  }

}

void breakOutData(char c)
{
  //Order of data
  //baromin,batt_lvl,dailyrainin,dewptf,high_glitch,humidity,light_lvl,low_glitch,measurementTime,rainin,tempf,timestamp,winddir,winddir_avg2m,windgustdir,windgustdir_10m,windgustmph,windgustmph_10m,windspdmph_avg2m,windspeedmph
  
  if(c == ',')
  {
    Serial.print(dataPosition);
    Serial.println(": " + weatherData[dataPosition]);
    dataPosition++; 
  }
  else
  {
    //Serial.println(weatherData[dataPosition]);
    weatherData[dataPosition] = weatherData[dataPosition] + c;
  }
}

void loop() 
{
  //Wait for the imp to ping us with the ! character
  if(Serial.available())
  {
    byte incoming = Serial.read();
    if(incoming == '!')
    {
        getWeatherData();
        Serial.println("Weather Download Complete");
    }
  
  }  
  
  delay(1000);
 
 
}
