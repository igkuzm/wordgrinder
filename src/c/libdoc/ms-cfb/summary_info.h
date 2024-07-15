/**
 * File              : summary_info.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 05.11.2022
 * Last Modified Date: 16.11.2022
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#ifndef SUMMARY_H_
#define SUMMARY_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include "property_set.h"
#include "cfb.h"

/*
 * COM defines a standard common property set for storing summary information about documents. 
 * The Summary Information property set must be stored in a stream object. That is, this property 
 * set must be stored as a simple property set. 
 *
 * All shared property sets are identified by a stream or storage name with the prefix "\005" (or 
 * 0x05) to show that it is a property set that can be shared among applications. The Summary 
 * Information property set is no exception. The name of the stream that contains the Summary 
 * Information property set is: "\005SummaryInformation"
 * The FMTID for the Summary Information property set is: F29F85E0-4FF9-1068-AB91-08002B27B3D9
 */

/*
 * function `summary_get_SummaryInformation`
 * Read properties from standard common property set file, execute callback for each found
 * property. Return error code. To stop function execution you may return non 0 in callback
 * function. 
 */
static int 
summary_get_SummaryInformation(struct cfb * cfb, void * user_data,
	int (*callback)(void * user_data, uint32_t propid, uint32_t dwType, uint8_t * value));

/*
 * function `summary_get_SummaryInformation`
 * Read properties from standard common property set file, execute callback for each found
 * property. Return error code. To stop function execution you may return non 0 in callback
 * function. 
 */
static int 
summary_get_DocumentSummaryInformation(struct cfb * cfb, void * user_data,
	int (*callback)(void * user_data, uint32_t propid, uint32_t dwType, uint8_t * value));


/*
 * IMP
 */

typedef struct type_of_property {
	uint32_t prop;
	int		 type;
} top_t;

/*
 * The following table lists the string property names for the Summary Information property set, 
 * along with the respective property identifiers and variable type (VT) indicators. The names 
 * are not typically stored in the property set, but are inferred from the Property ID value. The 
 * Property ID String entries shown here correspond to definitions found in the header files.
 */

/*
Name							Property ID string	Property	ID	VT type

Codepage						PIDSI_CODEPAGE		0x00000001	VT_I2
Title							PIDSI_TITLE			0x00000002	VT_LPSTR
Subject							PIDSI_SUBJECT		0x00000003	VT_LPSTR
Author							PIDSI_AUTHOR		0x00000004	VT_LPSTR
Keywords						PIDSI_KEYWORDS		0x00000005	VT_LPSTR
Comments						PIDSI_COMMENTS		0x00000006	VT_LPSTR
Template						PIDSI_TEMPLATE		0x00000007	VT_LPSTR
Last Saved By					PIDSI_LASTAUTHOR	0x00000008	VT_LPSTR
Revision Number					PIDSI_REVNUMBER		0x00000009	VT_LPSTR
Total Editing Time				PIDSI_EDITTIME		0x0000000A	VT_FILETIME (UTC)
Last Printed					PIDSI_LASTPRINTED	0x0000000B	VT_FILETIME (UTC)
Create  Time/Date				PIDSI_CREATE_DTM	0x0000000C	VT_FILETIME (UTC)
Last saved Time/Date			PIDSI_LASTSAVE_DTM	0x0000000D	VT_FILETIME (UTC)
Number of Pages					PIDSI_PAGECOUNT		0x0000000E	VT_I4
Number of Words					PIDSI_WORDCOUNT		0x0000000F	VT_I4
Number of Characters			PIDSI_CHARCOUNT		0x00000010	VT_I4
Thumbnail						PIDSI_THUMBNAIL		0x00000011	VT_CF
Name of Creating Application	PIDSI_APPNAME		0x00000012	VT_LPSTR
Security						PIDSI_SECURITY		0x00000013	VT_I4            
*/

