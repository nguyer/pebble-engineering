#pragma once
#include <pebble.h>

char *uppercase(char *str);

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