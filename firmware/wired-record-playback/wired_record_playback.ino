// Version 1 (wired), code 1: 4 servos, 4 potentiometers, 1 button, 1 switch. Record and replay.
// Source listing transcribed verbatim from the 2019 project report; comments are the original Spanish.

#include <Servo.h> // libreria servo

Servo servo_0;
Servo servo_1;
Servo servo_2;
Servo servo_3;

int sensorPin0 = A0;    // base rotante
int sensorPin1 = A1;    // mano
int sensorPin2 = A2;    // codo
int sensorPin3 = A3;    // pinza

int count0, arrayStep, arrayMax, countverz, Taster, stepsMax, steps, time = 1000, del = 1000, temp;
unsigned int  verz = 0;

//arrayStep=memoria donde guardaran los pasos en una matriz
//arrayMax=Maximo numero de pasos que pueden ser guardados en la memoria
//countverz=variable donde metemos el tiempo de retrazso entre los movimientos completos
//taster=el boton;
//steps=pasos individuales para el movimiento de los motores guardado como posiciones en el tiempo
//verz=tiempo obtenido en los calculos

//tiempo y variables de tiempo
long previousMillis1 = 0;
long previousMillis2 = 0;
long previousMillis3 = 0;
long previousMillis4 = 0;
long previousMicros = 0;
unsigned long currentMillis = millis();
unsigned long currentMicros = micros();

//matrices
int Delay[7] = {0,0,1,3,15,60,300}; //matriz para 'mapear' los retrasos a usar
int SensVal[4]; //sensores(las resistencias variables) a usar
float dif[4], ist[4], sol[4],  dir[4]; //diferencia entre posiciones almacenadas y la posicion momentanea o intermedia
int joint0[180]; //matrices para los servos
int joint1[180];
int joint2[180];
int joint3[180];
int top = 179; //maximo angulo posible, no debemos pasarnos de esto para evitar un mal funcionamiento

//estados
boolean playmode = false, Step = false;

void setup()
{
  pinMode(4, INPUT);  //establecer los pines de entrada en el arduino
  pinMode(6, INPUT);
  pinMode(13, OUTPUT);  //establecer la salida digital en el arduino para el led
  digitalWrite(13, HIGH);   //encender el led de la salida digital para indicar el encendido del dispositivo y que se estan ejecutanto las definiciones
  servo_0.attach(3);  //establecer las salidas digitales para los servomotores
  servo_1.attach(5);
  servo_2.attach(10);
  servo_3.attach(11);
  Serial.begin(115200);
  Serial.println("mini brazo robot listo...");
  //delay(1000);
  digitalWrite(13, LOW); //indicar con el led que el dispositivo esta listo para su uso
}

