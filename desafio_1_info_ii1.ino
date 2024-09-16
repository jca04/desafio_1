#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x20,16,2);

void calculatePeriod(unsigned long *period, int dataOfSensor, int *actualValue);
void calculateAmplitude(int **samplingOfdata, float *amplitude);
void calculateTypeOfWave(int *maxValue, int *minValue, int dataOfSensor);
void showDataInLcd(float amplitude, float frecuency, unsigned short typeWave);

unsigned const short PINSIGNAL = A0; //definicion de pines
unsigned const short PINPUSHBUTTON1 = 7;
unsigned const short PINPUSHBUTTON2 = 4;

//Recoleccion de datos, ejecucion principial del program
bool isRecolecting = false;
bool stopRecolectiong = false;
unsigned int index = 0;
int *samplingOfdata = new int[200];

//amplitud y frecuencia
unsigned long period = 0, firstTime = 0;
float frecuency = 0.0, amplitude = 0.0;
bool isDecreasing = false;
int actualValue = 0;

// tipo de onda
bool isSinusoidal = false;
bool isTriangular = false;
bool isSquare = false;
bool isUnknown = false;
int maxValue = 0;
int minValue = 0;
unsigned short typeWave; 

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
}


void loop()
{
  if (digitalRead(PINPUSHBUTTON1) == HIGH ){ //primer pulsador presionado
    if (!stopRecolectiong) isRecolecting = true;
    delay(50);
  }
  
  
  if (digitalRead(PINPUSHBUTTON2) == HIGH){ // segundo pulsador presionado
    if (isRecolecting) { // detener recoleccion de datos
      stopRecolectiong = true;
      isRecolecting = false;
      delay(50);
    }
  }
  
  
  if (isRecolecting && !stopRecolectiong){ // Recoleccion de datos
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Recolectando..");

    const int dataOfSensor = analogRead(PINSIGNAL); // datos entregados por el generador
    
    if (index < 200) *(samplingOfdata + index) = dataOfSensor;
    else index = 0;
   
    
    index++;
    
    //Calcular el periodo entre dos picos
    calculatePeriod(&period ,dataOfSensor, &actualValue);
   
    double seconds = period / 1000000.0; //microsegundos a segundo
    
    if (seconds > 0.0){
      frecuency = 1.0 / seconds; // frecuencia
    }
                         
    // Calcular tipo de onda
    //calculateTypeOfWave(&maxValue, &minValue, dataOfSensor);
  }
  
  if (stopRecolectiong){ // BLOQUE DE CODIGO CUANDO SE DETUVO LA RECOLECCION (AQUI VAN LOS ALGORITMOS)
     
    calculateAmplitude(&samplingOfdata, &amplitude);
    
    //Margen de error; 2v 
    float vol = (amplitude * 10.0) / 1023.0;
    
    if (vol >= 6.0) vol += 1.1; //margen error 1.3v
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(vol);
    
    showDataInLcd(vol, frecuency, 1);
    
    delay(50);
    stopRecolectiong = false;
    
    for (int i = 0; i < 200; i++) samplingOfdata[i] = 0;
    
  }
}

//Calcular el periodo 
void calculatePeriod(unsigned long *period, int dataOfSensor, int *actualValue){
  
    if ((*actualValue > dataOfSensor)&& !isDecreasing){ //cuando llego al pico y empieza a bajar
    
      unsigned const long time = micros(); //capturar el tiempo en microsegundos
  	
      if (firstTime != 0) *period = time - firstTime;
      
      firstTime = time;
      isDecreasing = true;
    }
    
    if (dataOfSensor > *actualValue && isDecreasing) isDecreasing = false; // el pico esta subiendo
    
    *actualValue = dataOfSensor;
}

//calcular la amplitud
void calculateAmplitude(int **samplingOfdata, float *amplitude){

  int max = *samplingOfdata[0], min = *samplingOfdata[0];
  
  for (int i = 0; i < 200; i++){
    if (*(*samplingOfdata + i) > max){
      if (*(*samplingOfdata + i) <= 1023)
    	max = *(*samplingOfdata + i);
    }
    	
    if (*(*samplingOfdata + i) < min){
      if (*(*samplingOfdata + i) != 0){
     	 min = *(*samplingOfdata + i);
      }
    }    
  }
  
  *amplitude = (max - min) / 2.0;
 
}

void calculateTypeOfWave(int *maxValue, int *minValue, int dataOfSensor) {

  if (dataOfSensor > *maxValue) *maxValue = dataOfSensor;
  if (dataOfSensor < *minValue) *minValue = dataOfSensor;

  // umbrales de tolerancia
  int amplitude = *maxValue - *minValue;
  int threshold = amplitude * 0.1; // 10% de la amplitud
  int halfAmplitude = *minValue + amplitude / 2;

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

      // Si la señal alcanza o se acerca a los picos de forma rápida, es cuadrada
      if (abs(*maxValue - dataOfSensor) <= threshold || abs(*minValue - dataOfSensor) <= threshold) {
        isSquare = true;
        isSinusoidal = false;
        isTriangular = false;
        isUnknown = false;
      }

      // Si la señal tiene picos menos pronunciados, es triangular
      if (abs(*maxValue - dataOfSensor) > threshold && abs(*minValue - dataOfSensor) > threshold) {
        isTriangular = true;
        isSinusoidal = false;
        isSquare = false;
        isUnknown = false;
      }

      // Si la señal sube y baja de manera más suave, es sinusoidal
      else if (abs(dataOfSensor - halfAmplitude) <= threshold) {
        isSinusoidal = true;
        isSquare = false;
        isTriangular = false;
        isUnknown = false;
      }

      // Si no se cumple ningún patrón claro, se clasifica como desconocida
      else {
        isUnknown = true;
        isSinusoidal = false;
        isSquare = false;
        isTriangular = false;
      }

    }
  }

  wasRising = isRising;

  // Despues se decide como se va a mostrar en el lcd
  lcd.clear();
  lcd.setCursor(0, 0);
  if (isSquare) {
    lcd.print("Onda Cuadrada");
  } else if (isSinusoidal) {
    lcd.print("Onda Sinusoidal");
  } else if (isTriangular) {
    lcd.print("Onda Triangular");
  } else if (isUnknown) {
    lcd.print("Onda Desconocida");
  }
}

//Mostrar datos en el lc
void showDataInLcd(float amplitude, float frecuency, unsigned short typeWave){
	//typewave
  	//1 senoidal
  	//2 triangular
  	//3 cuadrada
  	//4 desconocidad
  lcd.clear();
  lcd.setCursor(0,0);

  switch(typeWave){
    case 1:
      lcd.print("Onda senoidal");
    break;
    case 2:
      lcd.print("Onda triangular");
    break;
    case 3:
    	lcd.print("Onda cuadrada");
    break;
    case 4:
    	lcd.print("Desconocida");
    	delay(2000);
    	return;
    break;
    default:
    break;
  }
  
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Amplitud");
  lcd.setCursor(0,1);
  lcd.print(amplitude);
  lcd.print("V");
  
  delay(2000);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Frecuencia");
  lcd.setCursor(0,1);
  lcd.print(frecuency);
  lcd.print("hz");
  
  delay(2000);
  
  
}