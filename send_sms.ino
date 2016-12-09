#include <LGSM.h>
#include <LGPS.h>


#define CellNumber "6506653100"
#define SmsMessage "position" //later change this to the GPS position

double latitude;
double longitude;

gpsSentenceInfoStruct info; //needed to get the GPS data
char buff[256];

static unsigned char getComma(unsigned char num,const char *str)
{
  unsigned char i,j = 0;
  int len=strlen(str);
  for(i = 0;i < len;i ++)
  {
     if(str[i] == ',')
      j++;
     if(j == num)
      return i + 1; 
  }
  return 0; 
}

static double getDoubleNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atof(buf);
  return rev; 
}

static double getIntNumber(const char *s)
{
  char buf[10];
  unsigned char i;
  double rev;
  
  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atoi(buf);
  return rev; 
}

double parseGPGGA(const char* GPGGAstr)
{
  /* Refer to http://www.gpsinformation.org/dale/nmea.htm#GGA
   * Sample data: $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
   * Where:
   *  GGA          Global Positioning System Fix Data
   *  123519       Fix taken at 12:35:19 UTC
   *  4807.038,N   Latitude 48 deg 07.038' N
   *  01131.000,E  Longitude 11 deg 31.000' E
   *  1            Fix quality: 0 = invalid
   *                            1 = GPS fix (SPS)
   *                            2 = DGPS fix
   *                            3 = PPS fix
   *                            4 = Real Time Kinematic
   *                            5 = Float RTK
   *                            6 = estimated (dead reckoning) (2.3 feature)
   *                            7 = Manual input mode
   *                            8 = Simulation mode
   *  08           Number of satellites being tracked
   *  0.9          Horizontal dilution of position
   *  545.4,M      Altitude, Meters, above mean sea level
   *  46.9,M       Height of geoid (mean sea level) above WGS84
   *                   ellipsoid
   *  (empty field) time in seconds since last DGPS update
   *  (empty field) DGPS station ID number
   *  *47          the checksum data, always begins with *
   */

  String long_dir; 
  int tmp, tmp2, hour, minute, second, num ;
  String positionMessage; 
  
  if(GPGGAstr[0] == '$')
  {
//    tmp = getComma(1, GPGGAstr);
//    hour     = (GPGGAstr[tmp + 0] - '0') * 10 + (GPGGAstr[tmp + 1] - '0');
//    minute   = (GPGGAstr[tmp + 2] - '0') * 10 + (GPGGAstr[tmp + 3] - '0');
//    second    = (GPGGAstr[tmp + 4] - '0') * 10 + (GPGGAstr[tmp + 5] - '0');
//    
//    sprintf(buff, "UTC timer %2d-%2d-%2d", hour, minute, second);
//    Serial.println(buff);
    
    tmp = getComma(2, GPGGAstr);//Latitude is the 2nd number in the array
    latitude = getDoubleNumber(&GPGGAstr[tmp]);
    //hardcode to bring into google maps format
    latitude = latitude / 100;
    
    tmp = getComma(4, GPGGAstr);//longitude is the 4th number in the array
    longitude = getDoubleNumber(&GPGGAstr[tmp]);
    
    //in the product, need to find if longitude is W or E and convert appropriately
    //currently hardcode to convert to W longitude
    longitude = longitude*-1;
    //also hardcode to bring it into google maps format
    longitude = longitude / 100;
    
    
    //sprintf(buff, "latitude = %10.8f, longitude = %10.8f", latitude, longitude);//prints at 10 characters and 4 decimal places
    Serial.println(buff); 
    
//    tmp = getComma(7, GPGGAstr);
//    num = getIntNumber(&GPGGAstr[tmp]);    
//    sprintf(buff, "satellites number = %d", num);
//    Serial.println(buff); 
  }
  else
  {
    Serial.println("I did not get any GPS data"); 
  }
  return latitude, longitude;
}

//Actual code for the system!

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //this while loop checks to see if the sim card is ready (initialized). 
  //if it's not ready, it waits a second and tries again. 
  while(!LSMS.ready())
    {
    delay(1000);
    }

  //once the sim card is ready, it proceeds to send the message
  Serial.println("SIM ready for work!");
  Serial.println("I get here");
    
  //program the GPS module  
  LGPS.powerOn();
  Serial.println("LGPS Power on, and waiting ...");
  //delay(3000);
}

void loop() {
  // put your main code here, to run repeatedly:
//put code here that runs repeatedly such as checking on a sensor or sms is received
  char numBuf[20];
  int newSMSin; //this is a counter because the message is serial data
  char messageIn[10]; //create the array to put the message characters into
    
  if(LSMS.available()) // Check if there is new SMS
  {
    Serial.println("There is new message.");
    LSMS.remoteNumber(numBuf, 20); // display Number part
    Serial.print("From Number:");
    Serial.println(numBuf);
    Serial.print("Message Content:"); // display Content part
    while(true)
    {
      newSMSin = LSMS.read();  
      if(newSMSin < 0)
      break;
      //Serial.print("_a_");
      Serial.print((char)newSMSin);//this is reading the message character for character
      //need to put this message into a command array
    }
    Serial.println();//prints a new line
    LSMS.flush(); // delete message
  }
  
  //Get updated GPS position and print that
  LGPS.getData(&info);
  
  //Serial.println((char*)info.GPGGA); //info.GPGGA has the information about the latitude and longitude
  
  parseGPGGA((const char*)info.GPGGA);
  //here are the output latitude and longitude variables!
  sprintf(buff, "latitude = %10.8f, longitude = %10.8f", latitude, longitude);//prints at 10 characters and 4 decimal places

  //now need to put them into a sms message
  String stringLat = "lat = "; 
  String stringLon = " lon = ";
  String stringOut = stringLat + latitude;
  stringOut = stringOut + stringLon;
  stringOut = stringOut + longitude;
  
  Serial.println(stringOut);
  
// //the code below sends a message to a defined cell number
//    //beginSMS() specifies the destination number
//    LSMS.beginSMS(CellNumber);
//    //print writes data to the sms message content
//    LSMS.print(stringOut);
//  
//    //endSMS() sends the sms
//    //this if/else then checks the output of endSMS() to confirm that sms is sent or not
//    if(LSMS.endSMS())
//      {
//        Serial.println("SMS is sent");
//      }
//      else
//      {
//        Serial.println("SMS is not sent");
//      }


  delay(1000);
 
}
