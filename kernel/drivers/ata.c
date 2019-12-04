#include <kernel/ata.h>
#include <stdio.h>
#include <kernel/pit_timer.h>
#include <kernel/io_utils.h>

// Channels:
#define      ATA_PRIMARY      0x00
#define      ATA_SECONDARY    0x01

// ATA Standard:
#define IDE_ATA        0x00
#define IDE_ATAPI      0x01

// Interface type:
#define ATA_MASTER     0x00
#define ATA_SLAVE      0x01

// Adress mode:
#define LBA28 0x01
#define LBA48 0x02
#define CHS   0x00

// Status: 
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

// Errors:
#define ATA_ER_BBK      0x80    // Bad block
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // Media changed
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // Media change request
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

// Commands:
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

// Ident offesets:
#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

// Registers:
#define ATA_REG_DATA       0x00
#define ATA_REG_ERROR      0x01
#define ATA_REG_FEATURES   0x01
#define ATA_REG_SECCOUNT0  0x02
#define ATA_REG_LBA0       0x03
#define ATA_REG_LBA1       0x04
#define ATA_REG_LBA2       0x05
#define ATA_REG_HDDEVSEL   0x06
#define ATA_REG_COMMAND    0x07
#define ATA_REG_STATUS     0x07
#define ATA_REG_SECCOUNT1  0x08
#define ATA_REG_LBA3       0x09
#define ATA_REG_LBA4       0x0A
#define ATA_REG_LBA5       0x0B
#define ATA_REG_CONTROL    0x0C
#define ATA_REG_ALTSTATUS  0x0C
#define ATA_REG_DEVADDRESS 0x0D

struct IDEChannelRegisters {
   uint16_t base;  // I/O Base.
   uint16_t ctrl;  // Control Base
   uint16_t bmide; // Bus Master IDE
   uint8_t nIEN;  // nIEN (No Interrupt);
} channels[2];

uint8_t ide_buf[2048] = {0};
static uint8_t ide_irq_invoked = 0;

struct ide_device {
   uint8_t  reserved;      // 0 (Empty) or 1 (This Drive really exists).
   uint8_t  channel;       // 0 (Primary Channel) or 1 (Secondary Channel).
   uint8_t  drive;         // 0 (Master Drive) or 1 (Slave Drive).
   uint16_t type;          // 0: ATA, 1:ATAPI.
   uint16_t signature;     // Drive Signature
   uint16_t capabilities;  // Features.
   uint32_t commandSets; // Command Sets Supported.
   uint32_t size;        // Size in Sectors.
   uint8_t  model[41];     // Model in string.
} ide_devices[4];


void ide_write(uint8_t channel, uint8_t reg, uint8_t data) {
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
   if (reg < 0x08)
      outb(channels[channel].base  + reg - 0x00, data);
   else if (reg < 0x0C)
      outb(channels[channel].base  + reg - 0x06, data);
   else if (reg < 0x0E)
      outb(channels[channel].ctrl  + reg - 0x0A, data);
   else if (reg < 0x16)
      outb(channels[channel].bmide + reg - 0x0E, data);
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

uint8_t ide_read(uint8_t channel, uint8_t reg) {
   uint8_t result = 0;
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
   if (reg < 0x08)
      result = inb(channels[channel].base + reg - 0x00);
   else if (reg < 0x0C)
      result = inb(channels[channel].base  + reg - 0x06);
   else if (reg < 0x0E)
      result = inb(channels[channel].ctrl  + reg - 0x0A);
   else if (reg < 0x16)
      result = inb(channels[channel].bmide + reg - 0x0E);
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
   return result;
}

uint8_t ide_readw(uint8_t channel, uint8_t reg) {
   uint8_t result = 0;
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
   if (reg < 0x08)
      result = inw(channels[channel].base + reg - 0x00);
   else if (reg < 0x0C)
      result = inw(channels[channel].base  + reg - 0x06);
   else if (reg < 0x0E)
      result = inw(channels[channel].ctrl  + reg - 0x0A);
   else if (reg < 0x16)
      result = inw(channels[channel].bmide + reg - 0x0E);
   if (reg > 0x07 && reg < 0x0C)
      ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
   return result;
}

uint8_t ide_polling(uint8_t channel, uint32_t advanced_check) {
 
   // (I) Delay 400 nanosecond for BSY to be set:
   // -------------------------------------------------
   for(int i = 0; i < 4; i++)
      ide_read(channel, ATA_REG_ALTSTATUS); // Reading the Alternate Status port wastes 100ns; loop four times.
 
   // (II) Wait for BSY to be cleared:
   // -------------------------------------------------
   while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY)
      ; // Wait for BSY to be zero.
 
   if (advanced_check) {
      uint8_t state = ide_read(channel, ATA_REG_STATUS); // Read Status Register.
 
      // (III) Check For Errors:
      // -------------------------------------------------
      if (state & ATA_SR_ERR)
         return 2; // Error.
 
      // (IV) Check If Device fault:
      // -------------------------------------------------
      if (state & ATA_SR_DF)
         return 1; // Device Fault.
 
      // (V) Check DRQ:
      // -------------------------------------------------
      // BSY = 0; DF = 0; ERR = 0 so we should check for DRQ now.
      if ((state & ATA_SR_DRQ) == 0)
         return 3; // DRQ should be set
 
   }
 
   return 0; // No Error.
 
}

