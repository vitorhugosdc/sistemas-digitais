#define F_CPU 16000000UL      //define a frequencia do microcontrolador - 16MHz

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>  // Para usar a função rand()

#define LED2  PD2
#define LED3  PD3
#define LED4  PD4
#define LED5  PD5
#define TONE_PIN PD6

#define BUTTON8 PB1
#define BUTTON9 PB2
#define BUTTON10 PB3
#define BUTTON11 PB4

int sequencia[16] = {};
int botoes[4] = {BUTTON8, BUTTON9, BUTTON10, BUTTON11};
int leds[4] = {LED2, LED3, LED4, LED5};
int tons[4] = {262, 294, 330, 349};

int nota[] = {
    660, 660, 660, 510, 660, 770, 380, 510, 380, 320, 440, 480, 450, 430, 380, 660, 760, 860, 700, 760, 660, 520, 580, 480, 510, 
    380, 320, 440, 480, 450, 430, 380, 660, 760, 860, 700, 760, 660, 520, 580, 480, 500, 760, 720, 680, 620, 650, 380, 430, 500,
    430, 500, 570, 500, 760, 720, 680, 620, 650, 1020, 1020, 1020, 380, 500, 760, 720, 680, 620, 650, 380, 430, 500, 430, 500,
    570, 585, 550, 500, 380, 500, 500, 500, 500, 760, 720, 680, 620, 650, 380, 430, 500, 430, 500, 570, 500, 760, 720, 680, 620,
    650, 1020, 1020, 1020, 380, 500, 760, 720, 680, 620, 650, 380, 430, 500, 430, 500, 570, 585, 550, 500, 380, 500, 500, 500,
    500, 500, 500, 500, 580, 660, 500, 430, 380, 500, 500, 500, 500, 580, 660, 870, 760, 500, 500, 500, 500, 580, 660, 500,
    430, 380, 660, 660, 660, 510, 660, 770, 380
};

int duracaoNota[] = {
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 80, 100, 100, 100, 80, 50, 100, 80, 50, 80, 80, 80, 80, 100, 100, 100,
    100, 80, 100, 100, 100, 80, 50, 100, 80, 50, 80, 80, 80, 80, 100, 100, 100, 100, 150, 150, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 150, 200, 80, 80, 80, 100, 100, 100, 100, 100, 150, 150, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 150, 150, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 150, 200, 80, 80, 80, 100, 100, 100,
    100, 100, 150, 150, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 60, 80, 60, 80, 80, 80, 80, 80, 80, 60,
    80, 60, 80, 80, 80, 80, 80, 60, 80, 60, 80, 80, 80, 80, 80, 80, 100, 100, 100, 100, 100, 100, 100
};

int pausaNota[] = {
    150, 300, 300, 100, 300, 550, 575, 450, 400, 500, 300, 330, 150, 300, 200, 200, 150, 300, 150, 350, 300, 150, 150, 500, 450,
    400, 500, 300, 330, 150, 300, 200, 200, 150, 300, 150, 350, 300, 150, 150, 500, 300, 100, 150, 150, 300, 300, 150, 150, 300,
    150, 100, 220, 300, 100, 150, 150, 300, 300, 300, 150, 300, 300, 300, 100, 150, 150, 300, 300, 150, 150, 300, 150, 100, 420,
    450, 420, 360, 300, 300, 150, 300, 300, 100, 150, 150, 300, 300, 150, 150, 300, 150, 100, 220, 300, 100, 150, 150, 300, 300,
    300, 150, 300, 300, 300, 100, 150, 150, 300, 300, 150, 150, 300, 150, 100, 420, 450, 420, 360, 300, 300, 150, 300, 150, 300,
    350, 150, 350, 150, 300, 150, 600, 150, 300, 350, 150, 150, 550, 325, 600, 150, 300, 350, 150, 350, 150, 300, 150, 600, 150,
    300, 300, 100, 300, 550, 575
};

int rodada = 0;
int passo = 0;
int botaoPressionado = 0;
bool gameOver = false;
int dificuldade1 = 1000; // Tempo de espera da rodada
int dificuldade2 = 300; // Velocidade de reprodução da sequência
int dificuldade3 = 200; // Velocidade após reprodução da sequência

