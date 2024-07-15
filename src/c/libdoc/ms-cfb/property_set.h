/**
 * File              : property_set.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 04.11.2022
 * Last Modified Date: 16.11.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef PROPERTY_SET_H_
#define PROPERTY_SET_H_

#include <stdbool.h>
#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
/*
 * While the potential for uses of persistent property sets is not fully tapped, there are  
 * currently two primary uses:
 *		Storing summary information with an object such as a document
 *		Transferring property data between objects
 *
 * COM property sets were designed to store data that is suited to representation as a moderately 
 * sized collection of fine-grained values. Data sets that are too large for this to be feasible 
 * should be broken into separate streams, storages, and/or property sets. The COM property set 
 * data format was not meant to provide a substitute for a database of many tiny objects.
 *
 * COM provides implementations of the property set interfaces for various objects, along with 
 * three helper functions. The following section describes some performance characteristics of 
 * these implementations.
 *
 * IPropertySetStorage–Compound File Implementation
 * The compound file implementation, which provides the IStorage and IStream interfaces, also 
 * provides the IPropertySetStorage and IPropertyStorage interfaces. Given a compound file 
 * implementation of IStorage, the IPropertySetStorage interface can be obtained by calling 
 * IUnknown::QueryInterface.
 *
 * IPropertySetStorage–NTFS File System Implementation
 * The IPropertySetStorage and IPropertyStorage interfaces can also be obtained for NTFS files 
 * that are not compound files. Therefore, it is possible to obtain these interfaces for all 
 * files on an NTFS volume.
 *
 * IPropertySetStorage–Stand-alone Implementation
 * When this implementation of IPropertySetStorage and IPropertyStorage is instantiated, it is 
 * given a pointer to an object that supports the IStorage interface. It then manipulates 
 * property set storages within that storage object. Thus, it is possible to access and 
 * manipulate property sets on any object that supports .
 *
 * IPropertySetStorage Implementation Considerations
 * There are several issues to consider in providing an implementation of the IPropertySetStorage 
 * interface.
 */


/*
 * function `property_set_get`
 * Read properties from standard common property set file, execute callback for each found
 * property. Return error code. To stop function execution you may return non 0 in callback
 * function. 
 */
static int 
property_set_get(FILE * fp, void * user_data,
	int (*callback)(void * user_data, uint32_t propid, uint32_t dwType, uint8_t * value));


/*
 * Error codes
 */
enum {
	PSET_NO_ERR,     //no error
	PSET_CB_STOP,    //stopped by callback
	PSET_ERR_FILE,   //error to read file
	PSET_ERR_HEADER, //error to read header
	PSET_ERR_ALLOC,  //memory allocation error
};

/*
 * Property types
 */
