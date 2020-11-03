#include "gpio_utils.h"

int init_bcm2835(){
    // Initialize bcm2835
    if(!bcm2835_init()){
        // fprintf(stderr, "Erro na inicialização do bcm2835\n");
        return -1;
    };

    bcm2835_gpio_fsel(LAMP_1, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(LAMP_2, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(LAMP_3, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(LAMP_4, BCM2835_GPIO_FSEL_OUTP);

	bcm2835_gpio_fsel(AIR_1, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(AIR_2, BCM2835_GPIO_FSEL_OUTP);

	bcm2835_gpio_fsel(PRESENCE_SENSOR_1, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(PRESENCE_SENSOR_2, BCM2835_GPIO_FSEL_INPT);

	bcm2835_gpio_fsel(OPENNING_SENSOR_1, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(OPENNING_SENSOR_2, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_fsel(OPENNING_SENSOR_3, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(OPENNING_SENSOR_4, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_fsel(OPENNING_SENSOR_5, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(OPENNING_SENSOR_6, BCM2835_GPIO_FSEL_INPT);

    return 0;
}

int set_device(int key){
    int device = key & 0xF0;
    int value = key & 0x0F;
    if(value != HIGH && value != LOW){
        return -1;
    }
    switch(device){
        case COD_LAMP_1:{
            bcm2835_gpio_write(LAMP_1, value);
            break;
        }
        case COD_LAMP_2:{
            bcm2835_gpio_write(LAMP_2, value);
            break;
        }
        case COD_LAMP_3:{
            bcm2835_gpio_write(LAMP_3, value);
            break;
        }
        case COD_LAMP_4:{
            bcm2835_gpio_write(LAMP_4, value);
            break;
        }
        case COD_AIR_1:{
            bcm2835_gpio_write(AIR_1, value);
            break;
        }
        case COD_AIR_2:{
            bcm2835_gpio_write(AIR_2, value);
            break;
        }
        default:{
            return -2;
            break;
        }
    }

    return 0;
}