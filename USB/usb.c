#include <stdlib.h>
#include <stdio.h>
#include <libusb-1.0/libusb.h>
#include <stdbool.h>

libusb_device_handle *handle;
struct libusb_config_descriptor *config;

void save_device(libusb_context* context)
{
    libusb_device **list;
    ssize_t count=libusb_get_device_list(context,&list);
    if(count<0) {perror("libusb_get_device_list"); exit(-1);}
    ssize_t i=0;
    bool find = false;
    while(i<count && !find){
        libusb_device *device=list[i];
        struct libusb_device_descriptor desc;
        int status=libusb_get_device_descriptor(device,&desc); 
        if(status!=0) continue;
        uint8_t bus=libusb_get_bus_number(device); // récupère le numéro du bus sur lequel le périphérique est connecté
        uint8_t address=libusb_get_device_address(device); // récupère l'adresse sur le bus
        printf("Device Found @ (Bus:Address) %d:%d\n",bus,address);
        printf("Vendor ID 0x%x\n",desc.idVendor);
        printf("Product ID 0x%x\n",desc.idProduct);
    
        // Recherche du bon périphérique
        uint16_t idVendor = 0x1234;
        uint16_t idProduct = 0x4321;
        if(desc.idVendor == idVendor && desc.idProduct == idProduct)
        {
            int status = libusb_open(device, &handle);
            find = true;
            if(status!=0){ perror("libusb_open"); exit(-1);}
        }
        i++;
    }
    libusb_free_device_list(list,1);
}

void config_usb_device(uint8_t endPoints[2][3])
{
    libusb_device *device = libusb_get_device(handle);
    
    // récupération de la config 0
    int status = libusb_get_config_descriptor(device, 0, &config);
    if(status!=0){ perror("libusb_get_config_descriptor"); exit(-1);}
    printf("Config : %x\n", config->bConfigurationValue);
    
    // affichage des interfaces
    printf("Number of interfaces : %x\n\n", config->bNumInterfaces);
    
    //uint8_t endPoints[2][3]; // 2 = config->bNumInterfaces
    int k;
    
    // détacher toutes les interfaces du noyau
    for(int i = 0; i<2; i++) // 2 = config->bNumInterfaces
    {
        // si le méchant noyau est passé avant nous
        printf("Num of interface %d : %x\n", i, config->interface[i].altsetting[0].bInterfaceNumber);
        if(libusb_kernel_driver_active(handle,config->interface[i].altsetting[0].bInterfaceNumber))
        {
            status=libusb_detach_kernel_driver(handle,config->interface[i].altsetting[0].bInterfaceNumber);
            if(status!=0){ perror("libusb_detach_kernel_driver"); exit(-1); }
        }
    }
    
    // utilisation de la config d'indice 0
    status=libusb_set_configuration(handle,config->bConfigurationValue);
    if(status!=0){ perror("libusb_set_configuration"); exit(-1); }
    
    // réclamation de toutes les interfaces
    for(int i = 0; i<2; i++) // 2 = config->bNumInterfaces
    {
        // réclammation de l'interface
        int status=libusb_claim_interface(handle,config->interface[i].altsetting[0].bInterfaceNumber);
        if(status!=0){ perror("libusb_claim_interface"); exit(-1); }
        
        // récupération des end points
        k = 0;
        printf("Number of endpoints for interface %d : %x\n", i, config->interface[i].altsetting[0].bNumEndpoints);
        for(int j=0; j<config->interface[i].altsetting[0].bNumEndpoints; j++)
        {
            if(k<2 && (config->interface[i].altsetting[0].endpoint[j].bmAttributes & 0x03) == LIBUSB_TRANSFER_TYPE_INTERRUPT)
            {
                endPoints[i][k] = config->interface[i].altsetting[0].endpoint[j].bEndpointAddress;
                //printf("%d, %d : Endpoint address : 0x%x\n", i, j, endPoints[i][k]);
                k++;
            }
        }
        endPoints[i][2] = k; // nombre de endpoint de type interruption (<3)
    }
}

void free_interfaces()
{
    struct libusb_config_descriptor *config_desc;
    libusb_device *device = libusb_get_device(handle);
    
    // récupération de la config active
    int status = libusb_get_active_config_descriptor(device, &config_desc);
    if(status!=0){ perror("libusb_get_config_descriptor"); exit(-1);}
    
    // relache des interfaces
    for(int i = 0; i<config_desc->bNumInterfaces; i++)
    {
        status=libusb_release_interface(handle,config_desc->interface[i].altsetting[0].bInterfaceNumber);
        if(status!=0){ perror("libusb_release_interface"); exit(-1); }
    }
    
    libusb_close(handle);
}

// fonction d'envoi de données par le bus USB
void Envoi(char c, int endpoint_out)
{
    unsigned char data[8] = {c}; // caractère à envoyer (correspond aux lettres pour allumer et éteindre les LEDs)
    int size = sizeof(data); // taille des données à envoyer
    int timeout = 100; // timeout (ms)
        
    // interruption OUT
    int bytes_out;
    int status = libusb_interrupt_transfer(handle,endpoint_out,data,size,&bytes_out,timeout);
    if(status!=0){perror("libusb_interrupt_transfer_S"); exit(-1);}
}

// fonction de réception de données par le bus USB
char Reception(int endpoint_in)
{
    unsigned char data[8]; // données reçues
    int size=sizeof(data); // taille des données reçues
    int timeout=100; // timeout (ms)

    // interruption IN
    int bytes_in;
    int status=libusb_interrupt_transfer(handle,endpoint_in,data,size,&bytes_in,timeout);
    if(status!=0){perror("libusb_interrupt_transfer_R"); exit(-1);}
    return data[0];
}

int main()
{
    printf("Start program\n");
    
    // récupération des endpoints
    libusb_context *context;
    int status = libusb_init(&context);
    if(status!=0) {perror("libusb_init"); exit(-1);}
    save_device(context);
    uint8_t endPoints[2][3];
    config_usb_device(endPoints);
    for(int i=0; i<2; i++)
        for(int j=0; j<endPoints[i][2]; j++)
            printf("Interface %d ; Endpoint %d ; Adresse 0x%x\n", i, j, endPoints[i][j]);
    
    // communication USB avec l'atmega16u2 -- non fonctionnelle
    /*
        unsigned char joystick;
    unsigned char buttons;
    char c;
    
    while (c != 'S')
    {
        c = getchar();
        Envoi(c, endPoints[1][0]);
        
        joystick = Reception(endPoints[0][1]);
        printf("%c\n",joystick);
        
        buttons = Reception(endPoints[1][1]);
        printf("%c\n",buttons);
        
    }
    */
    
    // libération des interfaces
    free_interfaces();
    
    // supression du context
    libusb_exit(context);
    printf("End program\n");
    return 0;
}
