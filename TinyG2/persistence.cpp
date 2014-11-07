/*
 * persistence.cpp - persistence functions
 * This file is part of the TinyG2 project
 *
 * Copyright (c) 2013 - 2014 Alden S. Hart Jr.
 *
 * This file ("the software") is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2 as published by the
 * Free Software Foundation. You should have received a copy of the GNU General Public
 * License, version 2 along with the software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, you may use this file as part of a software library without
 * restriction. Specifically, if other files instantiate templates or use macros or
 * inline functions from this file, or you compile this file and link it with  other
 * files to produce an executable, this file does not by itself cause the resulting
 * executable to be covered by the GNU General Public License. This exception does not
 * however invalidate any other reasons why the executable file might be covered by the
 * GNU General Public License.
 *
 * THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
 * WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 * SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "tinyg2.h"
#include "persistence.h"
#include "canonical_machine.h"
#include "report.h"
//#include "util.h"

#ifdef __AVR
#include "xmega/xmega_eeprom.h"
#endif

/***********************************************************************************
 **** STRUCTURE ALLOCATIONS ********************************************************
 ***********************************************************************************/

nvmSingleton_t nvm;

/***********************************************************************************
 **** GENERIC STATIC FUNCTIONS AND VARIABLES ***************************************
 ***********************************************************************************/


/***********************************************************************************
 **** CODE *************************************************************************
 ***********************************************************************************/

void persistence_init()
{
#ifdef __AVR
	nvm.nvm_base_addr = NVM_BASE_ADDR;
	nvm.nvm_profile_base = 0;
#endif
	return;
}

/*
 * read_persistent_value()	- return value (as float) by index
 *
 *	It's the responsibility of the caller to make sure the index does not exceed range
 */

#ifdef __AVR
stat_t read_persistent_value(nvObj_t *nv)
{
	int8_t nvm_byte_array[NVM_VALUE_LEN];
	uint16_t nvm_address = nvm.nvm_profile_base + (nv->index * NVM_VALUE_LEN);
	(void)EEPROM_ReadBytes(nvm_address, nvm_byte_array, NVM_VALUE_LEN);
	memcpy(&nv->value, &nvm_byte_array, NVM_VALUE_LEN);
	return (STAT_OK);
}
#endif // __AVR

#ifdef __ARM
stat_t read_persistent_value(nvObj_t *nv)
{
	nv->value = 0;
	return (STAT_OK);
}
#endif // __ARM

/*
 * write_persistent_value() - write to NVM by index, but only if the value has changed
 *
 *	It's the responsibility of the caller to make sure the index does not exceed range
 *	Note: Removed NAN and INF checks on floats - not needed
 */

#ifdef __AVR
stat_t write_persistent_value(nvObj_t *nv)
{
	if (cm.cycle_state != CYCLE_OFF) return(rpt_exception(STAT_FILE_NOT_OPEN));	// can't write when machine is moving
	float tmp_value = nv->value;
	ritorno(read_persistent_value(nv));
	if (fp_NE(nv->value, tmp_value)) {
		nv->value = tmp_value;
		int8_t nvm_byte_array[NVM_VALUE_LEN];
		memcpy(&nvm_byte_array, &tmp_value, NVM_VALUE_LEN);
		uint16_t nvm_address = nvm.nvm_profile_base + (nv->index * NVM_VALUE_LEN);
		(void)EEPROM_WriteBytes(nvm_address, nvm_byte_array, NVM_VALUE_LEN);
	} else {
		nv->value = tmp_value;		// restore value in case the NVM was a NAN
	}
	return (STAT_OK);
}
#endif // __AVR

#ifdef __ARM
stat_t write_persistent_value(nvObj_t *nv)
{
	if (cm.cycle_state != CYCLE_OFF) return(rpt_exception(STAT_FILE_NOT_OPEN));	// can't write when machine is moving
	return (STAT_OK);
}
#endif // __ARM
