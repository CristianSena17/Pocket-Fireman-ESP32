//IoT2 2023/2

//Declaração da Task para o processamento no segundo núcleo
TaskHandle_t Task1;

//Bibliotecas
#include <arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTTYPE DHT22

//Definicoes para o MQTT:
#define Controle_Incendio    "Controle_Incendio"
#define Estado_Vibracao      "Estado_Vibracao"

#define ID_MQTT  "Pocket Fireman ESP32 - IoT2"     // id mqtt (para identificação de sessão)

//Rede WI-FI que deseja se conectar:
const char* SSID     = "WIFI"; 
const char* PASSWORD = "Senha";  

//Pinos:
const int botao = GPIO_NUM_35;      // Entrada do botão de emergência
const int DHTPIN = GPIO_NUM_32;     // Pino de sinal do DHT22
const int LRed = GPIO_NUM_27;       // Led Vermelho
const int LGreen = GPIO_NUM_26;     // Led Verde
const int LBlue = GPIO_NUM_25;      // Led Azul
const int SmokePIN = GPIO_NUM_34;   // Pino de sinal do MQ2

//Configuração do Broker:
const char* BROKER_MQTT = "test.mosquitto.org";
int BROKER_PORT = 1883;   // Porta do Broker MQTT

//Declaração dos objetos:
DHT dht(DHTPIN, DHTTYPE);           // Cria o objeto do sensor DHT com seu pino e tipo
WiFiClient espClient;               // Cria o objeto espClient
PubSubClient MQTT(espClient);       // Instancia o Cliente MQTT passando o objeto espClient

//Variáveis:
float temp = 0.0;
int fumaca = 0;


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
      //MQTT.subscribe(Controle_Incendio);
      //MQTT.subscribe(Estado_Vibracao);
      
    }else{
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Havera nova tentatica de conexao");
      controleLED(3);  //LED Vermelho
    }
  }  
}


/*
   Descrição: Reconecta-se ao WiFi
*/
void reconnectWiFi(void){
  //Se já está conectado a rede WI-FI, nada é feito.
  if (WiFi.status() == WL_CONNECTED)
    return;

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
   Descrição: Controla a cor do LED RGB através do número informado
   Parâmetros: Um valor inteiro
*/
void controleLED(int cor){
  if(cor == 1){                    // LED Verde
    digitalWrite(LGreen, HIGH);
    digitalWrite(LBlue, LOW);
    digitalWrite(LRed, LOW);
  }else if(cor == 2){              // LED azul
    digitalWrite(LGreen, LOW);
    digitalWrite(LBlue, HIGH);
    digitalWrite(LRed, LOW);
  }else{                           // LED Vermelho
    digitalWrite(LGreen, LOW);
    digitalWrite(LBlue, LOW);
    digitalWrite(LRed, HIGH);
  }
}


/*
   Descrição: Faz as leituras dos dados do sensor DHT22, verifica seu funcionamento e imprime no monitor serial 
*/
void controleDHT(){
  temp = dht.readTemperature();                  // Leitura de Temperatura

  //Verifica se o sensor DHT22 está respondendo
  if (isnan(temp)) {
    Serial.println(F("Falha no sensor DHT"));
    controleLED(3);                              // LED Vermelho
    return;
  }

  //Imprime os valores no monitor serial
  Serial.print(temp);
  Serial.println("°C ");
}


/*
   Descrição: Faz as leituras dos dados do sensor MQ2, verifica seu funcionamento e imprime no monitor serial 
*/
void controleMQ2(){
  fumaca = analogRead(SmokePIN);                  // Leitura de Fumaça

  //Verifica se o sensor DHT22 está respondendo
  if (isnan(fumaca)) {
    Serial.println(F("Falha no sensor de fumaca"));
    controleLED(3);                               // LED Vermelho
    return;
  }

  //Imprime os valores no monitor serial
  Serial.print("Fumaca: ");
  Serial.println(fumaca);
}


/*
   Descrição: Efetua a leitura de temperatura e fumaça e faz as atuações necessárias
*/
void controleDeIncendio(){ 
  if(temp >= 42.0 && temp <= 65.0){                                
    controleLED(2);                                // LED azul
    MQTT.publish(Controle_Incendio, "L");          // Publica o estado da vibração
    Serial.println("Enviado MQTT: L");
    
  }else if(temp > 65.0 && temp <= 75.0){   
    controleLED(2);                                // LED azul
    MQTT.publish(Controle_Incendio, "M");          // Publica o estado da vibração
    Serial.println("Enviado MQTT: M");

  }else if(temp > 75.0 || fumaca >= 600){   
    controleLED(2);                                // LED azul
    MQTT.publish(Controle_Incendio, "H");          // Publica o estado da vibração
    Serial.println("Enviado MQTT: H");

  }else{   
    controleLED(1);                                // LED verde
  }
}


//----------------------Core 2----------------------


/*
   Descrição: Procedimento paralelo que verifica o estado do botão de emergência.
*/
void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){ 
    //Quando o botão de emergência é acionado
    if(digitalRead(botao) == 0){
      MQTT.publish(Controle_Incendio, "H");          // Publica o estado da vibração
      Serial.println("Enviado MQTT: H (por pressionamento do botao)");
      controleLED(2);                                // LED azul
      delay(5000);
    }
    
    delay(50);
  } 
}


//--------------------------------------------------


void setup() {
  Serial.begin(115200);

  //Iniciando o sensor DHT22 
  dht.begin();

  pinMode(botao, INPUT);
  pinMode(LRed, OUTPUT);
  pinMode(LGreen, OUTPUT);
  pinMode(LBlue, OUTPUT);

  //Inicializa a conexao wi-fi
  initWiFi();

  //Inicializa a conexao ao broker MQTT
  initMQTT();

  //Configuração da tarefa no core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* função que implementa a tarefa */
                    "Task1",     /* nome da tarefa */
                    10000,       /* número de palavras a serem alocadas para uso com a pilha da tarefa */
                    NULL,        /* parâmetro de entrada para a tarefa (pode ser NULL) */
                    1,           /* prioridade da tarefa (0 a N) */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* Núcleo que executará a tarefa */ 
                                   
  delay(500);
}


void loop() {
  //Controles principais:
  controleDHT();
  controleMQ2();
  controleDeIncendio();
  
  //keep-alive da comunicação com broker MQTT
  MQTT.loop();


  if (!MQTT.connected())
    reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita

  reconnectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
  
  delay(550);
}