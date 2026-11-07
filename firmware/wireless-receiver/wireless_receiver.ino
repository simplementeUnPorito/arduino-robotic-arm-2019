// Version 2 (wireless), slave/receiver: drives the servos from Bluetooth data.
// Source listing transcribed verbatim from the 2019 project report; comments are the original Spanish.

#include <ServoTimer2.h>

#include <AltSoftSerial.h>

AltSoftSerial BTSerial;
int posicionMomentanea[11],j;
char c=' ';
String s;
boolean NL = true;
boolean imprimir = true;

ServoTimer2 servoMuneca;
ServoTimer2 servoPulgar;
ServoTimer2 servoIndice;
ServoTimer2 servoDedoMedio;
ServoTimer2 servoAnular;
ServoTimer2 servoMenique;
ServoTimer2 servoXBrazo;
ServoTimer2 servoYBrazo;
ServoTimer2 servoXAnteBrazo;
ServoTimer2 servoYAnteBrazo;
ServoTimer2 servoY2Brazo;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.setTimeout(100);
    Serial.print("Sketch:   ");   Serial.println(__FILE__);
    Serial.print("Uploaded: ");   Serial.println(__DATE__);
    Serial.println(" ");

    BTSerial.begin(9600);
    Serial.println("BTserial inicializado en 9600");
    Serial.println(" ");
    delay(1000);
    BTSerial.print("AT+IMME0");

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

    Serial.println("mini brazo robot listo...");
    delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (BTSerial.available())
    {
        c = BTSerial.read();
        Serial.write(c);
        s+=c;
    }

    if(c==']')
    {

      for(j=0;j<11;j++)
      {
        posicionMomentanea[j]= obtenerParteString(s,',',j).toInt();
        if((posicionMomentanea[j] > 2500) || (posicionMomentanea[j] < 400)){
         imprimir = false;
        }
        if(j == 10 && imprimir == true)
        {
          servoMuneca.write(posicionMomentanea[0]);
          servoPulgar.write(posicionMomentanea[1]);
          servoIndice.write(posicionMomentanea[2]);
          servoDedoMedio.write(posicionMomentanea[3]);
          servoAnular.write(posicionMomentanea[4]);
          servoMenique.write(posicionMomentanea[5]);
          servoXAnteBrazo.write(posicionMomentanea[6]);
          servoYAnteBrazo.write(posicionMomentanea[7]);
          servoXBrazo.write(posicionMomentanea[8]);
          servoYBrazo.write(posicionMomentanea[9]);
          servoY2Brazo.write(posicionMomentanea[10]);
        }
      }
      s="";
      imprimir = true;

    }

}

String obtenerParteString(String dato, char separador, int indice)
{

    int stringDato = 0;
    String parteDato = "";

    for(int i = 0; i<dato.length()-1; i++) {

      if(dato[i]==separador) {
        stringDato++;

      }else if(stringDato==indice) {

        parteDato.concat(dato[i]);

      }else if(stringDato>indice) {

        return parteDato;
        break;

      }
    }

    return parteDato;
  }
