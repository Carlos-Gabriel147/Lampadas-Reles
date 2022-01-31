#include <EEPROM.h>

void setup(){
    Serial.begin(57600);
    EEPROM.begin(4096);

    //Âncoras//
    EEPROM.write(456, 0);
    for(int c=457; c<488; c++){
        EEPROM.write(c, 0xff);
    }

    //Conjuntos//
    for(byte aa=0; aa<8; aa++){
        EEPROM.write(aa, 0xff);
    }

    for(int i=8; i<256; i++){
        EEPROM.write(i, 0x00);
    }

    EEPROM.commit();
}

void loop(){

    Serial.println("Âncoras:");
    for(int i=456; i<488; i++){
        Serial.print(EEPROM.read(i));
        Serial.print(" / ");
    }
    Serial.println();
    Serial.println();

    Serial.println("Conjuntos:");
    for(int i=0; i<249; i+=8){
        Serial.print(i/8+1);
        Serial.print(": ");
        for(int j=0; j<8; j++){
            Serial.print(EEPROM.read(i+j));
            Serial.print(" / ");
        }
        Serial.println();
    }

    delay(10000);
}
