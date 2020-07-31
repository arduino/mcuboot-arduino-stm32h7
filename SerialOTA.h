/*
 * SerialOTA.h
 *
 *  Created on: Jul 30, 2020
 *      Author: gdbeckstein
 */

#ifndef SERIALOTA_H_
#define SERIALOTA_H_

#include "platform/FileHandle.h"
#include "BlockDevice.h"

#define OTA_PAYLOAD_BUFFER_SIZE 384

class SerialOTA
{

public:

    typedef struct {
        uint16_t crc16;             /** CRC16 of entire packet */
        uint16_t sequence_num;      /** Sequence number of the OTA update, the special value 0xFFFF means the transfer is over. Wraps at 0xFFFE */
        uint16_t payload_size;      /** Payload size of this packet in bytes */
    } packet_header_t;

    typedef enum {
        OTA_SUCCESS         = 0x0,  /** success */
        OTA_OUT_OF_SPACE    = 0x1,  /** update_bd has run out of space for the update */
        OTA_CRC_ERROR       = 0x2,  /** CRC error on last packet */
        OTA_OUT_OF_SYNC     = 0x3,  /** Sequence number received out of sync */
    } ota_error_t;

public:

    /**
     * Instantiate a SerialOTA interface
     *
     * @param[in] serial Serial FileHandle reference to use
     * @param[in] update_bd BlockDevice to store the update in
     */
    SerialOTA(mbed::FileHandle& serial, mbed::BlockDevice& update_bd);

    /**
     * Begins the SerialOTA update process
     *
     * This is a blocking function call and will only
     * exit upon error or completion
     *
     * @retval error
     */
    ota_error_t begin(void);

protected:

    mbed::FileHandle& _serial;

    mbed::BlockDevice& _update_bd;

    uint16_t _current_seq;

    uint8_t _payload_buffer[OTA_PAYLOAD_BUFFER_SIZE];

};

#endif /* SERIALOTA_H_ */
