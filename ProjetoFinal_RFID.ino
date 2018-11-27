/*
 * -------------------------------Controle de Acesso com RFID-----------------------------
 * 
 * CARTÃO MASTER: 226 84 18 187
 * 
 * INFORMAÇÕES DO CIRCUITO:
 * Pinos do sensor -> Pinos do Arduino
 *  3.3v           ->       3.3v
 *  GND            ->       GND
 *  RST            ->       pin 9
 *  IRQ            ->       off
 *  MISO           ->       pin 12
 *  MOSI           ->       pin 11
 *  SCK            ->       pin 13
 *  SDA            ->       pin 10
 * 
 */

#include <MFRC522.h>
#include <SPI.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 10
#define RST_PIN 9

#define YEL_LED 8
#define VERDE_LED 7
#define BLU_LED 6

#define BRAN_BUTTON 3
#define AZUL_BUTTON 4
#define VERM_BUTTON 5


const int pinosButtons[3] = {2, 3, 4};

// Inicializa o display no endereco 0x27
//LiquidCrystal_I2C lcd(0x3F,2,1,0,4,5,6,7,3, POSITIVE);
LiquidCrystal_I2C lcd(0x3F,16,2);

//CONFIGURAÇÃO DO DISPLAY
  int screenWidth = 16;  
  int screenHeight = 2; 
  //line1 = Scroll linha superior
  String line1 = "PRETO:cancelar BRANCO:cadastra cartao AZUL = deletar cartao";

  //line2 = Linha inferior - estatica  
  String line2 = "  Modo Master";

  int stringStart, stringStop = 0;  
  int scrollCursor = screenWidth;  
  int tamanho =0;
  

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;

String uidString;
byte uid[4];
const byte cardMaster[4] = {175, 105, 176, 121};  //esse valor deve ser gravado nas primeiras 4 posições da memória EEPROM.





void setup(){

  for(int i=0; i<4; i++){
    EEPROM.write(i,cardMaster[i]);
  }
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  
  lcd.init();              
  lcd.backlight();
  mensageminicial();
  
  pinMode(YEL_LED, OUTPUT);
  pinMode(VERDE_LED, OUTPUT);
  pinMode(BLU_LED, OUTPUT);


  for(int i=0; i<3; i++){
    pinMode(pinosButtons[i], INPUT);
  }

  
  Serial.println("Aproxime o cartão.");
  delay(500);

}

void loop(){
  
  // ESPERA POR UM CARTÃO E QUANDO ENCONTRA O ARMAZENA EM 'uidString' E 'uid[4]'
  mensageminicial();
  readRFID();
  
  Serial.println("UID: " + uidString);
  
  if(busca()){  //CARTÃO ESTÁ CADASTRADO
    if(comparaMaster()){  //MASTER: CADASTRAR OU REMOVER UM CARTÃO.
      
      /*
       * PARTE DOS BOTÕES
       * 
       * BOTÃO PRETO:  CANCELAR OPERAÇÃO E VOLTAR AO INICIO DO LOOP
       * BOTÃO BRANCO: CADASTRAR O PRÓXIMO CARTÃO LIDO
       * BOTÃO AZUL:   REMOVER O PRÓXIMO CARTÃO LIDO
       *
       */
         
        digitalWrite(YEL_LED, HIGH); //Luz para informar que está no modo master.fica acesa até findar.
        
        Serial.println("Modo Master.");
        Serial.println("Escolha uma opção:");
        Serial.println("BOTÃO PRETO - CANCELAR OPERAÇÃO");
        Serial.println("BOTÃO BRANCO - CADASTRAR O PRÓXIMO CARTÃO LIDO");
        Serial.println("BOTÃO AZUL - REMOVER O PRÓXIMO CARTÃO LIDO");


        int Button = esperaButtons();
        
        if(Button == 0){
          Serial.println("Operação cancelada.");
          lcd.clear();
          lcd.print("   operacao");  
          lcd.setCursor(0,1);
          lcd.print("   cancelada");
          delay(1500);
          digitalWrite(YEL_LED, LOW); 
          lcd.clear();
          return; 
          
        }else if(Button == 1){
          digitalWrite(BLU_LED, HIGH);
          Serial.println("Modo de cadastro. Aproxime o cartão que deseja cadastrar.");
          lcd.clear();
          lcd.print("Modo de cadastro");
          readRFID();
          addCard();
          digitalWrite(BLU_LED, LOW);
          digitalWrite(YEL_LED, LOW);
          lcd.clear();
          
          
        }else if(Button == 2){
          digitalWrite(BLU_LED, HIGH);
          Serial.println("Modo de remoção. Aproxime o cartão que deseja remover.");
          lcd.clear();
          lcd.print("     Modo de");
          lcd.setCursor(0,1);
          lcd.print("     remocao");
          readRFID();
          removerCard();
          digitalWrite(BLU_LED, LOW);
          digitalWrite(YEL_LED, LOW);
          lcd.clear();
        }
      }else{  //TAG CADASTRADA PORÉM NÃO É MASTER. ACESSO LIBERADO
        Serial.println("ACESSO LIBERADO.");
        digitalWrite(VERDE_LED, HIGH);
        lcd.clear();
        lcd.print("     acesso");
        lcd.setCursor(0,1);
        lcd.print("    liberado");
        delay(1500);
        digitalWrite(VERDE_LED, LOW);
        lcd.clear();
      }
  }else{
    Serial.println("ACESSO NEGADO.");
    lcd.clear();
    lcd.print("     acesso");
    lcd.setCursor(0,1);
    lcd.print("     negado");
    delay(1500);
    lcd.clear();
  }
  
}



