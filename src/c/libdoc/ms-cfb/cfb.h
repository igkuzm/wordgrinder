/**
 * File              : cfb.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 03.11.2022
 * Last Modified Date: 29.05.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef CFB_H_
#define CFB_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#include "byteorder.h"
#include "log.h"

/* A Compound File is made up of a number of virtual
 * streams. These are collections of data that behave as a
 * linear stream, although their on-disk format may be
 * fragmented. Virtual streams can be user data, or they can
 * be control structures used to maintain the file. Note
 * that the file itself can also be considered a virtual
 * stream.  All allocations of space within a Compound File
 * are done in units called sectors. The size of a sector is
 * definable at creation time of a Compound File, but for
 * the purposes of this document will be 512 bytes.  A
 * virtual stream is made up of a sequence of sectors.  The
 * Compound File uses several different types of sector:
 * Fat, Directory, Minifat, DIF, and Storage.  A separate
 * type of 'sector' is a Header, the primary difference
 * being that a Header is always 512 bytes long (regardless
 * of the sector size of the rest of the file) and is always
 * located at offset zero (0).  With the exception of the
 * header, sectors of any type can be placed anywhere within
 * the file.  The function of the various sector types is
 * discussed below.  In the discussion below, the term SECT
 * is used to describe the location of a sector within a
 * virtual stream (in most cases this virtual stream is the
 * file itself). Internally, a SECT is represented as a
 * ULONG.*/

/*
 * TIPES
 */

typedef uint32_t ULONG    ; //[4 bytes]
typedef uint16_t USHORT   ; //[2 bytes]
typedef uint16_t OFFSET   ; //[2 bytes]
typedef ULONG SECT        ; //[4 bytes]
typedef ULONG FSINDEX     ; //[4 bytes]
typedef USHORT FSOFFSET   ; //[2 bytes]
typedef ULONG DFSIGNATURE ; //[4 bytes]
typedef uint8_t BYTE      ; //[1 byte]
typedef uint16_t WORD     ; //[2 bytes]
typedef uint32_t DWORD    ; //[4 bytes]
typedef WORD DFPROPTYPE   ; //[2 bytes]
typedef ULONG SID         ; //[4 bytes]

#ifndef CLSID
typedef struct CLSID {
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;
}CLSID;
#endif
typedef CLSID GUID        ;//[16 bytes]

typedef struct tagFILETIME {
	DWORD dwLowDateTime; 
	DWORD dwHighDateTime; 
} FILETIME, TIME_T        ;//[8 bytes]      


static const SECT MAXSECT        = 0xFFFFFFFB; 
static const SECT DIFSECT        = 0xFFFFFFFC; 
static const SECT FATSECT        = 0xFFFFFFFD;
static const SECT ENDOFCHAIN     = 0xFFFFFFFE;
static const SECT FREESECT       = 0xFFFFFFFF;
 
/*
 * HEADER
 *
 * The Header contains vital information for the
 * instantiation of a Compound File.  Its total length is
 * 512 bytes. There is exactly one Header in any Compound
 * File, and it is always located beginning at offset zero
 * in the file.
 *
 * Fat Sectors
 *
 * The Fat is the main allocator for space within a Compound
 * File. Every sector in the file is represented within the
 * Fat in some fashion, including those sectors that are
 * unallocated (free). The Fat is a virtual stream made up
 * of one or more Fat Sectors.  Fat sectors are arrays of
 * SECTs that represent the allocation of space within the
 * file. Each stream is represented in the Fat by a chain,
 * in much the same fashion as a DOS file-allocation- table
 * (FAT). To elaborate, the set of Fat Sectors can be
 * considered together to be a single array -- each cell in
 * that array contains the SECT of the next sector in the
 * chain, and this SECT can be used as an index into the Fat
 * array to continue along the chain. Special values are
 * reserved for chain terminators (ENDOFCHAIN = 0xFFFFFFFE),
 * free sectors (FREESECT = 0xFFFFFFFF), and sectors that
 * contain storage for Fat Sectors (FATSECT = 0xFFFFFFFD) or
 * DIF Sectors (DIFSECT = 0xFFFFFFC), which are not chained
 * in the same way as the others.  The locations of Fat
 * Sectors are read from the DIF (Double-indirect Fat),
 * which is described below. The Fat is represented in
 * itself, but not by a chain – a special reserved SECT
 * value (FATSECT = 0xFFFFFFFD) is used to mark sectors
 * allocated to the Fat.  A SECT can be converted into a
 * byte offset into the file by using the following formula:
 * SECT << ssheader._uSectorShift + sizeof(ssheader). This
 * implies that sector 0 of the file begins at byte offset
 * 512, not at 0.
 *
 * MiniFat Sectors Since space for streams is always
 * allocated in sector-sized blocks, there can be
 * considerable waste when storing objects much smaller than
 * sectors (typically 512 bytes). As a solution to this
 * problem, we introduced the concept of the MiniFat. The
 * MiniFat is structurally equivalent to the Fat, but is
 * used in a different way. The virtual sector size for
 * objects represented in the Minifat is 1 <<
 * ssheader._uMiniSectorShift (typically 64 bytes) instead
 * of 1 << ssheader.  _uSectorShift (typically 512 bytes).
 * The storage for these objects comes from a virtual stream
 * within the Multistream (called the Ministream).  The
 * locations for MiniFat sectors are stored in a standard
 * chain in the Fat, with the beginning of the chain stored
 * in the header.  A Minifat sector number can be converted
 * into a byte offset into the ministream by using the
 * following formula: SECT << ssheader._uMiniSectorShift.
 * (This formula is different from the formula used to
 * convert a SECT into a byte offset in the file, since no
 * header is stored in the Ministream).  The Ministream is
 * chained within the Fat in exactly the same fashion as any
 * normal stream. It is referenced by the first Directory
 * Entry (SID 0).
 *
 * DIF Sectors The Double-Indirect Fat is used to represent
 * storage of the Fat. The DIF is also represented by an
 * array of SECTs, and is chained by the terminating cell in
 * each sector array. As an optimization, the first 109 Fat
 * Sectors are represented within the header itself, so no
 * DIF sectors will be found in a small (< 7 MB) Compound
 * File.  The DIF represents the Fat in a different manner
 * than the Fat represents a chain. A given index into the
 * DIF will contain the SECT of the Fat Sector found at that
 * offset in the Fat virtual stream. For instance, index 3 in
 * the DIF would contain the SECT for Sector #3 of the Fat.
 * The storage for DIF Sectors is reserved in the Fat, but is
 * not chained there (space for it is reserved by a special
 * SECT value , DIFSECT=0xFFFFFFFC). The location of the
 * first DIF sector is stored in the header.
 * A value of ENDOFCHAIN=0xFFFFFFFE is stored in the pointer
 * to the next DIF sector of the last DIF sector.*/ 

 
