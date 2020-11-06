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

int set_device(int cod, int outp[]){
    switch(cod){
        case COD_LAMP_1:{
            bcm2835_gpio_write(LAMP_1, 1-outp[0]);
            break;
        }
        case COD_LAMP_2:{
            bcm2835_gpio_write(LAMP_2, 1-outp[1]);
            break;
        }
        case COD_LAMP_3:{
            bcm2835_gpio_write(LAMP_3, 1-outp[2]);
            break;
        }
        case COD_LAMP_4:{
            bcm2835_gpio_write(LAMP_4, 1-outp[3]);
            break;
        }
        case COD_AIR_1:{
            bcm2835_gpio_write(AIR_1, 1-outp[4]);
            break;
        }
        case COD_AIR_2:{
            bcm2835_gpio_write(AIR_2, 1-outp[5]);
            break;
        }
        default:{
            return -2;
            break;
        }
    }

    return 0;
}

void get_gpio_all(int inpt[], int outp[]){
    get_gpio_inpt(inpt);
    get_gpio_outp(outp);
}

void get_gpio_inpt(int inpt[]){
    inpt[0] = bcm2835_gpio_lev(PRESENCE_SENSOR_1);
    inpt[1] = bcm2835_gpio_lev(PRESENCE_SENSOR_2);
    
    inpt[2] = bcm2835_gpio_lev(OPENNING_SENSOR_1);
    inpt[3] = bcm2835_gpio_lev(OPENNING_SENSOR_2);
    inpt[4] = bcm2835_gpio_lev(OPENNING_SENSOR_3);
    inpt[5] = bcm2835_gpio_lev(OPENNING_SENSOR_4);
    inpt[6] = bcm2835_gpio_lev(OPENNING_SENSOR_5);
    inpt[7] = bcm2835_gpio_lev(OPENNING_SENSOR_6);
}

void get_gpio_outp(int outp[]){
    outp[0] = bcm2835_gpio_lev(LAMP_1);
    outp[1] = bcm2835_gpio_lev(LAMP_2);
    outp[2] = bcm2835_gpio_lev(LAMP_3);
    outp[3] = bcm2835_gpio_lev(LAMP_4);

    outp[4] = bcm2835_gpio_lev(AIR_1);
    outp[5] = bcm2835_gpio_lev(AIR_2);
}

int get_gpio_change(int inpt[], int outp[]){
    // inpt é prioridade por causa do alarme, entao vai primeiro
    if(inpt[0] != bcm2835_gpio_lev(PRESENCE_SENSOR_1)){
        inpt[0] = bcm2835_gpio_lev(PRESENCE_SENSOR_1);
        return COD_PRESENCE_SENSOR_1;
    }
    if(inpt[1] != bcm2835_gpio_lev(PRESENCE_SENSOR_2)){
        inpt[1] = bcm2835_gpio_lev(PRESENCE_SENSOR_2);
        return COD_PRESENCE_SENSOR_2;
    }

    if(inpt[2] != bcm2835_gpio_lev(OPENNING_SENSOR_1)){
        inpt[2] = bcm2835_gpio_lev(OPENNING_SENSOR_1);
        return COD_OPENNING_SENSOR_1;
    }
    if(inpt[3] != bcm2835_gpio_lev(OPENNING_SENSOR_2)){
        inpt[3] = bcm2835_gpio_lev(OPENNING_SENSOR_2);
        return COD_OPENNING_SENSOR_2;
    }
    if(inpt[4] != bcm2835_gpio_lev(OPENNING_SENSOR_3)){
        inpt[4] = bcm2835_gpio_lev(OPENNING_SENSOR_3);
        return COD_OPENNING_SENSOR_3;
    }
    if(inpt[5] != bcm2835_gpio_lev(OPENNING_SENSOR_4)){
        inpt[5] = bcm2835_gpio_lev(OPENNING_SENSOR_4);
        return COD_OPENNING_SENSOR_4;
    }
    if(inpt[6] != bcm2835_gpio_lev(OPENNING_SENSOR_5)){
        inpt[6] = bcm2835_gpio_lev(OPENNING_SENSOR_5);
        return COD_OPENNING_SENSOR_5;
    }
    if(inpt[7] != bcm2835_gpio_lev(OPENNING_SENSOR_6)){
        inpt[7] = bcm2835_gpio_lev(OPENNING_SENSOR_6);
        return COD_OPENNING_SENSOR_6;
    }

    if(outp[0] != bcm2835_gpio_lev(LAMP_1)){
        outp[0] = bcm2835_gpio_lev(LAMP_1);
        return COD_LAMP_1;
    }
    if(outp[1] != bcm2835_gpio_lev(LAMP_2)){
        outp[1] = bcm2835_gpio_lev(LAMP_2);
        return COD_LAMP_2;
    }
    if(outp[2] != bcm2835_gpio_lev(LAMP_3)){
        outp[2] = bcm2835_gpio_lev(LAMP_3);
        return COD_LAMP_3;
    }
    if(outp[3] != bcm2835_gpio_lev(LAMP_4)){
        outp[3] = bcm2835_gpio_lev(LAMP_4);
        return COD_LAMP_4;
    }
      
    if(outp[4] != bcm2835_gpio_lev(AIR_1)){
        outp[4] = bcm2835_gpio_lev(AIR_1);
        return COD_AIR_1;
    }
    if(outp[5] != bcm2835_gpio_lev(AIR_2)){
        outp[5] = bcm2835_gpio_lev(AIR_2);
        return COD_AIR_2;
    }

    return 0x0;
}