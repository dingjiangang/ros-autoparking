/******************************************************************
 * Filename: autoparking.h
 * Version: v1.0
 * Author: Meng Peng
 * Date: 2020-04-28
 * Description: declare global variables and funtions for this project
 * 
 ******************************************************************/

#ifndef AUTOPARKING_H_
#define AUTOPARKING_H_

#define setbit(x, y) x|=(1<<y)
#define clrbit(x, y) x&=~(1<<y)
//  x   x   x   x   0   0   0   0
// high four bits not used
// 0 bit: parking space for perpendicular parking
// 1 bit: parking type is perpendicular
// 2 bit: parking space for parallel parking
// 3 bit: parking type is parallel

extern const float range_diff;              // range difference to distinguish turn point 
extern const float distance_search;         // maximum distance between car and parking space
extern const float parallel_width;          // minimum width of parallel parking space
extern const float parallel_length;         // minimum length of parallel parking space
extern const float perpendicular_width;     // minimum width of perpendicular parking space
extern const float perpendicular_length;    // minimum length of perpendicular parking space
extern const float car_width;               // minimum width of car
extern const float car_length;              // minimum length of car

#endif