/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2008 University of Paderborn

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QEMU_UART.hh>
#include <assembler.h>
#include "comm/AddressProtocol.hh"
#include <kernel/Kernel.hh>

extern Kernel* theOS;

/*---------------------------------------------------------------------------*/
QEMU_UART::~QEMU_UART()
/*---------------------------------------------------------------------------*/
{

}

/*---------------------------------------------------------------------------*/
QEMU_UART::QEMU_UART( const char* name, int4 a ) :
    CommDeviceDriver( name )
/*---------------------------------------------------------------------------*/
{
    headerFound = false;
    currentPosition = 0;
    addr = a;

    // only enable interrupts from this device if you really use it as a communication device
    // e.g prototype 1 uses this device as a character device (direct reading and writing, no sockets)
    // enableIRQ();
}



/*---------------------------------------------------------------------------*/
ErrorT QEMU_UART::enableIRQ()
/*---------------------------------------------------------------------------*/
{
    return cNotImplemented;
}

/*---------------------------------------------------------------------------*/
ErrorT QEMU_UART::disableIRQ()
/*---------------------------------------------------------------------------*/
{
    return cNotImplemented;
}

/*---------------------------------------------------------------------------*/
ErrorT QEMU_UART::readByte( char* byteptr )
/*---------------------------------------------------------------------------*/
{
    inputSCC( 1, (byte*) byteptr );
    return cOk;
}

/*---------------------------------------------------------------------------*/
ErrorT QEMU_UART::writeByte( char byte )
/*---------------------------------------------------------------------------*/
{
    outputSCC( -1, byte );
    return cOk;
}

/*---------------------------------------------------------------------------*/
ErrorT QEMU_UART::readBytes( char* bytes, unint4 &length )
/*---------------------------------------------------------------------------*/
{
    ErrorT err;
    unint4 count;

    for ( count = 0, err = cOk; ( ( count < length ) && ( err == cOk ) ); count++ )
        err = inputSCC( 1, (byte*) bytes + count );
    length = ( err == cOk ) ? count : count - 1;
    return err;
}

/*---------------------------------------------------------------------------*/
ErrorT QEMU_UART::writeBytes( const char* bytes, unint4 length )
/*---------------------------------------------------------------------------*/
{
    unint count;
    ErrorT err;

    for ( count = 0, err = cOk; ( count < length ) && ( err == cOk ); count++ )
        err = outputSCC( -1, bytes[ count ] );

    return err;
}

/*---------------------------------------------------------------------------*/
void QEMU_UART::sendByte( byte Data )
/*---------------------------------------------------------------------------*/
{
    volatile unsigned char *port = (unsigned char *) addr;
    *port = (unsigned char) Data;
}

/*---------------------------------------------------------------------------*/
bool QEMU_UART::isTransmitBufferFull()
/*---------------------------------------------------------------------------*/
{
    return false;
}

/*---------------------------------------------------------------------------*/
bool QEMU_UART::isReceiveBufferFull()
/*---------------------------------------------------------------------------*/
{
    return false;
}

/*---------------------------------------------------------------------------*/
byte QEMU_UART::recvByte()
/*---------------------------------------------------------------------------*/
{
    volatile unsigned char *port = (unsigned char *) addr;
    return (byte) *port;
}

/*---------------------------------------------------------------------------*/
ErrorT QEMU_UART::inputSCC( int4 Timeout, byte *c )
/*---------------------------------------------------------------------------*/
{
    int ret = cError;

    while ( Timeout == -1 || Timeout-- ) {
        {
            if ( hasPendingData() ) {
                *c = recvByte(); /* get char */
                ret = cOk;
                break;
            }
        }
    } /* end while */
    return ret;
}

/*---------------------------------------------------------------------------*/
bool QEMU_UART::hasPendingData()
/*---------------------------------------------------------------------------*/
{
    return true;
}

/*---------------------------------------------------------------------------*/
ErrorT QEMU_UART::outputSCC( int4 Timeout, byte c )
/*---------------------------------------------------------------------------*/
{
    int ret = cError;

    while ( Timeout == -1 || Timeout-- ) {
        {
            if ( !isTransmitBufferFull() ) {
                sendByte( c ); /* output char */
                ret = cOk;
                break;
            }
        }
    } /* end while */
    return ret;
}

ErrorT QEMU_UART::send(packet_layer* packet, char* dest_addr, int addr_len, int2 fromProtocol_ID ) {
    // this device ignores dest_addr and addr_len since we got
    // a 1-1 communication here (serial line!!)
    // only devices with more than 1 connected neighbor (lan,wlan,bluetooth ...)
    // need to lookup the mac address of the host with dest_addr (may use ARP)
    // but not this device!

   /* PacketHeader_ packetHeader;

    // check if we support this packet.
    if ( len > COMDD_MAX_PACKET_LENGTH ) {
        return cError;
    }

    packetHeader.packetHead[ 0 ] = 0x55;
    packetHeader.packetHead[ 1 ] = 0x55;
    packetHeader.packetHead[ 2 ] = 0x55;
    packetHeader.packetHead[ 3 ] = 0x55;
    packetHeader.addressProtocolID = fromProtocol_ID;
    //memcpy(&packetHeader.destinationAddress, dest_addr, 4);
    packetHeader.packetLength = len;

    // calc data checksum
    packetHeader.dataChecksum = byte[ 0 ];
    for ( unint2 i = 1; i < len; i++ ) {
        packetHeader.dataChecksum ^= byte[ i ];
    }

    // calc header checksum
    packetHeader.headerChecksum = ( (char*) &packetHeader )[ 0 ];
    for ( unint2 i = 1; i < sizeof(PacketHeader_) - 2; i++ ) {
        packetHeader.headerChecksum ^= ( (char*) &packetHeader )[ i ];
    }

    // send the header to the wire
    writeBytes( (char*) &packetHeader, sizeof(PacketHeader_) );
    // send the data to the wire
    writeBytes( byte, len );
*/
    return cOk;
}

