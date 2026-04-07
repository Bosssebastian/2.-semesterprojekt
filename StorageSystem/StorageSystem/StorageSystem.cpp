#include <stdio.h>
#include "pico/stdlib.h"

#include "uartClass.h"



int main()
{
    stdio_init_all();

    

    sleep_ms(2000);
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    UartClass myUart(0, 0, 1, 115200);

    gpio_put(25, 1);

    printf("INITIALIZED\n");

    int choice;

    while (true) {
        
        
        //Requesting Data
        //uint8_t flag = UartClass::REQ;
        //uint8_t msgType = UartClass::GET_CURRENT;

        /*
        er lige igang med at lave menu system.
        std::cin >> choice;
        printf("Input: ");
        */
        uint8_t value = 0;
        
        bool ok = myUart.requestPackage(UartClass::GET_CURRENT, value);

        if (ok) {
            printf("Returned data = %d\n", value);
        } else {
            printf("requestPackage failed\n");
        }


        sleep_ms(100);
        
        
        
        /*
        //RESPONDER code
        uint8_t flag = 0;
        uint8_t msgType = 0;
        uint8_t data = 0;

        int dataCurrent = 30;
        int dataPosition = 50;

        myUart.readPackage(flag, msgType, data);

        if (flag != 0 || msgType != 0 || data != 0) {
            printf("Recieved: %d %d %d\n", flag, msgType, data);
        }

        if (flag == UartClass::REQ) {
            myUart.UartClass::writePackage(UartClass::ACK, msgType, 0);
            sleep_ms(10); //REMEMBER TO REMOVE

            if (msgType == UartClass::GET_CURRENT) {
                
                myUart.UartClass::writePackage(UartClass::RES, msgType, dataCurrent);
                printf("RETURNING CURRENT!\n");
            }

            if (msgType == UartClass::GET_POSITION) {
                
                myUart.UartClass::writePackage(UartClass::RES, msgType, dataPosition);
                printf("RETURNING POSITION!\n");
            }

            if (msgType == UartClass::CHANGE_POSITION) {
                
                myUart.UartClass::writePackage(UartClass::RES, msgType, 1);
                printf("CHANGING POSITION!\n");
            }
        
        
        }
        sleep_ms(100);
        */

        /*
        //Writing Data
        uint8_t flag = 1;
        uint8_t msgType = 2;
        uint8_t data = 3;

        myUart.writePackage(flag, msgType, data);

        sleep_ms(100);
        */

        /* 


        //READING DATA
        uint8_t flag = 0;
        uint8_t msgType = 0;
        uint8_t data = 0;

        myUart.readPackage(flag, msgType, data);

        if (flag != 0 || msgType != 0 || data != 0) {
            printf("Recieved: %d %d %d\n", flag, msgType, data);
        }

        sleep_ms(100);

        */

        /*
        sleep_ms(100);
        gpio_put(25, 1);
        sleep_ms(100);
        gpio_put(25, 0);
        myUart.sendPackage();
        myUart.readPackage();
        myUart.requestPackage();
        myUart.clearRXQue();
        */

    }
}
