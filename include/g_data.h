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
	int x;
	int y;
} vec2i;

typedef struct vec3i {
    int x;
    int y;
    int z;
} vec3i;

typedef struct wall {
	int x, y;
	uint8_t texture;
} wall_t;

typedef struct {
	uint8_t idx, end;
	int z1, z2;
	uint16_t color;
	double dist;
	vec2 pos;
} sector_t;

typedef struct {
	vec3 position;
	float rotation;
	float sin_rotation, cos_rotation;
} entity_t;

typedef struct {
	int w, h;
	uint16_t* pixels;
} texture_t;
