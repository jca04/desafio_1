#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x20,16,2);

void calculatePeriod(unsigned long *period, int dataOfSensor, int *actualValue);
void calculateAmplitude(int **samplingOfdata, int *amplitude);
void calculateTypeOfWave(int *samplingOfData);
bool checkSquareWave(int *data);
bool checkTriangularWave(int *data, int size);
bool checkSinusoidalWave(int *data);
void identifyWave(int *data);

unsigned const short PINSIGNAL = A0; //definicion de pines
unsigned const short PINPUSHBUTTON1 = 7;
unsigned const short PINPUSHBUTTON2 = 4;

//Recoleccion de datos, ejecucion principial del program
bool isRecolecting = false;
bool stopRecolectiong = false;
unsigned int index = 0;
unsigned int SIZE = 50;
int *samplingOfdata = new int[SIZE];
bool isAnalyzing = false;  // Bandera para el análisis

//amplitud y frecuencia
unsigned long period = 0, firstTime = 0;
float frecuency = 0.0;
bool isDecreasing = false;
int actualValue = 0;
int amplitude = 0;

// tipo de onda
float THRESHOLD = 0.2;


void setup()
{
  pinMode(PINSIGNAL, INPUT);
  pinMode(PINPUSHBUTTON1, INPUT_PULLUP);
  pinMode(PINPUSHBUTTON2, INPUT_PULLUP);
  Serial.begin(9600);
  
  for (int i = 0; i < SIZE; i++) {
  	samplingOfdata[i] = 0;
  }
  
  lcd.init();
  lcd.backlight();
  lcd.print("Bienvenido...");
  lcd.setCursor(0,1);
  lcd.print("Opte Senales...");
}


void loop()
{
  if (digitalRead(PINPUSHBUTTON1) == HIGH ){ //primer pulsador presionado
    if (!stopRecolectiong) isRecolecting = true;
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
    
    if (index < SIZE)*(samplingOfdata + index) = dataOfSensor;
    //if (index < SIZE) samplingOfdata[index] = dataOfSensor;
    else index = 0;
    index++;
    
    Serial.println(dataOfSensor);
    
    //Calcular el periodo entre dos picos
    calculatePeriod(&period ,dataOfSensor, &actualValue);
   
    double seconds = period / 1000000.0; //microsegundos a segundo
    
    if (seconds > 0.0){
      frecuency = 1.0 / seconds; // frecuencia
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(frecuency);
    }
    
  }
  
  if (stopRecolectiong){ // BLOQUE DE CODIGO CUANDO SE DETUVO LA RECOLECCION (AQUI VAN LOS ALGORITMOS)
     
    Serial.println("=================0");
    calculateAmplitude(&samplingOfdata, &amplitude);
    float vol = (amplitude * 10.0) / 1023.0;
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(vol);
    
    // Calcular tipo de onda
    
    calculateTypeOfWave(samplingOfdata);
    checkSinusoidalWave(samplingOfdata);
    identifyWave(samplingOfdata);
    delete[] samplingOfdata;
  	samplingOfdata = nullptr;
    isRecolecting = false;
    stopRecolectiong = false;
    index = 0;
    //samplingOfdata = new int[SIZE];
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
void calculateAmplitude(int **samplingOfdata, int *amplitude){

  int max = *samplingOfdata[0], min = *samplingOfdata[0];
  
  for (int i = 0; i < SIZE; i++){
    if (*(*samplingOfdata + i) > max)
    	max = *(*samplingOfdata + i);
    
    if (*(*samplingOfdata + i) < min){
      if (*(*samplingOfdata + i) != 0){
     	 min = *(*samplingOfdata + i);
      }
    }    
  }
  
  *amplitude = (max - min) / 2;
 
}


// Función para determinar el tipo de onda
void calculateTypeOfWave(int *samplingOfData) {
  bool isSquare = false, isSinusoidal = false, isTriangular = false, isUnknown = true;
	
  int size = sizeof(samplingOfData) / sizeof(samplingOfData[0]);
  if (checkSquareWave(samplingOfData)) {
    isSquare = true;
    isUnknown = false;
  } else if (checkTriangularWave(samplingOfData, size)) {
    isTriangular = true;
    isUnknown = false;
  } else if (checkSinusoidalWave(samplingOfData)) {
    isSinusoidal = true;
    isUnknown = false;
  } else {
    isUnknown = true;
  }
  
  Serial.println("triangular");
  Serial.println(isTriangular);

  // Mostrar el resultado en la pantalla LCD
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

// Función para identificar una onda cuadrada
bool checkSquareWave(int *data) {
  for (int i = 1; i < SIZE - 1; i++) {
    if (data[i] != data[i - 1] && data[i] != data[i + 1]) {
      return false;
    }
  }
  return true;
}

// Función para identificar una onda triangular
bool checkTriangularWave(int *data, int size) {
  bool isIncreasing = data[1] > data[0];  // Determinar si inicia en subida o bajada
  int previousValue = data[0];

  for (int i = 1; i < size; i++) {
    int currentChange = data[i] - previousValue;

    if (isIncreasing) {
      if (currentChange < 0) {
        // Cambió de subir a bajar
        isIncreasing = false;
      }
    } else {
      if (currentChange > 0) {
        // Cambió de bajar a subir (ya no es triangular)
        return false;
      }
    }

    // Actualizar el valor anterior para la siguiente comparación
    previousValue = data[i];
  }

  // Si terminó en una fase de bajada, entonces es una onda triangular válida
  return !isIncreasing;
}

//bool checkSinusoidalWave(int *data) {
  //unsigned int size = sizeof(data) / sizeof(data[0]);
  //return !checkSquareWave(data) && !checkTriangularWave(data, size);
//}

bool checkSinusoidalWave(int *data) {
    if (SIZE < 3) {
        return false; // No hay suficientes datos para hacer la verificación
    }

    double derivada[SIZE - 1];

    // Calcular la derivada discreta
    for (int i = 0; i < SIZE - 1; i++) {
        derivada[i] = data[i + 1] - data[i];
    }

    // Calcular el promedio y la desviación estándar de la derivada
    double sum = 0;
    double sumSq = 0;

    for (int i = 0; i < SIZE - 1; i++) {
        sum += derivada[i];
        sumSq += derivada[i] * derivada[i];
    }

    double promedio = sum / (SIZE - 1);
    double desviacion = sqrt((sumSq / (SIZE - 1)) - (promedio * promedio));

    Serial.print("Promedio de la derivada: ");
    Serial.println(promedio);
    Serial.print("Desviación estándar de la derivada: ");
    Serial.println(desviacion);

    // Verificar si la desviación estándar es cercana a cero
    if (desviacion < 5.0) {
        return true; // La onda es probable que sea sinusoidal
    } else {
        return false; // La onda no es sinusoidal
    }
}


void identifyWave(int *data) {
    if (SIZE < 2) {
        Serial.println("No hay suficientes datos para la verificación de continuidad.");
        return;
    }

    double maxDifference = 0.0;
    double threshold = 10.0; // Umbral para considerar un cambio como una discontinuidad

    for (int i = 0; i < SIZE - 1; i++) {
        double difference = abs(data[i + 1] - data[i]);
        if (difference > maxDifference) {
            maxDifference = difference;
        }
    }

    Serial.print("Diferencia máxima entre valores consecutivos: ");
    Serial.println(maxDifference);

    if (maxDifference > threshold) {
        Serial.println("La señal puede no ser continua (hay discontinuidades).");
    } else {
        Serial.println("La señal parece ser continua.");
    }
}

