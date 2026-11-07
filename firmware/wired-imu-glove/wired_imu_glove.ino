// Version 1 (wired), code 2: 11 servos, 6 potentiometers, 2 MPU-6050 IMUs (Arduino Mega).
// Source listing transcribed verbatim from the 2019 project report; comments are the original Spanish.

#include <Servo.h> // libreria servo
#include <Wire.h> //libreria comunicacion con giroscopios

//Variables para bluetooth

int LED=16;
int SW=15;
int BOTON=14;

Servo servoMuneca;
Servo servoPulgar;
Servo servoIndice;
Servo servoDedoMedio;
Servo servoAnular;
Servo servoMenique;
Servo servoXBrazo;
Servo servoYBrazo;
Servo servoXAnteBrazo;
Servo servoYAnteBrazo;
Servo servoY2Brazo;

int sensorMuneca = A0;
int sensorPulgar = A1;
int sensorIndice = A2;
int sensorDedoMedio = A3;
int sensorAnular = A4;
int sensorMenique = A5;
int sensorY2Brazo = A6;

//Direccion I2C de la IMU
#define MPU 0x68
#define MPU2 0x69
//Ratios de conversion
#define A_R 16384.0
#define G_R 131.0

//Conversion de radianes a grados 180/PI
#define RAD_A_DEG = 57.295779

//MPU-6050 da los valores en enteros de 16 bits
//Valores sin refinar
int16_t AcXBrazo, AcYBrazo, AcZBrazo, GyXBrazo, GyYBrazo, GyZBrazo;
int16_t AcXAnteBrazo, AcYAnteBrazo, AcZAnteBrazo, GyXAnteBrazo, GyYAnteBrazo, GyZAnteBrazo;
//Datos de los MPUs
float AccBrazo[2];
float GyBrazo[2];
float AngleBrazo[2];
float AccAnteBrazo[2];
float GyAnteBrazo[2];
float AngleIntermedio[2];
float AngleAnteBrazo[2];

int count0, arrayPasos, arrayMax, countDelay, botonEstado, pasosMax, pasos, time = 1000, del = 1000, temp;
unsigned int  diferenciaTiempo = 0;

//arrayPasos=memoria donde guardaran los pasos en una matriz
//arrayMax=Maximo numero de pasos que pueden ser guardados en la memoria
//countDelay=variable donde metemos el tiempo de retraso entre los movimientos completos
//botonEstado=el boton;
//pasos=pasos individuales para el movimiento de los motores guardado como posiciones en el tiempo
//diferenciaTiempo=tiempo obtenido en los calculos

//tiempo y variables de tiempo
long millisPreviosSistema = 0;
long millisPreviosBoton2 = 0;
long millisPreviosBoton = 0;
long microsPrevios = 0;
unsigned long millisActual = millis();
unsigned long microsActual = micros();

//matrices
int Delay[7] = {0,0,1,3,15,60,300}; //matriz para 'mapear' los retrasos a usar
int SensVal[7]; //sensores(las resistencias variables) a usar
float posicionDiferencia[11], posicionMomentanea[11], siguientePosicion[11],  microPaso[11]; //diferencia entre posiciones almacenadas y la posicion momentanea o intermedia
int joint0[180]; //matrices para los servos
int joint1[180];
int joint2[180];
int joint3[180];
int joint4[180];
int joint5[180];
int joint6[180];
int joint7[180];
int joint8[180];
int joint9[180];
int joint10[180];

int top = 179; //maximo angulo posible, no debemos pasarnos de esto para evitar un mal funcionamiento

//estados
boolean playmode = false, Step = false;