struct StructuredStorageHeader {  // [offset from start in
																	// bytes, length in bytes]
	BYTE _abSig[8];               // [000H,08] {0xd0, 0xcf,
																// 0x11, 0xe0, 0xa1, 0xb1,
																// 0x1a, 0xe1} for current
																// version, was {0x0e, 0x11,
																// 0xfc, 0x0d, 0xd0, 0xcf,
																// 0x11, 0xe0} on old, beta
																// 2 files (late ’92) which
																// are also supported by the
																// reference implementation
	CLSID _clid;                  // [008H,16] class id (set
																// with WriteClassStg,
																// retrieved with
																// GetClassFile/ReadClassStg)
	USHORT _uMinorVersion;        // [018H,02] minor version
																// of the format: 33 is
																// written by reference
																// implementation
	USHORT _uDllVersion;          // [01AH,02] major version
																// of the dll/format: 3 is
																// written by reference
																// implementation
	USHORT _uByteOrder;           // [01CH,02] 0xFFFE:
																// indicates Intel
																// byte-ordering
	USHORT _uSectorShift;         // [01EH,02] size of sectors
																// in power-of-two
																// (typically 9, indicating
																// 512-byte sectors)
	USHORT _uMiniSectorShift;     // [020H,02] size of
																// mini-sectors in
																// power-of-two (typically
																// 6, indicating 64-byte
																// mini-sectors)
	USHORT _usReserved;           // [022H,02] reserved, must be zero
	ULONG  _ulReserved1;          // [024H,04] reserved, must be zero
	ULONG  _ulReserved2;          // [028H,04] reserved, must be zero
	FSINDEX _csectFat;            // [02CH,04] number of SECTs
																// in the FAT chain
	SECT	_sectDirStart;        // [030H,04] first SECT in the
															// Directory chain
	DFSIGNATURE _signature;       // [034H,04] signature used
																// for transactionin: must
																// be zero.  The reference
																// implementation does not
																// support transactioning
	ULONG _ulMiniSectorCutoff;    // [038H,04] maximum size
																// for mini-streams:
																// typically 4096 bytes
	SECT  _sectMiniFatStart;      // [03CH,04] first SECT in
																// the mini-FAT chain
	FSINDEX _csectMiniFat;        // [040H,04] number of SECTs
																// in the mini-FAT chain
	SECT  _sectDifStart;          // [044H,04] first SECT in the DIF chain
	FSINDEX _csectDif;            // [048H,04] number of SECTs
																// in the DIF chain
	SECT _sectFat[109];           // [04CH,436] the SECTs of
																// the first 109 FAT sectors
};
typedef struct StructuredStorageHeader cfb_header;


