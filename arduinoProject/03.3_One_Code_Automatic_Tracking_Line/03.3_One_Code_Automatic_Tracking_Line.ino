#define PIN_SERVO      2    //Controlla un eventuale servo motore
#define MOTOR_DIRECTION     1 //If the direction is reversed, change 0 to 1, Variabile per invertire la direzione dei motori.
#define PIN_DIRECTION_LEFT  4 //Variabile per invertire la direzione dei motori
#define PIN_DIRECTION_RIGHT 3 //Pin digitali per il controllo della direzione dei motori.

//Pin per controllare la velocità dei motori con segnali PWM (Pulse Width Modulation).
// Questi sensori restituiscono valori binari: 0: Linea rilevata (nero). 1: Linea assente (bianco o superficie diversa).
#define PIN_MOTOR_PWM_LEFT  6 
#define PIN_MOTOR_PWM_RIGHT 5

//Pin digitale per inviare impulsi ultrasonici.
#define PIN_SONIC_TRIG    7

//Pin digitale per ricevere l'eco degli impulsi, calcolando il tempo di ritorno per stimare la distanza. Scopo: Rilevare ostacoli davanti alla macchina per evitare collisioni.
#define PIN_SONIC_ECHO    8
//Pin collegato al ricevitore IR per controllare la macchina tramite un telecomando.
#define PIN_IRREMOTE_RECV 9

//SPI (Serial Peripheral Interface): Protocollo di comunicazione utilizzato per connettere dispositivi come moduli wireless o sensori avanzati.
//Scopo: Potrebbe essere usato per un modulo wireless
#define PIN_SPI_CE      9
#define PIN_SPI_CSN     10
#define PIN_SPI_MOSI    11
#define PIN_SPI_MISO    12
#define PIN_SPI_SCK     13

//Misura la tensione della batteria tramite un ingresso analogico.
#define PIN_BATTERY     A0
//Controlla un cicalino per notifiche sonore.
#define PIN_BUZZER      A0

//Collegati ai tre sensori IR che rilevano la linea.
#define PIN_TRACKING_LEFT A1  
#define PIN_TRACKING_CENTER A2
#define PIN_TRACKING_RIGHT  A3

//Valore minimo PWM per superare la resistenza iniziale del motore. Evitare che i motori restino fermi con segnali PWM troppo bassi.
#define MOTOR_PWM_DEAD    10

//Velocità impostata a 0 per fermare i motori.
#define TK_STOP_SPEED          0
//Velocità base per avanzare, compensata in base alla tensione della batteria.
//tk_VoltageCompensationToSpeed: Variabile che aumenta o diminuisce la velocità per bilanciare le variazioni di tensione.
#define TK_FORWARD_SPEED        (90 + tk_VoltageCompensationToSpeed    )

/*
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
*/

//define different speed levels, Scopo: Regolare la reattività della macchina nelle curve in base alla posizione rilevata dai sensori.
int tk_VoltageCompensationToSpeed;  //define Voltage Speed Compensation
//TK_TURN_SPEED_LV4 e LV3: Velocità elevate per curve meno strette.
#define TK_TURN_SPEED_LV4       (160 + tk_VoltageCompensationToSpeed   )
#define TK_TURN_SPEED_LV3       (130 + tk_VoltageCompensationToSpeed   )
//TK_TURN_SPEED_LV2 e LV1: Velocità negative per inversione dei motori (curve strette).
#define TK_TURN_SPEED_LV2       (-120 + tk_VoltageCompensationToSpeed  )
#define TK_TURN_SPEED_LV1       (-140 + tk_VoltageCompensationToSpeed  )

float batteryVoltage = 0;
bool isBuzzered = false;

//Configura i pin come input o output e Calibra i sensori di tracciamento.
void setup() {
  pinsSetup(); 
  getTrackingSensorVal(); 
}