void setup()
{
  pinMode(BOTON, INPUT);  //establecer los pines de entrada en el arduino
  pinMode(SW, INPUT);
  pinMode(LED, OUTPUT);  //establecer la salida digital en el arduino para el led
  digitalWrite(LED, HIGH);   //encender el led de la salida digital para indicar el encendido del dispositivo y que se estan ejecutanto las definiciones
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);Wire.begin();

  Wire.begin();
  Wire.beginTransmission(MPU2);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);

  servoMuneca.attach(3);  //establecer las salidas digitales para los servomotores
  servoPulgar.attach(4);
  servoIndice.attach(5);
  servoDedoMedio.attach(6);
  servoAnular.attach(7);
  servoMenique.attach(8);
  servoXAnteBrazo.attach(9);
  servoYAnteBrazo.attach(10);
  servoXBrazo.attach(11);
  servoYBrazo.attach(12);
  servoY2Brazo.attach(13);
  Serial.begin(9600);

  Serial.println("mini brazo robot listo...");
  delay(1000);
  digitalWrite(LED, LOW); //indicar con el led que el dispositivo esta listo para su uso
}

void loop() //void en bucle del sistema,
{
  millisActual = millis(); //iniciar el conteo del tiempo en milisegundos y microsegundos
  microsActual = micros();

  // leer el boton para decidir el modo a ejecutar
  Button();

  if(!playmode) //si la variable playmode es falsa, iniciar el modo manual
  {
    if(millisActual - millisPreviosSistema > 25) //esperar unos 25 milisegundos hasta la proxima actualizacion del modo manual, ahorrando memoria
    {
      if (arrayPasos < top)
      {
        millisPreviosSistema = millisActual; //guardar el ultimo cambio que se realizo, reseteando el loop
        readPot(); //obtener los valores de los sensores/potenciometros
        mapping();//mapear los milisegundos para los servomotores
        move_servo(); //mover los servos a su posicion actual

      } //terminar el conteo cuando llegue al maximo
    } // terminar la verificacion de los pasos
  } // termianr el movimiento manual, pasar el movimiento automatico

  else if(playmode) // iniciar el movimiento automatico
  {
    if (Step) //los siguientes pasos seran leidos desded la matriz
    {
      digitalWrite(LED, HIGH); //encender led indicando que el modo automatico se ha iniciado
      if (arrayPasos < arrayMax) //comprobar que no llegamos al limite de la memoria del dispositivo
      {
        arrayPasos += 1; //pasar a la siguiente posicion de la matriz
        Read(); //leer la matriz
        calculate(); //calcular la mayor diferencia entre posicion anterior y posicion actual, ademas de calcular las otras y finalizar los mas pequeños
        Step = 0;
        digitalWrite(LED, LOW);
      }
      else // si la lectura de la matriz finaliza, comenzar de nuevo
      {
        arrayPasos = 0; //
        calc_pause(); // calcular pausa entre movimientos leidos de los potenciometros
        countDelay = 0; //inciar conteo de nuevo
        while(countDelay < diferenciaTiempo)// mientras el contador no llegue al tope...
        {//nos mantenemos en un bucle hasta que comience de nuevo
          countDelay += 1;
          calc_pause();
          /*digitalWrite(LED, HIGH); delay(25);
          digitalWrite(LED, LOW); delay(975);*/
        }
      }
      //Serial.println(arrayPasos);
    }
    else // realizar los movimientos de los servos
    {
      if (microsActual - microsPrevios > time) //ejecutar los pasos guardados separados por los microsegundos calculados
      {
        microsPrevios = microsActual;
        play_servo();
      }
    }
  }// terminar el modo automatico

// ---------------------------------------------------------------------------------Hardware:Switch para pausar sistema
    while (digitalRead(SW) == true)
      {
        digitalWrite(LED, HIGH); delay(500);
        digitalWrite(LED, LOW); delay(500);
      }
// ---------------------------------------------------------------------------------- prueba en monitor serial
    /*if(millisActual - millisPreviosBoton2 > 5000)
    {
      millisPreviosBoton2 = millisActual;
      /*count0 = 0;
      while(count0 < 4)
      {
        int val = SensVal[count0];
      // val = map(val, 142, 888, 0, 180);
        Serial.println(val);
        //Serial.println("test");
        count0 += 1;
      }
      Serial.println(playmode);
      Serial.println(arrayPasos);
      Serial.println(arrayMax);
      Serial.println(" ");
    }*/
}

