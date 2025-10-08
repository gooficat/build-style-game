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

typedef struct lut {
	int16_t *t, *b;
} lut_t;

typedef struct wall {
	int x, y;
	uint8_t texture;
	int8_t to;
} wall_t;

typedef struct {
	uint8_t idx, end;
	int z1, z2;
	uint16_t color;
	double dist;
	vec2 pos;
	lut_t ceil_lut, floor_lut;
} sector_t;

typedef struct {
	vec3 position;
	float rotation;
	float sin_rotation, cos_rotation;
	uint8_t current_sector;
} entity_t;

typedef struct {
	int w, h;
	uint16_t* pixels;
} texture_t;