/*
 * Directory Sectors
 *
 * The Directory is a structure used to contain per-stream
 * information about the streams in a Compound File, as well
 * as to maintain a tree-styled containment structure. It is
 * a virtual stream made up of one or more Directory
 * Sectors. The Directory is represented as a standard chain
 * of sectors within the Fat. The first sector of the
 * Directory chain (the Root Directory Entry)
 *
 * Each level of the containment hierarchy (i.e. each set of
 * siblings) is represented as a red-black tree. The parent
 * of this set of sibilings will have a pointer to the top
 * of this tree. This red-black tree must maintain the
 * following conditions in order for it to be valid: 1. The
 * root node must always be black. Since the root directory
 * (see below) does not have siblings, it's color is
 * irrelevant and may therefore be either red or black.  2.
 * No two consecutive nodes may both be red.  3. The left
 * child must always be less than the right child. This
 * relationship is defined as: - A node with a shorter name
 * is less than a node with a longer name (i.e. compare 
 *length of the name)
 *		- For nodes with the same length names, compare the
 *		two names.  The simplest implementation of the above
 *		invariants would be to mark every node as black, in
 *		which case the tree is simply a binary tree.  A
 *		Directory Sector is an array of Directory Entries, a
 *		structure represented in the diagram below. Each user
 *		stream within a Compound File is represented by a
 *		single Directory Entry.  The Directory is considered
 *		as a large array of Directory Entries. It is useful to
 *		note that the Directory Entry for a stream remains at
 *		the same index in the Directory array for the life of
 *		the stream – thus, this index (called an SID) can be
 *		used to readily identify a given stream.  The
 *		directory entry is then padded out with zeros to make
 *		a total size of 128 bytes.  Directory entries are
 *		grouped into blocks of four to form Directory Sectors.
 *
 * Root Directory Entry
 *
 * The first sector of the Directory chain (also referred to
 * as the first element of the Directory array, or SID 0) is
 * known as the Root Directory Entry and is reserved for two
 * purposes: First, it provides a root parent for all
 * objects stationed at the root of the multi-stream.
 * Second, its function is overloaded to store the size and
 * starting sector for the Mini-stream.  The Root Directory
 * Entry behaves as both a stream and a storage. All of the
 * fields in the Directory Entry are valid for the root. The
 * Root Directory Entry’s Name field typically contains the
 * string “RootEntry” in Unicode, although some versions of
 * structured storage (particularly the preliminary
 * reference implementation and the Macintosh version) store
 * only the first letter of this string, “R” in the name.
 * This string is always ignored, since the Root Directory
 * Entry is known by its position at SID 0 rather than by
 * its name, and its name is not otherwise used. New
 * implementations should write “RootEntry” properly in the
 * Root Directory Entry for consistency and support
 * manipulating files created with only the “R” name.
 *
 * Other Directory Entries
 *
 * Non-root directory entries are marked as either stream
 * (STGTY_STREAM) or storage (STGTY_STORAGE) elements.
 * Storage elements have a _clsid, _time[], and _sidChild
 * values; stream elements may not. Stream elements have
 * valid _sectStart and _ulSize members, whereas these
* fields are set to zero for storage elements (except as
		* noted above for the Root Directory Entry).  To
* determine the physical file location of actual stream data
* from a stream directory entry, it is necessary to
* determine which FAT (normal or mini) the stream exists
* within. Streams 
* whose _ulSize member is less than the _ulMiniSectorCutoff
* value for the file exist in the ministream, and so the
* _startSect is used as an index into the MiniFat (which
		* starts at _sectMiniFatStart) to track the chain of
* mini-sectors through the mini-stream (which is, as 
		* noted earlier, the standard (non-mini) stream referred
		* to by the Root Directory Entry’s _sectStart value).
* Streams whose _ulSize member is greater than the
* _ulMiniSectorCutoff value for the file exist as standard
* streams – their _sectStart value is used as an index into
* the 
* standard FAT which describes the chain of full sectors
* containing their data).
 *
 * Storage Sectors Storage sectors are simply collections of
 * arbitrary bytes. They are the building blocks of user
 * streams, and no restrictions are imposed on their
 * contents. Storage sectors are represented as chains in
 * the Fat, and each storage chain (stream) will have a
 * single Directory Entry associated with it.
 */

typedef enum tagSTGTY { 
	STGTY_INVALID   = 0,
	STGTY_STORAGE   = 1,
	STGTY_STREAM    = 2,
	STGTY_LOCKBYTES = 3,
	STGTY_PROPERTY  = 4,
	STGTY_ROOT      = 5,
} STGTY;

typedef enum tagDECOLOR { 
	DE_RED          = 0,
	DE_BLACK        = 1,
} DECOLOR;


struct StructuredStorageDirectoryEntry {
																			// [offset from start
																			// in bytes, length in
																			// bytes]                                               
	BYTE _ab[32*sizeof(WORD)];          // [000H,64] 64 bytes.
																			// The Element name in
																			// Unicode, padded
																			// with zeros to fill
																			// this byte array
	WORD _cb;                           // [040H,02] Length of
																			// the Element name in
																			// characters, not
																			// bytes
	BYTE _mse;                          // [042H,01] Type of
																			// object: value taken
																			// from the STGTY
																			// enumeration
	BYTE _bflags;                       // [043H,01] Value
																			// taken from DECOLOR
																			// enumeration.
	SID _sidLeftSib;                    // [044H,04] SID of
																			// the left-sibling of
																			// this entry in the
																			// directory tree
	SID _sidRightSib;                   // [048H,04] SID of
																			// the right-sibling
																			// of this entry in
																			// the directory tree
	SID _sidChild;                      // [04CH,04] SID of
																			// the child acting as
																			// the root of all the
																			// children of this
																			// element (if
																			// _mse=STGTY_STORAGE)
	GUID _clsId;                        // [050H,16] CLSID of
																			// this storage (if
																			// _mse=STGTY_STORAGE)
	DWORD _dwUserFlags;                 // [060H,04] User
																			// flags of this
																			// storage (if
																			// _mse=STGTY_STORAGE)
	TIME_T _time[2];                    // [064H,16]
																			// Create/Modify
																			// time-stamps (if
																			// _mse=STGTY_STORAGE) 
	SECT _sectStart;                    // [074H,04] starting
																			// SECT of the stream
																			// (if
																			// _mse=STGTY_STREAM) 
	ULONG _ulSize;                      // [078H,04] size of
																			// stream in bytes (if
																			// _mse=STGTY_STREAM)
  DFPROPTYPE _dptPropType;          // [07CH,02] Reserved
																			// for future use.
// Must be zero.                                           
};
typedef struct StructuredStorageDirectoryEntry cfb_dir;


