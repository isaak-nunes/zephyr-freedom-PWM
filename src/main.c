#include <zephyr/kernel.h>             // Funções básicas do Zephyr (ex: k_msleep, k_thread, etc.)
#include <zephyr/device.h>             // API para obter e utilizar dispositivos do sistema
#include <zephyr/drivers/gpio.h>       // API para controle de pinos de entrada/saída (GPIO)
#include <pwm_z42.h>                // Biblioteca personalizada com funções de controle do TPM (Timer/PWM Module)
#include <math.h>

#define SLEEP_TIME_MS 500

//34464 é o valor maximo
#define TPM_MODULE 1024 * 16        // Define a frequência do PWM fpwm = (TPM_CLK / (TPM_MODULE * PS))
// Valores de duty cycle correspondentes a diferentes larguras de pulso
uint16_t duty_0  = 0;
uint16_t duty_2  = TPM_MODULE/4;        // 25% de duty cycle (muito brilho)
uint16_t duty_5  = TPM_MODULE/2;        // 50% de duty cycle (meio brilho)
uint16_t duty_7  = 3 * TPM_MODULE/4;                 // 75% de duty cycle (pouco brilho)
uint16_t duty_10  = TPM_MODULE;         // 100% de duty cycle (led desligado)
//IGNORE OS COMENTARIOS, O LED TA LIGADO NO ZERO E DESLIGADO DO 1024!!!!

enum estado_led {
    VERMELHO,
    AMARELO,
    VERDE,
    AZUL
    //laranja
    //magenta
    //ciano
    
};

//função que facilita a seleção de cor do led imbutido :)
void Led_rgb (int LED_VERMELHO, int LED_VERDE, int LED_AZUL){
    //multiplo pra facilitar o processo pra ficar bonito
    int m = floor(TPM_MODULE/256);
        pwm_tpm_CnV(TPM2, 0, TPM_MODULE - LED_VERMELHO * m);
        pwm_tpm_CnV(TPM2, 1, TPM_MODULE - LED_VERDE * m);
        pwm_tpm_CnV(TPM0, 1, TPM_MODULE - LED_AZUL * m);
}

//Função para piscar o led rgb npisca vezes com duração de tpisca ms
void pisca(int red, int green, int blue, int tpis, int npis){
		for (int i = 0; i < npis; i++) {
		k_msleep(tpis);
        Led_rgb(red,green,blue);
        k_msleep(tpis);
        Led_rgb(0,0,0);
		}

}

//maquina de estado baseado na cor do lado
void cor_led(enum estado_led state) {
    //Iniciação das pinos de led
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18);     // Led vermelho
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 19);     // Led verde
    pwm_tpm_Ch_Init(TPM0, 1, TPM_PWM_H, GPIOD, 1);      // Led azul

    //Iniciação dos pinos de uso geral
    pwm_tpm_Ch_Init(TPM0, 2, TPM_PWM_H, GPIOD, 2);      // porta de uso geral
    pwm_tpm_Ch_Init(TPM0, 3, TPM_PWM_H, GPIOD, 3);      // porta de uso geral

    switch (state) {
        case VERMELHO:
        //configuração dos pinos de led
        Led_rgb(255,0 ,0);
        //configuração dos pinos de uso geral
            pwm_tpm_CnV(TPM0, 2, duty_0);
            pwm_tpm_CnV(TPM0, 3, duty_0);
            break;

        case AMARELO:
        //configuração dos pinos de led
        Led_rgb(255,255,0);

        //configuração dos pinos de uso geral
            pwm_tpm_CnV(TPM0, 2, duty_10);
            pwm_tpm_CnV(TPM0, 3, duty_5);
            break;

        case VERDE:
        //configuração dos pinos de led
        Led_rgb(0,255,0);

        //configuração dos pinos de uso geral
            pwm_tpm_CnV(TPM0, 2, duty_5);
            pwm_tpm_CnV(TPM0, 3, duty_10);
            break;

        case AZUL:
        //configuração dos pinos de led
        Led_rgb(0,0,255);

        //configuração dos pinos de uso geral
            pwm_tpm_CnV(TPM0, 2, duty_10);
            pwm_tpm_CnV(TPM0, 3, duty_10);
            break;
    }
}

int main(void) {
    //iniciação das portas
        pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);
        pwm_tpm_Init(TPM0, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

    //estado inical
    enum estado_led current_state = VERMELHO; // Initial state

    while (1) { 
        cor_led(current_state);

        switch (current_state) {
            case VERMELHO:
                k_sleep(K_SECONDS(5));
                pisca(255,0,0, 300, 3);
                k_msleep(30);
                current_state = VERDE;
                break;

            case VERDE:
                k_sleep(K_SECONDS(3)); 
                pisca(0,255,0, 300, 3);
                current_state = AMARELO; 
                k_msleep(30);
                break;

            case AMARELO:
                k_sleep(K_SECONDS(3)); 
                pisca(255,255,0, 300, 3);
                k_msleep(30);
                current_state = AZUL;
                break;

            case AZUL:
                k_sleep(K_SECONDS(7)); 
                pisca(0,0,255, 300, 3);
                k_msleep(30);
                current_state = VERDE; 
                break;
        }
    }
}