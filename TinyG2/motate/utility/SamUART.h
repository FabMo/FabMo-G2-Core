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
    
	template<uint8_t usartPeripheralNumber>
	struct USART {
        typedef SamCommon< USART<usartPeripheralNumber> > common;
		static Usart * const usart() { return USART0; };
        static const uint32_t peripheralId() { return ID_USART0; }; 
		static const IRQn_Type usartIRQ() { return USART0_IRQn; };

		// Transmit/receive pins for this USART
		OutputPin<18> txPin;
		InputPin<19> rxPin;
		
        USART(const uint32_t baud = 57600) {
            //hardware.init();
            init(baud, /*fromConstructor =*/ true);
        };

        static void enable() {
			usart()->US_CR |= US_CR_TXEN | US_CR_RXEN;
		};
        
        static void disable () {
			usart()->US_CR &= ~(US_CR_TXEN | US_CR_RXEN);		
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
			txPin.setMode(kPeripheralA);
			//rxPin.setMode(kPeripheralA);		
			setOptions(baud);
			enable();
        };
		
		void setOptions(const uint32_t baud) {
			// Set the baud rate
			usart()->US_MR = US_MR_CHRL_8_BIT | US_MR_USART_MODE_NORMAL | US_MR_PAR_NO | US_MR_NBSTOP_1_BIT;
//			usart()->US_BRGR = US_BRGR_CD
			usart()->US_BRGR = SystemCoreClock / (16*baud);
			
		}
	};    
}

#endif /* end of include guard: SAMUSART_H_ONCE */