uint8_t ide_print_error(uint32_t drive, uint8_t err) {
   if (err == 0)
      return err;
 
   printf("IDE :");
   if (err == 1) {printf("- Device Fault\n     "); err = 19;}
   else if (err == 2) {
      uint8_t st = ide_read(ide_devices[drive].channel, ATA_REG_ERROR);
      if (st & ATA_ER_AMNF)   {printf("- No Address Mark Found\n     ");   err = 7;}
      if (st & ATA_ER_TK0NF)   {printf("- No Media or Media Error\n     ");   err = 3;}
      if (st & ATA_ER_ABRT)   {printf("- Command Aborted\n     ");      err = 20;}
      if (st & ATA_ER_MCR)   {printf("- No Media or Media Error\n     ");   err = 3;}
      if (st & ATA_ER_IDNF)   {printf("- ID mark not Found\n     ");      err = 21;}
      if (st & ATA_ER_MC)   {printf("- No Media or Media Error\n     ");   err = 3;}
      if (st & ATA_ER_UNC)   {printf("- Uncorrectable Data Error\n     ");   err = 22;}
      if (st & ATA_ER_BBK)   {printf("- Bad Sectors\n     ");       err = 13;}
   } else  if (err == 3)           {printf("- Reads Nothing\n     "); err = 23;}
     else  if (err == 4)  {printf("- Write Protected\n     "); err = 8;}
     else  if (err == 5) {printf("- Can't access ATAPI drive.\n     ");}
     else  if (err == 6) {printf("- No such drive.\n     ");}
     else  if (err == 7) {printf("- Tried to read ouside the disk.\n     ");}
   printf("- [%s %s] %s\n",
      (const char *[]){"Primary", "Secondary"}[ide_devices[drive].channel], // Use the channel as an index into the array
      (const char *[]){"Master", "Slave"}[ide_devices[drive].drive], // Same as above, using the drive
      ide_devices[drive].model);
 
   return err;
}

void ide_print_summary(){
   for (int i = 0; i < 4; i++){
      if (ide_devices[i].reserved == 1) {
         printf("IDE : Found %s Drive %dKB ID(%d) - %s\n",
            (const char *[]){"ATA", "ATAPI"}[ide_devices[i].type],         /* Type */
            ide_devices[i].size / 2,               /* Size */
            i,
            ide_devices[i].model
            );
      }
   }
}

