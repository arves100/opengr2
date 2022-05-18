/*!
	Project: libopengrn
	File: structures.h
	Contains basic structures of a Gr2 file

	This Source Code Form is subject to the terms of the Mozilla Public
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <stdint.h>

/*!
	Basic layout of a Gr2 file:

	[Header] [File Info] [Sector information] [Data]

	Sector information length is detected from the file info

	Data contains: FixUp Data, Marshalling Data and the various Sectors data

	A Granny2 file is divided in different sectors (usually 8) which contains informations
		about the file

	Mapped file:
	[Sector 0] [Sector 1] [Sector 2] [Sector 3] ...

	The Gr2 has two important sectors to keep track of:
		The Type sector which contains the information of the node that exists in the file
		The Root sector which contains the data of the file itself

	The content of a Gr2 is simply static structures that contains the model or animation info
	Some example can include: reference to texture, vertices data and so on

	As the structures that defines a Gr2 could be modified (for example in a new update of the Gr2),
		RAD introduced the Tag information in the File info, which contains the version of the
		structure data.

	The Gr2 dll convers the old tag version to a new one before parsing the data.

	As the structures can be dynamic, this means that games could provide custom nodes that would
		break importing or exporting (much like GameBryo Nif file format)

	The library, therefore, cannot be binded to a static structure (like the Gr2 dll does),
		which is why it provides two operation modes:
			- Low level operation mode
				This parses the node and allow the developer to take full controll of the Gr2 nodes
			- High level operation mode
				This contains a static structure that is usually correct on games which the Gr2 file
				has not been customized
*/

/*!
	Gr2 file header
*/
typedef struct SHeader
{
	uint32_t magic[4]; /* magic bytes (used for identification of the file) */
	uint32_t sizeWithSectors; /* size of the file info with sector info */
	uint32_t format; /* format of the file info */
	uint8_t extra[8]; /* unknown bytes */
} THeader;

/*!
	Reference of a place in the data
*/
typedef struct SReference
{
	uint32_t sector; /* sector where the reference points */
	uint32_t position; /* position of the sector where the reference points */
} TReference;

/*!
	Gr2 file information
*/
typedef struct SFileInfo
{
	int32_t format; /* file format info */
	uint32_t totalSize; /* total size of the file */
	uint32_t crc32; /* CRC32 of the file after the file info (Swapped endianness) */
	uint32_t fileInfoSize; /* size of the file info */
	uint32_t sectorCount; /* number of sectors (usually 8) */
	TReference type; /* reference where the type node exists */
	TReference root; /* reference where the root node exists */
	uint32_t tag; /* version of the type node */
	uint8_t extra[32]; /* unknown bytes */
} TFileInfo;

/*!
	The fixup contains informations about fixing the sector pointers
	(It's a way to use pointer based structure inside the code without crashing, like
		fixing the pointer of the ArtToolInfo node name or it's children pointer)

	The destination information contains where the data should point, while
	the source sector would contain the pointer information
*/
typedef struct SFixUpData
{
	uint32_t srcOffset; /* position of the current sector to modify the pointer */
	uint32_t dstSector; /* sector where the pointer should point */
	uint32_t dstOffset; /* position of where the pointer should point in the sector */
} TFixUpData;

/*!
	The marshalling data contains information on how to fix the sector data when
	the file endianness mismatch the CPU endianness

	The bytes of the node information are swapped based to match the correct CPU
	endianness

	The destination contains the node information to swap in order to retrive the
	member informations

	The source sector contains the data value to swap
*/
typedef struct SMarshallData
{
	uint32_t count; /* number of types to fix in the current sector */
	uint32_t srcOffset; /* offset where the data exists */
	uint32_t dstSector; /* sector where the node information points */
	uint32_t dstOffset; /* position where the node information exists */
} TMarshallData;

/*!
	Information of a Gr2 sector
*/
typedef struct SSector
{
	uint32_t compressType; /* type of compression used */
	uint32_t dataOffset; /* positition relative to the file where the sector data exists */
	uint32_t compressedLen; /* length of the data after compression (the one stored in the file) */
	uint32_t decompressLen; /* length of the data after decompression */
	uint32_t alignment; /* the sector decompressed data must align to this number (usually 4) */
	uint32_t oodleStop0; /* first 16 bits of Oodle compression, also used in endianness swap */
	uint32_t oodleStop1; /* first 8 bits of Oodle compression, also used in endianness swap */
	uint32_t fixupOffset; /* offset where the fixup data exists, relative to the file */
	uint32_t fixupSize; /* number of fixup data included in this sector */
	uint32_t marshallOffset; /* offset where the marshalling data exists, relative to the file */
	uint32_t marshallSize; /* number of marshalling data included in this sector */
} TSector;

/*!
	information of a Gr2 node
*/
typedef struct SNodeTypeInfo
{
	uint32_t type; /* the type of the node */
	uint64_t nameOffset; /* pointer to the name */
	uint64_t childrenOffset; /* pointer to the childrens of the node */
	int32_t arraySize; /* size of the array, if the node supports arrays */
	uint32_t extra[3]; /* unknown bytes */
	uint64_t extra4; /* pointer size */
} TNodeTypeInfo;

/*!
	Mesh transformation (tralsation+rotation+scaling)
*/
typedef struct STransformation
{
	uint32_t flags; // TODO: discover what flags
	float translation[3]; /// X,Y,Z traslation
	float rotation[4]; /// X,Y,Z,W rotation
	float scaleShear[3][3]; /// Scale matrix
} TTransformation;