static const unsigned char cfb_signature[8]     = 
{0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1};
static const unsigned char cfb_signature_old[8] = 
{0x0e, 0x11, 0xfc, 0x0d, 0xd0, 0xcf, 0x11, 0xe0};

/*
 * MS-CMF structure
 * countain file header, root dir header and pointers to
 * streams
 */
struct cfb {
	FILE * fp;         // pointer to file
	FILE * ministream; // pointer to ministream
	cfb_header header;
	cfb_dir root;
	bool biteOrder;
};

// error codes
enum {
	CFB_NO_ERR = 0,              // no errors
	CFB_READ_ERR = 0x1,          // error to read stream
	CFB_WRITE_ERR = 0x2,         // error to write stream
	CFB_SIG_ERR = 0x4,           // error CFB signature
	CFB_BYTEORDE_ERR = 0x8,      // error get file byte order
	CFB_FAT_ERR = 0x10,          // error in FAT stream
	CFB_MFAT_ERR = 0x20,         // error in miniFAT stream
	CFB_ROOT_ERR = 0x40,         // error in root dir
	CFB_HEADER_ERR = 0x80,       // error in file header
	CFB_DIF_ERR = 0x100,         // error in DIF
	CFB_ALLOC_ERR = 0x200,       // error in alloc
};

/*
 * IMP
 */

static void _cfb_dir_sw(cfb_dir * dir){
	
	dir->_cb = bswap_16(dir->_cb);
	
	dir->_sidLeftSib = bswap_32(dir->_sidLeftSib);
	dir->_sidRightSib = bswap_32(dir->_sidRightSib);
	dir->_sidChild = bswap_32(dir->_sidChild);
	
	dir->_clsId.a = bswap_32(dir->_clsId.a); 
	dir->_clsId.b = bswap_32(dir->_clsId.b); 
	dir->_clsId.c = bswap_32(dir->_clsId.c); 
	dir->_clsId.d = bswap_32(dir->_clsId.d); 

	dir->_dwUserFlags = bswap_32(dir->_dwUserFlags);
	dir->_time[0].dwLowDateTime = bswap_32(dir->_time[0].dwLowDateTime);
	dir->_time[0].dwHighDateTime = bswap_32(dir->_time[0].dwHighDateTime);
	dir->_time[1].dwLowDateTime = bswap_32(dir->_time[1].dwLowDateTime);
	dir->_time[1].dwHighDateTime = bswap_32(dir->_time[1].dwHighDateTime);	

	dir->_sectStart = bswap_32(dir->_sectStart);
	dir->_ulSize = bswap_32(dir->_ulSize);
	dir->_dptPropType = bswap_16(dir->_dptPropType);
}

//return len of utf8 string
static size_t _utf16_to_utf8(WORD * utf16, int len, char * utf8){
	int i, k = 0;
	for (i = 0; i < len; ++i) {
		WORD wc = utf16[i];
		if (wc <= 0x7F) {
			// Plain single-byte ASCII.
			utf8[k++] = (char) wc;
		}
		else if (wc <= 0x7FF) {
			// Two bytes.
			utf8[k++] = 0xC0 |  (wc >> 6);
			utf8[k++] = 0x80 | ((wc >> 0) & 0x3F);
		}
		else if (wc <= 0xFFFF) {
			// Three bytes.
			utf8[k++] = 0xE0 |  (wc >> 12);
			utf8[k++] = 0x80 | ((wc >> 6) & 0x3F);
			utf8[k++] = 0x80 | ((wc >> 0) & 0x3F);
		}
		else{
			// Invalid char; don't encode anything.
		}
	}
	//null-terminate
	utf8[k] = 0;	

	return k;
}

/*
 * The FAT is the main allocator for space within a compound
 * file. Every sector in the file is represented within the
 * FAT in some fashion, including those sectors that are
 * unallocated
 *(free). The FAT is a sector chain that is made up of one
 or more FAT sectors.
 */

