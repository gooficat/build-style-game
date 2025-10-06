#pragma once

#include <stdint.h>

typedef struct vec2 {
    float x;
    float y;
} vec2;

typedef struct vec3 {
    float x;
    float y;
    float z;
} vec3;

typedef struct vec2i {
	int x, y;
} ve2i;

typedef struct wall {
	float x1, y1;
	float x2, y2;
	uint16_t color;
} wall_t;

typedef struct {
	uint8_t idx, end;
	float z1, z2;
	uint16_t color;
} sector_t;
