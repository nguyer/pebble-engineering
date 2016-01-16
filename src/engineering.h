#pragma once
#include <pebble.h>
	
#define KEY_TEMPERATURE 0
#define KEY_SHOW_NUMBERS 1
#define KEY_SHOW_SECOND_HAND 2
#define KEY_SHOW_DATE 3
#define KEY_SHOW_TEMPERATURE 4
#define KEY_COLOR_BACKGROUND 5
#define KEY_COLOR_LABEL 6
#define KEY_COLOR_HOUR_MARKS 7
#define KEY_COLOR_MINUTE_MARKS 8
#define KEY_COLOR_HOUR_HAND 9
#define KEY_COLOR_MINUTE_HAND 10
#define KEY_COLOR_SECOND_HAND 11

#define INSET PBL_IF_ROUND_ELSE(1, 0)
#define HOURS_RADIUS 3

char *uppercase(char *str);

#ifdef PBL_RECT
static const GPathInfo MINUTE_HAND_POINTS = {
  9,
  (GPoint []) {
	{ -2, 0},
	{ 2, 0 },
	{ 2, -20 },
	{ 4, -20 },
    { 4, -60 },
	{ 0, -64 },
	{ -4, -60 },
	{ -4, -20 },
	{ -2, -20 }
  }
};
static const GPathInfo HOUR_HAND_POINTS = {
  9, (GPoint []){
    { -2, 0 },
	{ 2, 0 },
	{ 2, -15 },
	{ 4, -15 },
    { 4, -44 },
	{ 0, -48 },
	{ -4, -44 },
	{ -4, -15 },
	{ -2, -15 }
  }
};
#else
static const GPathInfo MINUTE_HAND_POINTS = {
  9,
  (GPoint []) {
	{ -2, 0},
	{ 2, 0 },
	{ 2, -25 },
	{ 4, -25 },
    { 4, -75 },
	{ 0, -79 },
	{ -4, -75 },
	{ -4, -25 },
	{ -2, -25 }
  }
};
static const GPathInfo HOUR_HAND_POINTS = {
  9, (GPoint []){
    { -2, 0 },
	{ 2, 0 },
	{ 2, -20 },
	{ 4, -20 },
    { 4, -59 },
	{ 0, -63 },
	{ -4, -59 },
	{ -4, -20 },
	{ -2, -20 }
  }
};
#endif