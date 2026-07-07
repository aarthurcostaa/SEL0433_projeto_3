# SEL0433_projeto_3 - Arthur Alves da Costa (13751207)

Aplicações em C utilizando a ESP32

# Parte 1 - Controle de LEDs via sinal PWM

## Pinout

Conecte os terminais do seu LED RGB aos seguintes pinos do ESP32 (lembre-se de usar resistores limitadores de corrente adequados para cada cor):

| Cor do LED | Pino GPIO (ESP32) |
| :--- | :--- |
| **Vermelho (Red)** | GPIO 18 |
| **Verde (Green)** | GPIO 19 |
| **Azul (Blue)** | GPIO 21 |

---

## Explicação do código

### 1. Inicialização do PWM

A função `init_pwm()` é responsável por preparar o hardware:
* Configurado para operar em 5000 Hz com uma resolução de 8 bits (valores de 0 a 255).
* Cada pino GPIO é atrelado a um canal independente (`LEDC_CHANNEL_0` a `LEDC_CHANNEL_2`), todos sincronizados pelo mesmo timer.

### 2. A Lógica de Transição
Para evitar o efeito visual indesejado de "dente de serra" (onde o LED atinge o brilho máximo e apaga abruptamente), o código utiliza variáveis de direção (`dirRed`, `dirGreen`, `dirBlue`). 
A potência do LED é incrementada até atingir 255. Quando esse teto é alcançado, o multiplicador de direção é invertido para -1, fazendo o valor decrescer suavemente até 0, repetindo o ciclo.

### 3. Atualização do Duty Cycle

No framework ESP-IDF, a atualização do brilho de um LED exige dois passos explícitos por motivos de sincronização de hardware:
1. `ledc_set_duty()`: Define o novo valor alvo na memória.
2. `ledc_update_duty()`: Aplica a mudança efetivamente no pino físico.

## Resultados 

Esquemático do circuito simulado no Wokwi (`https://wokwi.com/projects/468849739485062145`)

<img width="671" height="338" alt="proj3_1" src="https://github.com/user-attachments/assets/81036183-f010-4ff5-94ee-ca15389c6427" />


# Parte 2 - Controle de servo motor + Display de dados + Botão de interrupção

## Pinout

| Componente | Pino ESP32 | Observação |
| :--- | :--- | :--- |
| **Potenciômetro** | `GPIO 34` | ADC1 Canal 6. |
| **Servo Motor (Sinal)** | `GPIO 18` | Saída do sinal PWM (LEDC). Alimente o servo com 5V (VIN). |
| **OLED (SDA)** | `GPIO 21` | Linha de dados do barramento I2C. |
| **OLED (SCL)** | `GPIO 22` | Linha de clock do barramento I2C. |
| **Botão (Push-Button)** | `GPIO 14` | Ligar a outra perna ao GND. Usa pull-up interno. |

---

## Explicação do código

### 1. Driver LEDC 
Para gerar o sinal PWM do Servo Motor, foi utilizado o periférico LEDC (`driver/ledc.h`).
* O LEDC apresentou uma implementação mais simples do que o MCPWM que estava dando erros na simulação no Wokwi.
* A resolução foi configurada para 14 bits (0 a 16383), permitindo um pulso (Duty Cycle) que varia de 410 (0 graus) a 2048 (180 graus).

### 2. Leitura Analógica (ADC OneShot)
Utilizando a biblioteca `esp_adc/adc_oneshot.h` nativa do ESP-IDF v5.
* A leitura dos valores do potenciômetro foi feita com resolução de 12 bits (0 a 4095).
* A função `mapear_adc_para_duty` realiza a conversão linear direta desse valor para os limites de Duty Cycle do servo.

### 3. Interrupção por Hardware e Debounce
O botão no GPIO 14 aciona uma interrupção na borda de descida (`GPIO_INTR_NEGEDGE`).
* Como botões mecânicos geram ruído, a função `button_isr_handler` utiliza `xTaskGetTickCountFromISR()` para ignorar qualquer acionamento repetido em um intervalo menor que 250ms.
* Ao ser acionado, ele inverte a variável global `modo_fixo_90`, alternando o sistema entre o Modo Manual e o Modo Fixo.

### 4. Mini-Driver OLED(I2C)
* A comunicação I2C é feita a 400kHz (`I2C_MODE_MASTER`).
* O código contém sua própria tabela hexadecimal `font5x8`, que mapeia os caracteres ASCII para desenhar as letras pixel por pixel no controlador SSD1306 (endereço `0x3C`).
* O display é dividido em 8 "páginas" horizontais e 128 colunas, permitindo posicionar o cursor com `oled_set_cursor` e escrever textos de forma instantânea.

---

## Loop principal

A função `app_main()` inicializa os periféricos e entra em um loop infinito (`while(1)`) protegido por um delay do FreeRTOS (`vTaskDelay`) para não travar o Watchdog do processador.

1. Verifica se a variável `modo_fixo_90` está ativa ou não.
2. Modo Manual (False): Lê o ADC, calcula o ângulo (0-180), atualiza o PWM do motor e escreve as informações no display OLED.
3. Modo Fixo (True): Ignora o ADC, trava o ângulo em 90 graus, calcula o duty cycle correspondente e avisa no display que o modo está bloqueado.

---

## Resultados

Esquemático do circuito montado na plataforma Wokwi (https://wokwi.com/projects/468828941190292481`)

<img width="569" height="373" alt="proj3_2" src="https://github.com/user-attachments/assets/4308b6aa-2560-4103-8539-b1bd35216e96" />

Exemplo de funcionamento 

<img width="667" height="327" alt="proj3_2_1" src="https://github.com/user-attachments/assets/0430d0d5-6bd0-4054-8f34-08f945f16c95" />

