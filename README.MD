# OpenGr2
Open source Granny2 Input/Output library and tools written in C.

## Compatibility table

| Functionality | Status |
| ------------- | ------ |
| Basic parsing | ⚠️ (File Format 6 only, missing File Format 7) |
| Basic writing | ❌ (WIP, missing Node creations) |
| Big Endian files | ❌ (Theorical parsing support added with the exception of marshalling) |
| 64-bit pointer files | ❌ (Typeinfo parsing should be ok, everything else no) |
| Oodle-0 compression/decompression | ❌ |
| Oodle-1 compression/decompression | ⚠️ (Only decompression is supported) |
| Bitknit-1 compression/decompression | ❌ |
| Bitknit-2 compression/decompression | ❌ |

## Low Level/High Level API
The Granny2 format was built to be extensible by its creators, game companies could alter the nodes that contains the meshes or its structure.

A Gr2 from a game might not contain custom or specific nodes that are required for a different game, for this reason libopengrn supports two mode of operations, respectively called Low Level and High Level operation.

### Low level operations
The low level API contains the core of the Gr2 file parsing and be used to parse any GR2 file without knowing it's node values or data structure (how a mesh is rappresented inside a Gr2).

You should use this API if you want to manipulate all the informations of your Gr2 file or you want to modify a specific file for a game

### High level operations (WIP)
The high level API provides serialization of the common Gr2 structure used by the majority of games, it is built upon the functionality of the Low level API and provides a simple abstraction to it.
This API is designed to be easy to use but less powerfull that the low level API.

You should generally use this API unless you want to do custom functionalities for your Gr2.