/*
  Il codice principale del loop è progettato per leggere i dati dai sensori di tracciamento e
  decidere come controllare i motori, permettendo alla macchina di seguire una linea. 
  La logica principale si basa sul valore combinato dei tre sensori (sinistro, centrale, destro),
  che indicano la posizione della linea rispetto alla macchina.
*/
void loop() {
  u8 trackingSensorVal = 0;
  trackingSensorVal = getTrackingSensorVal(); //get sensor value

  switch (trackingSensorVal)
  {
    case 0:   //000
      motorRun(TK_FORWARD_SPEED, TK_FORWARD_SPEED); //car move forward
      break;
    case 7:   //111
      motorRun(TK_STOP_SPEED, TK_STOP_SPEED); //car stop
      break;
    case 1:   //001
      motorRun(TK_TURN_SPEED_LV4, TK_TURN_SPEED_LV1); //car turn
      break;
    case 3:   //011
      motorRun(TK_TURN_SPEED_LV3, TK_TURN_SPEED_LV2); //car turn right
      break;
    case 2:   //010
    case 5:   //101
      motorRun(TK_FORWARD_SPEED, TK_FORWARD_SPEED);  //car move forward
      break;
    case 6:   //110
      motorRun(TK_TURN_SPEED_LV2, TK_TURN_SPEED_LV3); //car turn left
      break;
    case 4:   //100
      motorRun(TK_TURN_SPEED_LV1, TK_TURN_SPEED_LV4); //car turn right
      break;
    default:
      break;
  }
}

/*
La funzione tk_CalculateVoltageCompensation calcola una compensazione della velocità dei motori in base alla tensione della batteria. 
Questo serve a mantenere un comportamento uniforme della macchina anche quando la tensione della batteria varia.
*/
void tk_CalculateVoltageCompensation() {
  getBatteryVoltage();
  float voltageOffset = 7 - batteryVoltage;
  tk_VoltageCompensationToSpeed = 30 * voltageOffset;
}

/*
La funzione getTrackingSensorVal ha lo scopo di leggere i valori dai 3 sensori di tracciamento e combinarli in un unico valore binario a 3 bit che rappresenta lo stato di rilevamento della linea nera.
Quando viene rilevata una linea nera su un lato, il valore del lato sarà 0 oppure il valore sarà 1
*/
u8 getTrackingSensorVal() {
  u8 trackingSensorVal = 0;
  //== 1 ? 1 : 0, Restituisce: 1: Il sensore rileva una superficie chiara (bianco). 0: Il sensore rileva una linea scura (nero).
  trackingSensorVal = (digitalRead(PIN_TRACKING_LEFT) == 1 ? 1 : 0) << 2 | (digitalRead(PIN_TRACKING_CENTER) == 1 ? 1 : 0) << 1 | (digitalRead(PIN_TRACKING_RIGHT) == 1 ? 1 : 0) << 0;
  return trackingSensorVal;
}

/*Configura i pin come input/output per comunicare con i sensori e i motori. Questa funzione configura i pin del Arduino) per specificare il loro utilizzo, ovvero se devono:
Inviare segnali (OUTPUT): Comandare motori, cicalini, LED. O ricevere segnali (INPUT): Leggere i valori da sensori.
*/
void pinsSetup() {
  //define motor pin
  pinMode(PIN_DIRECTION_LEFT, OUTPUT);
  pinMode(PIN_MOTOR_PWM_LEFT, OUTPUT);
  pinMode(PIN_DIRECTION_RIGHT, OUTPUT);
  pinMode(PIN_MOTOR_PWM_RIGHT, OUTPUT);
  //define ultrasonic moduel pin
  pinMode(PIN_SONIC_TRIG, OUTPUT);
  pinMode(PIN_SONIC_ECHO, INPUT);
  //define tracking sensor pin
  pinMode(PIN_TRACKING_LEFT, INPUT);
  pinMode(PIN_TRACKING_RIGHT, INPUT);
  pinMode(PIN_TRACKING_CENTER, INPUT);
  setBuzzer(false);
}

