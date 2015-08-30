//
//  config.h
//  ADC2014
//
//  Created by Yuxing Han on 8/28/15.
//  Copyright (c) 2015 Yuxing Han. All rights reserved.
//

#ifndef config_h
#define config_h

#include "NN_RST.h"


#define QUERY_SIZE 3
#define QUERY_K 5

#ifdef DEBUG_TEST
#define POINT_NUM 10000
#endif

double coor1[2] = {0.275298, 0.672161};
double coor2[2] = {0.536726, 0.667874};
double coor3[2] = {0.245277, 0.053420};
Point p1(coor1, 2);
Point p2(coor2, 2);
Point p3(coor3, 2);
NN_RST rst(&p1), rst2(&p2), rst3(&p3);

vector<NN_RST*> rst_vector{&rst, &rst2, &rst3};


#endif