// ---------------------------------------------------------------------------------------- voids extras usados
void calc_pause()//leer los potenciometros y mapear los tiempos de delay despues de realizar cada movimiento
{
    readPot();
    temp = SensVal[3];
    if (temp < 0) temp = 0;
    temp = map(temp, 0, 680, 0 ,6);
    diferenciaTiempo = Delay[temp];
    //prueba en monitor serial
    /*Serial.print(temp);
          Serial.print(" ");
          Serial.print(diferenciaTiempo);
          Serial.print(" ");
          Serial.println(countDelay);*/
}

void readPot() //leer las entradas analogicas y sumarles algunas compensaciones para correcciones mecanicas
{
   SensVal[0] = analogRead(sensorMuneca); SensVal[0] += -10; // rotor de base y
   SensVal[1] = analogRead(sensorPulgar); SensVal[1] += 280; // hombro
   SensVal[2] = analogRead(sensorIndice); SensVal[2] += -50; // muñeca y
   SensVal[3] = analogRead(sensorDedoMedio); // SensVal[3] += 0;// pinza
   SensVal[4] = analogRead(sensorAnular); // SensVal[3] += 0;// rotor de base x
   SensVal[5] = analogRead(sensorMenique); // SensVal[3] += 0;// muñeca x
   SensVal[5] = analogRead(sensorY2Brazo);
   //Serial.print(SensVal[2]);Serial.print(" "); // prueba en el monitor serial

   //Leer los valores del Acelerometro de la IMU-Brazo
   Wire.beginTransmission(MPU);
   Wire.write(0x3B); //Pedir el registro 0x3B - corresponde al AcXBrazo
   Wire.endTransmission(false);
   Wire.requestFrom(MPU,6,true); //A partir del 0x3B, se piden 6 registros
   AcXBrazo=Wire.read()<<8|Wire.read(); //Cada valor ocupa 2 registros
   AcYBrazo=Wire.read()<<8|Wire.read();
   AcZBrazo=Wire.read()<<8|Wire.read();

   //Se calculan los angulos Y, X respectivamente de IMU-Brazo
   AccBrazo[1] = atan(-1*(AcXBrazo/A_R)/sqrt(pow((AcYBrazo/A_R),2) + pow((AcZBrazo/A_R),2)))*RAD_TO_DEG;
   AccBrazo[0] = atan((AcYBrazo/A_R)/sqrt(pow((AcXBrazo/A_R),2) + pow((AcZBrazo/A_R),2)))*RAD_TO_DEG;

    //Leer los valores del Acelerometro de la IMU-AnteBrazo
    Wire.beginTransmission(MPU2);
    Wire.write(0x3B); //Pedir el registro 0x3B - corresponde al AcXBrazo
    Wire.endTransmission(false);
    Wire.requestFrom(MPU2,6,true); //A partir del 0x3B, se piden 6 registros
    AcXAnteBrazo=Wire.read()<<8|Wire.read(); //Cada valor ocupa 2 registros
    AcYAnteBrazo=Wire.read()<<8|Wire.read();
    AcZAnteBrazo=Wire.read()<<8|Wire.read();

    //Se calculan los angulos Y, X respectivamente.(segundo IMU)
    AccAnteBrazo[1] = atan(-1*(AcXAnteBrazo/A_R)/sqrt(pow((AcYAnteBrazo/A_R),2) + pow((AcZAnteBrazo/A_R),2)))*RAD_TO_DEG;
    AccAnteBrazo[0] = atan((AcYAnteBrazo/A_R)/sqrt(pow((AcXAnteBrazo/A_R),2) + pow((AcZAnteBrazo/A_R),2)))*RAD_TO_DEG;

    //Leer los valores del Giroscopio
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU,4,true); //A diferencia del Acelerometro, solo se piden 4 registros
    GyXBrazo=Wire.read()<<8|Wire.read();
    GyYBrazo=Wire.read()<<8|Wire.read();

    //Calculo del angulo del Giroscopio
    GyBrazo[0] = GyXBrazo/G_R;
    GyBrazo[1] = GyYBrazo/G_R;

    //Aplicar el Filtro Complementario
    AngleBrazo[0] = 0.98 *(AngleBrazo[0]+GyBrazo[0]*0.010) + 0.02*AccBrazo[0];
    AngleBrazo[1] = 0.98 *(AngleBrazo[1]+GyBrazo[1]*0.010) + 0.02*AccBrazo[1];

    Wire.beginTransmission(MPU2);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU2,4,true); //A diferencia del Acelerometro, solo se piden 4 registros
    GyXAnteBrazo=Wire.read()<<8|Wire.read();
    GyYAnteBrazo=Wire.read()<<8|Wire.read();

    //Calculo del angulo del Giroscopio
    GyAnteBrazo[0] = GyXAnteBrazo/G_R;
    GyAnteBrazo[1] = GyYAnteBrazo/G_R;

    //Aplicar el Filtro Complementario
    AngleIntermedio[0] =( 0.98 *(AngleIntermedio[0]+GyAnteBrazo[0]*0.010) + 0.02*AccAnteBrazo[0]);
    AngleIntermedio[1] =( 0.98 *(AngleIntermedio[1]+GyAnteBrazo[1]*0.010) + 0.02*AccAnteBrazo[1]);

    //Calcular Angulo del AnteBrazo
    AngleAnteBrazo[0]= AngleIntermedio[0]-AngleBrazo[0];
    AngleAnteBrazo[1]= AngleIntermedio[1]-AngleBrazo[1];

}

