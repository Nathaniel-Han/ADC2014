//
//  global.h
//  ADC2014
//
//  Created by Yuxing Han on 8/29/15.
//  Copyright (c) 2015 Yuxing Han. All rights reserved.
//

#ifndef global_h
#define global_h


#include "adc2014_utility.h"
#include "config.h"
#include <boost/thread/thread.hpp>

boost::mutex io_mutex;
boost::mutex bool_mutex;
boost::mutex disk_mutex;
ISpatialIndex* tree = nullptr;
bool continued = true;

unordered_map<id_type, double> enhanced_mbr; // node_id(internel) -> corresponding maximum speed
vector<Taxi_Point> point_set;


std::priority_queue<QEntry, vector<QEntry>, QEntry::ascending> Q;
//vector<int> C;                 // traj candidates
std::unordered_map<int, bitset<QUERY_SIZE>> CTestHelper;



#endif
