/*
    Tamaño de la pantalla 240x320 pixeles
*/
// Librería para el protocolo de datos seriales síncronos SPI (Serial Peripheral Interface)
#include "SPI.h"
// Librería para gráficos, con un conjunto de primitivas: puntos, líneas, círculos, etc.
#include "Adafruit_GFX.h" 
// Librería para el display LCD TFT (Thin Film Transistor) 
#include "Adafruit_ILI9341.h"

// Include Sprites
#include "Sprite.h"

// Se definen los pines para la conexión entre el Arduino Mega y el TFT LCD ILI9341
#define TFT_DC 7
#define TFT_CS 6
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST 10
#define TFT_MISO 12

#define botonRight 18
#define botonLeft 19
#define botonUP 20
#define botonDown 21
#define botonAgacharse 16
#define botonSaltar 17

int count = 0; // Variable global para el contador

#define TIME_LIMIT 20 // Tiempo límite en segundos (8 minutos)

const int XMAX = 240; // Alto del display
const int YMAX = 320; // Ancho del display
int x = 0; // Posicion x del jugador
int y = 0; // Posicion y del jugador
int x_old = 0; // Posicion x del jugador
int y_old = 0; // Posicion y del jugador
int playerFrame = 0; // Fotograma del jugador

bool gameActive = true; // Indica si el juego está activo
bool buzzerActive = false; // Bandera para controlar el buzzer
const int buzzerPin = 5; // Pin donde conectaste el buzzer

//Posiciones de las monedas en el Display
int monedaX1=3;
int monedaY1=45;

int monedaX2=85;
int monedaY2=25;

int monedaX3=140;
int monedaY3=65;

int monedaX4=40;
int monedaY4=110;

int monedaX5=120;
int monedaY5=170;

int monedaX6=220;
int monedaY6=145;

int monedaX7=15;
int monedaY7=215;

int monedaX8=190;
int monedaY8=240;

int monedaX9=220;
int monedaY9=275;

int monedaX10=95;
int monedaY10=276;

const uint8_t UP = 0;
const uint8_t DOWN = 1;
const uint8_t RIGHT = 2;
const uint8_t LEFT = 3;
const uint8_t AGACHARSE = 4;
const uint8_t SALTAR = 5;

unsigned long lastPlayerUpdate = 0;
unsigned long animationDelay = 10;

// Variables para el temporizador
unsigned long startTime; // Variable para almacenar el tiempo inicial

// Se instancia el objeto screen del tipo Adafruit_ILI9341 cuyos
// parámetros corresponden a los pines de conexión definidos anteriormente
Adafruit_ILI9341 screen = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

GFXcanvas1 canvas(12, 16);
GFXcanvas1 canvasPlataforma(28, 6);
GFXcanvas1 canvasEstrella(13, 14);
GFXcanvas1 canvasMoneda(9, 9);
GFXcanvas1 canvasEnemigo1(10, 11);

// Declaración de funciones
void setPlayerPosition(int x, int y);
void animatePlayer(void);
void moverPlayer(uint8_t direccion);
void moverPlayerOld(uint8_t direccion);

void moverPlayerDerecha(void);
void moverPlayerIzquierda(void);
void moverPlayerArriba(void);
void moverPlayerAbajo(void);
void moverPlayerAgacharse(void);
void moverPlayerSaltar(void);

void animatePlataforma(void);
void animateMoneda(void);
void animateEstrella(void);
void animateObstaculo(void);

// Función para calcular y mostrar el tiempo restante

void updateTimer() {
    if (!gameActive) {
        return; // Salir si el juego ya terminó
    }

    unsigned long currentTime = millis() / 1000;
    unsigned long elapsedTime = currentTime - startTime;
    unsigned long remainingTime = (elapsedTime < TIME_LIMIT) ? (TIME_LIMIT - elapsedTime) : 0;

    unsigned int minutes = remainingTime / 60;
    unsigned int seconds = remainingTime % 60;

    Serial.print("Tiempo restante: ");
    Serial.print(minutes);
    Serial.print(":");
    if (seconds < 10) Serial.print("0");
    Serial.println(seconds);

    if (remainingTime == 0) {
        Serial.println("¡Tiempo agotado! GAME OVER.");

        // Activar el buzzer inmediatamente
        tone(buzzerPin, 1000, 500); // Suena el buzzer con tono 1000Hz durante 500ms

        // Mostrar mensaje en pantalla
        screen.fillScreen(ILI9341_BLACK);
        screen.setTextColor(ILI9341_RED);
        screen.setTextSize(3);
        screen.setCursor(40, 100);
        screen.println("GAME OVER");

        delay(2000); // Pausa de 2 segundos
        
        // Simular apagado
        screen.fillScreen(ILI9341_BLACK);

        Serial.println("Reiniciando el juego...");
        asm volatile("jmp 0"); // Reinicia el programa desde el inicio
    }
}