enum {
  PSET_EMPTY            = 0,        //Not specified
  PSET_NULL             = 1,        //NULL
  PSET_I2               = 2,        //A 2-byte integer
  PSET_I4               = 3,        //A 4-byte integer
  PSET_R4               = 4,        //A 4-byte real
  PSET_R8               = 5,        //An 8-byte real
  PSET_CY               = 6,        //Currency
  PSET_DATE             = 7,        //A date
  PSET_BSTR             = 8,        //A string
  PSET_DISPATCH         = 9,        //An IDispatch pointer
  PSET_ERROR            = 10,       //An SCODE value
  PSET_BOOL             = 11,       //A Boolean value. True is -1 and false is 0
  PSET_VARIANT          = 12,       //A variant pointer
  PSET_UNKNOWN          = 13,       //An IUnknown pointer
  PSET_DECIMAL          = 14,       //A 16-byte fixed-pointer value
  PSET_I1               = 16,       //A character
  PSET_UI1              = 17,       //An unsigned character
  PSET_UI2              = 18,       //An unsigned short
  PSET_UI4              = 19,       //An unsigned long
  PSET_I8               = 20,       //A 64-bit integer.
  PSET_UI8              = 21,       //A 64-bit unsigned integer
  PSET_INT              = 22,       //An integer
  PSET_UINT             = 23,       //An unsigned integer
  PSET_VOID             = 24,       //A C-style void
  PSET_HRESULT          = 25,       //An HRESULT value
  PSET_PTR              = 26,       //A pointer type
  PSET_SAFEARRAY        = 27,       //A safe array. Use VT_ARRAY in VARIANT
  PSET_CARRAY           = 28,       //A C-style array
  PSET_USERDEFINED      = 29,       //A user-defined type
  PSET_LPSTR            = 30,       //A null-terminated string
  PSET_LPWSTR           = 31,       //A wide null-terminated string
  PSET_RECORD           = 36,       //A user-defined type
  PSET_INT_PTR          = 37,       //A signed machine register size width
  PSET_UINT_PTR         = 38,       //An unsigned machine register size width
  PSET_FILETIME         = 64,       //A FILETIME value
  PSET_BLOB             = 65,       //Length-prefixed bytes
  PSET_STREAM           = 66,       //The name of the stream follows
  PSET_STORAGE          = 67,       //The name of the storage follows
  PSET_STREAMED_OBJECT  = 68,       //The stream contains an object
  PSET_STORED_OBJECT    = 69,       //The storage contains an object
  PSET_BLOB_OBJECT      = 70,       //The blob contains an object
  PSET_CF               = 71,       //A clipboard format
  PSET_CLSID            = 72,       //A class ID
  PSET_VERSIONED_STREAM = 73,       //A stream with a GUID version
  PSET_BSTR_BLOB        = 0xfff,    //Reserved
  PSET_VECTOR           = 0x1000,   //A simple counted array
  PSET_ARRAY            = 0x2000,   //A SAFEARRAY pointer
  PSET_BYREF            = 0x4000,   //A void pointer for local use
  PSET_RESERVED         = 0x8000,
  PSET_ILLEGAL          = 0xffff,
  PSET_ILLEGALMASKED    = 0xfff,
  PSET_TYPEMASK         = 0xfff
} ;

/*
 * At the beginning of the property set stream is a header. It consists of a byte-order 
 * indicator, a format version, the originating operating system version, the class identifier 
 * (CLSID), and a reserved field.
 */
typedef struct tagPROPERTYSETHEADER 
{ 
    // Header 
    uint16_t   wByteOrder;// Always 0xFFFE 
    uint16_t   wFormat;   // Always 0 
    uint32_t   dwOSVer;   // System version(0x0002 Win32, 0x0001 Macintosh, 0x0000 Win16) 
    uint32_t   clsID[4];  // Application CLSID 
    uint32_t   count;     // count of sections 
} PROPERTYSETHEADER;

/*
 * The second part of the property set stream contains one Format Identifier/Offset Pair. The 
 * FMTID is the name of the property set; it uniquely identifies how to interpret the contents of 
 * the following section. The Offset is the distance, in bytes, from the start of the whole 
 * stream to where the section begins.
 */
typedef struct tagFORMATIDOFFSET 
{ 
    uint32_t  fmtid[4] ;  // FMTID - Name of the section. 
    uint32_t  dwOffset ;  // Offset for the section. 
} FORMATIDOFFSET;

/*
 * The section is the third part of the property set stream and contains the actual property set 
 * values.
 * A section contains:
 *		Byte count for the section which is inclusive of the byte count itself.
 *		Array of 32-bit Property ID/Offset pairs.
 *		Array of property Type Indicators/Value pairs.
 * Offsets are the distance from the start of the section to the start of the property (type, 
 * value) pair. This allows a section to be copied as an array of bytes without any translation 
 * of internal structure.
 * 
 * The section size data type indicates the size, in bytes, of the section. Because the section 
 * size is the first 4 bytes, sections can be copied as an array of bytes. The section size 
 * should always be a multiple of four.
 * For example, an empty section, that is, one with zero properties in it, would have a byte 
 * count of eight (the DWORD byte count and the DWORD count of properties). The section itself 
 * would contain the 8 bytes: 08 00 00 00 00 00 00 00.
 */
typedef struct tagPROPERTYSECTIONHEADER 
{ 
    uint32_t  cbSection ;    // Size of Section 
    uint32_t  cProperties ;  // Count of Properties in section 
} PROPERTYSECTIONHEADER; 

 
typedef struct tagPROPERTYIDOFFSET 
{ 
    uint32_t  propid;    // Name of property 
    uint32_t  dwOffset;  // Offset from start of section to property 
} PROPERTYIDOFFSET; 