void ide_initialize(uint32_t BAR0, uint32_t BAR1, uint32_t BAR2, uint32_t BAR3,
uint32_t BAR4) {
 
   size_t i, j, k, count = 0;
 
   // 1- Detect I/O Ports which interface IDE Controller:
   channels[ATA_PRIMARY  ].base  = (BAR0 & 0xFFFFFFFC) + 0x1F0 * (!BAR0);
   channels[ATA_PRIMARY  ].ctrl  = (BAR1 & 0xFFFFFFFC) + 0x3F6 * (!BAR1);
   channels[ATA_SECONDARY].base  = (BAR2 & 0xFFFFFFFC) + 0x170 * (!BAR2);
   channels[ATA_SECONDARY].ctrl  = (BAR3 & 0xFFFFFFFC) + 0x376 * (!BAR3);
   channels[ATA_PRIMARY  ].bmide = (BAR4 & 0xFFFFFFFC) + 0; // Bus Master IDE
   channels[ATA_SECONDARY].bmide = (BAR4 & 0xFFFFFFFC) + 8; // Bus Master IDE

      // 2- Disable IRQs:
   ide_write(ATA_PRIMARY  , ATA_REG_CONTROL, 2);
   ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);

      // 3- Detect ATA-ATAPI Devices:
   for (i = 0; i < 2; i++)
      for (j = 0; j < 2; j++) {
 
         uint8_t err = 0, type = IDE_ATA, status;
         ide_devices[count].reserved = 0; // Assuming that no drive here.
 
         // (I) Select Drive:
         ide_write(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4)); // Select Drive.
         sleep(1); // Wait 1ms for drive select to work.
 
         // (II) Send ATA Identify Command:
         ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
         sleep(1); // This function should be implemented in your OS. which waits for 1 ms.
                   // it is based on System Timer Device Driver.
 
         // (III) Polling:
         if (ide_read(i, ATA_REG_STATUS) == 0) continue; // If Status = 0, No Device.
 
         while(1) {
            status = ide_read(i, ATA_REG_STATUS);
            if ((status & ATA_SR_ERR)) {err = 1; break;} // If Err, Device is not ATA.
            if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) break; // Everything is right.
         }
 
         // (IV) Probe for ATAPI Devices:
 
         if (err != 0) {
            uint8_t cl = ide_read(i, ATA_REG_LBA1);
            uint8_t ch = ide_read(i, ATA_REG_LBA2);
 
            if (cl == 0x14 && ch ==0xEB)
               type = IDE_ATAPI;
            else if (cl == 0x69 && ch == 0x96)
               type = IDE_ATAPI;
            else
               continue; // Unknown Type (may not be a device).
 
            ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
            sleep(1);
         }
 
         // (V) Read Identification Space of the Device:
        for(size_t l = 0; l<256; l++)
		{
			*(uint16_t *)(ide_buf + l*2) = ide_readw(i, ATA_REG_DATA);
		}
 
         // (VI) Read Device Parameters:
         ide_devices[count].reserved     = 1;
         ide_devices[count].type         = type;
         ide_devices[count].channel      = i;
         ide_devices[count].drive        = j;
         ide_devices[count].signature    = *((uint16_t *)(ide_buf + ATA_IDENT_DEVICETYPE));
         ide_devices[count].capabilities = *((uint16_t *)(ide_buf + ATA_IDENT_CAPABILITIES));
         ide_devices[count].commandSets  = *((uint32_t *)(ide_buf + ATA_IDENT_COMMANDSETS));
 
         // (VII) Get Size:
         if (ide_devices[count].commandSets & (1 << 26))
            // Device uses 48-Bit Addressing:
            ide_devices[count].size   = *((uint32_t *)(ide_buf + ATA_IDENT_MAX_LBA_EXT));
         else
            // Device uses CHS or 28-bit Addressing:
            ide_devices[count].size   = *((uint32_t *)(ide_buf + ATA_IDENT_MAX_LBA));
 
         // (VIII) String indicates model of device (like Western Digital HDD and SONY DVD-RW...):
         for(k = 0; k < 40; k += 2) {
            ide_devices[count].model[k] = ide_buf[ATA_IDENT_MODEL + k + 1];
            ide_devices[count].model[k + 1] = ide_buf[ATA_IDENT_MODEL + k];}
         ide_devices[count].model[40] = 0; // Terminate String.
 
         count++;
      }

   ide_print_summary();
}