static SECT _cfb_next_sect_in_FAT_chain(SECT sect, struct cfb * cfb){
/*
 * The FAT is an array of sector numbers that represent the
 * allocation of space within the file, grouped into FAT
 * sectors. Each stream is represented in the FAT by a
 * sector chain, in much the same fashion as a FAT file
 * system.
 */	
#ifdef DEBUG
	LOG("get next SECT in FAT chain for: 0x%x:\t", sect);
#endif		
	int i;

	if (sect > MAXSECT)
		return ENDOFCHAIN;	

/*
 * If Header Major Version is 3, there MUST be 128 fields
 * specified to fill a 512-byte sector.  If Header Major
 * Version is 4, there MUST be 1,024 fields specified to
 * fill a 4,096-byte sector.
 */	
	DWORD ssize = 1 << cfb->header._uSectorShift; //sector size
	FSINDEX SECTn = ssize/4; // number of sectors in FAT

/* The DIFAT sectors are linked together by the last field
 * in each DIFAT sector. As an optimization, the first 109
 * FAT sectors are represented within the header itself.  No
 * DIFAT sectors are needed in a compound file that is
 * smaller than 6.875 megabytes (MB) for a 512-byte sector
 * compound file (6.875 MB = (1 header sector + 109 FAT
 * sectors x 128 non-empty entries) × 512 bytes per sector).
 */ 
	if (sect < ssize * 109){ // FAT is in header

		FSINDEX FAT_INDEX = sect / SECTn;
		FSINDEX SECT_INDEX = sect - (FAT_INDEX * SECTn);

		// get SECT offset
		SECT FAT = cfb->header._sectFat[FAT_INDEX];
		if (cfb->biteOrder) 
			FAT = bswap_32(FAT);		
		DWORD off = 
			FAT * ssize + ssize
			+ SECT_INDEX * 4;
		// seek to offset
		fseek(cfb->fp, off, SEEK_SET);

		// read sect
		SECT ch;
		if (fread(&ch, 4, 1, cfb->fp) != 1){
#ifdef DEBUG
	LOG("Error to read SECT from offset: %u", off);
#endif		
			return ENDOFCHAIN;
		};
		if (cfb->biteOrder) 
			ch = bswap_32(ch);		

#ifdef DEBUG
	LOG("0x%x", ch);
#endif		
		return ch;

	} 
// FAT is in DIFAT

/* DIFAT
 * double-indirect file allocation table (DIFAT): A
 * structure that is used to locate FAT sectors in a
 * compound file.
 *
 *		FAT Sector Location (variable): This field specifies
 *		the FAT sector number in a DIFAT.  If Header Major
 *		Version is 3, there MUST be 127 fields specified to
 *		fill a 512-byte sector minus the "Next DIFAT Sector
 *		Location" field.  If Header Major Version is 4, there
 *		MUST be 1,023 fields specified to fill a 4,096-byte
 *		sector minus the "Next DIFAT Sector Location" field.
*/	
	
	FSINDEX FATn = 127; //number of FAT sectors in DIFAT
	if (cfb->header._uDllVersion == 4)
		FATn = 1023;
	
	FSINDEX DIFAT_INDEX = (sect - SECTn * 109) / FATn * SECTn;
	FSINDEX FAT_INDEX   = sect - (SECTn * 109) - (DIFAT_INDEX * SECTn);
	FSINDEX SECT_INDEX  = sect - (SECTn * 109) - (DIFAT_INDEX * SECTn) - (FAT_INDEX * SECTn);

	// get DIFAT 
	SECT DIFAT = cfb->header._sectDifStart;
	for (i = 0; i < DIFAT_INDEX; ++i) {
		// next DIFAT is in 128 (1024) field 
		DWORD off = DIFAT * ssize + ssize + (FATn * 4); 
		fseek(cfb->fp, off, SEEK_SET);
		SECT ch;
		if (fread(&ch, 4, 1, cfb->fp) != 1){
#ifdef DEBUG
	LOG("Error to read DIFAT from offset: %u", off);
#endif		
			return ENDOFCHAIN;
		};
		if (cfb->biteOrder) 
			ch = bswap_32(ch);
		DIFAT = ch;	
	}
	// get FAT 
	SECT FAT;
	DWORD off = DIFAT * ssize + ssize + (FAT_INDEX * 4); 
	fseek(cfb->fp, off, SEEK_SET);
	SECT ch;
	if (fread(&ch, 4, 1, cfb->fp) != 1){
#ifdef DEBUG
	LOG("Error to read FAT from offset: %u", off);
#endif		
		return ENDOFCHAIN;
	};
	if (cfb->biteOrder) 
		ch = bswap_32(ch);
	FAT = ch;	

	// get SECT offset
	off = FAT * ssize + ssize + (SECT_INDEX * 4);
	// seek to offset
	fseek(cfb->fp, off, SEEK_SET);
	if (fread(&ch, 4, 1, cfb->fp) != 1){
#ifdef DEBUG
	LOG("Error to read SECT from offset: %u", off);
#endif		
		return ENDOFCHAIN;
	};
	if (cfb->biteOrder) 
		ch = bswap_32(ch);
#ifdef DEBUG
	LOG("0x%x", ch);
#endif		
	return ch;	
}

static SECT _cfb_next_sect_in_mFAT_chain(SECT sect, struct cfb * cfb){
/*
 *  The mini FAT is used to allocate space in the mini
 *  stream. The mini stream is divided into smaller,
 *  equal-length sectors, and the sector size that is used
 *  for the mini stream is specified from the Compound File
 *  Header (64 bytes).
 */	

#ifdef DEBUG
	LOG("get next SECT in mFAT chain for: 0x%x:\t", sect);
#endif		
	int i;

	if (sect > MAXSECT)
		return ENDOFCHAIN;	

/*
 * The mini stream is chained within the FAT in exactly the
 * same fashion as any normal stream.  The mini stream's
 * starting sector is referenced in the first directory
 * entry (root storage stream ID 0).
 */	
	DWORD ssize = 1 << cfb->header._uSectorShift; //sector size - for mFAT it is 512(4096)
	FSINDEX SECTn = ssize/4; // number of sectors in mFAT
	
	FSINDEX mFAT_INDEX = sect / SECTn;
	FSINDEX SECT_INDEX = sect - (mFAT_INDEX * SECTn);

	// get FAT 
	SECT mFAT = cfb->header._sectMiniFatStart;
	for (i = 0; i < mFAT_INDEX; ++i) {
		// next mFAT is in FAT chain
		mFAT = _cfb_next_sect_in_FAT_chain(mFAT, cfb); 
	}
	
	// get SECT offset
	DWORD off = mFAT * ssize + ssize + (SECT_INDEX * 4);
	// seek to offset
	fseek(cfb->fp, off, SEEK_SET);
	SECT ch;
	if (fread(&ch, 4, 1, cfb->fp) != 1){
#ifdef DEBUG
	LOG("Error to read SECT from offset: %u", off);
#endif		
		return ENDOFCHAIN;
	};
	if (cfb->biteOrder) 
		ch = bswap_32(ch);
#ifdef DEBUG
	LOG("0x%x", ch);
#endif		
	return ch;	
}

