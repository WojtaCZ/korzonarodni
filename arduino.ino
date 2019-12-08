#include <TVout.h>
#include <fontALL.h>
#include <string.h>
#define DEVICE_ID 'a'

TVout TV;

//Promenne pro prichizi string a signalizaci kompletniho prijeti
String inputString = "";
bool stringComplete = false; 


void setup() {
  //Zapne se serial
  Serial.begin(230400);
  
  //Nahodi se PAL pro TV
  TV.begin(PAL, 120, 96);
  TV.select_font(font6x8);
  
  //Alokace pameti pro string
  inputString.reserve(200);
}

void loop() {
  //Pokud se prijal cely string, vypise se na TV
  if (stringComplete) {
    
    //Vypocita se delka stringu
    unsigned int out_len = inputString.length();

    //Ze stringu se vytvori char array
    char output[out_len];
    inputString.toCharArray(output,out_len);

    //Zpravy pro debug
    //Serial.println(inputString);
    //Serial.print("ID=");
    //Serial.print(output[0]);

    //Pokud jsme prijali zpravu s ID naseho zarizeni, vypiseme i na TV, pokud ne, ignoruje se
    if(output[0] == DEVICE_ID){
    
      //Vycisti se obrazovka
      TV.clear_screen();
      //Pomalu se vypise "animace psaci stroj"

      for(int i = 1; i < out_len; i++){   
        TV.print(output[i]);
        TV.delay(40);
      }
    }
      
    //Resetuji se promenne
    inputString = "";
    stringComplete = false;
    
  }
}




//SerialEvent se provede pokud prijdou na RX data
void serialEvent() {
  while (Serial.available() > 0) {
    //Nacte se byte
    char inChar = (char)Serial.read();
    //Prida se k vystupnimu stringu
    inputString += inChar;
    
    //Po null charakteru se nastavi flag na kompletni
    if (inChar == 0) {
      stringComplete = true;
      
    }

  }
}

