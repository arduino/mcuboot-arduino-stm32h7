/*
 * SerialOTA.cpp
 *
 *  Created on: Jul 31, 2020
 *      Author: gdbeckstein
 */

#include "SerialOTA.h"
#include "drivers/MbedCRC.h"

#define OTA_DONE_SEQ_NUM 0xFFFF

SerialOTA::SerialOTA(mbed::FileHandle& serial, mbed::BlockDevice& update_bd) : _serial(serial),
                     _update_bd(update_bd), _current_seq(0) {
}

SerialOTA::ota_error_t SerialOTA::begin(void) {

    mbed::MbedCRC<POLY_16BIT_CCITT, 16> ct;
    uint32_t bytes_collected = 0;
    packet_header_t header = { 0 };
    uint32_t crc;

    // Initialize the secondary block device
    _update_bd.init();

    // Erase the entire secondary block device
    _update_bd.erase(0x0, _update_bd.size());

    _serial.set_blocking(true);

    // Tell the serial bootloader host application to begin :)
    _serial.write("GO", 2);

    while(true) {

        // Read a packet from the FileHandle
        _serial.read(_payload_buffer, OTA_PAYLOAD_BUFFER_SIZE);

        // The first bytes will be the header
        memcpy(&header, _payload_buffer, sizeof(packet_header_t));

        // If the sequence number is OTA_DONE_SEQ_NUM we're done!
        if(header.sequence_num == OTA_DONE_SEQ_NUM) {
            return OTA_SUCCESS;
        }

        // Check the sequence number
        if(_current_seq != header.sequence_num) {
            return OTA_OUT_OF_SYNC;
        }

        // Check the make sure the update_bd can fit the bytes
        if((bytes_collected + header.payload_size) > _update_bd.size()) {
            return OTA_OUT_OF_SPACE;
        }

        // Check the CRC16 to make sure the data is not corrupt
        ct.compute(&_payload_buffer[2],
                (header.payload_size+sizeof(packet_header_t)-sizeof(uint16_t)),
                &crc);

        if(crc != header.crc16) {
            return OTA_CRC_ERROR;
        }

        // Everything checks out, proceed to writing the bytes to the update_bd
        _update_bd.program(&_payload_buffer[sizeof(packet_header_t)],
                bytes_collected,
                header.payload_size);

        // Update sequence number and bytes_collected
        _current_seq++;
        bytes_collected += header.payload_size;

    }
}
