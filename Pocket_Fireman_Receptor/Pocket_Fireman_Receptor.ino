//IoT2 2023/2

//Bibliotecas
#include <arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

//Definicoes para o MQTT:
#define Controle_Incendio    "Controle_Incendio"
#define Estado_Vibracao      "Estado_Vibracao"

#define ID_MQTT  "2Pocket Fireman ESP32 - IoT2"     //id mqtt (para identificação de sessão)

//Rede WI-FI que deseja se conectar:
const char* SSID     = "WIFI"; 
const char* PASSWORD = "Senha";

//Pinos:
const int PonteH = 18;       // Sinal da ponte H
const int LRed = 16;         // Led Vermelho

//Configuração do Broker:
const char* BROKER_MQTT = "test.mosquitto.org";
int BROKER_PORT = 1883;      // Porta do BROKER_MQTT

//Declaração dos objetos:
WiFiClient espClient;                     // Cria o objeto espClient
PubSubClient MQTT(espClient);             // Instancia o Cliente MQTT passando o objeto espClient

//Variáveis:
unsigned long tempoAtivacao = 5000;       // Tempo de ativação dos alertas (em ms)
unsigned long startTime = 0;              // Referência para o tempo de ativação dos alertas
String macAddress;                        // Armazena o MAC do próprio ESP


/*
   Descrição: Inicializa e conecta-se na rede WI-FI desejada
*/
void initWiFi(void){
  delay(10);
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");

  reconnectWiFi();
}


/*
   Descrição: Inicializa parâmetros de conexão MQTT(endereço do broker, porta e seta função de callback)
*/
void initMQTT(void){
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //Informa qual broker e porta deve ser conectado
  MQTT.setCallback(mqtt_callback);            //Atribui função de callback
}


/*
   Descrição: Função de callback (esta função é chamada toda vez que uma informação de um dos tópicos subescritos chega)
   Parâmetros: Um topico, o payload e seu tamanho
*/
void mqtt_callback(char* topic, byte* payload, unsigned int length){
  String msg;

  //Obtem a string do payload recebido
  for (int i = 0; i < length; i++){
    char c = (char)payload[i];
    msg += c;
  }

  Serial.print("Chegou a seguinte string via MQTT: ");
  Serial.println(msg);

  //Toma ação dependendo da string recebida:
  
  if (msg.equals("L")){
    controleVibracao(1);
    Serial.println("Vibracao nivel 1 (L) ativada");

  }else if (msg.equals("M")){
    controleVibracao(2);
    Serial.println("Vibracao nivel 2 (M) ativada");

  }else if (msg.equals("H")){
    controleVibracao(3);
    Serial.println("Vibracao nivel 3 (H) ativada");

  }else if (msg.equals("TXL")){
    tempoAtivacao = 3000;
    Serial.println("Tempo de vibracao definido em 3s");
    MQTT.publish(Estado_Vibracao, "Tempo de vibracao definido em 3s");
    MQTT.publish(Estado_Vibracao, macAddress.c_str());    //Publica o MAC do ESP

  }else if (msg.equals("TL")){
    tempoAtivacao = 30000;
    Serial.println("Tempo de vibracao definido em 30s");
    MQTT.publish(Estado_Vibracao, "Tempo de vibracao definido em 30s");
    MQTT.publish(Estado_Vibracao, macAddress.c_str());    //Publica o MAC do ESP

  }else if (msg.equals("TM")){
    tempoAtivacao = 60000;
    Serial.println("Tempo de vibracao definido em 1 min");
    MQTT.publish(Estado_Vibracao, "Tempo de vibracao definido em 1 min");
    MQTT.publish(Estado_Vibracao, macAddress.c_str());    //Publica o MAC do ESP

  }else if (msg.equals("TH")){
    tempoAtivacao = 300000;
    Serial.println("Tempo de vibracao definido em 5 min");
    MQTT.publish(Estado_Vibracao, "Tempo de vibracao definido em 5 min");
    MQTT.publish(Estado_Vibracao, macAddress.c_str());    //Publica o MAC do ESP

  }else if (msg.equals("TXH")){
    tempoAtivacao = 1800000;
    Serial.println("Tempo de vibracao definido em 30 min");
    MQTT.publish(Estado_Vibracao, "Tempo de vibracao definido em 30 min");
    MQTT.publish(Estado_Vibracao, macAddress.c_str());    //Publica o MAC do ESP
  }
}


