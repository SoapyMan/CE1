
//////////////////////////////////////////////////////////////////////
//
// Copyright © 2000 - 2003 Richard A. Ellingson
// http://www.createwindow.com
// mailto:relling@antelecom.net
// 
//	File: 
//
//  Description:  
//
//	History:
//
//////////////////////////////////////////////////////////////////////

class Crc32Gen {
public:
	Crc32Gen();
	//! Creates a CRC from a text string 
	static uint GetCRC32(const char* text);
	static uint GetCRC32(const char* data, int size, uint ulCRC);

protected:
	uint crc32_table[256];  //!< Lookup table array 
	void init_CRC32_Table();  //!< Builds lookup table array 
	uint reflect(uint ref, char ch); //!< Reflects CRC bits in the lookup table 
	uint get_CRC32(const char* data, int size, uint ulCRC);
};
