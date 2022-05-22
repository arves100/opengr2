#pragma once

#include "structures.h"
#include "dllapi.h"

#include <stdbool.h>


/*!
	Information about element types
*/
typedef struct STypeInfo
{
	uint32_t size32; /* size of the element in 32bit mode */
	uint32_t size64; /* size of the element in 64bit mode */
	uint32_t swapSize; /* size for swap type in marshalling fix */
} TTypeInfo;

/*!
	Gr2 element types
*/
enum TypeIDs
{
	TYPEID_NONE = 0, /// No node
	TYPEID_INLINE = 1, /// Empty node with just children
	TYPEID_REFERENCE = 2, /// Reference to a pointer
	TYPEID_REFERENCETOARRAY = 3, /// Reference to an array
	TYPEID_ARRAYOFREFERENCES = 4, /// Array containing a numbers of pointers
	TYPEID_VARIANTREFERENCE = 5, /// Reference with offset
	TYPEID_REMOVED = 6, // TODO: We know this was used to be reference or a custom type, is there anything that reference this?
	TYPEID_REFERENCETOVARIANTARRAY = 7, // Reference to an array with offset
	TYPEID_STRING = 8, /// String
	TYPEID_TRANSFORM = 9, /// @see TTransformation
	TYPEID_REAL32 = 10, /// 32bit floating value
	TYPEID_INT8 = 11, /// 8bit number signed
	TYPEID_UINT8 = 12, /// 8bit number unsigned
	TYPEID_BINORMALINT8 = 13, // TODO: discover what changes between this and int8
	TYPEID_NORMALUINT8 = 14, // TODO: discover what changes between this and uint8
	TYPEID_INT16 = 15, /// 16bit number signed
	TYPEID_UINT16 = 16, /// 16bit number unsigned
	TYPEID_BINORMALINT16 = 17, // TODO: discover what changes between this and int16
	TYPEID_NORMALUINT16 = 18, // TODO: discover what changes between this and uint16
	TYPEID_INT32 = 19, /// 32bit number signed
	TYPEID_UINT32 = 20, /// 32bit number unsigned
	TYPEID_REAL16 = 21, // half-sized floating value
	TYPEID_EMPTYREFERENCE = 22, /// Reference to nothing
	// End of known elements as of Granny 2.12.0.2 (mostly this will be all)

	TYPEID_MAX,
};

/*!
	Global holder of element type info
*/
extern OG_DLLAPI TTypeInfo ELEMENT_TYPE_INFO[23];


/*!
	Parses the node information from raw data
	@param data The data to parse
	@param info Output structure
	@param is64 Set this to true if you are loading a 64-bit file
	@param offset This variable should be set to the current offset of the data, the new offset will be projected into this variable
	@return true if the parsing succeeded, otherwise false
*/
bool TypeInfo_Parse(const uint8_t* data, TNodeTypeInfo* info, bool is64, uint64_t* offset);
