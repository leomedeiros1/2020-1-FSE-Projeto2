#ifndef GPIO_UTILS_H
#define GPIO_UTILS_H

#include <bcm2835.h>

#define LAMP_1 RPI_V2_GPIO_P1_11  // Lâmpada 01 (Cozinha) - 17
#define LAMP_2 RPI_V2_GPIO_P1_12  // Lâmpada 02 (Sala) - 18
#define LAMP_3 RPI_V2_GPIO_P1_13  // Lâmpada 03 (Quarto 01) - 27
#define LAMP_4 RPI_V2_GPIO_P1_15  // Lâmpada 04 (Quarto 02) - 22
#define AIR_1 RPI_V2_GPIO_P1_16   // Ar-Condicionado 01 (Quarto 01) - 23
#define AIR_2 RPI_V2_GPIO_P1_18   // Ar-Condicionado 02 (Quarto 02) - 24
#define PRESENCE_SENSOR_1 RPI_V2_GPIO_P1_22 // Sensor de Presença 01 (Sala) - 25
#define PRESENCE_SENSOR_2 RPI_V2_GPIO_P1_37 // Sensor de Presença 02 (Cozinha) - 26
#define OPENNING_SENSOR_1 RPI_V2_GPIO_P1_29 // Sensor Abertura 01 (Porta Cozinha) - 05
#define OPENNING_SENSOR_2 RPI_V2_GPIO_P1_31 // Sensor Abertura 02 (Janela Cozinha) - 06
#define OPENNING_SENSOR_3 RPI_V2_GPIO_P1_32 // Sensor Abertura 03 (Porta Sala) - 12
#define OPENNING_SENSOR_4 RPI_V2_GPIO_P1_36 // Sensor Abertura 04 (Janela Sala) - 16
#define OPENNING_SENSOR_5 RPI_V2_GPIO_P1_38 // Sensor Abertura 05 (Janela Quarto 01) - 20
#define OPENNING_SENSOR_6 RPI_V2_GPIO_P1_40 // Sensor Abertura 06 (Janela Quarto 02) - 21

#define COD_LAMP_1 0x01
#define COD_LAMP_2 0x02  // Lâmpada 02 (Sala) - 18
#define COD_LAMP_3 0x03  // Lâmpada 03 (Quarto 01) - 27
#define COD_LAMP_4 0x04  // Lâmpada 04 (Quarto 02) - 22
#define COD_AIR_1 0x05   // Ar-Condicionado 01 (Quarto 01) - 23
#define COD_AIR_2 0x06   // Ar-Condicionado 02 (Quarto 02) - 24
#define COD_PRESENCE_SENSOR_1 0x10 // Sensor de Presença 01 (Sala) - 25
#define COD_PRESENCE_SENSOR_2 0x11 // Sensor de Presença 02 (Cozinha) - 26
#define COD_OPENNING_SENSOR_1 0x12 // Sensor Abertura 01 (Porta Cozinha) - 05
#define COD_OPENNING_SENSOR_2 0x13 // Sensor Abertura 02 (Janela Cozinha) - 06
#define COD_OPENNING_SENSOR_3 0x14 // Sensor Abertura 03 (Porta Sala) - 12
#define COD_OPENNING_SENSOR_4 0x15 // Sensor Abertura 04 (Janela Sala) - 16
#define COD_OPENNING_SENSOR_5 0x16 // Sensor Abertura 05 (Janela Quarto 01) - 20
#define COD_OPENNING_SENSOR_6 0x17 // Sensor Abertura 06 (Janela Quarto 02) - 21

int init_bcm2835();
int set_device(int cod, int outp[]);
void get_gpio_inpt(int inpt[]);
void get_gpio_outp(int outp[]);
void get_gpio_all(int inpt[], int outp[]);
int get_gpio_change(int inpt[], int outp[]);

#endif