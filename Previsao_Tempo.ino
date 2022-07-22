/***************************************************
  Previsão do Tempo com Arduino MEGA e Display LCD Exemplo

  Criado em 15 de Julho de 2021 por Michel Galvão
 ****************************************************/

// Inclusão das Biblitecas
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <Ethernet.h>
#include <SPI.h>

//Configuração Display LCD I2C
/* Para saber o endereço I2C do Display LCD, faça o upolad do código
    de exemplo i2c_scanner. Você pode acessar o código
    em: Arquivo -> Exemplos -> Wire ->  i2c_scanner. Faça o upload,
    já com o circuito previamente montado, e abra o Monitor Serial.
    Você verá a mensagem 'I2C device found at address 0x27!'
    indicando o endereço à que o Display LCD está endereçado.
*/
LiquidCrystal_I2C lcd(0x27, 16, 2); // LiquidCrystal_I2C nomeDoObjeto(endereço I2C, Quantidade de Colunas, Quantidade de Linhas);

// Variáveis de Armazenamento dos dados do Tempo
int temperatura;
char* dataConsulta;
char* horarioConsulta;
char* descricaoTempo;
char* diaOuNoite;
char* LocalParaConsulta;
int umidade;
char* velocidadeVento;
char* nascerDoSol;
char* poenteDoSol;
char* dataPrevisao1;
char* diaDaSemanaPrevisao1;
int maxPrevisao1;
int minPrevisao1;
char* descricaoTempoPrevisao1;
char* dataPrevisao2;
char* diaDaSemanaPrevisao2;
int maxPrevisao2;
int minPrevisao2;
char* descricaoTempoPrevisao2;
char* dataPrevisao3;
char* diaDaSemanaPrevisao3;
int maxPrevisao3;
int minPrevisao3;
char* descricaoTempoPrevisao3;
char* dataPrevisao4;
char* diaDaSemanaPrevisao4;
int maxPrevisao4;
int minPrevisao4;
char* descricaoTempoPrevisao4;