#define PIDSI_CODEPAGE		0x00000001
#define PIDSI_TITLE			0x00000002
#define PIDSI_SUBJECT		0x00000003
#define PIDSI_AUTHOR		0x00000004
#define PIDSI_KEYWORDS		0x00000005
#define PIDSI_COMMENTS		0x00000006
#define PIDSI_TEMPLATE		0x00000007
#define PIDSI_LASTAUTHOR	0x00000008
#define PIDSI_REVNUMBER		0x00000009
#define PIDSI_EDITTIME		0x0000000A
#define PIDSI_LASTPRINTED	0x0000000B
#define PIDSI_CREATE_DTM	0x0000000C
#define PIDSI_LASTSAVE_DTM	0x0000000D
#define PIDSI_PAGECOUNT		0x0000000E
#define PIDSI_WORDCOUNT		0x0000000F
#define PIDSI_CHARCOUNT		0x00000010
#define PIDSI_THUMBNAIL		0x00000011
#define PIDSI_APPNAME		0x00000012
#define PIDSI_SECURITY		0x00000013

static const top_t SIPS[] =              //Summary Information property set 
{
	{0x00000001,  PSET_I2       },
	{0x00000002,  PSET_LPSTR    },
	{0x00000003,  PSET_LPSTR    },
	{0x00000004,  PSET_LPSTR    },
	{0x00000005,  PSET_LPSTR    },
	{0x00000006,  PSET_LPSTR    },
	{0x00000007,  PSET_LPSTR    },
	{0x00000008,  PSET_LPSTR    },
	{0x00000009,  PSET_LPSTR    },
	{0x0000000A,  PSET_FILETIME },
	{0x0000000B,  PSET_FILETIME },
	{0x0000000C,  PSET_FILETIME },
	{0x0000000D,  PSET_FILETIME },
	{0x0000000E,  PSET_I4       },
	{0x0000000F,  PSET_I4       },
	{0x00000010,  PSET_I4       },
	{0x00000011,  PSET_CF       },
	{0x00000012,  PSET_LPSTR    },
	{0x00000013,  PSET_I4       },
};

/*
 * A DocumentSummaryInformation and UserDefined property set is an extension to the Summary 
 * Information property set. Both property sets can exist simultaneously.
 *
 * The name of the stream that contains the DocumentSummaryInformation property set is 
 * "\005DocumentSummaryInformation". The format identifier (FMTID) for the 
 * DocumentSummaryInformation property set is D5CDD502-2E9C-101B-9397-08002B2CF9AE.
 *
 * This stream also has a separate section for the custom-user-defined properties as in the 
 * DocumentSummaryInformation and UserDefined property sets.
 * These two property sets are the only ones for which a single stream can hold multiple property 
 * sets. 
 */


/*
 * The following table lists the added properties to the DocumentSummaryInformation and 
 * UserDefined property set. As in the SummaryInformation property set, the names are not 
 * typically stored in the property set, but are inferred from the property identifier.
 */

/*
Property name		Property identifier	Property identifier value	VARIANT type

Codepage			PIDSI_CODEPAGE		0x00000001					VT_I2
Category	        PIDDSI_CATEGORY		0x00000002					VT_LPSTR
PresentationTarget	PIDDSI_PRESFORMAT	0x00000003					VT_LPSTR
Bytes	            PIDDSI_BYTECOUNT	0x00000004					VT_I4
Lines			    PIDDSI_LINECOUNT	0x00000005					VT_I4
Paragraphs			PIDDSI_PARCOUNT		0x00000006					VT_I4
Slides				PIDDSI_SLIDECOUNT	0x00000007					VT_I4
Notes				PIDDSI_NOTECOUNT	0x00000008					VT_I4
HiddenSlides		PIDDSI_HIDDENCOUNT	0x00000009					VT_I4
MMClips				PIDDSI_MMCLIPCOUNT	0x0000000A					VT_I4
ScaleCrop			PIDDSI_SCALE		0x0000000B					VT_BOOL
HeadingPairs		PIDDSI_HEADINGPAIR	0x0000000C					VT_VARIANT | VT_VECTOR
TitlesofParts		PIDDSI_DOCPARTS		0x0000000D					VT_VECTOR | VT_LPSTR
Manager				PIDDSI_MANAGER		0x0000000E					VT_LPSTR
Company				PIDDSI_COMPANY		0x0000000F					VT_LPSTR
LinksUpToDate		PIDDSI_LINKSDIRTY	0x00000010					VT_BOOL                
*/

