#include <EEPROM.h>
#define RESET_EEPROM true
void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  Serial.println("");
  
  if ( RESET_EEPROM ) {
    for (int i = 0; i < 512; i++) {
      EEPROM.write(i, 0);
    }
    EEPROM.commit();
    delay(500);
  }

}

void loop() {
  
}