//Criação de matriz para um caractere personalizado (símbolo grau)
/* Para mais detalhes de como criar um caractere personalizado:
   acesse o site https://blog.eletrogate.com/guia-completo-do-display-lcd-arduino/#titulo3;
*/
byte grau[] = {
  B00111,
  B00101,
  B00111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

String woeid = "INSIRA_AQUI_SEU_WOIED"; // WOIED de sua cidade
String chave = "INSIRA_AQUI_SUA_CHAVE_DA_API"; // Sua Chave da API HG Brasil Weather

void setup() {
  lcd.init();// Inicializa o Display LCD
  lcd.createChar(0, grau); // Armazena na memória do LCD o caractere criado;
  lcd.backlight(); // Deixa  a luz de fundo do siplay LCD ligada
  lcd.clear(); // Limpa a tela LCD

  Serial.begin(9600); // Configura a taxa de transferência para transmissão serial

  Ethernet.init(53); // Configura o pino CS (seleção de chip) para o módulo Ethernet
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED}; // Define o endereço MAC virtual para o módulo Ethernet o utilizar
  if (Ethernet.begin(mac) == 0) { // Se o módulo Ethernet não inicializar, ...
    Serial.println(F("Falha ao configurar Ethernet"));

    if (Ethernet.hardwareStatus() == EthernetNoHardware) { // Se o módulo Ethernet não encontrado, ...
      Serial.println("A placa Ethernet não foi encontrada. Desculpe, não pode ser executado sem hardware. :(");

      while (Ethernet.hardwareStatus() == EthernetNoHardware) { // Tenta encontrar o módulo Ethernet
        delay(100); // Espera 100 ms entre as tentativas
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) { // Se o link estiver desligado, ...
      Serial.println("O cabo Ethernet não está conectado.");

      while (Ethernet.linkStatus() == LinkOFF) { // Tenta fazer link Ligado
        delay(100); // Espera 100 ms entre as tentativas
      }
    }
  } else {
    Serial.println(F("Ethernet OK!"));
  }

  // Exibe uma tela inicial (Splash screen)
  lcd.setCursor(0, 0);
  lcd.print("Previsao Tempo  ");
  lcd.setCursor(0, 1);
  lcd.print("com Arduino MEGA");
  delay(2500);
}

void loop() {
  atualizarDadosDoTempo(); // Chama função para atualizar variáveis do Tempo e da Previsão do tempo

  /* Exibição de 'Telas' no display LCD com informações do Tempo e da
     Previsão do tempo com delays necessários para não exceder o limite
     de requisições da API do Tempo que no plano gratuito é 2500 requisições
  */
  telaLcd0();
  delay(1000);
  telaLcd1();
  delay(14000);
  telaLcd2();
  delay(4000);
  telaLcd3();
  delay(4000);
  telaLcd4();
  delay(4000);
  telaLcd5();
  delay(4000);
  telaLcd6();
  delay(4000);
  telaLcd7PrevisaoDia1();
  delay(4000);
  telaLcd8PrevisaoDia1();
  delay(4000);
  telaLcd9PrevisaoDia2();
  delay(4000);
  telaLcd10PrevisaoDia2();
  delay(4000);
  telaLcd11PrevisaoDia3();
  delay(4000);
  telaLcd12PrevisaoDia3();
  delay(4000);
  telaLcd13PrevisaoDia4();
  delay(4000);
  telaLcd14PrevisaoDia4();
  delay(4000);
  telaLcd15();
  delay(1000);
  /* Tempo delay Total: 60000 milissegundos = 60 segundos;
     Total de Requisições para API por dia: [(60 segundos tem em 1 minuto)/(60 segundos de intervalo)]*(60 minutos)*(24 horas) = 1440 requisições;
     Máximo de requisões para a API no plano gratuito: 2500 requisições
  */
}

void atualizarDadosDoTempo() { // Função para atualizar variáveis do Tempo e da Previsão do tempo
  Serial.println(F("Conectando à API do Tempo..."));

  EthernetClient client; // Cria um cliente para se conectar ao servidor da API do Tempo
  client.setTimeout(10000); // Define o máximo de milissegundos para aguardar os dados de fluxo

  if (!client.connect("api.hgbrasil.com", 80)) { // Se conectado ao endereço do servidor, na porta 80, com sucesso...
    Serial.println(F("Conexão falhou :("));

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Conexao com API falhou. Veja a internet");

    while (!client.connect("api.hgbrasil.com", 80)) { // enquanto não estiver conectado ao endereço do servidor,...
      delay(450);
      lcd.scrollDisplayLeft(); // rola para a esquerda
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Conexao com API restaurada!");


      // Função para rolagem de LCD personalizada. informe a velocidade de Rolagem e a quantidad  e de caracteres, respectivamente. Para mais detalhes, veja a criação da função mais abaixo.
    rolamentoLCD(5, strlen("Conexao com API restaurada!") + strlen("API connection restored!"));
    return;
  }

  Serial.println(F("Conectado!"));

  // Envia pedido HTTP ao servidor da API do Tempo, solicitando dados de 4 dias (hoje + 3 dias à frente) em array_limit=4.
  //O número de array_limit é um inteiro limitando o número de itens em arrays do retorno
  client.println("GET /weather?array_limit=4&fields=only_results,temp,date,time,description,currently,city,humidity,wind_speedy,sunrise,sunset,forecast,date,weekday,max,min,description,&key=" + chave
                 + "&woeid=" + woeid
                 + " HTTP/1.0");
  client.println(F("Host: api.hgbrasil.com"));
  client.println(F("Connection: close"));

  if (client.println() == 0) { // se o retorno de dados do servidor conectado for dde 0 bytes,...
    Serial.println(F("Falha ao enviar pedido"));
    client.stop(); // Desconecta-se do servidor
    return;
  }

  // Verifica o status do HTTP
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  // Deve ser "HTTP / 1.0 200 OK" ou "HTTP / 1.1 200 OK"
  if (strcmp(status + 9, "200 OK") != 0) {
    Serial.print(F("Resposta inesperada: "));
    Serial.println(status);
    client.stop();
    return;
  }

  // Pular cabeçalhos HTTP
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Resposta inválida"));
    client.stop();
    return;
  }

  // Alocar o documento JSON
  // Use arduinojson.org/v6/assistant para calcular a capacidade.
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 600; // Necessário 600 bytes de memória reservada para o processo de Desserialização do documento JSON
  DynamicJsonDocument doc(capacity);

  // Analisa o objeto JSON
  DeserializationError error = deserializeJson(doc, client);
  if (error) {
    Serial.print(F("deserializeJson() falhou: "));
    Serial.println(error.f_str());
    client.stop();
    return;
  }

  // Extrai os valores do tempo e repassa para as variáveis globais
  Serial.println(F("Resposta:"));
  temperatura = doc["temp"].as<int>();
  Serial.println(doc["temp"].as<int>());
  dataConsulta = doc["date"].as<char*>();
  Serial.println(doc["date"].as<char*>());
  horarioConsulta = doc["time"].as<char*>();
  Serial.println(doc["time"].as<char*>());
  descricaoTempo = doc["description"].as<char*>();
  Serial.println(doc["description"].as<char*>());
  diaOuNoite = doc["currently"].as<char*>();
  Serial.println(doc["currently"].as<char*>());
  LocalParaConsulta = doc["city"].as<char*>();
  Serial.println(doc["city"].as<char*>());
  umidade = doc["humidity"].as<int>();
  Serial.println(doc["humidity"].as<int>());
  velocidadeVento = doc["wind_speedy"].as<char*>();
  Serial.println(doc["wind_speedy"].as<char*>());
  nascerDoSol = doc["sunrise"].as<char*>();
  Serial.println(doc["sunrise"].as<char*>());
  poenteDoSol = doc["sunset"].as<char*>();
  Serial.println(doc["sunset"].as<char*>());

  // Extrai os valores da Previsão do tempo e repassa para as variáveis globais
  Serial.println("\nPrevisão do Tempo\n");
  int indice = 1;
  for (JsonObject elem : doc["forecast"].as<JsonArray>()) {
    const char* date = elem["date"];
    const char* weekday = elem["weekday"];
    int max = elem["max"];
    int min = elem["min"];
    const char* description = elem["description"];

    switch (indice) {
      case 1:
        dataPrevisao1 = date;
        diaDaSemanaPrevisao1 = weekday;
        maxPrevisao1 = max;
        minPrevisao1 = min;
        descricaoTempoPrevisao1 = description;
        break;
      case 2:
        dataPrevisao2 = date;
        diaDaSemanaPrevisao2 = weekday;
        maxPrevisao2 = max;
        minPrevisao2 = min;
        descricaoTempoPrevisao2 = description;
        break;
      case 3:
        dataPrevisao3 = date;
        diaDaSemanaPrevisao3 = weekday;
        maxPrevisao3 = max;
        minPrevisao3 = min;
        descricaoTempoPrevisao3 = description;
        break;
      case 4:
        dataPrevisao4 = date;
        diaDaSemanaPrevisao4 = weekday;
        maxPrevisao4 = max;
        minPrevisao4 = min;
        descricaoTempoPrevisao4 = description;
        break;
    }
    Serial.print("dia ");
    Serial.println(date);
    Serial.println(weekday);
    Serial.println(max);
    Serial.println(min);
    Serial.println(description);
    Serial.println("\n");
    indice++;
  }

  // Desconecta-se do servidor
  client.stop();
}

// Funções de Exibição das Telas no display LCD
void telaLcd0() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tempo Atual     ");
  lcd.setCursor(0, 1);
  lcd.print("     AGORA      ");
}
void telaLcd1() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(temperatura);
  lcd.write(0);//grau caractere
  lcd.print("C ");
  lcd.print(umidade);
  lcd.print("%RH ");
  lcd.print(diaOuNoite);
  lcd.print("  ");
  lcd.setCursor(0, 1);
  lcd.print(horarioConsulta);
  lcd.print(" ");
  lcd.print(dataConsulta);
}

