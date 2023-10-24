#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>


//**************************************
//*********** MQTT CONFIG **************
//**************************************
const char *mqtt_server = "io.adafruit.com";
const int mqtt_port = 1883;
const char *mqtt_user = "   ";
const char *mqtt_pass = "     ";
const char *root_topic_subscribe = "    ";
const char *root_topic_publish = "      ";


//**************************************
//*********** WIFICONFIG ***************
//**************************************
const char* ssid = "    ";
const char* password =  "    ";



//**************************************
//*********** GLOBALES   ***************
//**************************************
WiFiClient espClient;
PubSubClient client(espClient);
char msg[25];
long count=0;

float vol;
float por;
const int echo=16;
const int trigger=18;
const int led=15;
long tiempo;
float distancia;
float vl_dis;

//************************
//** F U N C I O N E S ***
//************************
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();
void setup_wifi();

void setup() {
  Serial.begin(115200);
  pinMode(echo,INPUT);
  pinMode(trigger,OUTPUT);
  pinMode(led,OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  
  if (!client.connected()) {
		reconnect();
	}

  if (client.connected()){
    //************************
    //** SENSOR ***
    //************************
    digitalWrite(trigger,HIGH);
    delayMicroseconds(2);
    digitalWrite(trigger,LOW);
    delayMicroseconds(10);
    digitalWrite(trigger,HIGH);
    
    tiempo=pulseIn(echo,HIGH);//us=microsegundos

    if (tiempo>600){
        distancia = tiempo*0.035/2;
    }else{
      distancia = tiempo*0.03/2;
    }
    
    if (distancia!=0){
      vl_dis=distancia;

      vol=200*300*(200-vl_dis);
      por=(vol*100)/12000000;
      


      Serial.print("Nivel: ");
      Serial.print(por);      //Enviamos serialmente el valor de la distancia
      Serial.print("cm3");
      Serial.println();
    }

    String str = String(por);
    str.toCharArray(msg,25);
    client.publish(root_topic_publish,msg);
    client.publish("     ",msg);
    delay(5000);
  }
  client.loop();
}


//*****************************
//***    CONEXION WIFI      ***
//*****************************
void setup_wifi(){
	delay(10);
	// Nos conectamos a nuestra red Wifi
	Serial.println();
	Serial.print("Conectando a ssid: ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("Conectado a red WiFi!");
	Serial.println("Dirección IP: ");
	Serial.println(WiFi.localIP());
}



//*****************************
//***    CONEXION MQTT      ***
//*****************************

void reconnect() {

	while (!client.connected()) {
		Serial.print("Intentando conexión Mqtt...");
		// Creamos un cliente ID
		String clientId = "Adentro";
		clientId += String(random(0xffff), HEX);
		// Intentamos conectar
		if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
			Serial.println("Conectado!");
			// Nos suscribimos
			if(client.subscribe(root_topic_subscribe)){
        Serial.println("Suscripcion: OK");
      }else{
        Serial.println("Fallo Suscripciión");
      }
		} else {
			Serial.print("Falló :( con error -> ");
			Serial.print(client.state());
			Serial.println(" Intentando de nuevo en 5 seg");
			delay(5000);
		}
	}
}


//*****************************
//***       CALLBACK        ***
//*****************************

void callback(char* topic, byte* payload, unsigned int length){
	String incoming = "";
	Serial.print("Mensaje recibido desde -> ");
	Serial.print(topic);
	Serial.println("");
	for (int i = 0; i < length; i++) {
		incoming += (char)payload[i];
	}
	incoming.trim();
	Serial.println("Mensaje -> " + incoming);

}