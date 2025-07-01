
const int trigPin = 2;
const int echoPin = 3;
const int IN1 = 9;
const int IN2 = 11;

int ki = 0, kp = 160, kd = 0;
int vel = 255;
int v = 0;
int erro = 0, erro_anterior = 0;
int I, D, P, PID;
float setpoint = 9.0;

long duration;
float distance = 0;

unsigned long ultimoEnvio = 0;
unsigned long tempoCondicao = 0;
bool alertaEnviado = false;

String bufferBluetooth = "";

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  Serial.begin(9600);  // Monitor serial (USB)
  
}

void loop() {
  medicao_sensor();

  if (millis() - ultimoEnvio >= 1000) {
    ultimoEnvio = millis();
    Serial.print(distance);
    Serial.println(";");
  }

  receberSetpoint();
  calculo_erro();
  calculo_PID();
  controle_motor();
  verificarAlerta();
}

void medicao_sensor() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = 18 - duration * 17 / 1000;

  Serial.print("Distância: ");
  Serial.print(distance);
  Serial.println(" cm");
}

void calculo_erro() {
  erro = distance - setpoint;
}

void calculo_PID(){
    if (erro == 0){
  I=0;}
  P = erro;
  I = I + erro;
  if(I > 255){
    I = 255;
  }
  else if(I < -255){
    I = -255;
  }
  D = erro - erro_anterior;
  PID = (kp*P) + (ki*I) + (kd*D);
  erro_anterior = erro;
  }

void controle_motor() {
  int pwmBomba = 0;

  if (PID <= 0) {
    v = 0;
  } else {
    v = vel - PID;
    v = constrain(v, 0, 255);
  }

  pwmBomba = v;

  analogWrite(IN2, pwmBomba);
  analogWrite(IN1, LOW);
}

void verificarAlerta() {
  if (v >= 255 && erro > 0) {
    if (tempoCondicao == 0) {
      tempoCondicao = millis();
    } else if (millis() - tempoCondicao >= 5000 && !alertaEnviado) {
      enviarAlerta();
      alertaEnviado = true;
    }
  } else {
    tempoCondicao = 0;
    alertaEnviado = false;
  }
}

void enviarAlerta() {
  String alerta = "ALERTA: Nível baixo.";
  Serial.println(alerta);
  Serial.println(alerta);
}

void receberSetpoint() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      processarComando(bufferBluetooth);
      bufferBluetooth = "";
    } else {
      bufferBluetooth += c;
    }
  }
}

void processarComando(String comando) {
  comando.trim();
  float novoSetpoint = comando.toFloat();
  if (novoSetpoint >= 0 && novoSetpoint <= 15) {
    setpoint = novoSetpoint;
    Serial.print("Novo setpoint recebido: ");
    Serial.println(setpoint);
  } else {
    Serial.println("Setpoint inválido recebido.");
  }
}