static int cfb_dir_name(cfb_dir * dir, char * name){
	int size = dir->_cb/2;
	WORD ab[size];
	int i, c;
	for (i = 0, c = 0; i < dir->_cb; ++i, ++c) {
		char ch0 = dir->_ab[i++];	
		char ch1 = dir->_ab[i];	
		if (ch1 == 0)
			ab[c] = ch0;
		else
			ab[c] = (ch0 << 8) + ch1;
	}

	if (!_utf16_to_utf8(ab, dir->_cb, name))
		return -1;

	return 0;
}

static FILE * cfb_get_stream_by_dir(struct cfb * cfb, cfb_dir * dir) {
#ifdef DEBUG
	char dirname[BUFSIZ];
	cfb_dir_name(dir, dirname);	
	LOG("dirname: %s", dirname);
#endif

	ULONG st = dir->_ulSize;    //size of stream

#ifdef DEBUG
	LOG("stream size: %u", st);
#endif
	SECT  sect = dir->_sectStart; // start position in FAT/miniFAT chain
#ifdef DEBUG
	LOG("stream start sector: 0x%x", dir->_sectStart);
#endif

	DWORD ssize;  //sector size	
	DWORD sstart; //start for sectors - 0 for mFAT, ssize for FAT; 
	FILE * fp;    // file pointer - ministream for mFAT, main stream for FAT
	SECT (*get_next_sect)(SECT sect, struct cfb * cfb); // get next sect function
	
	//check FAT or miniFAT
	//use miniFAT is size < 4096
	//for root always use FAT
	if (st < cfb->header._ulMiniSectorCutoff && dir->_mse != STGTY_ROOT){
#ifdef DEBUG
	LOG("stream is minifat");
#endif		
		//use miniFAT
		ssize = 1 << cfb->header._uMiniSectorShift;
		sstart = 0;
		fp = cfb->ministream;
		get_next_sect = _cfb_next_sect_in_mFAT_chain;
	}
	else {
#ifdef DEBUG
	LOG("stream is fat");
#endif				
		//use FAT
		ssize = 1 << cfb->header._uSectorShift;
		sstart = ssize;
		fp = cfb->fp;
		get_next_sect = _cfb_next_sect_in_FAT_chain;
	} 
	
	DWORD off = sect * ssize + sstart; //offset
	
#ifdef DEBUG
	LOG("sectorsize: %d", ssize);
	LOG("offset: %d"    , off  );
#endif				
	
	//create stream
	FILE * stream = tmpfile();

	//copy data
	while (sect != ENDOFCHAIN) {
		// seek to start offset
		fseek(fp, off, SEEK_SET);

		// read/write data
		char buf[ssize];
		if (fread (buf, ssize, 1,
					fp) != 1)
		{
			ERR("fread");
			return NULL;
		}
		fwrite(buf, ssize, 1, stream );
		
		// get next FAT/miniFAT
		sect = get_next_sect(sect, cfb);

		// get next offset 
		off = sect * ssize + sstart;
	}

	fseek(stream, 0, SEEK_SET);
	return stream;	
}

static int cfb_dir_by_sid(struct cfb * cfb, SID sid, void * user_data,
		int (*callback)(void * user_data, cfb_dir dir))
{
	int i;
	
	//goto dir data
	ULONG p = 
		512 
		+ sid*sizeof(cfb_dir) 
		+ (cfb->header._sectDirStart << cfb->header._uSectorShift);
	fseek(cfb->fp, p, SEEK_SET);

	//copy dir data
	cfb_dir dir;

	if (fread(&dir, sizeof(cfb_dir), 1, cfb->fp) != 1)
		return -1;

	if (cfb->biteOrder)
		_cfb_dir_sw(&dir);
	
	if (callback)
		callback(user_data, dir);

	return 0;
}

static int cfb_dir_callback(void * user_data, cfb_dir dir){
	cfb_dir * _dir = (cfb_dir *)user_data;
	*_dir = dir;
	return 0;
}

static int cfb_get_dir_by_sid(struct cfb * cfb, cfb_dir * dir, SID sid){
	return cfb_dir_by_sid(cfb, sid, dir, cfb_dir_callback);
}