/*
   Descrição: Reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
              em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
*/
void reconnectMQTT(void){
  while (!MQTT.connected()){
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT)){
      Serial.println("Conectado com sucesso ao broker MQTT!");
      MQTT.subscribe(Controle_Incendio);
      
    }else{
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Havera nova tentatica de conexao");
    }
  }  
}


/*
   Descrição: Reconecta-se ao WiFi
*/
void reconnectWiFi(void){
  //Se já está conectado a rede WI-FI, nada é feito.
  if (WiFi.status() == WL_CONNECTED){
    macAddress = WiFi.macAddress();
    return;
  }

  //Caso contrário, são efetuadas tentativas de conexão
  WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println("\nIP obtido: ");
  Serial.println(WiFi.localIP());
}


/*
   Descrição: Controla a intensidade da vibração através do número informado
   Parâmetros: Um valor inteiro
*/
void controleVibracao(int severidade){
  if(severidade == 1){ 
    MQTT.publish(Estado_Vibracao, "Severidade = 1 (L)");  //Publica o estado da vibração
    MQTT.publish(Estado_Vibracao, macAddress.c_str());    //Publica o MAC do ESP
    Serial.println("Enviado MQTT: Severidade = 1 (L)");

    startTime = millis(); // Registra o tempo inicial

    while (millis() - startTime < tempoAtivacao) {  // Vibra o motor e pisca o LED pelo tempo escolhido em tempoAtivacao
      digitalWrite(LRed, HIGH);
      digitalWrite(PonteH, HIGH);
      delay(1000); 
      digitalWrite(LRed, LOW);
      digitalWrite(PonteH, LOW);
      delay(1000); 
    }

  }else if(severidade == 2){              
    MQTT.publish(Estado_Vibracao, "Severidade = 2 (M)");  //Publica o estado da vibração
    MQTT.publish(Estado_Vibracao, macAddress.c_str());    //Publica o MAC do ESP
    Serial.println("Enviado MQTT: Severidade = 2 (M)");

    startTime = millis(); // Registra o tempo inicial

    while (millis() - startTime < tempoAtivacao) {  // Vibra o motor e pisca o LED pelo tempo escolhido em tempoAtivacao
      digitalWrite(LRed, HIGH);
      digitalWrite(PonteH, HIGH);
      delay(500); 
      digitalWrite(LRed, LOW);
      digitalWrite(PonteH, LOW);
      delay(500); 
    }

  }else if(severidade == 3){                          
    MQTT.publish(Estado_Vibracao, "Severidade = 3 (H)");  //Publica o estado da vibração
    MQTT.publish(Estado_Vibracao, macAddress.c_str());    //Publica o MAC do ESP
    Serial.println("Enviado MQTT: Severidade = 3 (H)");

    startTime = millis(); // Registra o tempo inicial

    while (millis() - startTime < tempoAtivacao) {  // Vibra o motor e pisca o LED pelo tempo escolhido em tempoAtivacao
      digitalWrite(LRed, HIGH);
      digitalWrite(PonteH, HIGH);
      delay(300); 
      digitalWrite(LRed, LOW);
      digitalWrite(PonteH, LOW);
      delay(300); 
    }
  }
}


//------------------------------------------------------


void setup() {
  Serial.begin(115200);
  
  pinMode(PonteH, OUTPUT);
  pinMode(LRed, OUTPUT);

  //Inicializa a conexao wi-fi
  initWiFi();

  //Inicializa a conexao ao broker MQTT
  initMQTT();
                                   
  delay(500);
}


void loop() {
  //keep-alive da comunicação com broker MQTT
  MQTT.loop();

  if (!MQTT.connected())
    reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita

  reconnectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
  
  delay(550);
}