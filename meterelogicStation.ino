#include <SD.h>
#include <SPI.h>
#include <SDISerial.h>
#include <DHT.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#define DATALINE_PIN 2 //pino do 5tm
#define INVERTED 1 //não sei, mas é do 5tm tbm
#define DHTPIN 7 //PINO DIGITAL UTILIZADO PELO DHT22
#define DHTTYPE DHT22 //DEFINE O MODELO DO SENSOR (DHT22 / AM2302)

File myFile;//variavel do arquivo

SDISerial sdi_serial_connection(DATALINE_PIN, INVERTED); //configuração do 5tm
DHT dht(DHTPIN, DHTTYPE); //PASSA OS PARÂMETROS PARA A FUNÇÃO

const int PINO_ONEWIRE = 8; // Define pino do sensor de temperatura
OneWire oneWire(PINO_ONEWIRE); // Cria um objeto OneWire, coisa do sensor de temperatura
DallasTemperature sensor(&oneWire); // Informa a referencia da biblioteca dallas temperature para Biblioteca onewire (coisa do sensor de temperatura)
DeviceAddress endereco_temp;

int pinoSS = 10; // Pin 10 para sd
const int AirValue =20 ; //you need to replace this value with Value_1
const int WaterValue = 58; //you need to replace this value with Value_2
int intervals = (AirValue - WaterValue)/3;
float soilMoistureValue = 0;
float soilMoistureValuei;
int erro=0;

char* fivetmSensor(){
  char* service_request = sdi_serial_connection.sdi_query("?M!",1000); 
  //This sends a message to query the first device on the bus - which in use seems to refresh the values of the sensor readings, but does not return the sensor reading itself. Instead, it returns a message that tells you the maximum wait before the measurement is ready. timeout set for one second.
  //you can use the time returned above to wait for the service_request_complete
  char* service_request_complete = sdi_serial_connection.wait_for_response(1000);
  //dont worry about waiting too long it will return once it gets a response
  return sdi_serial_connection.sdi_query("?D0!",1000); //will query the 5TM for sensor data
}


int soilMoistureSensor()
{
  soilMoistureValue = analogRead(A0); //put Sensor insert into soil
  soilMoistureValue= (1-(soilMoistureValue/1023))*100;
  soilMoistureValue= ((soilMoistureValue-AirValue)/(WaterValue-AirValue))*100;
  soilMoistureValuei=int(soilMoistureValue);
  if(soilMoistureValuei<0)
    erro++;  

    return soilMoistureValuei;
 }

int temperatureSoilSensor(){
    sensor.requestTemperatures(); // Envia comando para realizar a conversão de temperatura
  if (!sensor.getAddress(endereco_temp,0)) { // Encontra o endereco do sensor no barramento
    return -1000; // Sensor conectado, imprime mensagem de erro
  } else {
    return sensor.getTempCByIndex(0); // Busca temperatura para dispositivo
  }
}
 
void setup() {
  pinMode(pinoSS, OUTPUT); // Declara pinoSS como saída
  sensor.begin(); ; // Inicia o sensor
  sdi_serial_connection.begin(); // start our SDI connection 
  dht.begin(); //INICIALIZA A FUNÇÃO do sensor de ar
  Serial.begin(9600); // start our uart
  Serial.println("Estação inicializada"); 
  if (SD.begin()) { // Inicializa o SD Card
Serial.println("SD Card pronto para uso."); // Imprime na tela
}
else {
Serial.println("Falha na inicialização do SD Card.");
}
  delay(3000); // startup delay to allow sensor to powerup and output its DDI serial string
}


void loop() {
  myFile = SD.open("estacao.txt", FILE_WRITE);
  uint8_t wait_for_response_ms = 1000;
  char* fivetmvalue = fivetmSensor(); // valor do 5tm 2-5 umidade 7-10 temp
  int moistureSoil = soilMoistureSensor(); //valor do sensor capacitive soil moisture sensor v1.2, umidade do solo
  int temperatureSoil = temperatureSoilSensor(); //valor do sensor DS18B20, temperatura do solo
  int airHumidity = dht.readHumidity(); //valor do sensor dht22, humidade do ar
  int airTemperature = dht.readTemperature(); //valor do sensor dht22, temperatura do ar
  

  myFile.print("Umidade do solo: ");
  myFile.println(moistureSoil);
  myFile.print("Temperatura do solo: ");
  myFile.print(temperatureSoil);
  myFile.println("°C");
  
  myFile.println("Umidade 5tm: ");
  for (int i=2; i<6; i++){
      if (i==5){
          myFile.println( fivetmvalue[i]);
      }
      else
          myFile.print( fivetmvalue[i]);
      }
  myFile.println("Temperatura 5tm: ");    
  for (int i=7; i<11; i++){
      if (i==10){
          myFile.println( fivetmvalue[i]);
      }
      else
          myFile.print( fivetmvalue[i]);
      }

  myFile.print("Umidade do ar: ");
  myFile.println(airHumidity);
  myFile.print("Temperatura do ar: ");
  myFile.print(airTemperature);
  myFile.println("°C");
  
myFile.close();
delay(5000);
}