ErrorT QEMU_UART::broadcast( packet_layer* packet, int2 fromProtocol_ID  ) {
    return send( packet, 0, 0,fromProtocol_ID );

}

void QEMU_UART::recv() {
    PacketHeader_* packetHeader;

    // this method got called since the opb_uart raised an external interrupt
    // this interrupt will be active as long as the opb_uarts receive fifo is not empty
    // so the first thing to do is to clear the uarts fifo and store the bytes in our private buffer
    // then we can reactivate the irq on this device

    while ( this->hasPendingData() ) {
        // check for out of bounds error
        if ( currentPosition >= sizeof( packetBuffer ) ) {
            // start all over again (we may miss a packet)
            currentPosition = 0;
            headerFound = false;
        }
        // read 1 byte and dont wait
        //inputSCC(1,packetBuffer+currentPosition);
        packetBuffer[ currentPosition ] = recvByte();
        currentPosition++;
    }


    this->interruptPending = false;
    // we may now reactivate the irq since the fifo has been cleared
    this->enableIRQ();

    // at this point we may have the following situations :
    // 1. a whole packet (header + data) is inside the buffer
    // 2. only parts of the packet are inside the buffer

    // if we dont have a packet header yet, try to find it!
    if ( !headerFound ) {
        for ( int i = 0; i + sizeof(PacketHeader_) < currentPosition; i++ ) {

            if ( packetBuffer[ i ] == 0x55 && packetBuffer[ i + 1 ] == 0x55 && packetBuffer[ i + 2 ] == 0x55
                    && packetBuffer[ i + 3 ] == 0x55 ) {
                // ok we may have received the header (enough bytes received)
                // with some luck, we found a header now. lets try
                packetHeader = (PacketHeader_*) &packetBuffer[ i ];

                // calc header checksum
                unint2 headerChecksum = ( (char*) packetHeader )[ 0 ];
                for ( unint2 j = 1; j < sizeof(PacketHeader_) - 2; j++ ) {
                    headerChecksum ^= ( (char*) packetHeader )[ j ];
                }

                // check if checksum correct
                if ( headerChecksum == packetHeader->headerChecksum ) {
                    // heureka, we found a header!
                    headerFound = true;
                    // move all bytes to the beginning
                    // we got some bytes infront of the header that we consider as garbage
                    if ( i != 0 ) {
                        for ( int j = i; j < currentPosition; j++ ) {
                            packetBuffer[ j - i ] = packetBuffer[ j ];
                        }
                        currentPosition = currentPosition - i;
                    }
                    // escape the iteration since we found a header by now
                    // but : the packet might still be incomplete!
                    break;

                } // if headerchecksum

            } // if packerbuffer = 0x55 ..
        } // for i = 0 ..
    } // if !headerFound

    // if we have a header, check if the packet is complete.
    if ( headerFound ) {
        // adjuste pointer to packetheader since it is now at the
        // beginning of the buffer
        packetHeader = (PacketHeader_*) &packetBuffer[ 0 ];
        if ( packetHeader->packetLength + sizeof(PacketHeader_) <= currentPosition ) {
            // we have more than enough bytes in the buffer to consider
            // this packet as complete. check if its valid

            // calc data checksum
            unint2 dataChecksum = packetBuffer[ sizeof(PacketHeader_) ];
            for ( unint2 i = 1; i < packetHeader->packetLength; i++ ) {
                dataChecksum ^= packetBuffer[ sizeof(PacketHeader_) + i ];
            }

            if ( dataChecksum == packetHeader->dataChecksum ) {
                // this is a valid packet. deliver it
                AddressProtocol* aproto = theOS->getProtocolPool()->getAddressProtocolbyId(
                        packetHeader->addressProtocolID );
                if ( aproto != 0 ) {
                    aproto->recv( &packetBuffer[ 0 ] + sizeof(PacketHeader_), packetHeader->packetLength, this );
                }
            }

            // packet has been processed or was invalid. discard what we have
            headerFound = false;
            unint2 len = packetHeader->packetLength + sizeof(PacketHeader_);

            // clean up, move data to front of buffer
            // (we can't simply discard the whole buffer, because maybe the data belongs
            // to a following packet)
            for ( int j = 0; j < len; j++ ) {
                packetBuffer[ j ] = packetBuffer[ j + len ];
            }
            currentPosition = currentPosition - len;
        }
    }
}

