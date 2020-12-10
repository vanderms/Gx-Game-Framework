#ifndef GX_TILEMAP_H
#define GX_TILEMAP_H
#include "../../Gx.h"


void nTilemapCreate(sElement* base, 
		const char* path, sMatrix matrix, const int* sequence
);

//... some useful macros for creating sequecences

#define xTilemap(rows, columns, ...) (sMatrix){rows, columns}, (int[rows * columns])__VA_ARGS__

#define x1(...) __VA_ARGS__
#define x2(...) __VA_ARGS__, __VA_ARGS__
#define x3(...) __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x4(...) __VA_ARGS__, __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x5(...) __VA_ARGS__, __VA_ARGS__, __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x6(...) x5(__VA_ARGS__), __VA_ARGS__
#define x7(...) x5(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__
#define x8(...) x5(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x9(...) x5(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x10(...) x5(__VA_ARGS__), x5(__VA_ARGS__)
#define x11(...) x10(__VA_ARGS__), __VA_ARGS__
#define x12(...) x10(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__
#define x13(...) x10(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x14(...) x10(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x15(...) x10(__VA_ARGS__), x5(__VA_ARGS__)
#define x16(...) x10(__VA_ARGS__), x5(__VA_ARGS__), __VA_ARGS__
#define x17(...) x10(__VA_ARGS__), x5(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__
#define x18(...) x10(__VA_ARGS__), x5(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x19(...) x10(__VA_ARGS__), x5(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x20(...) x10(__VA_ARGS__), x10(__VA_ARGS__)
#define x21(...) x20(__VA_ARGS__), __VA_ARGS__
#define x22(...) x20(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__
#define x23(...) x20(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x24(...) x20(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x25(...) x20(__VA_ARGS__), x5(__VA_ARGS__)
#define x26(...) x20(__VA_ARGS__), x5(__VA_ARGS__), __VA_ARGS__
#define x27(...) x20(__VA_ARGS__), x5(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__
#define x28(...) x20(__VA_ARGS__), x5(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x29(...) x20(__VA_ARGS__), x5(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x30(...) x20(__VA_ARGS__), x10(__VA_ARGS__)
#define x31(...) x30(__VA_ARGS__), __VA_ARGS__
#define x32(...) x30(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__
#define x33(...) x30(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x34(...) x30(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x35(...) x30(__VA_ARGS__), x5(__VA_ARGS__)
#define x36(...) x30(__VA_ARGS__), x5(__VA_ARGS__), __VA_ARGS__
#define x37(...) x30(__VA_ARGS__), x5(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__
#define x38(...) x30(__VA_ARGS__), x5(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x39(...) x30(__VA_ARGS__), x5(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x40(...) x30(__VA_ARGS__), x10(__VA_ARGS__)
#define x41(...) x40(__VA_ARGS__), __VA_ARGS__
#define x42(...) x40(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__
#define x43(...) x40(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x44(...) x40(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x45(...) x40(__VA_ARGS__), x5(__VA_ARGS__)
#define x46(...) x40(__VA_ARGS__), x5(__VA_ARGS__), __VA_ARGS__
#define x47(...) x40(__VA_ARGS__), x5(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__
#define x48(...) x40(__VA_ARGS__), x5(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x49(...) x40(__VA_ARGS__), x5(__VA_ARGS__), __VA_ARGS__, __VA_ARGS__, __VA_ARGS__, __VA_ARGS__
#define x50(...) x40(__VA_ARGS__), x10(__VA_ARGS__)
#endif // !GX_TILEMAP_H