static int _cfb_dir_find(struct cfb *cfb, cfb_dir * dir, const char * name, void * user_data,
		int (*callback)(void * user_data, cfb_dir dir))
{
#ifdef DEBUG
	LOG("name: %s", name);
#endif		
	
	if(!dir)
		return -1;
	//check name
	char dirname[BUFSIZ]; 
	if (cfb_dir_name(dir, dirname))
		return -1;
	int res;
	size_t name_len = strlen(name);
	size_t dirname_len = strlen(dirname);
	if (name_len < dirname_len)
		res = -1;
	else if (name_len > dirname_len)
		res = 1;
	else 
		res = strcmp(name, dirname);
	if (res == 0)
		callback(user_data, *dir);
	if (res < 0){
		//check left
		if (dir->_sidLeftSib != -1){
			cfb_dir new_dir;
			cfb_dir_by_sid(cfb, dir->_sidLeftSib, &new_dir, cfb_dir_callback);

			return _cfb_dir_find(cfb, &new_dir, name, user_data, callback);
		}
	} else {
		//check right
		if (dir->_sidRightSib != -1){

			cfb_dir new_dir;
			cfb_dir_by_sid(cfb, dir->_sidRightSib, &new_dir, cfb_dir_callback);
			return _cfb_dir_find(cfb, &new_dir, name, user_data, callback);
		}	
	}
	return 0;
}
static int cfb_dir_by_name(struct cfb * cfb, const char * name, void * user_data,
		int (*callback)(void * user_data, cfb_dir dir))
{
#ifdef DEBUG
	LOG("name: %s", name);
#endif		
	
	cfb_dir dir;
	cfb_get_dir_by_sid(cfb, &dir, cfb->root._sidChild);
	return _cfb_dir_find(cfb, &dir, name, user_data, callback);
}

static int cfb_get_dir_by_name(struct cfb * cfb, cfb_dir * dir, const char * name){
#ifdef DEBUG
	LOG("name: %s", name);
#endif		
	
	return cfb_dir_by_name(cfb, name, dir, cfb_dir_callback);
}

static FILE * cfb_get_stream_by_sid(struct cfb * cfb, SID sid) {
#ifdef DEBUG
	LOG("sid: %d", sid);
#endif		
	
	cfb_dir dir;
	if (cfb_get_dir_by_sid(cfb, &dir, sid))
		return NULL; //no dir
	
	return cfb_get_stream_by_dir(cfb, &dir);
};

static FILE * cfb_get_stream_by_name(struct cfb * cfb, const char * name) {
#ifdef DEBUG
	LOG("dir name: %s", name);
#endif		
	
	cfb_dir dir;
	if (cfb_get_dir_by_name(cfb, &dir, name))
		return NULL; //no dir
	
	return cfb_get_stream_by_dir(cfb, &dir);
}

#define cfb_get_stream(cfb, arg)\
	_Generic((arg), \
			char*:       cfb_get_stream_by_name, \
			int:         cfb_get_stream_by_sid, \
			cfb_dir *:   cfb_get_stream_by_dir \
	)((cfb), (arg))	

static int _cfb_init(struct cfb * cfb, FILE *fp){
#ifdef DEBUG
	LOG("start");
#endif

	memset(cfb, 0, sizeof(struct cfb));
	
	int error = 0;

	int i; //iterator 
	
	off_t p; // offset in stream 
	
	cfb->fp   = fp;
	cfb->biteOrder = false;
	
	//get byte order
	uint16_t byteOrder;
	fseek(fp, 0x01C, SEEK_SET);	
	if (fread(&byteOrder, 2, 1, fp) != 1) {
#ifdef DEBUG
	LOG("error to get byte orger");
#endif		
		ERR("can't read MS CFB file");
		return CFB_READ_ERR|CFB_BYTEORDE_ERR;
	}

	if (byteOrder == 0xFFFE){        // no need to change byte order
	} else if (byteOrder == 0xFEFF){ //need to change byte order
		cfb->biteOrder = true;
	} else {                         //error
#ifdef DEBUG
	LOG("byte orger check error: %x", byteOrder);
#endif									 
		ERR("can't read MS CFB file");
		return CFB_BYTEORDE_ERR;
	}

	// get file header 
	// Header is always 512 bytes long and is always located at offset zero (0).
	fseek(fp, 0, SEEK_SET);	
	if (fread(&cfb->header, 512, 1, fp) != 1){
#ifdef DEBUG
	LOG("can't read file header");
#endif									
		ERR("can't read MS CFB file");		 
		return CFB_READ_ERR|CFB_HEADER_ERR;
	}

	// make byte order change
	if (cfb->biteOrder){
		cfb->header._clid.a = bswap_32(cfb->header._clid.a); 
		cfb->header._clid.b = bswap_32(cfb->header._clid.b); 
		cfb->header._clid.c = bswap_32(cfb->header._clid.c); 
		cfb->header._clid.d = bswap_32(cfb->header._clid.d); 

		cfb->header._uMinorVersion = bswap_16(cfb->header._uMinorVersion);
		cfb->header._uDllVersion = bswap_16(cfb->header._uDllVersion);
		cfb->header._uSectorShift = bswap_16(cfb->header._uSectorShift);
		cfb->header._uMiniSectorShift = bswap_16(cfb->header._uMiniSectorShift);
		cfb->header._usReserved = bswap_16(cfb->header._usReserved);
		cfb->header._ulReserved1 = bswap_32(cfb->header._ulReserved1); 
		cfb->header._ulReserved2 = bswap_32(cfb->header._ulReserved2); 
		cfb->header._csectFat = bswap_32(cfb->header._csectFat); 
		cfb->header._sectDirStart = bswap_32(cfb->header._sectDirStart); 
		cfb->header._signature = bswap_32(cfb->header._signature); 
		cfb->header._ulMiniSectorCutoff = bswap_32(cfb->header._ulMiniSectorCutoff); 
		cfb->header._sectMiniFatStart = bswap_32(cfb->header._sectMiniFatStart); 
		cfb->header._csectMiniFat = bswap_32(cfb->header._csectMiniFat); 
		cfb->header._sectDifStart = bswap_32(cfb->header._sectDifStart); 
		cfb->header._csectDif = bswap_32(cfb->header._csectDif); 
	}
	
	/* check signature */
#ifdef DEBUG
	LOG("check signature");
#endif									 
	bool signature = true;
	unsigned char * ptr = (unsigned char *)(cfb->header._abSig);

	for (i=0; i<8; i++){
		if ((*ptr != cfb_signature[i] && *ptr != cfb_signature_old[i])){
			signature = false;
			break;
		}
		ptr++;
	}	

	if (!signature){
#ifdef DEBUG
	LOG("error signature: ");
	for (i=0; i<8; i++){	
		LOG("%x, ", cfb->header._abSig[i]);
	}
#endif									 
		ERR("can't read MS CFB file");		 
		return CFB_SIG_ERR;
	}

	if (cfb->header._csectMiniFat > 0){
#ifdef DEBUG
	LOG("get mini stream");
#endif									 
/*
 * The mini stream is chained within the FAT in exactly the same fashion as any normal stream. 
 * The mini stream's starting sector is referenced in the first directory entry (root storage 
 * stream ID 0).
 */	
		cfb_get_dir_by_sid(cfb, &(cfb->root), 0);
		cfb->ministream = cfb_get_stream_by_dir(cfb, &(cfb->root));

	} else {
		LOG("No miniFAT stream in file\n");
	}
	
	return error;
}

