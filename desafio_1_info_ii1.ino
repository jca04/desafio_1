#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x20,16,2);

void calculatePeriod(unsigned long *period, int dataOfSensor, int *actualValue);
void calculateAmplitude(int **samplingOfdata, float *amplitude);
unsigned short calculateTypeOfWave(int *samplingOfData);
void showDataInLcd(float amplitude, float frecuency, unsigned short typeWave);
bool checkSquareWave(int *data);
bool checkTriangularWave(int *data);
bool checkSinusoidalWave(int *data);

unsigned const short PINSIGNAL = A0; //definicion de pines
unsigned const short PINPUSHBUTTON1 = 7;
unsigned const short PINPUSHBUTTON2 = 4;

//Recoleccion de datos, ejecucion principial del program
bool isRecolecting = false;
bool stopRecolectiong = false;
unsigned int index = 0;
const unsigned int SIZE = 50;
int *samplingOfdata = new int[SIZE];

//amplitud y frecuencia
unsigned long period = 0, firstTime = 0;
float frecuency = 0.0, amplitude = 0.0;
bool isDecreasing = false;
int actualValue = 0;

// tipo de onda
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

    if (index < SIZE) *(samplingOfdata + index) = dataOfSensor;
    else index = 0;
   
    
    index++;
    
    //Calcular el periodo entre dos picos
    calculatePeriod(&period ,dataOfSensor, &actualValue);
   
    double seconds = period / 1000000.0; //microsegundos a segundo
    
    if (seconds > 0.0){
      frecuency = 1.0 / seconds; // frecuencia
    }
  }
  
  if (stopRecolectiong){ // BLOQUE DE CODIGO CUANDO SE DETUVO LA RECOLECCION (AQUI VAN LOS ALGORITMOS)
     
    calculateAmplitude(&samplingOfdata, &amplitude);
    
    //Margen de error; 2v 
    float vol = (amplitude * 10.0) / 1023.0;
    
    if (vol >= 6.0) vol += 1.1; //margen error 1.3v
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(vol);

    typeWave = calculateTypeOfWave(samplingOfdata);

    showDataInLcd(vol, frecuency, typeWave);
    
    delay(50);
    stopRecolectiong = false;
    
    for (int i = 0; i < SIZE; i++) samplingOfdata[i] = 0;
    index = 0;
    
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

// Función para determinar el tipo de onda
unsigned short calculateTypeOfWave(int *samplingOfData) {

  if (checkSquareWave(samplingOfData)) {
    return 3;
  } 
  if (checkTriangularWave(samplingOfData)) {
    return 2;
  } 
  if (checkSinusoidalWave(samplingOfData)) {
    return 1;
  } 
  
  return 4;

}

// Función para identificar una onda cuadrada
bool checkSquareWave(int *data) {
  for (int i = 1; i < SIZE - 1; i++) {
    if (data[i] != data[i - 1] && data[i] != data[i + 1]) {
      return false;
    }
  }
  return true;
}

// En estas dos funciones se escogio utilizar double debido a la precisión que ofrece con respecto a float

// Función para identificar una onda triangular
bool checkTriangularWave(int *data) {
  if (SIZE < 3) {
        return false; // No hay suficientes datos para hacer la verificación
    }

    // Calcular la derivada discreta
    // El resultado es la pendiente entre esos dos puntos
    double derivate[SIZE - 1];
    for (int i = 0; i < SIZE - 1; i++) {
        derivate[i] = data[i + 1] - data[i];
    }

    // Contar cuántas veces la pendiente cambia abruptamente
    int abruptChanges = 0;
    for (int i = 1; i < SIZE - 2; i++) {
        double changePrevious = derivate[i - 1];
        double actualChange = derivate[i];

        // Detectar cambios abruptos en la pendiente
        if (fabs(actualChange - changePrevious) > 10.0) { // Umbral para cambio abrupto
            abruptChanges++;
        }
    }

    // Si hay muchos cambios abruptos
    if (abruptChanges > (SIZE / 5)) { 
        return true; // La onda es triangular
    } else {
        return false; // La onda no es triangular
    }
}

bool checkSinusoidalWave(int* data) {
    if (SIZE < 3) {
        return false; // No hay suficientes datos para hacer la verificación
    }

    // Calcular la derivada discreta
    double derivate[SIZE - 1];
    for (int i = 0; i < SIZE - 1; i++) {
        derivate[i] = data[i + 1] - data[i];
    }

    // Verificar si las pendientes cambian suavemente
    double smoothChanges = 0;
    for (int i = 1; i < SIZE - 2; i++) {
        double changePrevious = derivate[i - 1];
        double actualChange = derivate[i];

        // Detectar si los cambios son suaves
        if (fabs(actualChange - changePrevious) < 5.0) { // Umbral para cambio suave
            smoothChanges++;
        }
    }

    // Si la mayoría de los cambios son suaves
    if (smoothChanges > (SIZE / 2)) {
        return true; // La onda es sinusoidal
    } else {
        return false; // La onda no es sinusoidal
    }
}