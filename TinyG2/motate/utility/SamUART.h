/*
 utility/SamUART.h - Library for the Motate system
 http://tinkerin.gs/
 
 Copyright (c) 2013 Robert Giseburt
 
 This file is part of the Motate Library.
 
 This file ("the software") is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License, version 2 as published by the
 Free Software Foundation. You should have received a copy of the GNU General Public
 License, version 2 along with the software. If not, see <http://www.gnu.org/licenses/>.
 
 As a special exception, you may use this file as part of a software library without
 restriction. Specifically, if other files instantiate templates or use macros or
 inline functions from this file, or you compile this file and link it with  other
 files to produce an executable, this file does not by itself cause the resulting
 executable to be covered by the GNU General Public License. This exception does not
 however invalidate any other reasons why the executable file might be covered by the
 GNU General Public License.
 
 THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
 WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 */

#ifndef SAMUART_H_ONCE
#define SAMUART_H_ONCE

#include "sam.h"

#include "MotatePins.h"
#include "SamCommon.h"

namespace Motate {
    
	/*    
    enum SPIMode {
        
        kSPIPolarityNormal     = 0,
        kSPIPolarityReversed   = SPI_CSR_CPOL,
        
        // Using the wikipedia deifinition of "normal phase," see:
        //   http://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus#Clock_polarity_and_phase
        // Wikipedia, in turn, sites Freescale's SPI Block Guide:
        //   http://www.ee.nmt.edu/~teare/ee308l/datasheets/S12SPIV3.pdf
        
        // This makes the Phase flag INVERTED from that of the SAM3X/A datasheet.
        
        kSPIClockPhaseNormal   = SPI_CSR_NCPHA,
        kSPIClockPhaseReversed = 0,
        
        // Using the wikipedia/freescale mode numbers (and the SAM3X/A datashgeet agrees).
        // The arduino mode settings appear to mirror that of wikipedia as well,
        //  so we should all be in agreement here.
        kSPIMode0              = kSPIPolarityNormal   | kSPIClockPhaseNormal,
        kSPIMode1              = kSPIPolarityNormal   | kSPIClockPhaseReversed,
        kSPIMode2              = kSPIPolarityReversed | kSPIClockPhaseNormal,
        kSPIMode3              = kSPIPolarityReversed | kSPIClockPhaseReversed,
        
        kSPI8Bit               = SPI_CSR_BITS_8_BIT,
        kSPI9Bit               = SPI_CSR_BITS_9_BIT,
        kSPI10Bit              = SPI_CSR_BITS_10_BIT,
        kSPI11Bit              = SPI_CSR_BITS_11_BIT,
        kSPI12Bit              = SPI_CSR_BITS_12_BIT,
        kSPI13Bit              = SPI_CSR_BITS_13_BIT,
        kSPI14Bit              = SPI_CSR_BITS_14_BIT,
        kSPI15Bit              = SPI_CSR_BITS_15_BIT,
        kSPI16Bit              = SPI_CSR_BITS_16_BIT
	};
    */
	
    // This is an internal representation of the peripheral.
    // This is *not* to be used externally.
	template<uint8_t usartPeripheralNumber>
    struct _USARTHardware {
        // TOOD: Hmmm.
    };
    
    template<>
    struct _USARTHardware<0u> : SamCommon< _USARTHardware<0u> > {
		static Usart * const usart() { return USART0; };
        static const uint32_t peripheralId() { return ID_USART0; }; 
		static const IRQn_Type usartIRQ() { return USART0_IRQn; };
        
        static const uint8_t usartPeripheralNum=0;
        
        typedef _USARTHardware<0u> this_type_t;
        typedef SamCommon< this_type_t > common;

        /* We have to play some tricks here, because templates and static members are tricky.
         * See https://groups.google.com/forum/#!topic/comp.lang.c++.moderated/yun9X6OMiY4
         *
         * Basically, we want a guard to make sure we dont itinig the SPI0 IC modules every time
         * we create a new SPI object for the individual chip selects.
         *
         * However, since we don't use the module *directly* in the code, other than to init it,
         * the optimizer removes that object and it's init in it's entrety.
         *
         * The solution: Make sure each SPI<> object calls hardware.init(), and then use a static guard
         * in init() to prevent re-running it.
         */
        
        void init() {
            static bool inited = false;
            if (inited)
                return;
            inited = true;
        
            common::enablePeripheralClock();
 
 			// Normal mode
			// Baud rate clock = system master clock
            usart()->US_MR = 0;
		};
        
        _USARTHardware() :  SamCommon< this_type_t >() {
//            init();
        };

        static void enable() {
			usart()->US_CR |= US_CR_TXEN | US_CR_RXEN;
		};
        
        static void disable () {
			usart()->US_CR &= ~(US_CR_TXEN | US_CR_RXEN);		
        };
        
    };
    
	template<uint8_t usartPeripheralNumber>
	struct USART {
        static _USARTHardware< usartPeripheralNumber > hardware;        
        typedef SamCommon< USART<usartPeripheralNumber> > common;
        static Usart * const usart() { return hardware.usart(); };
		
        USART(const uint32_t baud = 57600) {
            hardware.init();
            init(baud, /*fromConstructor =*/ true);
        };
        
		// TODO: Undo the "blockyness" here - switch to interrupts and buffers or somesuch
		void butc(uint8_t c) {
			// Wait for transmit holding register to be ready to receive another character
			while(!(usart()->US_CSR & US_CSR_TXRDY));
			// Transmit a character
			usart()->US_THR = c;	
		};
		
		void write(const uint8_t *buffer, size_t size) {
			while(size > 0) {
				butc(*buffer++);
				size--;
			}
		}
		
        void init(const uint32_t baud, const bool fromConstructor=false) {
			setOptions(baud);
			hardware.enable();
        };
		
		void setOptions(const uint32_t baud) {
			// Set the baud rate
			usart()->US_MR = US_MR_CHRL_8_BIT | US_MR_USART_MODE_NORMAL | US_MR_PAR_NO | US_MR_NBSTOP_1_BIT;
			usart()->US_BRGR = SystemCoreClock / baud;
		}
	};    
}

#endif /* end of include guard: SAMUSART_H_ONCE */