void checkMoneda(int x,int y,int monedaX, int monedaY);
bool checkColision(uint8_t objetoX,uint8_t objetoY,uint8_t ancho,uint8_t alto);

// Configuración 
void setup() {
    Serial.begin(9600); // Inicializar comunicación serial
    Serial.println("Serial inicializado");

    SPI.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0)); // 40 MHz

    // Mostrar mensaje "START GAME" en el display
    screen.begin(); // Inicialización de la pantalla
    screen.fillScreen(ILI9341_BLACK); // Pinta la pantalla de negro
    screen.setTextColor(ILI9341_GREEN); // Texto en color verde
    screen.setTextSize(3); // Tamaño grande para el texto
    screen.setCursor(40, 100); // Posiciona el mensaje en la pantalla
    screen.println("START GAME");

    // Activar buzzer al iniciar
    tone(buzzerPin, 1000, 500); // Suena el buzzer con tono de 1000Hz durante 500ms
    delay(500); // Esperar a que termine el sonido del buzzer
    noTone(buzzerPin); // Apagar el buzzer
    delay(2000); // Pausa para que el mensaje sea visible

    screen.fillScreen(ILI9341_BLACK); // Limpiar la pantalla después del mensaje inicial

    // Configurar tiempo inicial
    startTime = millis() / 1000; // Guardar el tiempo inicial en segundos

    // Configuración de interrupciones externas
    attachInterrupt(digitalPinToInterrupt(botonRight), moverPlayerDerecha, HIGH);
    attachInterrupt(digitalPinToInterrupt(botonLeft), moverPlayerIzquierda, HIGH);
    attachInterrupt(digitalPinToInterrupt(botonUP), moverPlayerArriba, HIGH);
    attachInterrupt(digitalPinToInterrupt(botonDown), moverPlayerAbajo, HIGH);
    attachInterrupt(digitalPinToInterrupt(botonAgacharse), moverPlayerAgacharse, HIGH);
    attachInterrupt(digitalPinToInterrupt(botonSaltar), moverPlayerSaltar, HIGH);

    // Configuración inicial de pantalla y jugador
    screen.setTextColor(ILI9341_WHITE);
    screen.setTextSize(2);
    setPlayerPosition(0, YMAX - 32);

    sei(); // Habilitar interrupciones globales
}

// Lazo infinito, codigo que se repite constantemente
void loop() {   
    
    animatePlataforma();
    unsigned long currentTime = millis();

    //Llama al Tiempo Restante de Juego
    updateTimer(); // Actualizar y mostrar el tiempo restante

    // Animación del jugador
    if (currentTime - lastPlayerUpdate > animationDelay) {
        animatePlayer();
        lastPlayerUpdate = currentTime;
    }
    /*checkMoneda(monedaX1,monedaY1);
    checkMoneda(monedaX2,monedaY2);
    checkMoneda(monedaX3,monedaY3);
    checkMoneda(monedaX4,monedaY4);
    checkMoneda(monedaX5,monedaY5);
    checkMoneda(monedaX6,monedaY6);
    checkMoneda(monedaX7,monedaY7);
    checkMoneda(monedaX8,monedaY8);
    checkMoneda(monedaX9,monedaY9);*/
    checkMoneda(x,y,monedaX10,monedaY10);
}

// Fija la posicion x,y del jugador
void setPlayerPosition(int x1, int y1) {
    x = x1;
    y = y1;
}

// Anima el jugador
void animatePlayer(void) {

        noInterrupts();
        screen.fillRect(x_old-1, y_old-1, 14, 20, ILI9341_BLACK);
        //screen.drawBitmap(x_old, y_old, canvas.getBuffer(), canvas.width(), canvas.height(), ILI9341_BLACK, ILI9341_BLACK);

        //canvas.drawBitmap(0, 0, PlayerIdle, 12, 16, ILI9341_GREEN);
        screen.drawBitmap(x, y, PlayerIdle, 12, 16, ILI9341_GREEN);
        //screen.drawBitmap(x, y, canvas.getBuffer(), canvas.width(), canvas.height(), ILI9341_GREEN, ILI9341_BLACK);
        
        // Actualiza las posiciones anteriores
        x_old = x;
        y_old = y;

        interrupts();

        count++;
        if(count == 2)
            count = 0; 
        delay(300);
}