/*
 * Following the Count of Properties property set value is an array of Property Identifiers/
 * Offset Pairs property set values. Property Identifiers are 32-bit values that uniquely 
 * identify a property within a section. The Offset Pairs indicate the distance from the start of 
 * the section to the start of the property Type/Value Pair. Because the offsets are relative to 
 * the section, sections can be copied as an array of bytes.
 * Property identifiers are not sorted in any particular order. Properties can be omitted from 
 * the stored property set; readers must not rely on a specific order or range of property 
 * identifiers.
 * 
 * The actual properties follow the table of Property Identifiers/Offset Pairs property set 
 * values. Each property is stored as a DWORD, followed by the data type value.
 * Type indicators and their associated values are described in the PROPVARIANT structure.
 * All Type/Value pairs must begin on a 32-bit boundary. Thus, values may be followed with null 
 * bytes to align the subsequent pair on a 32-bit boundary.
 * The following example code calculates how many bytes are required to align on a 32-bit 
 * boundary, given a count of bytes.
 *  cbAdd = (((cbCurrent + 3) >> 2) << 2) - cbCurrent ;
 *
 *  Within a vector of values, each repetition of a simple scalar value smaller than 32 bits must 
 *  align with its natural alignment rather than with a 32-bit alignment. In practice, this is 
 *  only significant for types VT_UI1, VT_UI2, VT_I2, and VT_BOOL (which have one-byte or two-
 *  byte natural alignment). All other types have four-byte natural alignment. Some types, for 
 *  example, VT_R8, actually have 8-byte natural alignment, but are stored as if they have four-
 *  byte alignment.
 *
 *  A property value with type indicator VT_I2 | VT_VECTOR would include:
 *		A DWORD element count.
 *		A sequence of packed two-byte integers with no null padding between them.
 *
 * A property value of type identifier VT_LPSTR | VT_VECTOR would include:
 *		A DWORD element count (DWORD cElems).
 *		A sequence of strings (char rgch[]), each preceded by a length-count DWORD and possibly 
 * followed by null padding to round to a 32-bit boundary.
 */
typedef struct tagSERIALIZEDPROPERTYVALUE 
{ 
    uint32_t  dwType;    // Property Type 
    uint32_t  value[1];  // Property Value 
} SERIALIZEDPROPERTYVALUE;


/*
 * Get properties from FILE
 */

//switch bite order
uint64_t PS_DDWORD_SW (uint64_t i)
{
    unsigned char c1, c2, c3, c4, c5, c6, c7, c8;

	c1 = i & 255;
	c2 = (i >> 8) & 255;
	c3 = (i >> 16) & 255;
	c4 = (i >> 24) & 255;
	c5 = (i >> 32) & 255;
	c6 = (i >> 40) & 255;
	c7 = (i >> 48) & 255;
	c8 = (i >> 54) & 255;

	return ((uint64_t)c1 << 54) + 
		   ((uint64_t)c2 << 48) + 
		   ((uint64_t)c3 << 40) + 
		   ((uint64_t)c4 << 32) + 
		   ((uint64_t)c5 << 24) + 
		   ((uint64_t)c6 << 16) + 
		   ((uint64_t)c7 << 8)  + c8;
}

uint32_t PS_DWORD_SW (uint32_t i)
{
    unsigned char c1, c2, c3, c4;

	c1 = i & 255;
	c2 = (i >> 8) & 255;
	c3 = (i >> 16) & 255;
	c4 = (i >> 24) & 255;

	return ((int)c1 << 24) + 
		   ((int)c2 << 16) + 
		   ((int)c3 << 8)  + c4;
}

uint16_t PS_WORD_SW (uint16_t i)
{
    unsigned char c1, c2;
    
	c1 = i & 255;
	c2 = (i >> 8) & 255;

	return (c1 << 8) + c2;
}


