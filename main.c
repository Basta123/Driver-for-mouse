#include <stdio.h>
#include <string.h>
#include <libusb-1.0/libusb.h>
#include <stdlib.h>

#define DEV_INTERFACE 0

#define VID 0x80ee
#define PID 0x0021


int main(int argc, char *argv[]) {

    libusb_init(NULL); // инициализируем LibUSB библиотеку

    struct libusb_device *dev; // создаем указатель на наше устройство
    struct libusb_device_handle *handle; // создаем указатель на обработчик устройства

    libusb_set_debug(NULL, 3);  // задаем уровень вывода отладочных сообщений

    // с помощью VID и PID получаем наше устройство
    if (handle = libusb_open_device_with_vid_pid(NULL, VID, PID)) printf("Device opened successfully.\n");
    else {
        printf("Operation failed(");
        exit(1);
    }

    dev = libusb_get_device(handle);

    struct libusb_device_descriptor descriptor;

    libusb_get_device_descriptor(dev, &descriptor);
    libusb_set_configuration(handle, 0);

    struct libusb_config_descriptor *ConfigDescriptor;
    libusb_get_config_descriptor(dev, 0, &ConfigDescriptor);
    libusb_set_auto_detach_kernel_driver(handle, 1);

    int r = 0;
// привязан ли драйвер к ОС libusb_kernel_driver_active / detach_kernel_driver
    r = libusb_kernel_driver_active(handle, DEV_INTERFACE);
    if (r == 0) printf("Kernel isn't Using Driver : %i\n", r);
    else printf("driver is attached to kernel : %i\n", r);

    r = libusb_detach_kernel_driver(handle, DEV_INTERFACE);
    if (r == 0) printf("Device detached successfully from the kernel : %i\n", r);
    else printf("Error detaching from the kernel : %i\n", r);
    libusb_set_configuration(handle, 0);

    // резервируем интерфейс девайса за нашим драйвером.
    // Если другой драйвер или программа используют наше устройство, то функция скажет, что интерфейс занят

    r = libusb_claim_interface(handle, 0);

    if (r == 0) {
        printf("Interface Claimed !!\n");
    }

    printf("Interface Claim Status : %d\n", r);
    printf("idProduct : %d\n", descriptor.idProduct);
    printf("idVendor : %d\n", descriptor.idVendor);
    printf("SerialNumber : %d\n", descriptor.iSerialNumber);
    printf("Device Protocol : %d\n", descriptor.bDeviceProtocol);
    printf("Report Length : %d\n", descriptor.bLength);
    printf("Decriptor Type : %d\n", descriptor.bDescriptorType);
    printf("NumConfigurations : %d\n", descriptor.bNumConfigurations);
    printf("NumInterfaces : %d\n", ConfigDescriptor->bNumInterfaces);
    printf("End Points : %d\n", ConfigDescriptor->interface->altsetting->bNumEndpoints);
    printf("End PointAddress : %d\n", ConfigDescriptor->interface->altsetting->endpoint->bEndpointAddress);
    printf("InterfaceProtocol : %d\n", ConfigDescriptor->interface->altsetting->bInterfaceProtocol);
    printf("InterfaceNumber : %d\n", ConfigDescriptor->interface->altsetting->bInterfaceNumber);
    printf("InterfaceClass : %d\n", ConfigDescriptor->interface->altsetting->bInterfaceClass);
    printf("InterfaceAlternateSetting : %d\n", ConfigDescriptor->interface->altsetting->bAlternateSetting);

    uint16_t packageSIze = ConfigDescriptor->interface->altsetting->endpoint->wMaxPacketSize;

    // libusb_get_string_descriptor_ascii(handle,descriptor.idProduct, string, sizeof(string));
    int numBytes = 4;
    int errCount = 0;
    int actual;
    unsigned char endPoint = 0x81;
    int i;
    int horiz = 5;
    int vert = 5;
    unsigned char data[packageSIze];

    while (1) {
        for (int i = 0; i < numBytes; i++) data[i] = 0;

        r = libusb_interrupt_transfer(handle, endPoint, data, packageSIze, &actual,
                                      3000); // считываем данные в реальном времени
        //  system("clear");
        printf("r: %i\n", r);
        printf("actualLength : %i\n", actual); // выводим информацию о действительных/фактически полученных данных

        horiz += data[1];
        vert += data[2];
        if (horiz >= 0 && vert >= 0 && horiz <= 150 && vert <= 50) {
            for (i = 0; i < vert; i++) printf("\n");
            for (i = 0; i < horiz; i++) printf(".");
            // Интерпретируем полученные данные
            if (data[0] == 0) printf("X");
            else if (data[0] == 1) printf("L");
            else if (data[0] == 2) printf("R");
            else if (data[0] == 3) printf("LR");
            else if (data[0] == 4) printf("M");
        } else {
            if (horiz < 0) horiz = 0;
            else if (horiz > 150) horiz = 150;
            if (vert < 0) vert = 0;
            else if (vert > 50) vert = 50;
        }

        // Интерпретируем полученные данные
        // Например, если четвертый байт массива data будет равен цифру 1, то колесо мыши крутится на данный момент вверх
        // если же цифра равна -1, то колесо крутиться вниз
        if (data[3] == 1) printf("\nSCROLL UP");
        if (data[3] == -1) printf("\nSCROLL DOWN");
        printf("\n(%d, %d)", horiz, vert);
        if (r < 0) errCount++;
        // после того как errCount станет больше 100, будет осуществлен выход из цикла
        // то есть после того как наша функция начнет возвращать ошибку(после отсоединения нашего устройства от хоста),
        // мы выйдем из цикла, в котором считывали и интерпритировали данные.
        if (errCount >= 100) break;
        libusb_clear_halt(handle, endPoint);

    }

    // отпускаем захваченный интерфейс
    libusb_release_interface(handle, 0);

    //привязываем драйвер ОС к устройству
    libusb_attach_kernel_driver(handle, 0);

    if (libusb_attach_kernel_driver(handle, 0)) printf("Attached kernel driver.\n");

    // заканчиваем работу с нашим устройством
    libusb_close(handle);
    printf("Closing Driver.\n");

    return EXIT_SUCCESS;
}