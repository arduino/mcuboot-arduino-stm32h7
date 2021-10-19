#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "FATFileSystem.h"

#ifndef CORE_CM7  
  #error Format QSPI flash by uploading the sketch to the M7 core instead of the M4 core.
#endif


QSPIFBlockDevice root(PD_11, PD_12, PF_7, PD_13,  PF_10, PG_6, QSPIF_POLARITY_MODE_1, 40000000);
mbed::MBRBlockDevice ota_data(&root, 2);
mbed::FATFileSystem ota_data_fs("fs");

void printBuffer(char *buf, int size) {
  int i;
  for(i = 0; i< size; i++)
  {
    Serial.print(buf[i], HEX);
  }
  Serial.println("");
}

void create_scratch_file(void) {
  FILE* fp = fopen("/fs/scratch.bin", "wb");
  char buffer[128] = {0xFF};
  int size=0;

  while(size < 128 * 1024) {
    fwrite(buffer, 128, 1, fp);
    size += 128;
  }

  fclose(fp);
}

void create_update_file(void) {
  FILE* fp = fopen("/fs/update.bin", "wb");
  char buffer[128] = {0xFF};
  int size=0;

  while(size < 768 * 1024) {
    fwrite(buffer, 128, 1, fp);
    size += 128;
  }

  fclose(fp);
}

void setup() {

  Serial.begin(115200);
  while (!Serial);
  
  mbed::MBRBlockDevice::partition(&root, 2, 0x0B, 1024 * 1024, 14 * 1024 * 1024);

  int err =  ota_data_fs.mount(&ota_data);
  if (err) {
    // Reformat if we can't mount the filesystem
    // this should only happen on the first boot
    Serial.println("No filesystem for OTA firmware was found, creating");
    err = ota_data_fs.reformat(&ota_data);
  }
  
  Serial.println("Format update file");
  create_update_file();
  Serial.println("Update DONE");

  Serial.println("Format scratch file");
  create_scratch_file();    
  Serial.println("Scratch DONE");
}

void loop() {

}
