// Arthur Aves da Costa (13751207)

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

// Definição dos pinos GPIO para o LED RGB

#define PIN_RED   18
#define PIN_GREEN 19
#define PIN_BLUE  21

// Configurações do Periférico LEDC (PWM nativo)

#define PWM_FREQ       5000             // Frequência de 5 kHz
#define PWM_RESOLUTION LEDC_TIMER_8_BIT // Resolução de 8 bits (0-255)

// Taxas de incremento independentes para cada cor

#define INC_GREEN 5
#define INC_BLUE  (INC_GREEN * 2) // 10
#define INC_RED   (INC_GREEN * 3) // 15

// Função para inicializar o hardware do PWM (LEDC)

void init_pwm(void) {
    //  Configuração do Timer do LEDC
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = PWM_RESOLUTION,
        .freq_hz          = PWM_FREQ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    
    // Tratamento de erro na configuração do Timer
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Configuração dos Canais
    ledc_channel_config_t ledc_channel[3] = {
        {
            .gpio_num       = PIN_RED,
            .speed_mode     = LEDC_LOW_SPEED_MODE,
            .channel        = LEDC_CHANNEL_0,
            .intr_type      = LEDC_INTR_DISABLE,
            .timer_sel      = LEDC_TIMER_0,
            .duty           = 0,
            .hpoint         = 0
        },
        {
            .gpio_num       = PIN_GREEN,
            .speed_mode     = LEDC_LOW_SPEED_MODE,
            .channel        = LEDC_CHANNEL_1,
            .intr_type      = LEDC_INTR_DISABLE,
            .timer_sel      = LEDC_TIMER_0,
            .duty           = 0,
            .hpoint         = 0
        },
        {
            .gpio_num       = PIN_BLUE,
            .speed_mode     = LEDC_LOW_SPEED_MODE,
            .channel        = LEDC_CHANNEL_2,
            .intr_type      = LEDC_INTR_DISABLE,
            .timer_sel      = LEDC_TIMER_0,
            .duty           = 0,
            .hpoint         = 0
        }
    };

    // Aplica a configuração para cada canal com tratamento de erro
    
    for (int i = 0; i < 3; i++) {
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel[i]));
    }
}

void app_main(void) {
    
    // Inicializa o PWM
    
    init_pwm();

    // Variáveis para armazenar o Duty Cycle atual de cada canal
    
    int dutyRed   = 0;
    int dutyGreen = 0;
    int dutyBlue  = 0;

    // Variáveis para controlar a direção do incremento (1 = subindo, -1 = descendo)
    
    int dirRed   = 1;
    int dirGreen = 1;
    int dirBlue  = 1;

    printf("Incrementos -> Vermelho: %d | Verde: %d | Azul: %d\n\n", INC_RED, INC_GREEN, INC_BLUE);
    
    while (1) {
        
        // Lógica de Fade para o Vermelho
        
        dutyRed += (INC_RED * dirRed);
        if (dutyRed >= 255) {
            dutyRed = 255;
            dirRed = -1;
        } else if (dutyRed <= 0) {
            dutyRed = 0;
            dirRed = 1;
        }

        // Lógica de Fade para o Verde
        
        dutyGreen += (INC_GREEN * dirGreen);
        if (dutyGreen >= 255) {
            dutyGreen = 255;
            dirGreen = -1;
        } else if (dutyGreen <= 0) {
            dutyGreen = 0;
            dirGreen = 1;
        }

        // Lógica de Fade para o Azul
        
        dutyBlue += (INC_BLUE * dirBlue);
        if (dutyBlue >= 255) {
            dutyBlue = 255;
            dirBlue = -1;
        } else if (dutyBlue <= 0) {
            dutyBlue = 0;
            dirBlue = 1;
        }

        // Setar e aplicar o PWM no ESP-IDF
        
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, dutyRed);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, dutyGreen);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, dutyBlue);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);

        // Envia os dados pela UART com cálculo em float
        
        printf("[PWM Logs C] -> R: %3d (%5.1f%%) | G: %3d (%5.1f%%) | B: %3d (%5.1f%%)\n",
               dutyRed,   ((float)dutyRed * 100.0) / 255.0,
               dutyGreen, ((float)dutyGreen * 100.0) / 255.0,
               dutyBlue,  ((float)dutyBlue * 100.0) / 255.0);

        // Delay de 150ms
        
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}