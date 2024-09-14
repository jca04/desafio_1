#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x20,16,2);

void calculatePeriod(unsigned long *period, int dataOfSensor, int *actualValue);
void calculateAmplitude(int **samplingOfdata, int *amplitude);
void calculateTypeOfWave(int *maxValue, int *minValue, int dataOfSensor);

unsigned const short PINSIGNAL = A0; //definicion de pines
unsigned const short PINPUSHBUTTON1 = 7;
unsigned const short PINPUSHBUTTON2 = 4;

//Recoleccion de datos, ejecucion principial del program
bool isRecolecting = false;
bool stopRecolectiong = false;
unsigned int size = 0, TotalIndex = 0;
int *samplingOfdata = new int[1000];

//amplitud y frecuencia
unsigned long period = 0, firstTime = 0;
float frecuency = 0.0;
bool isDecreasing = false;
int actualValue = 0;
int amplitude = 0;

// tipo de onda
bool isSinusoidal = false;
bool isTriangular = false;
bool isSquare = false;
int maxValue = 0;
int minValue = 0;

void setup()
{
  pinMode(PINSIGNAL, INPUT);
  pinMode(PINPUSHBUTTON1, INPUT_PULLUP);
  pinMode(PINPUSHBUTTON2, INPUT_PULLUP);
  Serial.begin(9600);
  
  lcd.init();
  lcd.backlight();
  lcd.print("Bienvenido...");
  lcd.setCursor(0,1);
  lcd.print("Opte Senales...");
}


void loop()
{
  if (digitalRead(PINPUSHBUTTON1) == HIGH ){ //primer pulsador presionado
  	isRecolecting = true;
  }
  
  if (digitalRead(PINPUSHBUTTON2) == HIGH){ // segundo pulsador presionado
    if (isRecolecting) { // detener recoleccion de datos
      stopRecolectiong = true;
      isRecolecting = false;
      delay(50);
    }
  }
  
  
  if (isRecolecting && !stopRecolectiong){ // Recoleccion de datos

    const int dataOfSensor = analogRead(PINSIGNAL); // datos entregados por el generador
    Serial.println(dataOfSensor);
    
    if (TotalIndex == 1000)TotalIndex = 0;
   
    *(samplingOfdata + TotalIndex) = dataOfSensor;
    TotalIndex++;
    
    //Calcular el periodo entre dos picos
    calculatePeriod(&period ,dataOfSensor, &actualValue);

    // Calcular tipo de onda
    calculateTypeOfWave(&maxValue, &minValue, dataOfSensor);
   
    double seconds = period / 1000000.0; //microsegundos a segundo
    
    if (seconds > 0.0){
      frecuency = 1.0 / seconds; // frecuencia
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(frecuency);
   	  //Serial.println(frecuency, 7);
    }
  }
  
  if (stopRecolectiong){ // BLOQUE DE CODIGO CUANDO SE DETUVO LA RECOLECCION (AQUI VAN LOS ALGORITMOS)
      
     for (int i = 0; i < 1000; i++){
      Serial.println(*(samplingOfdata + i));
     }
      
     delete[] samplingOfdata;
     samplingOfdata = nullptr;
  }
}

//Calcular el periodo 
void calculatePeriod(unsigned long *period, int dataOfSensor, int *actualValue){
  
    if ((*actualValue > dataOfSensor)&& !isDecreasing){ //cuando llego al pico y empieza a bajar
    
      unsigned const long time = micros(); //capturar el tiempo en microsegundos
  	
      if (firstTime != 0)*period = time - firstTime;
      
      firstTime = time;
      isDecreasing = true;
    }
    
    if (dataOfSensor > *actualValue && isDecreasing) isDecreasing = false; // el pico esta subiendo
    
    *actualValue = dataOfSensor;
}

//calcular la amplitud
void calculateAmplitude(int **samplingOfdata, int *amplitude){
	
}

// tipo de onda
void calculateTypeOfWave(int *maxValue, int *minValue, int dataOfSensor) {

  if (dataOfSensor > *maxValue) *maxValue = dataOfSensor;
  if (dataOfSensor < *minValue) *minValue = dataOfSensor;

  // umbrales de tolerancia
  int amplitude = *maxValue - *minValue;
  int threshold = amplitude * 0.1; // 10% de la amplitud
  int halfAmplitude = amplitude / 2;

  // verificar si los valores fluctúan rápidamente entre los máximos y mínimos
  static bool isRising = false;
  static bool wasRising = false;

  if (dataOfSensor > halfAmplitude + threshold) {
    isRising = true; // esta subiendo
  } else if (dataOfSensor < halfAmplitude - threshold) {
    isRising = false; // esta bajando
  }

  // Si ha habido un cambio de estado de subida o bajada o viceversa, es triangular o sinusoidal
  if (wasRising != isRising) {
    if (isRising) {

      // si la señal alterna entre los valores pico de forma rápida, es cuadrada
      if (abs(*maxValue - dataOfSensor) <= threshold) {
        isSquare = true;
        isSinusoidal = false;
        isTriangular = false;
      }

    }
  }

  wasRising = isRising;

  if (isSquare) { // Despues se decide como se va a mostrar en el lcd
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Cuadrada");
  }


}



