#include "base.h"
#include "usb_desc.h"
#include "ft245rom.h"

static uint8_t s_ft245rom_buf[FT245ROM_SIZE] = {0};

uint16_t ft245rom_checksum(const uint8_t *pbuf, uint8_t len)
{
    size_t i;
    uint16_t checksum;
    
    checksum = 0xAAAA;
    
    for (i = 0; i < len; i += 2) {
        checksum ^= ((uint16_t)pbuf[i + 1] << 8) | (uint16_t)pbuf[i];
        checksum = (checksum << 1) | (checksum >> 15);
    }

    return checksum;
}

int ft245rom_init(void)
{
    size_t i;
    uint8_t offset = 0;
    uint16_t checksum;
    uint8_t *pbuf = s_ft245rom_buf;

    if (Blaster_StringVendor[0] + Blaster_StringProduct[0] + Blaster_StringSerial[0] > FT245ROM_STR_LIMIT) {
        return -1;
    }

    *pbuf++ = 0x0;
    *pbuf++ = 0x0;
    for (i = 0; i < 6; i++) {
        *pbuf++ = Blaster_DeviceDescriptor[8 + i];  // vid/pid/ver
    }
    *pbuf++ = Blaster_ConfigDescriptor[7];  // attr
    *pbuf++ = Blaster_ConfigDescriptor[8];  // pwr
    *pbuf++ = 0x1C;                         // chip config
    *pbuf++ = 0x00;
    *pbuf++ = Blaster_DeviceDescriptor[2];  // usb ver
    *pbuf++ = Blaster_DeviceDescriptor[3];  // usb ver

    // strings offset and length
    offset = 0x80 | (pbuf - s_ft245rom_buf + 2 * 3);
    *pbuf++ = offset;
    *pbuf++ = Blaster_StringVendor[0];
    offset += Blaster_StringVendor[0];
    *pbuf++ = offset;
    *pbuf++ = Blaster_StringProduct[0];
    offset += Blaster_StringProduct[0];
    *pbuf++ = offset;
    *pbuf++ = Blaster_StringSerial[0];

    for (i = 0; i < Blaster_StringVendor[0]; i++) {
        *pbuf++ = Blaster_StringVendor[i];  // vendor string
    }
    for (i = 0; i < Blaster_StringProduct[0]; i++) {
        *pbuf++ = Blaster_StringProduct[i]; // product string
    }
    for (i = 0; i < Blaster_StringSerial[0]; i++) {
        *pbuf++ = Blaster_StringSerial[i];  // serial string
    }
    
    *pbuf++ = 0x2;
    *pbuf++ = 0x3;
    *pbuf++ = 0x1;
    *pbuf++ = 0x0;
    
    *pbuf++ = 'R';
    *pbuf++ = 'E';
    *pbuf++ = 'V';
    *pbuf++ = 'B';

    // fill zeros
    while ((pbuf - s_ft245rom_buf) < (FT245ROM_SIZE - 2)) {
        *pbuf++ = 0x0;
    }

    // checksum
    checksum = ft245rom_checksum(s_ft245rom_buf, FT245ROM_SIZE - 2);
    *pbuf++ = checksum & 0xFF;
    *pbuf++ = (checksum >> 8) & 0xFF;

    return 0;
}

uint16_t ft245rom_read(uint8_t addr)
{
    return ((uint16_t)s_ft245rom_buf[addr + 1] << 8) | s_ft245rom_buf[addr];
}