void mapping()//calcular los microsegundos para los movimientos de los servomotores dados por los potenciometros
{
  posicionMomentanea[0] = map(SensVal[0], 150, 900, 600, 2400);
  posicionMomentanea[1] = map(SensVal[1], 1000, 100, 550, 2400);
  posicionMomentanea[2] = map(SensVal[2], 120, 860, 400, 2500);
  posicionMomentanea[3] = map(SensVal[3], 1023, 0, 500, 2500);
  posicionMomentanea[4] = map(SensVal[4], 150, 900, 600, 2400);
  posicionMomentanea[5] = map(SensVal[5], 120, 860, 400, 2500);
  posicionMomentanea[6]=map(AngleBrazo[0],-90,90,500,2500);
  posicionMomentanea[7]=map(AngleBrazo[1],-90,90,500,2500);
  posicionMomentanea[8]=map(AngleIntermedio[0],-90,90,500,2500);
  posicionMomentanea[9]=map(AngleIntermedio[1],-90,90,500,2500);
  posicionMomentanea[10] = map(SensVal[5], 120, 860, 400, 2500);
  //Serial.println(posicionMomentanea[2]); // prueba en monitor serial
}
void record()
{
    joint0[arrayPasos] = posicionMomentanea[0]; //ingresar las posiciones de la matriz a los servomotores
    joint1[arrayPasos] = posicionMomentanea[1];
    joint2[arrayPasos] = posicionMomentanea[2];
    joint3[arrayPasos] = posicionMomentanea[3];
    joint4[arrayPasos] = posicionMomentanea[4];
    joint5[arrayPasos] = posicionMomentanea[5];
    joint6[arrayPasos] = posicionMomentanea[6];
    joint7[arrayPasos] = posicionMomentanea[7];
    joint8[arrayPasos] = posicionMomentanea[8];
    joint9[arrayPasos] = posicionMomentanea[9];
    joint10[arrayPasos] = posicionMomentanea[10];
}
void Read()
{
    siguientePosicion[0] = joint0[arrayPasos]; //leer los valores de la matriz e ingresarlos
    siguientePosicion[1] = joint1[arrayPasos];
    siguientePosicion[2] = joint2[arrayPasos];
    siguientePosicion[3] = joint3[arrayPasos];
    siguientePosicion[4] = joint4[arrayPasos];
    siguientePosicion[5] = joint5[arrayPasos];
    siguientePosicion[6] = joint6[arrayPasos];
    siguientePosicion[7] = joint7[arrayPasos];
    siguientePosicion[8] = joint8[arrayPasos];
    siguientePosicion[9] = joint9[arrayPasos];
    siguientePosicion[10] = joint10[arrayPasos];
}
void move_servo()
{                         //enviar los milisegundos de movimiento calculados para los servos
  servoMuneca.writeMicroseconds(posicionMomentanea[0]);
  servoPulgar.writeMicroseconds(posicionMomentanea[1]);
  servoIndice.writeMicroseconds(posicionMomentanea[2]);
  servoDedoMedio.writeMicroseconds(posicionMomentanea[3]);
  servoAnular.writeMicroseconds(posicionMomentanea[4]);
  servoMenique.writeMicroseconds(posicionMomentanea[5]);
  servoXBrazo.writeMicroseconds(posicionMomentanea[6]);
  servoYBrazo.writeMicroseconds(posicionMomentanea[7]);
  servoXAnteBrazo.writeMicroseconds(posicionMomentanea[8]);
  servoYAnteBrazo.writeMicroseconds(posicionMomentanea[9]);
  servoY2Brazo.writeMicroseconds(posicionMomentanea[10]);
/*
  Serial.print("Muneca: "); Serial.print(posicionMomentanea[0]); Serial.print("\t");
  Serial.print("Pulgar: "); Serial.print(posicionMomentanea[1]); Serial.print("\t");
  Serial.print("Indice: "); Serial.print(posicionMomentanea[0]); Serial.print("\t");
  Serial.print("Dedo Medio: "); Serial.print(posicionMomentanea[0]); Serial.print("\t");
  Serial.print("Anular: "); Serial.print(posicionMomentanea[0]); Serial.print("\t");
  Serial.print("Menique: "); Serial.print(posicionMomentanea[0]); Serial.print("\t");
  Serial.print("Brazo X: "); Serial.print(posicionMomentanea[0]); Serial.print("\t");
  Serial.print("Y: "); Serial.print(posicionMomentanea[0]); Serial.print("\t");
  Serial.print("Ante Brazo X: "); Serial.print(posicionMomentanea[0]); Serial.print("\t");
  Serial.print("Y: "); Serial.println(posicionMomentanea[0]);
*/
/*
  Serial.print("Angle X1: "); Serial.print(AngleBrazo[0]); Serial.print("\t");
  Serial.print("Angle Y1: "); Serial.print(AngleBrazo[1]); Serial.print("\t-------\t");

  Serial.print("Angle X2: "); Serial.print(AngleIntermedio[0]); Serial.print("\t");
  Serial.print("Angle Y2: "); Serial.print(AngleIntermedio[1]); Serial.print("\t-------\t");

  Serial.print("Angle Xdif: "); Serial.print(AngleAnteBrazo[0]); Serial.print("\t");
  Serial.print("Angle Ydif: "); Serial.print(AngleAnteBrazo[1]); Serial.print("\t");

  Serial.print("||\tPos X1: "); Serial.print(posXBrazo); Serial.print("\t");
  Serial.print("Pos Y1: "); Serial.print(posYBrazo); Serial.print("\t-------\t");

  Serial.print("Pos X2: "); Serial.print(posXAnteBrazo); Serial.print("\t");
  Serial.print("Pos Y2: "); Serial.print(posYAnteBrazo); Serial.print("\n");
  */
}