// Función que atienda la interrupción
void moverPlayerDerecha() {

    delay(300);
    if(digitalRead(botonRight) == HIGH) {
        Serial.println("Boton RIGHT");
        //moverPlayerOld();
        moverPlayer(RIGHT);
        canvas.fillScreen(0);

        if (!buzzerActive) {
            tone(buzzerPin, 2000, 20); // Tono agudo al mover
            buzzerActive = true;        // Activar la bandera
        }
        else {
        buzzerActive = false; // Reinicia la bandera cuando se deja de presionar    
        }   
    }    
}
void moverPlayerIzquierda(){

    canvas.fillScreen(0);
    delay(300);
    if(digitalRead(botonLeft) == HIGH) {
        Serial.println("Boton LEFT");
        moverPlayer(LEFT);

        if (!buzzerActive) {
            tone(buzzerPin, 2000, 20); // Tono agudo al mover
            buzzerActive = true;        // Activar la bandera
        }
        else {
        buzzerActive = false; // Reinicia la bandera cuando se deja de presionar
        }
    }
}
void moverPlayerArriba() {

    canvas.fillScreen(0);
    delay(300);
    if(digitalRead(botonUP) == HIGH) {
        Serial.println("Boton UP");
        moverPlayer(UP);

        if (!buzzerActive) {
            tone(buzzerPin, 4000, 20); // Tono agudo al mover
            buzzerActive = true;        // Activar la bandera
        }
        else {
        buzzerActive = false; // Reinicia la bandera cuando se deja de presionar
        }
    }
}
void moverPlayerAbajo() {

    canvas.fillScreen(0);
    delay(300);
    if(digitalRead(botonDown) == HIGH) {
        Serial.println("Boton Down");
        moverPlayer(DOWN);

        if (!buzzerActive) {
            tone(buzzerPin, 200, 20); // Tono agudo al mover
            buzzerActive = true;        // Activar la bandera
        }
        else {
        buzzerActive = false; // Reinicia la bandera cuando se deja de presionar
        }
    }
}

void moverPlayerAgacharse(){
    canvas.fillScreen(0);
    delay(300);
    if(digitalRead(botonAgacharse)== HIGH){
        Serial.println("Boton Agacharse");
        moverPlayer(AGACHARSE); 
    }
}

void moverPlayerSaltar(){
    canvas.fillScreen(0);
    delay(300);
    if(digitalRead(botonSaltar)== HIGH){
        Serial.println("Boton Saltar");
        moverPlayer(SALTAR); 
    }
}
// Mueve el jugador
void moverPlayer(uint8_t direccion) {
    x_old = x - 5;
    y_old = y - 2;
    uint8_t delta = 2;  // Valor del DELTA
    switch (direccion){
        case 0:  // Arriba
            y = y - delta;
            break;
        case 1:  // Abajo
            y = y + delta;
            break;
        case 2:  // Derecha
            x = x + delta;
            break;
        case 3:  // Izquierda
            x = x - delta;
            break;
        case 4:  // Agacharse
            y = y + delta;
            break;
        case 5:  // Saltar
            y = y - delta;
        break;
    }
}

void checkMoneda(int x,int y, int monedaX,int monedaY){

    if (checkColision(x, y, monedaX, monedaY)){

        Serial.println("Moneda Recogida!! + 10 Puntos");
    }

}
bool checkColision(uint8_t objetoX,uint8_t objetoY,uint8_t ancho,uint8_t alto){

    if ((x < objetoX + ancho)||(x + 12 > objetoX)||(y + 16 > objetoY)||(y > objetoY + alto))
        return false;
    else
        return true;
    //Si se quiere ver la posicion del jugador
    //Serial.println("Colision en x:");
    //Serial.println(x);
    //Serial.println("Colision en y:");
    //Serial.println(y);
}

