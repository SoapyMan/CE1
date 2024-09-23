#ifndef __ICompressionHelper_h__
#define __ICompressionHelper_h__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CStream;

struct ICompressionHelper
{
	//!
	virtual bool Write(CStream& outStream, const uchar inChar) = 0;
	//!
	virtual bool Read(CStream& inStream, uchar& outChar) = 0;
	//!
	virtual bool Write(CStream& outStream, const char* inszString) = 0;
	//!
	virtual bool Read(CStream& inStream, char* outszString, const DWORD indwStringSize) = 0;
};

#endif // __ICompressionHelper_h__