void loop() //void en bucle del sistema,
{
  currentMillis = millis(); //iniciar el conteo del tiempo en milisegundos y microsegundos
  currentMicros = micros();

  // leer el boton para decidir el modo a ejecutar
  Button();

  if(!playmode) //si la variable playmode es falsa, iniciar el modo manual
  {
    if(currentMillis - previousMillis1 > 25) //esperar unos 25 milisegundos hasta la proxima actualizacion del modo manual, ahorrando memoria
    {
      if (arrayStep < top)
      {
        previousMillis1 = currentMillis; //guardar el ultimo cambio que se realizo, reseteando el loop
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
      digitalWrite(13, HIGH); //encender led indicando que el modo automatico se ha iniciado
      if (arrayStep < arrayMax) //comprobar que no llegamos al limite de la memoria del dispositivo
      {
        arrayStep += 1; //pasar a la siguiente posicion de la matriz
        Read(); //leer la matriz
        calculate(); //calcular la mayor diferencia entre posicion anterior y posicion actual, ademas de calcular las otras y finalizar los mas pequeños
        Step = 0;
        digitalWrite(13, LOW);
      }
      else // si la lectura de la matriz finaliza, comenzar de nuevo
      {
        arrayStep = 0; //
        calc_pause(); // calcular pausa entre movimientos leidos de los potenciometros
        countverz = 0; //inciar conteo de nuevo
        while(countverz < verz)// mientras el contador no llegue al tope...
        {//nos mantenemos en un bucle hasta que comience de nuevo
          countverz += 1;
          calc_pause();
          digitalWrite(13, HIGH); delay(25);
          digitalWrite(13, LOW); delay(975);
        }
      }
      //Serial.println(arrayStep);
    }
    else // realizar los movimientos de los servos
    {
      if (currentMicros - previousMicros > time) //ejecutar los pasos guardados separados por los microsegundos calculados
      {
        previousMicros = currentMicros;
        play_servo();
      }
    }
  }// terminar el modo automatico

// ---------------------------------------------------------------------------------Hardware:Switch para pausar sistema
    while (digitalRead(4) == true)
      {
        digitalWrite(13, HIGH); delay(500);
        digitalWrite(13, LOW); delay(500);
      }
// ---------------------------------------------------------------------------------- prueba en monitor serial
    /*if(currentMillis - previousMillis2 > 5000)
    {
      previousMillis2 = currentMillis;
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
      Serial.println(arrayStep);
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
    verz = Delay[temp];
    //prueba en monitor serial
    /*Serial.print(temp);
          Serial.print(" ");
          Serial.print(verz);
          Serial.print(" ");
          Serial.println(countverz);*/
}

void readPot() //leer las entradas analogicas y sumarles algunas compensaciones para correcciones mecanicas
{
   SensVal[0] = analogRead(sensorPin0); SensVal[0] += -10; // rotor de base
   SensVal[1] = analogRead(sensorPin1); SensVal[1] += 280; // hombro
   SensVal[2] = analogRead(sensorPin2); SensVal[2] += -50; // mano
   SensVal[3] = analogRead(sensorPin3); // SensVal[3] += 0;// pinza
   //Serial.print(SensVal[2]);Serial.print(" "); // prueba en el monitor serial
}

void mapping()//calcular los microsegundos para los movimientos de los servomotores dados por los potenciometros
{
  ist[0] = map(SensVal[0], 150, 900, 600, 2400);// para giro del rotor
  ist[1] = map(SensVal[1], 1000, 100, 550, 2400);// para el giro del hombro
  ist[2] = map(SensVal[2], 120, 860, 400, 2500);// para el giro la mano
  ist[3] = map(SensVal[3], 1023, 0, 500, 2500);// para el giro de la pinza
  //Serial.println(ist[2]); // prueba en monitor serial
}
void record()
{
    joint0[arrayStep] = ist[0]; //ingresar las posiciones de la matriz a los servomotores
    joint1[arrayStep] = ist[1];
    joint2[arrayStep] = ist[2];
    joint3[arrayStep] = ist[3];
}
void Read()
{
    sol[0] = joint0[arrayStep]; //leer los valores de la matriz e ingresarlos
    sol[1] = joint1[arrayStep];
    sol[2] = joint2[arrayStep];
    sol[3] = joint3[arrayStep];
}
void move_servo()
{                         //enviar los milisegundos de movimiento calculados para los servos
  servo_0.writeMicroseconds(ist[3]); // pinza
  servo_1.writeMicroseconds(ist[2]); // mano
  servo_2.writeMicroseconds(ist[0]); // hombro
  servo_3.writeMicroseconds(ist[1]); // rotor
}

// ------------------------------------------------------------ calcular los pasos individuales
void calculate()
{
      // distancia de viaje para servomotores
      dif[0] = abs(ist[0]-sol[0]); //abs= calcular valor absoluto de un numero, o sea; mayor a cero, menor o igual
      dif[1] = abs(ist[1]-sol[1]);
      dif[2] = abs(ist[2]-sol[2]);
      dif[3] = abs(ist[3]-sol[3]);

      // verificar el mayor de los viajes entre todos los servos
      stepsMax = max(dif[0],dif[1]);
      stepsMax = max(stepsMax,dif[2]);
      stepsMax = max(stepsMax,dif[3]);
      //stepsMax es la mayor distancia que un servo debera viajar entre su posiciones actual y la siguiente, dada por el movimiento de los potenciometros en funcion del tiempo
      //Serial.println(stepsMax); //prueba en monitor serial

      if (stepsMax < 500) // retraso entre los movimientos mas grandes y los pequeños para que se vea mas fluido el movimeinto
        del = 1200;
      else
        del = 600;

       // calcular los pasos individuales para cada servo
       //esto se hace para que todos los servos se muevan en bucle y con sus diferentes valores aun con los tiempos dispares
       // esto hace que todos los servos realicen viajes con diferentes distancias al mismo tiempo
      if (sol[0] < ist[0]) dir[0] = 0-dif[0]/stepsMax; else dir[0] = dif[0]/stepsMax;
      if (sol[1] < ist[1]) dir[1] = 0-dif[1]/stepsMax; else dir[1] = dif[1]/stepsMax;
      if (sol[2] < ist[2]) dir[2] = 0-dif[2]/stepsMax; else dir[2] = dif[2]/stepsMax;
      if (sol[3] < ist[3]) dir[3] = 0-dif[3]/stepsMax; else dir[3] = dif[3]/stepsMax;
        //Serial.println(dir4); //prueba en el monitor serial

}
void play_servo()
{
    steps += 1;
    if (steps < stepsMax)//asegurar que no llegamos al limite de movimientos
    {
      //time = del*5;
      if(steps == 20) time = del*4;          // macimo tiempo
      else if(steps == 40) time = del*3;     //tiempo es el delay en microsegunos que esperamos hasta el loop principal
      else if(steps == 80) time = del*2;     //cuanto hacemos un micro paso, o sea un movimiento pequeño
      else if(steps == 100) time = del-1;
      if(steps == stepsMax-200) time = del*2;        // detener lectura de vector hacia atras, añadiendo 200 microsegundos antes del tiempo final)
      else if(steps == stepsMax-80) time = del*3;
      else if(steps == stepsMax-40) time = del*4;
      else if(steps == stepsMax-20) time = del*5;

      ist[0] += dir[0]; // establecer nuevas posiciones
      ist[1] += dir[1];
      ist[2] += dir[2];
      ist[3] += dir[3];

      servo_0.writeMicroseconds(ist[3]); // lectura de los servos en microsegundos, esta es la tena
      servo_1.writeMicroseconds(ist[2]); // mano
      servo_2.writeMicroseconds(ist[0]); // homnbro
      servo_3.writeMicroseconds(ist[1]); // rotor
    }
    else
    {
      Step = 1; // siguiente paso leido en la matriz
      steps = 0; // pasos intermedios de la matriz
    }
}

void data_out() // escribir los datos grabados en el monitor serial
{
  int i = 0;
  while(i < arrayMax)
  {
    digitalWrite(13, HIGH);
    i += 1;
    Serial.print(joint0[i]); Serial.print(", ");
  }
  Serial.println("Joint0");
  i = 0;
  while(i < arrayMax)
  {
    digitalWrite(13, HIGH);
    i += 1;
    Serial.print(joint1[i]); Serial.print(", ");
  }
  Serial.println("Joint1");
  i = 0;
  while(i < arrayMax)
  {
    digitalWrite(13, HIGH);
    i += 1;
    Serial.print(joint2[i]); Serial.print(", ");
  }
  Serial.println("Joint2");
  i = 0;
  while(i < arrayMax)
  {
    digitalWrite(13, HIGH);
    i += 1;
    Serial.print(joint3[i]); Serial.print(", ");
  }
  Serial.println("Joint3");
}

void Button() //checkear boton tanto para una como doble pulsacion
{
  if (digitalRead(6) == false)
  {
    delay(20);
    if (digitalRead(6) == true) // boton es soltado
    {
      if (Taster == 0)
      {
        Taster = 1;
        previousMillis3 = currentMillis;
        //Serial.print("Status Record "); Serial.println(Taster); //prueba en monitor serial
      }
      else if ((Taster == 1) && (currentMillis - previousMillis3 < 250))
      {
        Taster = 2;
        //Serial.println(Taster);
      }
      /*else if ((Taster == 2) && (currentMillis - previousMillis3 < 500))
      {
        Taster = 3;
        Serial.println(Taster);
      }*/ //prueba en monitor serial
    }
  }

    if ((Taster == 1) && (currentMillis - previousMillis3 > 1000)) // escribir la matriz
    {
      arrayStep += 1;
      arrayMax = arrayStep;
      record();
      Taster = 0;
      playmode = false;
      Serial.print("Grabar paso: "); Serial.println(arrayStep);
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
    }
    else if (Taster == 2)
    {
      arrayStep = 0;
      playmode = true;
      Taster = 0;
      Step = 1;
      Serial.println("Modo Automatico ");
      data_out();
      delay(250);
      digitalWrite(13, LOW);
    }
    /*if (Taster == 3)
    {
      // ++ arrayStep
      // playmode = 1;
      Taster = 0;
      Serial.println("Clear ");
    }*/ //prueba en el monitor serial
    if (currentMillis - previousMillis3 > 2000) // limpiar estados del boton
    {
      Taster = 0;
      //Serial.println("restart "); //indicacion en el monitor serial
    }
}