uint8_t ide_ata_access(uint8_t direction, uint8_t drive, uint32_t lba, 
   uint32_t numsects, uint16_t * data) {
   uint8_t lba_mode , cmd;
   uint8_t lba_io[6];
   uint32_t  channel      = ide_devices[drive].channel; // Read the Channel.
   uint32_t  slavebit     = ide_devices[drive].drive; // Read the Drive [Master/Slave]
   uint32_t  bus          = channels[channel].base; // Bus Base, like 0x1F0 which is also data port.
   uint32_t  words        = 256; // Almost every ATA drive has a sector-size of 512-byte.
   uint16_t cyl, i;
   uint8_t head, sect, err;
   
   err=0;

   if(ide_devices[drive].type == IDE_ATAPI){
      err=5;
   }

   if(ide_devices[drive].reserved == 0 ){
      err=6;
   }

   if(lba + numsects> ide_devices[drive].size){
      err=7;
   }

   if(err){
      return ide_print_error(drive, err);
   }

   ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN = (ide_irq_invoked = 0x0) + 0x02);


      // (I) Select one from LBA28, LBA48 or CHS;
   if (lba >= 0x10000000) { // Sure Drive should support LBA in this case, or you are
                            // giving a wrong LBsA.
      lba_mode  = LBA48;
      lba_io[0] = (lba & 0x000000FF) >> 0;
      lba_io[1] = (lba & 0x0000FF00) >> 8;
      lba_io[2] = (lba & 0x00FF0000) >> 16;
      lba_io[3] = (lba & 0xFF000000) >> 24;
      lba_io[4] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
      lba_io[5] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
      head      = 0; // Lower 4-bits of HDDEVSEL are not used here.
   } else if (ide_devices[drive].capabilities & 0x200)  { // Drive supports LBA?
      // LBA28:
      lba_mode  = LBA28;
      lba_io[0] = (lba & 0x00000FF) >> 0;
      lba_io[1] = (lba & 0x000FF00) >> 8;
      lba_io[2] = (lba & 0x0FF0000) >> 16;
      lba_io[3] = 0; // These Registers are not used here.
      lba_io[4] = 0; // These Registers are not used here.
      lba_io[5] = 0; // These Registers are not used here.
      head      = (lba & 0xF000000) >> 24;
   } else {
      // CHS:
      lba_mode  = CHS;
      sect      = (lba % 63) + 1;
      cyl       = (lba + 1  - sect) / (16 * 63);
      lba_io[0] = sect;
      lba_io[1] = (cyl >> 0) & 0xFF;
      lba_io[2] = (cyl >> 8) & 0xFF;
      lba_io[3] = 0;
      lba_io[4] = 0;
      lba_io[5] = 0;
      head      = (lba + 1  - sect) % (16 * 63) / (63); // Head number is written to HDDEVSEL lower 4-bits.
   }

      // (III) Wait if the drive is busy;
   while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY); // Wait if busy.

   // (IV) Select Drive from the controller;
   if (lba_mode == 0)
      ide_write(channel, ATA_REG_HDDEVSEL, 0xA0 | (slavebit << 4) | head); // Drive & CHS.
   else
      ide_write(channel, ATA_REG_HDDEVSEL, 0xE0 | (slavebit << 4) | head); // Drive & LBA

      // (V) Write Parameters;
   if (lba_mode == 2) {
      ide_write(channel, ATA_REG_SECCOUNT1,   0);
      ide_write(channel, ATA_REG_LBA3,   lba_io[3]);
      ide_write(channel, ATA_REG_LBA4,   lba_io[4]);
      ide_write(channel, ATA_REG_LBA5,   lba_io[5]);
   }
   ide_write(channel, ATA_REG_SECCOUNT0,   numsects);
   ide_write(channel, ATA_REG_LBA0,   lba_io[0]);
   ide_write(channel, ATA_REG_LBA1,   lba_io[1]);
   ide_write(channel, ATA_REG_LBA2,   lba_io[2]);

   if (lba_mode == 0 && direction == ATA_READ) cmd = ATA_CMD_READ_PIO;
   if (lba_mode == 1 && direction == ATA_READ) cmd = ATA_CMD_READ_PIO;   
   if (lba_mode == 2 && direction == ATA_READ) cmd = ATA_CMD_READ_PIO_EXT;

   if (lba_mode == 0 && direction == ATA_WRITE) cmd = ATA_CMD_WRITE_PIO;
   if (lba_mode == 1 && direction == ATA_WRITE) cmd = ATA_CMD_WRITE_PIO;
   if (lba_mode == 2 && direction == ATA_WRITE) cmd = ATA_CMD_WRITE_PIO_EXT;
   ide_write(channel, ATA_REG_COMMAND, cmd);

   if (direction == 0){
      // PIO Read.
      for (i = 0; i < numsects; i++) {
         if ((err = ide_polling(channel, 1)))
            return err; // Polling, set error and exit if there is.
         for(size_t l = 0; l<words; l++)
         {
            *(data + l) = inw(bus);
         }
      }
   } else {
   // PIO Write.
      for (i = 0; i < numsects; i++) {
        for(size_t l = 0; l<words; l++)
         {
            outw(bus, *(data + l));
         }
      }
      ide_write(channel, ATA_REG_COMMAND, (char []) {   ATA_CMD_CACHE_FLUSH,
                     ATA_CMD_CACHE_FLUSH,
                     ATA_CMD_CACHE_FLUSH_EXT}[lba_mode]);
      ide_polling(channel, 0); // Polling.
   }
 
   return 0;
}