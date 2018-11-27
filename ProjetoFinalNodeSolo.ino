#include <DHT.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <UniversalTelegramBot.h>

String apiKey = "E0KGERS52X48ZC29";     //  <-- seu Write API key do site ThingSpeak

// Inicializando a conexao WIFI com o Roteador
const char *ssid = "brisa-410363"; // nome do seu roteador WIFI (SSID)
const char *pass = "u6p7yt5w"; // senha do roteador WIFI
const char* server = "api.thingspeak.com";

// Inicializa o BOT Telegram - copie aqui a chave Token quando configurou o seu BOT - entre
#define BOTtoken "799701394:AAFRYqvV1Fe7nNOj89gcDAdZTq4ZvLG3NRw" // sua chave

WiFiClientSecure clientSecure;
WiFiClient client;

UniversalTelegramBot bot(BOTtoken, clientSecure);

int Bot_mtbs = 1000; // tempo entre a leitura das mensagens
long Bot_lasttime; // ultima mensagem lida
bool Start = false;


void handleNewMessages(int numNewMessages){

 Serial.print("Mensagem recebida = ");
 Serial.println(String(numNewMessages));
 
   for (int i = 0; i < numNewMessages; i++){
      String chat_id = String(bot.messages[i].chat_id);
      String text = bot.messages[i].text;
      String from_name = bot.messages[i].from_name;
      
   if (from_name == "") from_name = "Guest";

    if(text == "/getumidsolo"){
      float umidSolo = sensorUmidadeSolo();
      bot.sendMessage(chat_id, "Umidade do solo: " + String(umidSolo) + " %.\n", "");
    }

   if (text == "/start"){
      String welcome = "Bem-vindo ao Sistema IoT da Floricultura, " + from_name + ".\n";
      welcome += "Por favor, escolha um dos comandos a seguir:\n\n";
      welcome += "/getsensores : Para receber dados de todos os sensores;\n";
      welcome += "/gettemp : Para receber dados da Temperatura;\n";
      welcome += "/getumidatm : Para receber dados da Umidade do Ar;\n";
      welcome += "/getumidsolo : Para receber dados da Umidade Solo;\n";
      welcome += "/getlum : Para receber dados da Luminosidade;\n";
      welcome += "/stopsensores : Pausa o envio de dados de todos os sensores;\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }

    
   
   
   }
 }

// REALIZA A LEITURA DO SENSOR E CONVERTE PARA UM VALOR PERCENTUAL
//Observação: o ADC do NodeMCU permite até, no máximo, 3.3V. Dessa forma,
//            para 3.3V, obtem-se (empiricamente) 978 como leitura de ADC
float sensorUmidadeSolo(){
  int valorADC;
  float umidadePercent;

  valorADC = analogRead(0);
  Serial.print("Leitura da porta A0: ");
  Serial.println(valorADC);
     //Quanto maior o numero lido do ADC, menor a umidade.
     //Sendo assim, calcula-se a porcentagem de umidade por:
     //      
     //   Valor lido                 Umidade percentual
     //      _    0                           _ 100
     //      |                                |   
     //      |                                |   
     //      -   ValorADC                     - UmidadePercentual 
     //      |                                |   
     //      |                                |   
     //     _|_  978                         _|_ 0
     //
     //   (UmidadePercentual-0) / (100-0)  =  (ValorADC - 978) / (-978)
     //      Logo:
     //      UmidadePercentual = 100 * ((978-ValorADC) / 978)
  umidadePercent = -100 * (((float)valorADC-1024) / 757);
  Serial.print("Umidade percentual: ");
  Serial.print(umidadePercent);
  Serial.println("%");

  return umidadePercent;
  
}


void setup(){
  Serial.begin(115200);
  delay(10);
  
  Serial.println("Conectando-se à rede: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conexão Wi-Fi iniciada.");
}

void loop(){
    
   if (client.connect(server,80)){  
    String postStr = apiKey;
    float umidSolo = sensorUmidadeSolo();
                             postStr +="&field4="; //<-- atenção, esse é o campo 1 que você escolheu no canal do ThingSpeak
                             postStr += String(umidSolo);
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

                             Serial.print("Umidade do solo: ");
                             Serial.print(umidSolo);
                            
                             //Send to Thingspeak.");
                        }
          client.stop();
 
          Serial.println("\nWaiting...");
  
  // thingspeak needs minimum 15 sec delay between updates, i've set it to 20 seconds
  delay(20000);

   if(millis()>Bot_lasttime + Bot_mtbs){
     int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
     while(numNewMessages){
        Serial.println("\nResposta recebida pelo Telegram! \n");
        handleNewMessages(numNewMessages);
        numNewMessages=bot.getUpdates(bot.last_message_received + 1);
     }
     Bot_lasttime = millis();
     }
   }