#define PIDDSI_CATEGORY		0x00000002
#define PIDDSI_PRESFORMAT	0x00000003
#define PIDDSI_BYTECOUNT	0x00000004
#define PIDDSI_LINECOUNT	0x00000005
#define PIDDSI_PARCOUNT		0x00000006
#define PIDDSI_SLIDECOUNT	0x00000007
#define PIDDSI_NOTECOUNT	0x00000008
#define PIDDSI_HIDDENCOUNT	0x00000009
#define PIDDSI_MMCLIPCOUNT	0x0000000A
#define PIDDSI_SCALE		0x0000000B
#define PIDDSI_HEADINGPAIR	0x0000000C
#define PIDDSI_DOCPARTS		0x0000000D
#define PIDDSI_MANAGER		0x0000000E
#define PIDDSI_COMPANY		0x0000000F
#define PIDDSI_LINKSDIRTY	0x00000010

static const top_t DSIPS[] =              //DocumentSummaryInformation property set 
{
	{0x00000001,  PSET_I2				    },
	{0x00000002,  PSET_LPSTR                },
	{0x00000003,  PSET_LPSTR                },
	{0x00000004,  PSET_I4                   },
	{0x00000005,  PSET_I4                   },
	{0x00000006,  PSET_I4                   },
	{0x00000007,  PSET_I4                   },
	{0x00000008,  PSET_I4                   },
	{0x00000009,  PSET_I4                   },
	{0x0000000A,  PSET_I4                   },
	{0x0000000B,  PSET_BOOL                 },
	{0x0000000C,  PSET_VARIANT | PSET_VECTOR},
	{0x0000000D,  PSET_VECTOR | PSET_LPSTR  },
	{0x0000000E,  PSET_LPSTR                },
	{0x0000000F,  PSET_LPSTR                },
	{0x00000010,  PSET_BOOL                 },
};

/*
 * These properties have the following uses:
 * 
 * Category
 * A text string typed by the user that indicates what category the file belongs to (memo, proposal, and so on). It is useful for finding files of same type.
 *
 * PresentationTarget
 * Target format for presentation (35mm, printer, video, and so on).
 *
 * Bytes
 * Number of bytes.
 *
 * Lines
 * Number of lines.
 *
 * Paragraphs
 * Number of paragraphs.
 *
 * Slides
 * Number of slides.
 *
 * Notes
 * Number of pages that contain notes.
 *
 * HiddenSlides
 * Number of slides that are hidden.
 *
 * MMClips
 * Number of sound or video clips.
 *
 * ScaleCrop
 * Set to True (-1) when scaling of the thumbnail is desired. If not set, cropping is desired.
 *
 * HeadingPairs
 * Internally used property indicating the grouping of different document parts and the number of 
 * items in each group. The titles of the document parts are stored in the TitlesofParts 
 * property. The HeadingPairs property is stored as a vector of variants, in repeating pairs of 
 * VT_LPSTR (or VT_LPWSTR) and VT_I4 values. The VT_LPSTR value represents a heading name, and 
 * the VT_I4 value indicates the count of document parts under that heading.
 *
 * TitlesofParts
 * Names of document parts.
 * 
 * Manager
 * Manager of the project.
 * 
 * Company
 * Company name.
 *
 * LinksUpToDate
 * Boolean value to indicate whether the custom links are hampered by excessive noise, for all 
 * applications.
 */

int _summary_get(struct cfb * cfb, int doc_summary, void * user_data,
	int (*callback)(void * user_data, uint32_t propid, uint32_t dwType, uint8_t * value))
{
	char * dirname = "\005SummaryInformation";
	if (doc_summary) 
		dirname = "\005DocumentSummaryInformation";

	FILE * stream = cfb_get_stream_by_name(cfb, dirname);
	if (!stream)
		return PSET_ERR_FILE;	

	return  property_set_get(stream, user_data, callback);
}

int summary_get_SummaryInformation(struct cfb * cfb, void * user_data,
	int (*callback)(void * user_data, uint32_t propid, uint32_t dwType, uint8_t * value))
{
	return _summary_get(cfb, 0, user_data, callback);
}

int summary_get_DocumentSummaryInformation(struct cfb * cfb, void * user_data,
	int (*callback)(void * user_data, uint32_t propid, uint32_t dwType, uint8_t * value))
{
	return _summary_get(cfb, 1, user_data, callback);
}

#ifdef __cplusplus
}
#endif

#endif //SUMMARY_H_
