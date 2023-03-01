#include "debug_files.h"
#if DEBUG_FILE_FNS
#include "FS.h"
#include "SPIFFS.h"

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
int column = 0;
bool have_printed = 0;
   Serial.printf("Listing directory: %s\r\n", dirname);

   File root = fs.open(dirname);
   if(!root){
      Serial.println("− failed to open directory");
      return;
   }
   if(!root.isDirectory()){
      Serial.println(" − not a directory");
      return;
   }

   File file = root.openNextFile();
   while(file){
      if(file.isDirectory()){
         Serial.print("  DIR : ");
         Serial.println(file.name());
         if(levels){
            listDir(fs, file.name(), levels -1);
         }
      } else {
         Serial.printf("  FILE: %s\tSIZE: %d", file.name(), file.size());
	 Serial.printf("%s", column ? "\n" : "\t");
	 column ^= 1;
	 have_printed = !column;
      }
      file = root.openNextFile();
   }
   if(have_printed) Serial.println();
}

void my_dir(const char * dirname, uint8_t levels) {
    listDir(SPIFFS, dirname, levels);
}
#endif