void telaLcd2() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(descricaoTempo);
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 0);

  rolamentoLCD(5, strlen(descricaoTempo));

}

void telaLcd3() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Local: ");
  lcd.setCursor(0, 1);
  lcd.print(LocalParaConsulta);

  rolamentoLCD(5, strlen(LocalParaConsulta));
}

void telaLcd4() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Vento ");
  lcd.print(velocidadeVento);
  lcd.print("         ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

void telaLcd5() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Aurora  ");
  lcd.print(nascerDoSol);
  lcd.setCursor(0, 1);
  lcd.print("Poente  ");
  lcd.print(poenteDoSol);
}
void telaLcd6() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Previsao para os");
  lcd.setCursor(0, 1);
  lcd.print("proximos 3 dias ");
}
void telaLcd7PrevisaoDia1() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(dataPrevisao1);
  lcd.print(" ");
  lcd.print(diaDaSemanaPrevisao1);
  lcd.setCursor(0, 1);
  lcd.print("max ");
  lcd.print(maxPrevisao1);
  lcd.write(0);//grau caractere
  lcd.print("C min ");
  lcd.print(minPrevisao1);
  lcd.write(0);//grau caractere
}
void telaLcd8PrevisaoDia1() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(dataPrevisao1);
  lcd.print(" ");
  lcd.print(descricaoTempoPrevisao1);

  rolamentoLCD(5, strlen(dataPrevisao1) + 1 + strlen(descricaoTempoPrevisao1));
}
void telaLcd9PrevisaoDia2() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(dataPrevisao2);
  lcd.print(" ");
  lcd.print(diaDaSemanaPrevisao2);
  lcd.setCursor(0, 1);
  lcd.print("max ");
  lcd.print(maxPrevisao2);
  lcd.write(0);//grau caractere
  lcd.print("C min ");
  lcd.print(minPrevisao2);
  lcd.write(0);//grau caractere
}
void telaLcd10PrevisaoDia2() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(dataPrevisao2);
  lcd.print(" ");
  lcd.print(descricaoTempoPrevisao2);

  rolamentoLCD(5, strlen(dataPrevisao2) + 1 + strlen(descricaoTempoPrevisao2));
}
void telaLcd11PrevisaoDia3() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(dataPrevisao3);
  lcd.print(" ");
  lcd.print(diaDaSemanaPrevisao3);
  lcd.setCursor(0, 1);
  lcd.print("max ");
  lcd.print(maxPrevisao3);
  lcd.write(0);//grau caractere
  lcd.print("C min ");
  lcd.print(minPrevisao3);
  lcd.write(0);//grau caractere
}
void telaLcd12PrevisaoDia3() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(dataPrevisao3);
  lcd.print(" ");
  lcd.print(descricaoTempoPrevisao3);

  rolamentoLCD(5, strlen(dataPrevisao3) + 1 + strlen(descricaoTempoPrevisao3));
}
void telaLcd13PrevisaoDia4() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(dataPrevisao4);
  lcd.print(" ");
  lcd.print(diaDaSemanaPrevisao4);
  lcd.setCursor(0, 1);
  lcd.print("max ");
  lcd.print(maxPrevisao4);
  lcd.write(0);//grau caractere
  lcd.print("C min ");
  lcd.print(minPrevisao4);
  lcd.write(0);//grau caractere
}
void telaLcd14PrevisaoDia4() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(dataPrevisao4);
  lcd.print(" ");
  lcd.print(descricaoTempoPrevisao4);

  rolamentoLCD(5, strlen(dataPrevisao4) + 1 + strlen(descricaoTempoPrevisao4));
}
void telaLcd15() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fim da Previsao ");
  lcd.setCursor(0, 1);
  lcd.print("do tempo        ");
}

/* rolamentoLCD(velocidade, quantidadeDeCaracteres);
   velocidade: velocidade de rolagem no LCD. Quanto mais baixo o número mais rápido. Velocidade de rolagem rápida máxima: 1; Velocidade de rolagem lenta máxima: infinito, desde que o usuário queira;
   quantidadeDeCaracteres: quantidade de caracteres utilizado no display. Deve-se passar um número inteiro.
*/
void rolamentoLCD(int velocidade, int deslocamento) {

  velocidade = velocidade * 100;
  if (deslocamento > 16) {
    deslocamento = deslocamento - 16;
    delay(velocidade * 2);

    for (int i = 0; i < deslocamento + 1; i++) {
      lcd.scrollDisplayLeft();//fazer scrool
      delay(velocidade);
    }
    delay(velocidade * 1.5);
  }
}