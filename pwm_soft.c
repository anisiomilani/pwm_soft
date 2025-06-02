//#############################################################################
//
// ARQUIVO:    ex1_pwm_physical_software_control.c
//
// T�TULO:    Gera��o de PWM por Software e Configura��o Simulada
//
//! Este exemplo gera um PWM por software e simula sua configura��o via registrador.
//! Observar o brilho do LED e vari�veis no depurador do CCS.
//
//#############################################################################
//
// $Data de Lan�amento: $
// $Copyright:
// Copyright (C) 2013-2024 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribui��o e uso em formatos de c�digo-fonte e bin�rios, com ou sem
// modifica��o, s�o permitidos desde que as seguintes condi��es sejam
// atendidas:
//
//   As redistribui��es do c�digo-fonte devem reter o aviso de direitos autorais
//   acima, esta lista de condi��es e a seguinte isen��o de responsabilidade.
//
//   As redistribui��es em formato bin�rio devem reproduzir o aviso de direitos autorais
//   acima, esta lista de condi��es e a seguinte isen��o de responsabilidade na
//   documenta��o e/ou outros materiais fornecidos com a distribui��o.
//
//   Nem o nome da Texas Instruments Incorporated nem os nomes de
//   seus colaboradores podem ser usados para endossar ou promover produtos derivados
//   deste software sem permiss�o pr�via por escrito.
//
// ESTE SOFTWARE � FORNECIDO PELOS DETENTORES DOS DIREITOS AUTORAIS E COLABORADORES
// "AS IS" E QUAISQUER GARANTIAS EXPRESSAS OU IMPL�CITAS, INCLUINDO, MAS N�O
// SE LIMITANDO A, AS GARANTIAS IMPL�CITAS DE COMERCIALIZA��O E ADEQUA��O PARA
// UM PROP�SITO ESPEC�FICO S�O REJEITADAS. EM NENHUM CASO O DETENTOR DOS DIREITOS AUTORAIS
// OU COLABORADORES SER�O RESPONS�VEIS POR QUAISQUER DANOS DIRETOS, INDIRETOS, INCIDENTAIS,
// ESPECIAIS, EXEMPLARES OU CONSEQUENCIAIS (INCLUINDO, MAS N�O SE LIMITANDO A,
// AQUISI��O DE BENS OU SERVI�OS SUBSTITUTOS; PERDA DE USO, DATA OU LUCROS;
// OU INTERRUP��O DE NEG�CIOS) SEJA QUAL FOR A CAUSA E SOB QUALQUER TEORIA DE
// RESPONSABILIDADE, SEJA EM CONTRATO, RESPONSABILIDADE ESTRITA OU ATO IL�CITO
// (INCLUINDO NEGLIG�NCIA OU OUTRO) DECORRENTE DE QUALQUER FORMA DO USO DESTE
// SOFTWARE, MESMO SE AVISADO DA POSSIBILIDADE DE TAL DANO.
// $
//#############################################################################

// Arquivos Inclu�dos
#include "driverlib.h"
#include "device.h"

// --- Defini��es ---
#define LED_GPIO_PIN        31U     // GPIO do LED2 (Azul) na LaunchPadXL

#define PWM_COMPARE_MASK    0x03FFU // M�scara para bits 0-9 (valor de compara��o)
#define PWM_ENABLE_BIT      (1U << 10) // Bit 10: habilita PWM
#define PWM_INVERT_BIT      (1U << 11) // Bit 11: inverte sa�da

#define PWM_PERIOD_US       1000U   // Per�odo total do PWM em microssegundos

// Vari�veis Globais (Observar no depurador)
unsigned int g_pwmControlReg = 0x0000U; // Registrador de controle PWM simulado
float g_dutyCyclePercent = 50.0F;       // Ciclo de trabalho desejado (0.0 a 100.0)
unsigned int g_timeOn_us;               // Tempo LIGADO (LED ON)
unsigned int g_timeOff_us;              // Tempo DESLIGADO (LED OFF)
int g_PWMDuty =0;

// Prot�tipos de Fun��es
void initSystemPeripherals(void);
void initLEDGPIO(void);
void enablePWM(void);
void disablePWM(void);
void calculatePWMOnOffTimes(unsigned int compareValue);
unsigned int calculateCompareValueFromDutyCycle(float dutyCycle);
void setPWMDutyCycleAndRegister(float dutyCycle);
void generateSoftwarePWM(void);