void setupPorts() {
    DDRD |= (1 << LED2) | (1 << LED3) | (1 << LED4) | (1 << LED5);
    DDRD &= ~((1 << BUTTON8) | (1 << BUTTON9) | (1 << BUTTON10) | (1 << BUTTON11));
    PORTD |= (1 << BUTTON8) | (1 << BUTTON9) | (1 << BUTTON10) | (1 << BUTTON11); // Ativar pull-ups
}

void turnOnLED(uint8_t led) {
    PORTD |= (1 << led);
}

void turnOffLED(uint8_t led) {
    PORTD &= ~(1 << led);
}

uint8_t isButtonPressed(uint8_t button) {
    return !(PIND & (1 << button));
}

void tone(uint8_t pin, uint16_t frequency) {
    TCCR1B |= (1 << CS11);
    TCCR1B |= (1 << WGM12);
    OCR1A = (F_CPU / (frequency * 16UL)) - 1;
    TIMSK1 |= (1 << OCIE1A);
    DDRB |= (1 << pin);
}

ISR(TIMER1_COMPA_vect) {
    PORTB ^= (1 << TONE_PIN);
}

void noTone(uint8_t pin) {
    TIMSK1 &= ~(1 << OCIE1A);
    DDRB &= ~(1 << pin);
}

void delay(uint16_t ms) {
    while (ms--) {
        _delay_ms(1);
    }
}

void proximaRodada() {
    sequencia[rodada] = rand() % 4;
    rodada++;
}

void reproduzirSequencia() {
    for (int i = 0; i < rodada; i++) {
        tone(TONE_PIN, tons[sequencia[i]]);
        turnOnLED(leds[sequencia[i]]);
        delay(dificuldade2);
        noTone(TONE_PIN);
        turnOffLED(leds[sequencia[i]]);
        delay(dificuldade3);
    }
}

void aguardarJogador() {
    for (int i = 0; i < rodada; i++) {
        bool jogadaEfetuada = false;
        while (!jogadaEfetuada) {
            for (int j = 0; j <= 3; j++) {
                if (isButtonPressed(botoes[j])) {
                    botaoPressionado = j;
                    tone(TONE_PIN, tons[j]);
                    turnOnLED(leds[j]);
                    delay(300);
                    noTone(TONE_PIN);
                    turnOffLED(leds[j]);
                    jogadaEfetuada = true;
                }
            }
        }
        if (sequencia[passo] != botaoPressionado) {
            efeito2();
            gameOver = true;
            return;
        }
        passo++;
    }
    passo = 0;
}

void efeito1() {
    turnOnLED(LED2);
    turnOnLED(LED3);
    turnOnLED(LED4);
    turnOnLED(LED5);
    delay(1000);
    turnOffLED(LED2);
    turnOffLED(LED3);
    turnOffLED(LED4);
    turnOffLED(LED5);
    delay(1000);
}

void efeito2() {
    for (int i = 0; i < 4; i++) {
        turnOnLED(leds[i]);
        tone(TONE_PIN, 70);
        delay(100);
        turnOffLED(leds[i]);
        noTone(TONE_PIN);
    }
}

void efeito3() {
    for (int i = 0; i < sizeof(nota) / sizeof(nota[0]); i++) {
        tone(TONE_PIN, nota[i]);
        turnOnLED(LED2);
        turnOnLED(LED3);
        turnOnLED(LED4);
        turnOnLED(LED5);
        delay(duracaoNota[i]);
        turnOffLED(LED2);
        turnOffLED(LED3);
        turnOffLED(LED4);
        turnOffLED(LED5);
        delay(pausaNota[i]);
        noTone(TONE_PIN);
    }
}

int main(void) {
    setupPorts();
    sei(); 

    while (1) {
        if (gameOver) {
            efeito1();
        } else {
            proximaRodada();
            reproduzirSequencia();
            aguardarJogador();
            delay(dificuldade1);
            if (rodada > 7) {
                dificuldade1 = 500;  // Reduz o tempo de espera da rodada pela metade.
                dificuldade2 = 150;  // Aumenta a velocidade de reprodução da sequência.
                dificuldade3 = 100;  // Aumenta ainda mais a velocidade de reprodução da sequência.
            }
            if (rodada == 15) {
                efeito3();
                gameOver = true;
            }
        }
    }
    return 0;
}