//Plataformas en el Display//
void animatePlataforma() {

    //-----------------------------------PRIMERA LINEA DE PLATAFORMAS------------------------------------------
    //Plataforma con Moneda
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(-10, 60, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasMoneda.drawBitmap(0, 0, Money, 9, 9, ILI9341_YELLOW);
    screen.drawBitmap(monedaX1, monedaY1, canvasMoneda.getBuffer(), canvasMoneda.width(), canvasMoneda.height(), ILI9341_YELLOW, ILI9341_BLACK);
    //---------
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(35, 35, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);
    //Plataforma con Moneda
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(75, 40, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasMoneda.drawBitmap(0, 0, Money, 9, 9, ILI9341_YELLOW);
    screen.drawBitmap(monedaX2, monedaY2, canvasMoneda.getBuffer(), canvasMoneda.width(), canvasMoneda.height(), ILI9341_YELLOW, ILI9341_BLACK);
    //-------
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(120, 40, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(160, 40, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);
    //Platafroma Final CON ESTRELLA
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(210, 30, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasEstrella.drawBitmap(0, 0, Star, 13, 14, ILI9341_YELLOW);
    screen.drawBitmap(220, 10, canvasEstrella.getBuffer(), canvasEstrella.width(), canvasEstrella.height(), ILI9341_YELLOW, ILI9341_BLACK);

    //----------------------------------SEGUNDA LINEA DE PLATAFORMAS------------------------------------------

    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(35, 80, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(80, 90, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);
    //Plataforma con Moneda
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(130, 80, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasMoneda.drawBitmap(0, 0, Money, 9, 9, ILI9341_YELLOW);
    screen.drawBitmap(monedaX3, monedaY3, canvasMoneda.getBuffer(), canvasMoneda.width(), canvasMoneda.height(), ILI9341_YELLOW, ILI9341_BLACK);
    //-------

    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(170, 90, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(220, 115, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    //----------------------------------TERCERA LINEA DE PLATAFORMAS------------------------------------------

    //Plataforma con Moneda
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(30, 125, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasMoneda.drawBitmap(0, 0, Money, 9, 9, ILI9341_YELLOW);
    screen.drawBitmap(monedaX4, monedaY4, canvasMoneda.getBuffer(), canvasMoneda.width(), canvasMoneda.height(), ILI9341_YELLOW, ILI9341_BLACK);
    //--------------

    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(-10, 150, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(20, 168, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(70, 168, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);
    //Plataforma con Moneda
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(110, 187, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasMoneda.drawBitmap(0, 0, Money, 9, 9, ILI9341_YELLOW);
    screen.drawBitmap(monedaX5, monedaY5, canvasMoneda.getBuffer(), canvasMoneda.width(), canvasMoneda.height(), ILI9341_YELLOW, ILI9341_BLACK);
    //-----------------------
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(135, 155, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(165, 135, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(150, 200, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);
    //Plataforma con Moneda
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(210, 160, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasMoneda.drawBitmap(0, 0, Money, 9, 9, ILI9341_YELLOW);
    screen.drawBitmap(monedaX6, monedaY6, canvasMoneda.getBuffer(), canvasMoneda.width(), canvasMoneda.height(), ILI9341_YELLOW, ILI9341_BLACK);
    //-----------------------
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(185, 220, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    //----------------------------------CUARTA LINEA DE PLATAFORMAS------------------------------------------

    //Plataforma con Moneda
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(5, 230, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasMoneda.drawBitmap(0, 0, Money, 9, 9, ILI9341_YELLOW);
    screen.drawBitmap(monedaX7, monedaY7, canvasMoneda.getBuffer(), canvasMoneda.width(), canvasMoneda.height(), ILI9341_YELLOW, ILI9341_BLACK);
    //-----------
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(40, 240, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(80, 240, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(115, 270, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);
    //Plataforma con Moneda
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(180, 255, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasMoneda.drawBitmap(0, 0, Money, 9, 9, ILI9341_YELLOW);
    screen.drawBitmap(monedaX8, monedaY8, canvasMoneda.getBuffer(), canvasMoneda.width(), canvasMoneda.height(), ILI9341_YELLOW, ILI9341_BLACK);
    //-----------
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(225, 247, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    //----------------------------------QUINTA LINEA DE PLATAFORMAS------------------------------------------

    //Plataforma con Moneda
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(210, 290, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasMoneda.drawBitmap(0, 0, Money, 9, 9, ILI9341_YELLOW);
    screen.drawBitmap(monedaX9 , monedaY9, canvasMoneda.getBuffer(), canvasMoneda.width(), canvasMoneda.height(), ILI9341_YELLOW, ILI9341_BLACK);
    //-----------
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(160, 290, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);
    //Plataforma con Moneda
    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(85, 290, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasMoneda.drawBitmap(0, 0, Money, 9, 9, ILI9341_YELLOW);
    screen.drawBitmap(monedaX10, monedaY10, canvasMoneda.getBuffer(), canvasMoneda.width(), canvasMoneda.height(), ILI9341_YELLOW, ILI9341_BLACK);
    //-----------

    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(40, 300, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);

    canvasPlataforma.drawBitmap(0, 0, Plataforma, 28, 6, ILI9341_WHITE);
    screen.drawBitmap(5, 316, canvasPlataforma.getBuffer(), canvasPlataforma.width(), canvasPlataforma.height(), ILI9341_WHITE, ILI9341_BLACK);
}