/*
  La funzione motorRun è utilizzata per controllare i motori della macchina, specificando la direzione e la velocità per ciascun motore (sinistro e destro).
  speedl: Velocità del motore sinistro. speedr: Velocità del motore destro. I valori possono essere positivi o negativi: 
  Positivi: Indicano la rotazione in avanti; Negativi: Indicano la rotazione all'indietro.
*/
void motorRun(int speedl, int speedr) {
  int dirL = 0, dirR = 0;
  //I due blocchi servono a controllare separatamente il motore sinistro e destro. Se la velocità è positiva: Il motore gira avanti. 
  // Se la velocità è negativa: Il motore gira indietro
  if (speedl > 0) {
    dirL = 0 ^ MOTOR_DIRECTION;
  } else {
    dirL = 1 ^ MOTOR_DIRECTION;
    speedl = -speedl;
  }

  if (speedr > 0) {
    dirR = 1 ^ MOTOR_DIRECTION;
  } else {
    dirR = 0 ^ MOTOR_DIRECTION;
    speedr = -speedr;
  }

  speedl = constrain(speedl, 0, 255); // speedl Limita il valore della velocità tra 0 e 255.
  speedr = constrain(speedr, 0, 255); // speedr absolute value should be within 0~255

  //Imposta i pin di direzione dei motori (sinistro e destro) con i valori calcolati (dirL e dirR). Questi valori determinano la rotazione in avanti o indietro.
  digitalWrite(PIN_DIRECTION_LEFT, dirL);
  digitalWrite(PIN_DIRECTION_RIGHT, dirR);

  //Invia segnali PWM per controllare la velocità dei motori. La potenza inviata al motore è proporzionale al valore di speedl o speedr.
  analogWrite(PIN_MOTOR_PWM_LEFT, speedl);
  analogWrite(PIN_MOTOR_PWM_RIGHT, speedr);
}

/*Il codice della funzione getBatteryVoltage è progettato per leggere la tensione della batteria, 
calcolarla in base al valore analogico letto e restituire un risultato booleano che indica se la tensione è inferiore a una determinata soglia.
La funzione è utilizzata nel programma principale per verificare la tensione della batteria e garantire che sia sufficientemente alta per far funzionare correttamente la macchina.*/
bool getBatteryVoltage() {
  //La funzione esegue il controllo della tensione solo se il cicalino (isBuzzered) è disattivato
  if (!isBuzzered) {
    //Configura il pin della batteria (PIN_BATTERY) come input analogico, in modo che il microcontrollore possa leggere il livello di tensione.
    pinMode(PIN_BATTERY, INPUT);
    //Utilizza analogRead per leggere il valore dal pin analogico
    int batteryADC = analogRead(PIN_BATTERY);
    if (batteryADC < 614) // 3V/12V ,Voltage read: <2.1V/8.4V
    {
      //viene calcolata la tensione della batteria
      batteryVoltage = batteryADC / 1023.0 * 5.0 * 4;
      return true;
    }
  }
  return false;
}

//Controlla lo stato del buzzer accendendolo o spegnendolo in base al valore del parametro flag
void setBuzzer(bool flag) {
  isBuzzered = flag;
  pinMode(PIN_BUZZER, flag);
  digitalWrite(PIN_BUZZER, flag);
}

/*
Fa suonare il buzzer con un pattern di bip: beat: Numero di bip consecutivi in una sequenza. repeat: Numero di volte che la sequenza viene ripetuta.
*/
void alarm(u8 beat, u8 repeat) {
  beat = constrain(beat, 1, 9);
  repeat = constrain(repeat, 1, 255);
  for (int j = 0; j < repeat; j++) {
    for (int i = 0; i < beat; i++) {
      setBuzzer(true);
      delay(100);
      setBuzzer(false);
      delay(100);
    }
    delay(500);
  }
}

//Ferma la macchina e spegne il buzzer.
void resetCarAction() {
  motorRun(0, 0);
  setBuzzer(false);
}