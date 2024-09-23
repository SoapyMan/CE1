////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   crc32.h
//  Version:     v1.00
//  Created:     31/10/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __crc32_h__
#define __crc32_h__

#if _MSC_VER > 1000
#pragma once
#endif


class Crc32Gen {
public:
	Crc32Gen();
	//! Creates a CRC from a text string 
	static uint GetCRC32( const char *text );
	static uint GetCRC32( const char *data,int size,uint ulCRC );

protected:
	uint crc32_table[256];  //!< Lookup table array 
	void init_CRC32_Table();  //!< Builds lookup table array 
	uint reflect( uint ref, char ch); //!< Reflects CRC bits in the lookup table 
	uint get_CRC32( const char *data,int size,uint ulCRC );
};

#endif // __crc32_h__