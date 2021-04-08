// EL PRESENTE FUENTE FUE DESARROLLADO POR SEAMAN SRL Y SE 
// ENTREGA SIN GARANTIAS Y A MODO DE PRUEBA UNICAMENTE

// Esquema de conexion en Arduino UNO:
// RELE 1: PIN D2
// RELE 2: PIN D3
// SENSOR DE PROXIMIDAD INFRAROJO 1: PIN D10
// SENSOR DE PROXIMIDAD INFRAROJO 2: PIN D11
// LIDAR SCL: PIN A1
// LIDAR SDA: PIN A4
// NEOPIXEL 8 LEDS: PIN D5
// SPEAKER: PIN D13 

#include <VL53L0X.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

// ------- Declaracion de variables globales ----------
VL53L0X sensor;
int lastMeasure = 0;
int DesactiveTime = 0;
bool Active = false;

Adafruit_NeoPixel pixels(8, 5, NEO_GRB + NEO_KHZ800);

void setup()
{
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(10, INPUT);
  pinMode(11, INPUT);
  
  // ------ Opcional monitoreo de acciones ------------
  Serial.begin(9600);

  // ------ Comunicacion con el Lidar -----------------
  Wire.begin();

  sensor.init();
  sensor.setTimeout(500);
  sensor.startContinuous();

  // ------ Comunicacion con NeoPixel
  pixels.begin();
}

// -------- Generador de alera sonora -----------------
void ActiveSound()
{
  tone(13, 500, 100);
  delay(150);
  tone(13, 1000, 100);
  delay(150);
}

void DesactiveSound()
{
  tone(13, 500, 100);
  delay(150);
}
void loop()
{
  // Si el sistema se activo comando los reles de mando del motor de puerta
  if (Active == true)
  {
    if (digitalRead(10) == false && digitalRead(11) == false)  
    {
      // Si ambos sensores de proximidad estan desactivos se apagan los motores
      digitalWrite(2, HIGH); 
      digitalWrite(3, HIGH);
    }
    else
    {
      if (digitalRead(10) == true && digitalRead(11) == false)
      {
        // Si un sensor esta activo y el otro no activo el motor en la direccion opuesta al sensor desactivo para compensar
        // la posicion buscando volver a tener a ambos sensores activos
        digitalWrite(2, HIGH); 
        digitalWrite(3, LOW);  
      }
      else if (digitalRead(10) == false && digitalRead(11) == true)
      {
        // Lo mismo que antes pero en la direccion opuesta
        digitalWrite(2, LOW);
        digitalWrite(3, HIGH);  
      }
      else
      {
        // Ante cualquier otra situacion desactivo los mototores
        digitalWrite(2, HIGH); 
        digitalWrite(3, HIGH);
      }
    }
  }
  
  
  // Tomamos la medida del LIDAR
  int measure = sensor.readRangeContinuousMillimeters();

  // Depende la distancia a la que se ecuentre apago leds
  if (measure < 150)
  {
    if (Active == false)
    {
      // Solo se activa si vien de un estado desactivo, la distancia es menos a 150mm y ambos sensores de proximidad se activan
      if (digitalRead(10) == false && digitalRead(11) == false)
      {
        // Hago una transicion suave de azul a verde indicando que todo se activo bien
        for (int Intensidad_G = 0; Intensidad_G < 256; Intensidad_G++)
        {
          int Intensidad_B = 256 - Intensidad_G;
          
          for (int Pixel = 0; Pixel < 8; Pixel++)
            pixels.setPixelColor(Pixel, pixels.Color(0, Intensidad_G, Intensidad_B));
  
          pixels.show();
          delay(1);  
        }
        
        ActiveSound();
         
        Active = true;
      }
    }
  }
  else
  {
    if ((digitalRead(10) == true && digitalRead(11) == true) || Active == false)
    {
      DesactiveTime++;
      
      if (DesactiveTime > 5 || Active == false)
      { 
        // Apago los LEDs
        for (int Pixel = 0; Pixel < 8; Pixel++)
          pixels.setPixelColor(Pixel, pixels.Color(0, 0, 0));

        // Digo que empiezo a madir distancia recien a los 30cm, para ello invierto la escala
        int distance = 300 - measure;
        if (distance < 0)
          distance = 0;

        // Regla de 3 para convertir distancia en pixeles de 1 a 8 donde 300 son los 8 pixeles prendidos
        float DistanceToPixel = (800 / 150 * distance) / 100;

        // Prendo los Pixeles
        for (int Pixel = 0; Pixel < (int)DistanceToPixel; Pixel++)
          pixels.setPixelColor(Pixel, pixels.Color(0, 0, 255));

        // Para dar un efecto mas suave prendo el siguiente pixel a la mita de la intensidad
        if ((int)DistanceToPixel < 8)
          pixels.setPixelColor((int)DistanceToPixel, pixels.Color(0, 0, 60));
        
        if (Active == true)
        {
          // Hago una transicion suave de verde a naranja indicando que se desactivo
          for (int Intensidad_R = 0; Intensidad_R <= 150; Intensidad_R++)
          {
            int Intensidad_G = 256 - Intensidad_R;
            if (Intensidad_G < 50)
              Intensidad_G = 50;
              
            for (int Pixel = 0; Pixel < 8; Pixel++)
              pixels.setPixelColor(Pixel, pixels.Color(Intensidad_R, Intensidad_G, 0));
    
            pixels.show();
            delay(1);  
          }
     
          delay(100);

          // Apago los led suavemente en secuencia regresiva
          for (int Pixel = 7; Pixel >= 0; Pixel = Pixel - 1)
          {
            pixels.setPixelColor(Pixel, pixels.Color(0, 0, 0));
            pixels.show();
            delay(40);
          }
          
          Active = false;

          DesactiveSound(); 
        }
        DesactiveTime = 0;  
      }
    }
  }
      
  pixels.show();
  
  delay(100);
}
