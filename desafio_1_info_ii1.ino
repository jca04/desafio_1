#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x20,16,2);

void addToArrayPositions(int **array, unsigned int *size, unsigned int newSize); //definicon de funciones

unsigned const short PINSIGNAL = A0; //definicion de pines
unsigned const short PINPUSHBUTTON1 = 7;
unsigned const short PINPUSHBUTTON2 = 4;

bool isRecolecting = false; //definicion de variables y arreglo
bool stopRecolectiong = false;
unsigned int size = 0, TotalIndex = 0;
int *samplingOfdata = nullptr;

void setup()
{
  pinMode(PINSIGNAL, INPUT);
  pinMode(PINPUSHBUTTON1, INPUT);
  pinMode(PINPUSHBUTTON2, INPUT);
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
    }
  }
  
  
  if (isRecolecting && !stopRecolectiong){ // Recoleccion de datos
    const int dataOfSensor = analogRead(PINSIGNAL); // datos entregados por el generador
    Serial.println(dataOfSensor);
    
    if (TotalIndex == size){ //verificar si el tamaño del arreglo ya se lleno
      addToArrayPositions(&samplingOfdata, &size, (size == 0) ? size = 1 : size * 2); // añadir nuevas posiciones al array
    }
   
    samplingOfdata[TotalIndex] = dataOfSensor; //añadir el elemento al array
    TotalIndex++; //aumentar la posicion
  }
  
  delay(100);
  if (stopRecolectiong){ // BLOQUE DE CODIGO CUANDO SE DETUVO LA RECOLECCION (AQUI VAN LOS ALGORITMOS)
    //for (int i = 0; i < size; i++){
    //	Serial.println(samplingOfdata[i]);
    //}
    delete[] samplingOfdata;
    samplingOfdata = nullptr;
  }
}

void addToArrayPositions(int **array, unsigned int *size, unsigned int newSize){

  int *transitionArray = new int[newSize];
	
  for (int i = 0; i < *size; i++){
  	transitionArray[i] = (*array)[i];
  }
  
  delete[] *array;
  
  *array = transitionArray;
  *size = newSize;
}