/*
 * Armazena o UID nas variáveia globais uid(byte) e uidString(String)
 */
void readRFID(){
  boolean loop = false;
  do{
    if( rfid.PICC_IsNewCardPresent() ){
      loop = true;
      rfid.PICC_ReadCardSerial();
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

      // Check is the PICC of Classic MIFARE type
      if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
          piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
          piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("Tag incompatível com o leitor."));
        return;
      }

      uidString = String(rfid.uid.uidByte[0])+" "+String(rfid.uid.uidByte[1])+" "+String(rfid.uid.uidByte[2])+ " "+String(rfid.uid.uidByte[3]);
      for(int i=0; i<4; i++){
        uid[i] = rfid.uid.uidByte[i];
      }
    
      // Halt PICC
      rfid.PICC_HaltA();

      // Stop encryption on PCD
      rfid.PCD_StopCrypto1();

    }
    delay(100);
  }while(loop == false);
}


/*
 * Confere se o cartão lido está cadastrado, se estiver retorna "true" se não retorna "false"
 */
boolean busca(){
  boolean controle = false;
  int cont = 0;
  byte aux[4];
   int testaIgualdade;
  do{
    //laço para copiar um cartão da memória para a varável
    for(int i=0; i<4;i++){
      aux[i] = EEPROM.read(cont);
      cont++;
    }

    //laço para comparar e ver se o carão copiado da memória é igual ao lido
    testaIgualdade = 0;   //caso o valor final seja 4 então o cartão da memória e o cartão lido são iguais.
    for(int i=0; i<4; i++){
      if(aux[i] == uid[i]){
        testaIgualdade++;
      }
    }

    if(testaIgualdade == 4){
      controle = true;
    }
    
  }while(controle == false && cont < 1024);

  return controle;
}



/*
 * Função que compara o cartão lido com o cartão master, se ambos forem iguais retorna true
 */
boolean comparaMaster(){
  int cont = 0;
  for(int i=0; i<4;i++){
    if(uid[i] == cardMaster[i]){
      cont++;
    }
  }
  
  if(cont == 4){
    return true;
  }else{
    return false;
  }
}



//A MENSAGEM NO LCD É EXIBIDA ENQUANTO A PESSOA NÃO APERTA ALGUM BOTÃO
int esperaButtons(){
  boolean apert = false;
  int indexButton;
  while(!apert){
    for(int i=0; i<3; i++){
      if(digitalRead(pinosButtons[i]) == HIGH){
        // i informa qual botão foi pressionado
        indexButton = i; 
        apert = true;
      }
    }
    //para exibir mensagem no LCD:
    lcd.setCursor(scrollCursor, 0);  
    lcd.print(line1.substring(stringStart,stringStop));  
    lcd.setCursor(0, 1);  
    lcd.print(line2);  

    //Quanto menor o valor do delay, mais rapido o scroll  
    delay(150);  
    scroll_sup(); //Chama a rotina que executa o scroll  

    //Verifica o tamanho da string  
    tamanho = line1.length();  
    if (stringStart == tamanho)  
    {  
      stringStart = 0;  
      stringStop = 0;  
    } 
  }
  return indexButton;
}