int property_set_get(
			FILE * fp,            //file pointer
			void * user_data,     //data to transfer to callback
			int (*callback)(      //callback for each propertry - return non 0 to stop
				void * user_data, //data to transfet
				uint32_t propid,  //property id
				uint32_t dwType,  //property type
				uint8_t *value    //pointer to property value
			)
		)
{
	int i, k, c;
	size_t len;
	bool byteOrder = false;

	//get property stream header
	PROPERTYSETHEADER head;
	fseek(fp, 0, SEEK_SET);
	len = fread(&head, 1, sizeof(PROPERTYSETHEADER), fp);	
	if (len != sizeof(PROPERTYSETHEADER))
		return PSET_ERR_FILE; //error to read file
	
	//check bite order - should be 0xfffe
	if (head.wByteOrder == 0xFFFE){ // LE byte order 
	} else if (head.wByteOrder == 0xFEFF){
		// need to change byte order
		byteOrder = true;
	} else
		return PSET_ERR_HEADER; //error to read header
							
	if (byteOrder){
		head.wFormat = PS_WORD_SW(head.wFormat);
		head.dwOSVer = PS_DWORD_SW(head.dwOSVer);
		for (i = 0; i < 4; ++i) 
			head.clsID[i] = PS_DWORD_SW(head.clsID[i]); 

		head.count = PS_DWORD_SW(head.count);
	}
	
	//get sectors
	for (i = 0; i < head.count; ++i) {
		//format/offset pair
		FORMATIDOFFSET soff;
		fseek(fp, sizeof(PROPERTYSETHEADER) + i*sizeof(soff), SEEK_SET);
		fread(&soff, 1, sizeof(FORMATIDOFFSET), fp);
		if (byteOrder){
			for (i = 0; i < 4; ++i) 
				soff.fmtid[i] = PS_DWORD_SW(soff.fmtid[i]); 			
			soff.dwOffset = PS_DWORD_SW(soff.dwOffset);
		}		
		
		//get property section header
		PROPERTYSECTIONHEADER pshead;
		fseek(fp, soff.dwOffset, SEEK_SET);
		fread(&pshead, 1, sizeof(PROPERTYSECTIONHEADER), fp);		
		if (byteOrder){
			pshead.cbSection = PS_DWORD_SW(pshead.cbSection);
			pshead.cProperties = PS_DWORD_SW(pshead.cProperties);
		}

		//copy section to buf
		uint8_t * buf = malloc(pshead.cbSection*4);
		if (!buf)
			return PSET_ERR_ALLOC;
		fseek(fp, soff.dwOffset, SEEK_SET);
		fread(buf, 1, pshead.cbSection*4, fp);				

		//for each property
		for (k = 0; k < pshead.cProperties; ++k) {
			//get propery offset
			PROPERTYIDOFFSET poff;	
			fseek(fp, soff.dwOffset 
					+ sizeof(PROPERTYSECTIONHEADER) 
							+ k*sizeof(PROPERTYIDOFFSET), SEEK_SET);
			fread(&poff, 1, sizeof(PROPERTYIDOFFSET), fp);			
			if (byteOrder){
				poff.propid = PS_DWORD_SW(poff.propid);
				poff.dwOffset = PS_DWORD_SW(poff.dwOffset);
			}

			//get type/value pair 
			SERIALIZEDPROPERTYVALUE ptv;
			fseek(fp, soff.dwOffset + poff.dwOffset, SEEK_SET);
			fread(&ptv, 1, sizeof(SERIALIZEDPROPERTYVALUE), fp);			
			if (byteOrder)
				ptv.dwType = PS_DWORD_SW(ptv.dwType);

			//pointer to value
			uint8_t * ptr = buf + poff.dwOffset + 4;
			uint16_t *v16;
			uint32_t *v32;
			uint64_t *v64;
			
			if (byteOrder){
				if (ptv.dwType == PSET_I2||ptv.dwType ==  PSET_UI2){
					v16 = (uint16_t *)ptr;
					*v16 = PS_WORD_SW(*v16);
					ptr = (uint8_t *)v16;		  
				}
				else if (ptv.dwType == PSET_I4 || ptv.dwType == PSET_R4 || ptv.dwType == PSET_UI4){
					v32 = (uint32_t *)ptr;
					*v32 = PS_DWORD_SW(*v32);
					ptr = (uint8_t *)v32;		  
				}				
				else if (ptv.dwType == PSET_I8 || ptv.dwType == PSET_R8 || ptv.dwType == PSET_UI8 || ptv.dwType == PSET_I8){
					v64 = (uint64_t *)ptr;
					*v64 = PS_DDWORD_SW(*v64);
					ptr = (uint8_t *)v64;		  
				}				
			}

			//callback
			if (callback)
				if (callback(user_data, poff.propid, ptv.dwType, ptr))
					return PSET_CB_STOP;
		}	

		free(buf);
	}

	return PSET_NO_ERR;
}

#ifdef __cplusplus
}
#endif

#endif //PROPERTY_SET_H_
