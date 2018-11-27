#include <DHT.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <UniversalTelegramBot.h>

String apiKey = "3VSRJITECITQ3V00";     //  <-- seu Write API key do site ThingSpeak

// Inicializando a conexao WIFI com o Roteador
const char *ssid = "REDEIOT"; // nome do seu roteador WIFI (SSID)
const char *pass = "iotnet18"; // senha do roteador WIFI
const char* server = "api.thingspeak.com";

// Inicializa o BOT Telegram - copie aqui a chave Token quando configurou o seu BOT - entre
#define BOTtoken "787733932:AAEen1sPZrgx26qKz9fU6s_GehY6rOMIWOk" // sua chave

#define DHTPIN 0

DHT dht(DHTPIN, DHT11);

WiFiClientSecure clientSecure;
WiFiClient client;

UniversalTelegramBot bot(BOTtoken, clientSecure);

int Bot_mtbs = 1000; // tempo entre a leitura das mensagens
long Bot_lasttime; // ultima mensagem lida
bool Start = false;
const int ldr = A0;
const int ledIr = 5;
int ledIrStatus = 0;
bool controle = true;

void handleNewMessages(int numNewMessages){

 dht.begin();
 float t = dht.readTemperature();
 float h = dht.readHumidity(); 
 int l = analogRead(ldr);
 pinMode(DHTPIN, OUTPUT);
 
 Serial.print("Mensagem recebida = ");
 Serial.println(String(numNewMessages));
 
 
   for (int i = 0; i < numNewMessages; i++){
      String chat_id = String(bot.messages[i].chat_id);
      String text = bot.messages[i].text;
      String from_name = bot.messages[i].from_name;
      
   if (from_name == "") from_name = "Guest";


   if(text == "/stopsensores"){
      //deve parar de enviar dados para o thingSpeak
      controle = false;
      bot.sendMessage(chat_id, "Parado o envio de dados para o ThingSpeak.\n", "");
    }
    
    if(text == "/startsensores"){
      //deve iniciar a enviar dados para o thingSpeak
      controle = true;
      bot.sendMessage(chat_id, "Reiniciado o envio de dados para o ThingSpeak.\n", "");
    }
    
    if(text == "/statussensores"){
      String result = "";
      if(controle == true){
         result = "ativos";
      }else if (controle == false){
        result = "inativos";
      }
      bot.sendMessage(chat_id, "Os sensores estão: " + result +". \n", "");
    }
    
   if(text == "/gettemp"){
     Serial.print("Temperatura: ");
     Serial.print(t);
     bot.sendMessage(chat_id, "Temperatura: " + String(t) + " ºC.\n", ""); // envia mensagem
   }

   if(text == "/getumidatm"){
     Serial.print("Umidade do Ar: ");
     Serial.print(h);
     bot.sendMessage(chat_id, "Umidade do Ar: " + String(h) + "%\n", ""); // envia mensagem
   }

    if(text == "/getlum"){
     Serial.print("Luminosidade: ");
     Serial.print(l);
     bot.sendMessage(chat_id, "Luminosidade: " + String(l), ""); // envia mensagem
   }

   if(text == "/startatu"){
     digitalWrite(ledIr, HIGH);
     ledIrStatus = 1;
     bot.sendMessage(chat_id, "Irrigador ligado! ", ""); // envia mensagem
   }

   if(text == "/stopatu"){
     digitalWrite(ledIr, LOW);
     ledIrStatus = 0;
     bot.sendMessage(chat_id, "Irrigador desligado! ", ""); // envia mensagem
   }

   if (text == "/statusatu") { // comando estado do LED
     if (ledIrStatus == 1) {
       bot.sendMessage(chat_id, "Irrigador ligado!", ""); 
       }
     else{
       bot.sendMessage(chat_id, "Irrigador desligado!", "");
     }
   }
   /*if (text == "/ledoff"){ 
     ledStatus = 0;
     digitalWrite(ledPin,LOW ); // apaga LED azul
     bot.sendMessage(chat_id, "LED esta apagado", ""); // envia mensagem
   }*/
   
   /*if (text == "/status") { // comando estado do LED
     if (ledStatus) {
       bot.sendMessage(chat_id, "LED esta aceso", ""); 
       }
     else{
       bot.sendMessage(chat_id, "LED esta apagado", "");
     }
   }*/
   
   /*if (text == "/start"){
    String welcome = "Bem-vindo ao Sistema IoT da Floricultura, " + from_name + ".\n";
    welcome += "Por favor, escolha um dos comandos a seguir:\n\n";
    welcome += "/gettemp : Para receber dados da Temperatura;\n";
    welcome += "/getumidatm : Para receber dados da Umidade do Ar;\n";
    welcome += "/getlum : Para receber dados da Luminosidade;\n";
    bot.sendMessage(chat_id, welcome, "Markdown");
    }*/
   }
 }

void setup(){
  Serial.begin(115200);
  dht.begin();
  Serial.println("Conectando-se à rede: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conexão Wi-Fi iniciada.");
  pinMode(ledIr, OUTPUT);
  //delay(10);
  digitalWrite(ledIr, LOW);
}

void loop(){
 
   if(millis()>Bot_lasttime + Bot_mtbs){
     int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
     while(numNewMessages){
        Serial.println("\nResposta recebida pelo Telegram! \n");
        handleNewMessages(numNewMessages);
        numNewMessages=bot.getUpdates(bot.last_message_received + 1);
     }
     Bot_lasttime = millis();
     }
     
     if(controle == true){
      //deve estar depois do Telegram
      thingSpeak();
     }    
}

void thingSpeak(){
   if (client.connect(server,80)){ 
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int l = analogRead(ldr);
        if (isnan(h) || isnan(t)){
          //Serial.println("Falha na leitura do sensor!");
        return;
        } 
                             String postStr = apiKey;
                             postStr +="&field1="; //<-- atenção, esse é o campo 1 que você escolheu no canal do ThingSpeak
                             postStr += String(t);
                             postStr +="&field2=";
                             postStr += String(h);
                             postStr +="&field3=";
                             postStr += String(l);
                             postStr += "\r\n\r\n";
 
                             client.print("POST /update HTTP/1.1\n");
                             client.print("Host: api.thingspeak.com\n");
                             client.print("Connection: close\n");
                             client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
                             client.print("Content-Type: application/x-www-form-urlencoded\n");
                             client.print("Content-Length: ");
                             client.print(postStr.length());
                             client.print("\n\n");
                             client.print(postStr);

                             Serial.print("Temperatura: ");
                             Serial.print(t);
                             Serial.print(" graus Celsius, Umidade: ");
                             Serial.print(h);
                             Serial.println("%, Luminosidade: ");
                             Serial.print(l);
                             //Send to Thingspeak.");
                        }
          client.stop();
 
          Serial.println("\nWaiting...");
  
  // thingspeak needs minimum 15 sec delay between updates, i've set it to 20 seconds
  delay(15000);
 }

