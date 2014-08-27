#include <SPI.h>
#include <SFE_CC3000.h>
#include <SFE_CC3000_Client.h>
#include <SoftwareSerial.h>

// Attach the serial display's RX line to digital pin 2
SoftwareSerial LCD(4, 3); // RX, TX

// Pins
#define CC3000_INT      2   // Needs to be an interrupt pin (D2/D3)
#define CC3000_EN       7   // Can be any digital pin
#define CC3000_CS       10  // Preferred is pin 10 on Uno

// analog I/O pins
const byte light = A5;
const byte temp = A0;

// Wifi Setup
char ap_ssid[] = "";
char ap_password[] = "";
unsigned int ap_security = WLAN_SEC_WPA2;
unsigned int timeout = 30000;
#define IP_ADDR_LEN     4

// Global Variables
SFE_CC3000 wifi = SFE_CC3000(CC3000_INT, CC3000_EN, CC3000_CS);
SFE_CC3000_Client client = SFE_CC3000_Client(wifi);

//Weather Const
float tempVal;
int lightVal;
byte count = 2;
const float opVoltage = 4.7;
boolean firstItem = false;

//Sparkfun data service
char server[] = "data.sparkfun.com";
const String publicKey = "";
int dataPosition = 0;
String weatherData[17];

void clearScreen()
{
	LCD.write(0xFE);
	LCD.write(0x01);
}
void configureWifi()
{
	Serial.print("Connecting to SSID: ");
	Serial.println(ap_ssid);
	while (!wifi.connect(ap_ssid, ap_security, ap_password, timeout))
	{
		delay(100);
	}
}
void initWifi()
{
	boolean connectToWifi = true;

	while (connectToWifi)
	{
		// Initialize CC3000 (configure SPI communications)
		if (wifi.init()) {
			Serial.println("CC3000 initialization complete");
			connectToWifi = false;
		}
		else
		{
			delay(100);
		}
	}

	configureWifi();
}
void setup()
{
	// Initialize Serial port
	Serial.begin(9600);
	LCD.begin(9600);

	clearScreen();
	selectLineOne();
	LCD.print("My Home");
	selectLineTwo();
	LCD.print("Weather Forcast");

	initWifi();
	pinMode(light, INPUT);
	pinMode(temp, INPUT);
}
void getWeatherData()
{
	// Make a TCP connection to remote host
	Serial.print("Performing HTTP GET of: ");
	Serial.println(server);
	if (!client.connect(server, 80))
	{
	}
	else
	{
		// Make a HTTP GET request
		client.print("GET /output/");
		client.print(publicKey);
		client.print(".csv?page=1");

		client.println(" HTTP/1.1");
		client.print("Host: ");
		client.println(server);
		client.println("Connection: close");
		client.println();

		dataPosition = 0;
		char lastChar = '0';
		boolean beginData = false;
		firstItem = false;

		while (client.connected())
		{
			if (client.available())
			{
				char c = client.read();

				if (!beginData && lastChar == '7' && (c == 'b' || c == 'c'))
				{
					beginData = true;
				}
				else if (beginData)
				{
					breakOutData(c);
				}
				else
				{
					lastChar = c;
				}
			}
		}
	}
}
void breakOutData(char c)
{
	if (dataPosition < 17)
	{
		if (!firstItem && dataPosition == 0)
		{
			weatherData[dataPosition] = "";
			firstItem = true;
		}

		if (c == ',')
		{
			dataPosition++;
			if (dataPosition < 17)
			{
				weatherData[dataPosition] = "";
			}
		}
		else if (dataPosition != 9 && dataPosition != 6)
		{
			if (c != '\n' && c != ' ' && c != '\r')
			{
				weatherData[dataPosition] += c;
			}
		}
		else if (dataPosition == 16)
		{
			dataPosition++;
		}
	}
}
void selectLineOne()
{
	LCD.write(0xFE); //command flag
	LCD.write(128); //position
}
void selectLineTwo()
{
	LCD.write(0xFE); //command flag
	LCD.write(192); //position
}
void loop()
{
	if (count == 2)
	{
		getWeatherData();
		count = 0;
	}
	else
	{
		count++;
	}

	if (weatherData[8] != "")
	{
		clearScreen();
		selectLineOne();
		LCD.print("Temp = ");
		LCD.print(weatherData[8]);
		LCD.print("F");

		selectLineTwo();
		LCD.print("Humidity = ");
		LCD.print(weatherData[4]);
		LCD.print("%");

		delay(10000);
	}

	if (weatherData[16] != "")
	{
		clearScreen();
		selectLineOne();
		LCD.print("Wind = ");
		LCD.print(weatherData[16]);
		LCD.print("MPH");

		selectLineTwo();
		LCD.print("Gust = ");
		LCD.print(weatherData[14]);
		LCD.print("MPH");

		delay(10000);
	}

	if (weatherData[15] != "")
	{
		clearScreen();
		selectLineOne();
		LCD.print("Avg Wind:");
		LCD.print(weatherData[15]);
		LCD.print("MPH");

		selectLineTwo();
		LCD.print("Wind Dir: ");

		int weatherDir = weatherData[11].toInt();

		if (weatherDir < 380) LCD.print("ESE");
		else if (weatherDir < 393) LCD.print("ENE");
		else if (weatherDir < 414) LCD.print("E");
		else if (weatherDir < 456) LCD.print("SSE");
		else if (weatherDir < 508) LCD.print("SE");
		else if (weatherDir < 551) LCD.print("SSW");
		else if (weatherDir < 615) LCD.print("S");
		else if (weatherDir < 680) LCD.print("NNE");
		else if (weatherDir < 746) LCD.print("NE");
		else if (weatherDir < 801) LCD.print("WSW");
		else if (weatherDir < 833) LCD.print("SW");
		else if (weatherDir < 878) LCD.print("NNW");
		else if (weatherDir < 913) LCD.print("N");
		else if (weatherDir < 940) LCD.print("WNW");
		else if (weatherDir < 967) LCD.print("NW");
		else if (weatherDir < 990) LCD.print("W");
	}

	readSensors();

	clearScreen();
	selectLineOne();
	LCD.print("Inside Room");
	selectLineTwo();
	LCD.print("Temp = ");
	LCD.print(tempVal);
	LCD.print(" F");

	delay(10000);
}
void readSensors()
{
	tempVal = ((analogRead(temp)*opVoltage / 1024.0) - 0.5) * 100;
	tempVal = (tempVal * 9.0 / 5.0) + 32.0; // Convert to farenheit
	lightVal = analogRead(light);
}