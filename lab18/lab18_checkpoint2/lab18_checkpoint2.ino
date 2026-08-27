#define Y_LED 0
#define R_LED 15
#define G_LED 2
#define SW1 4
#define SW8 16
/*************************************************************
  This is a simple demo of sending and receiving some data.
  Be sure to check out other examples!
 *************************************************************/
#define BLYNK_TEMPLATE_ID "TMPL6CGISS-t4"
#define BLYNK_TEMPLATE_NAME "Checkpoint2"
#define BLYNK_AUTH_TOKEN "AqPpnRT0orAIv1qAAoti7-bB9Kcd-UmC"
/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Jack>/~";
char pass[] = "nmcu1234";
BlynkTimer timer;
// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable
  int value = param.asInt();
  if (value == 1)
    digitalWrite(R_LED, HIGH);
  else
    digitalWrite(R_LED, LOW);
}
BLYNK_WRITE(V1)
{
  int value = param.asInt();
  analogWrite (G_LED, value);
}
// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V100, "offImageUrl", "https://staticimage.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V100, "onImageUrl", "https://staticimage.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V100, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstartdevice-was-made");
}
// อ่านค่า SW1 อัปเดตสถานะให้หลอด Y_LED และส่งค่าไปยัง Blynk Virtual Pin 3
void ReadSW()
{
  uint8_t d = digitalRead(SW1);
  digitalWrite(Y_LED, d);   

  Blynk.virtualWrite(V3, d);
}
void setup()
{
  pinMode(Y_LED, OUTPUT);
  pinMode(G_LED, OUTPUT);
  pinMode(R_LED, OUTPUT);
  pinMode(SW1, INPUT);

  pinMode(SW8, INPUT);
Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // Setup a function to be called every 100 ms
  timer.setInterval(100L, ReadSW);
}
void loop()
{
  Blynk.run();
  timer.run();
}