// ------------------------------------------------------------ calcular los pasos individuales
void calculate()
{
      // distancia de viaje para servomotores
      posicionDiferencia[0] = abs(posicionMomentanea[0]-siguientePosicion[0]); //abs= calcular valor absoluto de un numero, o sea; mayor a cero, menor o igual
      posicionDiferencia[1] = abs(posicionMomentanea[1]-siguientePosicion[1]);
      posicionDiferencia[2] = abs(posicionMomentanea[2]-siguientePosicion[2]);
      posicionDiferencia[3] = abs(posicionMomentanea[3]-siguientePosicion[3]);
      posicionDiferencia[4] = abs(posicionMomentanea[4]-siguientePosicion[4]);
      posicionDiferencia[5] = abs(posicionMomentanea[5]-siguientePosicion[5]);
      posicionDiferencia[6] = abs(posicionMomentanea[6]-siguientePosicion[6]);
      posicionDiferencia[7] = abs(posicionMomentanea[7]-siguientePosicion[7]);
      posicionDiferencia[8] = abs(posicionMomentanea[8]-siguientePosicion[8]);
      posicionDiferencia[9] = abs(posicionMomentanea[9]-siguientePosicion[9]);
      posicionDiferencia[10] = abs(posicionMomentanea[10]-siguientePosicion[10]);

      // verificar el mayor de los viajes entre todos los servos
      pasosMax = max(posicionDiferencia[0],posicionDiferencia[1]);
      pasosMax = max(pasosMax,posicionDiferencia[2]);
      pasosMax = max(pasosMax,posicionDiferencia[3]);
      pasosMax = max(pasosMax,posicionDiferencia[4]);
      pasosMax = max(pasosMax,posicionDiferencia[5]);
      pasosMax = max(pasosMax,posicionDiferencia[6]);
      pasosMax = max(pasosMax,posicionDiferencia[7]);
      pasosMax = max(pasosMax,posicionDiferencia[8]);
      pasosMax = max(pasosMax,posicionDiferencia[9]);
      pasosMax = max(pasosMax,posicionDiferencia[10]);
      //pasosMax es la mayor distancia que un servo debera viajar entre su posiciones actual y la siguiente, dada por el movimiento de los potenciometros en funcion del tiempo
      //Serial.println(pasosMax); //prueba en monitor serial

      if (pasosMax < 500) // retraso entre los movimientos mas grandes y los pequeños para que se vea mas fluido el movimeinto
        del = 1200;
      else
        del = 600;

       // calcular los pasos individuales para cada servo
       //esto se hace para que todos los servos se muevan en bucle y con sus diferentes valores aun con los tiempos dispares
       // esto hace que todos los servos realicen viajes con diferentes distancias al mismo tiempo
      if (siguientePosicion[0] < posicionMomentanea[0]) microPaso[0] = 0-posicionDiferencia[0]/pasosMax; else microPaso[0] = posicionDiferencia[0]/pasosMax;
      if (siguientePosicion[1] < posicionMomentanea[1]) microPaso[1] = 0-posicionDiferencia[1]/pasosMax; else microPaso[1] = posicionDiferencia[1]/pasosMax;
      if (siguientePosicion[2] < posicionMomentanea[2]) microPaso[2] = 0-posicionDiferencia[2]/pasosMax; else microPaso[2] = posicionDiferencia[2]/pasosMax;
      if (siguientePosicion[3] < posicionMomentanea[3]) microPaso[3] = 0-posicionDiferencia[3]/pasosMax; else microPaso[3] = posicionDiferencia[3]/pasosMax;
      if (siguientePosicion[4] < posicionMomentanea[4]) microPaso[4] = 0-posicionDiferencia[4]/pasosMax; else microPaso[4] = posicionDiferencia[4]/pasosMax;
      if (siguientePosicion[5] < posicionMomentanea[5]) microPaso[5] = 0-posicionDiferencia[5]/pasosMax; else microPaso[5] = posicionDiferencia[5]/pasosMax;
      if (siguientePosicion[6] < posicionMomentanea[6]) microPaso[6] = 0-posicionDiferencia[6]/pasosMax; else microPaso[6] = posicionDiferencia[6]/pasosMax;
      if (siguientePosicion[7] < posicionMomentanea[7]) microPaso[7] = 0-posicionDiferencia[7]/pasosMax; else microPaso[7] = posicionDiferencia[7]/pasosMax;
      if (siguientePosicion[8] < posicionMomentanea[8]) microPaso[8] = 0-posicionDiferencia[8]/pasosMax; else microPaso[8] = posicionDiferencia[8]/pasosMax;
      if (siguientePosicion[9] < posicionMomentanea[9]) microPaso[9] = 0-posicionDiferencia[9]/pasosMax; else microPaso[9] = posicionDiferencia[9]/pasosMax;
      if (siguientePosicion[10] < posicionMomentanea[9]) microPaso[10] = 0-posicionDiferencia[10]/pasosMax; else microPaso[10] = posicionDiferencia[10]/pasosMax;
        //Serial.println(dir4); //prueba en el monitor serial

}
void play_servo()
{
    pasos += 1;
    if (pasos < pasosMax)//asegurar que no llegamos al limite de movimientos
    {
      //time = del*5;
      if(pasos == 20) time = del*4;          // macimo tiempo
      else if(pasos == 40) time = del*3;     //tiempo es el delay en microsegunos que esperamos hasta el loop principal
      else if(pasos == 80) time = del*2;     //cuanto hacemos un micro paso, o sea un movimiento pequeño
      else if(pasos == 100) time = del-1;
      if(pasos == pasosMax-200) time = del*2;        // detener lectura de vector hacia atras, añadiendo 200 microsegundos antes del tiempo final)
      else if(pasos == pasosMax-80) time = del*3;
      else if(pasos == pasosMax-40) time = del*4;
      else if(pasos == pasosMax-20) time = del*5;

      posicionMomentanea[0] += microPaso[0]; // establecer nuevas posiciones
      posicionMomentanea[1] += microPaso[1];
      posicionMomentanea[2] += microPaso[2];
      posicionMomentanea[3] += microPaso[3];
      posicionMomentanea[4] += microPaso[4];
      posicionMomentanea[5] += microPaso[5];
      posicionMomentanea[6] += microPaso[6];
      posicionMomentanea[7] += microPaso[7];
      posicionMomentanea[8] += microPaso[8];
      posicionMomentanea[9] += microPaso[9];
      posicionMomentanea[10] += microPaso[10];

      move_servo();
    }
    else
    {
      Step = 1; // siguiente paso leido en la matriz
      pasos = 0; // pasos intermedios de la matriz
    }
}