void addCard(){
  if(!busca()){
    
    int lugar = (EEPROM.read(1023))*4;
    if(lugar < 1020){            //OS ÚLTIMOS QUATRO ESPAÇOS SÃO RESERVADOS PARA OUTROS FINS
      // laço para percorrer a variável uid[4]
      for(int i=0; i<4;i++){
        EEPROM.write(lugar, uid[i]);
        lugar++;
      }
  
      lugar = EEPROM.read(1023) + 1;
      EEPROM.write(1023, lugar);
      Serial.println("Cartão cadastrado.");
      lcd.clear();
      lcd.print("     Cartao");
      lcd.setCursor(0,1);
      lcd.print("   cadastrado");
       
    }else{
      Serial.println("Memória cheia.");
      lcd.clear();
      lcd.print(" Memoria cheia");
    }
  }else{
    Serial.println("Esse cartão já está cadastrado.");
    lcd.clear();
    lcd.print("     Cartao");
    lcd.setCursor(0,1);
    lcd.print(" ja cadastrado");
    delay(1500);
    lcd.clear();
  }
}




void removerCard(){
  if(!busca()){
      Serial.println("Esse cartão já não é cadastrado.");
      lcd.clear();
      lcd.print("   Cartao ja");
      lcd.setCursor(0,1);
      lcd.print("nao e cadastrado");
      delay(1500);
      lcd.clear();
  }else{
    boolean controle = false;
    int cont = 0;
    byte aux[4];
    int testaIgualdade;
    do{
     //laço para copiar um cartão da memória para a varável
      for(int i=0; i<4;i++){
        aux[i] = EEPROM.read(cont);
        cont++;
      }

      //laço para comparar e ver se o carão copiado da memória é igual ao lido
      testaIgualdade = 0;   //caso o valor final seja 4 então o cartão da memória e o cartão lido são iguais.
      for(int i=0; i<4; i++){
        if(aux[i] == uid[i]){
          testaIgualdade++;
        }
      }

      if(testaIgualdade == 4){
        controle = true;
      }
    
    }while(controle == false && cont < 1024);


    if(controle = true && !comparaMaster()){
    
      int aux = cont - 4; //recebe o primerio lugar do cartão que deve ser removido
      do{
        EEPROM.write(aux, EEPROM.read(cont));
        aux++;
        cont++;
      }while (cont < (EEPROM.read(1023)*4));

      for(int i=0; i<4; i++){
        EEPROM.write(aux, 255);
        aux++;
      }
      int aux2 = EEPROM.read(1023) - 1;
      EEPROM.write(1023, aux2); 
      Serial.println("Cartão removido.");
      lcd.clear();
      lcd.print("     Cartao");
      lcd.setCursor(0,1);
      lcd.print("    removido");
      delay(1500);
      lcd.clear();
    }else{
      Serial.println("O Cartão Master não pode ser removido.");
      lcd.clear();
      lcd.print("Nao pode remover");
      lcd.setCursor(0,1);
      lcd.print("o cartao master");
      delay(1500);
      lcd.clear();
    }
   }
}


//------------------LDC---------------------//
void mensageminicial(){
  lcd.clear();
  lcd.print("   Aproxime o");  
  lcd.setCursor(0,1);
  lcd.print("     cartao");  
}



void scroll_sup()  
{  
  lcd.clear();  
  if(stringStart == 0 && scrollCursor > 0)
  {  
    scrollCursor--;  
    stringStop++;  
  } else if (stringStart == stringStop){  
    stringStart = stringStop = 0;  
    scrollCursor = screenWidth;  
  } else if (stringStop == line1.length() && scrollCursor == 0) {  
    stringStart++;  
  } else {  
    stringStart++;  
    stringStop++;  
  }  
}