static int cfb_open(struct cfb * cfb, const char * filename){
	FILE * newfile;
	FILE * fp = fopen(filename, "r");
	if (!fp){
#ifdef DEBUG
	LOG("can't open file: %s", filename);
#endif		
		return -1;
	}

	if (fseek(fp,0,SEEK_SET) == -1) {
		if ( errno == ESPIPE ) {
			//We got non-seekable file, create temp file
#ifdef DEBUG
	LOG("non-seekable file, create temp file...");
#endif			
			if((newfile=tmpfile()) == NULL) {
#ifdef DEBUG
	LOG("can't create temp file");
#endif				
				return -1;
			}
			int ch;
			while ((ch = fgetc(fp)) != EOF)
				fputc(ch, newfile);
			
			fclose(fp);
			fseek(newfile,0,SEEK_SET);
		} else {
			ERR("can't open file");		 
			return -1;
		}
	} else {
		newfile=fp;
	}	
	
	return _cfb_init(cfb, newfile); 
};


//resturn len of utf16 string
static size_t _utf8_to_utf16(const char * utf8, int len, WORD * utf16){
	int i;
	char *ptr = (char *)utf8;
	while (*ptr){ //iterate chars
		
		uint8_t utf8_char[4];

		//get utf32
		uint16_t utf16_char;
		if ((*ptr & 240) == 240) {
			//take last 3 bit from first char
			uint32_t byte0 = (*ptr++ & 7) << 18;	
			
			//take last 6 bit from second char
			uint16_t byte1 = (*ptr++ & 63) << 12;	
			
			//take last 6 bit from third char
			uint16_t byte2 = (*ptr++ & 63) << 6;	
			
			//take last 6 bit from forth char
			uint16_t byte3 = *ptr++ & 63;	
			
			utf16_char = (byte0 | byte1 | byte2 | byte3);					
		} 
		else if ((*ptr & 224) == 224) {
			//take last 4 bit from first char
			uint16_t byte0 = (*ptr++ & 15) << 12;	
			
			//take last 6 bit from second char
			uint16_t byte1 = (*ptr++ & 63) << 6;	
			
			//take last 6 bit from third char
			uint16_t byte2 = *ptr++ & 63;	

			utf16_char = (byte0 | byte1 | byte2);
		} 
		else if ((*ptr & 192) == 192){
			//take last 5 bit from first char
			uint16_t byte0 = (*ptr++ & 31) << 6;	
			
			//take last 6 bit from second char
			uint16_t byte1 = *ptr++ & 63;	

			utf16_char = (byte0 | byte1);
		}
		else {
			utf16_char = *ptr++;
		} 				
					
		utf16[i++] = utf16_char;
	}
	return i;
}

#define cfb_get_dir(cfb, dir, arg)\
	_Generic((arg), \
			char*: cfb_get_dir_by_name, \
			int:   cfb_get_dir_by_sid \
	)((cfb), (dir), (arg))	



static int cfb_get_dirs(struct cfb * cfb, void * user_data,
			int(*callback)(void * user_data, cfb_dir dir))
{
	int i=0, c;
	cfb_dir dir;
	c = cfb_get_dir_by_sid(cfb, &dir, i++);
	while(c == 0 && dir._ab[0]){
		if (callback){
			if (callback(user_data, dir)){
				return 1;
			}
		}
		c = cfb_get_dir_by_sid(cfb, &dir, i++);
	}

	return 0;
}

static void cfb_close(struct cfb * cfb){
	fclose(cfb->ministream);
	fclose(cfb->fp);
}

#ifdef __cplusplus
}
#endif

#endif //CFB_H_

// vim:ft=c
