#include <kernel/ps2.h>
#include <kernel/acpi.h>
#include <kernel/io_utils.h>
#include <kernel/pit_timer.h>
#include <stdio.h>

#define TIMEOUT_DURATION 50

#define PS2_DATA_PORT 0x60
#define PS2_CMD_PORT 0x64
#define ACK_CMD 0xFA
#define RESET_CMD 0xFF
#define IDENIFY_CMD 0xF2
#define SELFTESTPASSED_CMD 0xAA
#define DISABLESCANING_CMD 0xF5
#define ENABLESCANING_CMD 0xF4


static uint8_t PS2Controller_Exists(){
    struct FADT *fadt = (void*) getFADT();
    if(fadt){
        printf("%x\n", fadt->BootArchitectureFlags);
        if(fadt->BootArchitectureFlags & 0x2)
            return 1;
    }
    return 0;
}

static uint8_t PS2Controller_WaitForWrite(){
    uint8_t status = inb(PS2_CMD_PORT);
    pit_timeout_set(TIMEOUT_DURATION);
    while(status & 2){
        io_wait();
        status = inb(PS2_CMD_PORT);
        if(pit_timeout_reached())
        return 1;
    }
    pit_timeout_unset();
    return 0;
}

static uint8_t PS2Controller_WaitForRead(){
    uint8_t status = inb(PS2_CMD_PORT);
    pit_timeout_set(TIMEOUT_DURATION);
    while(!(status & 1)){
        io_wait();
        status = inb(PS2_CMD_PORT);
        if(pit_timeout_reached())
            return 1;
    }
    pit_timeout_unset();
    return 0;
}

void PS2Controller_Command(uint8_t comm){
    if(PS2Controller_WaitForWrite()==0){
        outb(PS2_CMD_PORT, comm);
    }else{
        printf("PS/2: Command %x failed!\n", comm);
    }
    io_wait();
}

uint8_t PS2Controller_Read(){
    if(PS2Controller_WaitForRead()==0){
        return inb(PS2_DATA_PORT);
    }
    else{
         printf("PS/2: Nothing to read!\n");
        return 0;
    }
}

void PS2Controller_Write(uint8_t data){
    if(PS2Controller_WaitForWrite()==0){
        outb(PS2_DATA_PORT, data);
    }else{
        printf("PS/2: Data writing: %x failed!\n");
    }
}

void PS2Controller_WriteSecondPort(uint8_t data){
    PS2Controller_Command(0xD4);
    PS2Controller_Write(data);
}

void PS2Controller_InstallDriver(uint8_t second_port){
    uint8_t result, identity=0, identity2=0;

    second_port ? PS2Controller_Write(DISABLESCANING_CMD) : PS2Controller_WriteSecondPort(DISABLESCANING_CMD);

    if((result = PS2Controller_Read()) != ACK_CMD){
        printf("PS/2: Disable scanning result: %x (port %d)\n", result, (second_port!=0?2:1));
        //return 0;
    }

    second_port ? PS2Controller_Write(IDENIFY_CMD) : PS2Controller_WriteSecondPort(IDENIFY_CMD);

    if((result = PS2Controller_Read()) != ACK_CMD){
        printf("PS/2: Identify error (port %d)\n", result, (second_port!=0?2:1));
        //return 0;
    }

    identity = PS2Controller_Read();
    identity2 = PS2Controller_Read();
    printf("PS/2: Identity response: %x, %x (port %d)\n", identity, identity2, (second_port!=0?2:1));

    switch(identity){
        case 0xAB: //keyboard
            
        break;
        case 0x00: // mouse

        break;
    }

    second_port ? PS2Controller_Write(ENABLESCANING_CMD) : PS2Controller_WriteSecondPort(ENABLESCANING_CMD);
}

uint8_t PS2Controller_Init(){
    uint8_t configuration, result;
    uint8_t dualchannel = 1;
    uint8_t port1device = 0;

    // if(!PS2Controller_Exists()){
    //     printf("Device does not have PS/2 Controller!\n");
    //     return 1;
    // }

    PS2Controller_Command(0xAD);// disable first PS/2 port
    PS2Controller_Command(0xA7); // disable second PS/2 port

    PS2Controller_Command(0x20); // command read configuration
    configuration = PS2Controller_Read();
    configuration &= 0xBC; // clear bits 0,1,6 (disable interrupts, and first port translation)
    if(!(configuration & 0x20))
        dualchannel =0;
    
    PS2Controller_Command(0x60); // Set configuration
    PS2Controller_Write(configuration);

    PS2Controller_Command(SELFTESTPASSED_CMD); // Perform self test
    result = PS2Controller_Read();
    if(result != 0x55){
        printf("PS/2: Self test failed!\n");
        return 1;
    }

    if(dualchannel){
        PS2Controller_Command(0xA8); // Enable second PS/2 port
        PS2Controller_Command(0x20); // Command read configuration
        PS2Controller_WaitForRead();
        configuration = PS2Controller_Read();
        if(configuration & 0x20)
            dualchannel = 0;

        PS2Controller_Command(0xA7); // Disable second PS/2 port again  
    }    

    // Perform Interface Tests
    PS2Controller_Command(0xAB);
    PS2Controller_WaitForRead();
    result = PS2Controller_Read();
    if(result != 0){
        printf("PS/2: First port test failed!\n");
        return 1;
    }
    if(dualchannel){
        // Perform Interface Tests
        PS2Controller_Command(0xAB);
        result = PS2Controller_Read();
        if(result != 0){
            printf("PS/2: Second port test failed!\n");
            dualchannel = 0;
        }
    }
    printf("PS/2: Controller tests performed successfully.\n");

    PS2Controller_Command(0xAE);
    if(dualchannel){
        PS2Controller_Command(0xA8);
    }

    PS2Controller_Command(0x20); // command read configuration
    configuration = PS2Controller_Read();
    configuration |= dualchannel ? 0x3 : 0x1; //enable interrupts

    PS2Controller_Command(0x60); // Set configuration
    PS2Controller_Write(configuration);

    //reset first device
    printf("PS/2: Send reset command to device (port 1).\n");
    PS2Controller_Write(RESET_CMD);
    if((result=PS2Controller_Read()) == ACK_CMD){
        printf("PS/2: Device on port 1 succesfully reseted.\n");
        port1device = 1;
        if((result=PS2Controller_Read()) == SELFTESTPASSED_CMD){
            printf("PS/2: Device on port 1 self test passed.\n");
        }
    }else if(result == 0){
        printf("No device on first port!\n");
    }
    else{
        printf("PS/2: Device on port 1 after reset returned %x.\n");
    }

    //reset second device
    if(dualchannel){
        printf("PS/2: Send reset command to devices (port 1).\n");
        PS2Controller_WriteSecondPort(RESET_CMD);
        if((result = PS2Controller_Read()) == ACK_CMD){
            printf("PS/2: Device on port 2 succesfully reseted.\n");
            if((result=PS2Controller_Read()) == SELFTESTPASSED_CMD){
                printf("PS/2: Device on port 2 self test passed.\n");
            }
        }else if(result == 0){
            dualchannel = 0;
            printf("No device on second port!\n");
        }else{
            dualchannel = 0;
            printf("PS/2: Device returned %x.\n");
        }   
    }

    PS2Controller_Read(); //flush

    if(port1device){
        PS2Controller_InstallDriver(0);
    }

    if(dualchannel){
        PS2Controller_InstallDriver(1);
    }

    return 0;
}