#include "StdAfx.h"
#include <ILog.h>
#include <Stream.h>
#include "System.h"
#include <zlib.h>
#if defined(LINUX)
#	include <sys/io.h>
#else
#	include <io.h>
#endif
#if defined(LINUX)
#include "ILog.h"
#endif

#include <fcntl.h>

//#define USE_COMPRESSION

bool CSystem::WriteCompressedFile(char* filename, void* data, uint bitlen)
{
	FILE* pFile = fxopen(filename, "wb+");
	if (!pFile)
		return false;

#ifdef USE_COMPRESSION
	gzFile f = gzdopen(fileno(pFile), "wb9");
	gzwrite(f, &bitlen, sizeof(int));
	gzwrite(f, data, BITS2BYTES(bitlen));
	gzclose(f);
#else
	fwrite(&bitlen, sizeof(int), 1, pFile);
	fwrite(data, BITS2BYTES(bitlen), 1, pFile);
	fclose(pFile);
#endif

	return true;
};

uint CSystem::GetCompressedFileSize(char* filename)
{
	FILE* pFile = fxopen(filename, "rb");
	if (!pFile)
		return 0;

#ifdef USE_COMPRESSION
	fseek(pFile, 0, SEEK_END);
	long nLen = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	if (nLen <= 0)
	{
		// gzread works incorrectly if the filesize is 0
		fclose(pFile);
		return (0);
	}

	fclose(pFile);
	pFile = fxopen(filename, "rb");
	if (!pFile)
		return 0;

	uint bitlen;
	gzFile f = gzdopen(fileno(pFile), "rb9");
	gzread(f, &bitlen, sizeof(int));
	gzclose(f);
#else
	fseek(pFile, 0, SEEK_END);
	long nLen = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	if (nLen <= 0)
	{
		fclose(pFile);
		return (0);
	}

	uint bitlen;
	fread(&bitlen, sizeof(int), 1, pFile);
	fclose(pFile);
#endif

	return bitlen;
}

uint CSystem::ReadCompressedFile(char* filename, void* data, uint maxbitlen)
{
	FILE* pFile = fxopen(filename, "rb");
	if (!pFile)
		return 0;

	/*
	fseek(pFile,0,SEEK_END);
	long nLen=ftell(pFile);
	fseek(pFile,0,SEEK_SET);
	*/

	uint bitlen;

#ifdef USE_COMPRESSION
	gzFile f = gzdopen(fileno(pFile), "rb9");
	gzread(f, &bitlen, sizeof(int));
	CRYASSERT(bitlen <= maxbitlen);  // FIXME: nicer if caller doesn't need to know buffer size in advance
	gzread(f, data, BITS2BYTES(bitlen));
	gzclose(f);
#else	
	fread(&bitlen, sizeof(int), 1, pFile);
	CRYASSERT(bitlen <= maxbitlen);  // FIXME: nicer if caller doesn't need to know buffer size in advance
	fread(data, BITS2BYTES(bitlen), 1, pFile);
	fclose(pFile);
#endif

	return bitlen;
};