// Fun��o Principal
void main(void)
{
    initSystemPeripherals();
    initLEDGPIO();

    // Configura o ciclo de trabalho inicial e o registrador simulado
    setPWMDutyCycleAndRegister(g_dutyCyclePercent);

    // Loop infinito para gerar o PWM
    for(;;)
    {
        setPWMDutyCycleAndRegister(g_dutyCyclePercent);
        generateSoftwarePWM();

        if (g_PWMDuty ==1){
             enablePWM();
          }
           else{
               disablePWM();
           }
    }


}





// Implementa��es de Fun��es

void initSystemPeripherals(void)
{
    Device_init();
    Device_initGPIO();
    enablePWM();
    Interrupt_initModule();
    Interrupt_initVectorTable();
    EINT; // Habilita Interrup��es Globais
    ERTM; // Habilita Depura��o em Tempo Real
}

void initLEDGPIO(void)
{
    GPIO_setPadConfig(LED_GPIO_PIN, GPIO_PIN_TYPE_STD);
    GPIO_setDirectionMode(LED_GPIO_PIN, GPIO_DIR_MODE_OUT);
    GPIO_writePin(LED_GPIO_PIN, 1); // LED inicia desligado (ativo baixo)
}

void enablePWM(void)
{
    g_pwmControlReg = g_pwmControlReg | PWM_ENABLE_BIT;
}

void disablePWM(void)
{
    g_pwmControlReg = g_pwmControlReg & (~PWM_ENABLE_BIT);
}

// Calcula tempos ON/OFF a partir do valor de compara��o do registrador.
void calculatePWMOnOffTimes(unsigned int compareValue)
{
    g_timeOn_us = compareValue;
    g_timeOff_us = PWM_PERIOD_US - g_timeOn_us;
}

// Converte ciclo de trabalho (%) para valor de compara��o (0 a PWM_PERIOD_US).
unsigned int calculateCompareValueFromDutyCycle(float dutyCycle)
{
    if (dutyCycle < 0.0F) dutyCycle = 0.0F;
    else if (dutyCycle > 100.0F) dutyCycle = 100.0F;
    return (unsigned int)((dutyCycle / 100.0F) * PWM_PERIOD_US);
}

// Configura ciclo de trabalho e atualiza registrador simulado e tempos ON/OFF.
void setPWMDutyCycleAndRegister(float dutyCycle)
{
    g_dutyCyclePercent = dutyCycle;

    unsigned int compareVal = calculateCompareValueFromDutyCycle(dutyCycle);

    unsigned int currentConfigBits = g_pwmControlReg & (~PWM_COMPARE_MASK);
    g_pwmControlReg = currentConfigBits | (compareVal & PWM_COMPARE_MASK);

    calculatePWMOnOffTimes(compareVal);
}

// Gera um ciclo da onda PWM por software no pino do LED.
void generateSoftwarePWM(void)
{
    if ((g_pwmControlReg & PWM_ENABLE_BIT) != 0U) // Se PWM habilitado
    {
        if ((g_pwmControlReg & PWM_INVERT_BIT) != 0U) // L�gica INVERTIDA
        {
            GPIO_writePin(LED_GPIO_PIN, 0); // Pino HIGH (LED OFF)
            DEVICE_DELAY_US(g_timeOn_us);

            GPIO_writePin(LED_GPIO_PIN, 1); // Pino LOW (LED ON)
            DEVICE_DELAY_US(g_timeOff_us);
        }
        else // L�gica NORMAL
        {
            GPIO_writePin(LED_GPIO_PIN, 0); // Pino LOW (LED ON)
            DEVICE_DELAY_US(g_timeOn_us);

            GPIO_writePin(LED_GPIO_PIN, 1); // Pino HIGH (LED OFF)
            DEVICE_DELAY_US(g_timeOff_us);
        }
    }
    else // PWM desabilitado
    {
        GPIO_writePin(LED_GPIO_PIN, 1); // LED OFF
        DEVICE_DELAY_US(PWM_PERIOD_US); // Aguarda per�odo completo
    }
}