void data_out() // escribir los datos grabados en el monitor serial
{
  int i = 0;
  while(i < arrayMax)
  {
    digitalWrite(LED, HIGH);
    i += 1;
    //Serial.print(joint0[i]); Serial.print(", ");
  }
  //Serial.println("Joint0");
  i = 0;
  while(i < arrayMax)
  {
    digitalWrite(LED, HIGH);
    i += 1;
    //Serial.print(joint1[i]); Serial.print(", ");
  }
  //Serial.println("Joint1");
  i = 0;
  while(i < arrayMax)
  {
    digitalWrite(LED, HIGH);
    i += 1;
    //Serial.print(joint2[i]); Serial.print(", ");
  }
  //Serial.println("Joint2");
  i = 0;
  while(i < arrayMax)
  {
    digitalWrite(LED, HIGH);
    i += 1;
    //Serial.print(joint3[i]); Serial.print(", ");
  }
  //Serial.println("Joint3");
}

void Button() //checkear boton tanto para una como doble pulsacion
{
  if (digitalRead(BOTON) == false)
  {
    delay(20);
    if (digitalRead(BOTON) == true) // boton es soltado
    {
      if (botonEstado == 0)
      {
        botonEstado = 1;
        millisPreviosBoton = millisActual;
        //Serial.print("Status Record "); Serial.println(botonEstado); //prueba en monitor serial
      }
      else if ((botonEstado == 1) && (millisActual - millisPreviosBoton < 250))
      {
        botonEstado = 2;
        //Serial.println(botonEstado);
      }
      /*else if ((botonEstado == 2) && (millisActual - millisPreviosBoton < 500))
      {
        botonEstado = 3;
        Serial.println(botonEstado);
      }*/ //prueba en monitor serial
    }
  }

    if ((botonEstado == 1) && (millisActual - millisPreviosBoton > 1000)) // escribir la matriz
    {
      arrayPasos += 1;
      arrayMax = arrayPasos;
      record();
      botonEstado = 0;
      playmode = false;
      //Serial.print("Grabar paso: "); Serial.println(arrayPasos);
      /*digitalWrite(LED, HIGH);
      delay(100);
      digitalWrite(LED, LOW);*/
    }
    else if (botonEstado == 2)
    {
      arrayPasos = 0;
      playmode = true;
      botonEstado = 0;
      Step = 1;
      //Serial.println("Modo Automatico ");
      data_out();
      /*delay(250);
      digitalWrite(LED, LOW); */
    }
    /*if (botonEstado == 3)
    {
      // ++ arrayPasos
      // playmode = 1;
      botonEstado = 0;
      Serial.println("Clear ");
    }*/ //prueba en el monitor serial
    if (millisActual - millisPreviosBoton > 2000) // limpiar estados del boton
    {
      botonEstado = 0;
      //Serial.println("restart "); //indicacion en el monitor serial
    }
}
