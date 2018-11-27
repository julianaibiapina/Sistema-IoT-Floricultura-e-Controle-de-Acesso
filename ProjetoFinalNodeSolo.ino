#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
const char *ssid = "brisa-410363"; 
const char *pass = "u6p7yt5w";

#define BOTtoken "787733932:AAEen1sPZrgx26qKz9fU6s_GehY6rOMIWOk"

WiFiClientSecure client;

UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 1000; // tempo entre a leitura das mensagens
long Bot_lasttime; // ultima mensagem lida
bool Start = false;

//---------------------------FUNÇÕES----------------------------//

void handleNewMessages(int numNewMessages){

  Serial.print("Mensagem recebida = ");
  Serial.println(String(numNewMessages));

  for(int i = 0; i < numNewMessages; i++){
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
      welcome += "/gettemp : Para receber dados da Temperatura;\n";
      welcome += "/getumidatm : Para receber dados da Umidade do Ar;\n";
      welcome += "/getumidsolo : Para receber dados da Umidade Solo;\n";
      welcome += "/getlum : Para receber dados da Luminosidade;\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}


// REALIZA A CONEXÃO COM UMA REDE WI-FI
void conexaoWifi(){
  client.stop();
  Serial.print("Conectando-se à rede ");
  Serial.print(ssid);
  Serial.println("... \n");
  
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
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
  conexaoWifi();
  
}

void loop(){
  //sensorUmidadeSolo();
  //delay(1000);


  if(millis()>Bot_lasttime + Bot_mtbs){
     int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
     while(numNewMessages){
        Serial.println("\n Resposta recebida pelo Telegram! \n");
        handleNewMessages(numNewMessages);
        numNewMessages=bot.getUpdates(bot.last_message_received + 1);
     }
     Bot_lasttime = millis();
  }

}

