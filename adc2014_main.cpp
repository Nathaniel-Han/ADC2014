//
//  main.cpp
//  RTreeTest
//
//  Created by Yuxing Han on 8/23/15.
//  Copyright (c) 2015 Yuxing Han. All rights reserved.
//
#include <string>
#include <unordered_map>
#include "NN_RST.h"

using namespace std;

int ParsePointFile();

#define QUERY_SIZE 3
double coor1[2] = {0.275298, 0.672161};
double coor2[2] = {0.536726, 0.667874};
double coor3[2] = {0.245277, 0.053420};
Point p1(coor1, 2);
Point p2(coor2, 2);
Point p3(coor3, 2);
NN_RST rst(&p1), rst2(&p2), rst3(&p3);

vector<NN_RST*> rst_vector{&rst, &rst2, &rst3};

boost::mutex io_mutex;
boost::mutex bool_mutex;
ISpatialIndex* tree=nullptr;
bool continued = true;

unordered_map<id_type, double> enhanced_mbr; // node_id(internel) -> corresponding maximum speed
vector<Taxi_Point> point_set;


void retrive()
{
    int d;
    for (int i=0; i< 100; i++)
    {
        d = rst.Get();
        {
            boost::mutex::scoped_lock lock(io_mutex);
            cout << i << ": " << d << endl;
        }
    }
    {
//        boost::mutex::scoped_lock lock(bool_mutex);
//        when procedure executed here, thread(gen) is wait for the condition, so 'continued' is safe
        continued = false;
        //wake thread(gen) so that it can stop
        rst.cond.notify_one();
    }
}

void gen()
{
    rst.Put();
}

int main(int argc, const char * argv[]) {
    string basename("tree");
    
    IStorageManager *diskfile = StorageManager::loadDiskStorageManager(basename);
    StorageManager::IBuffer* file = StorageManager::createNewRandomEvictionsBuffer(*diskfile, 10, false);
    tree = RTree::loadRTree(*file, 1);
    
    
     ParsePointFile();
//    Usage of IStatistics
    IStatistics *sts;
    tree->getStatistics(&sts);
//    cout << *sts <<endl;
    int max_level = dynamic_cast<RTree::Statistics*>(sts)->getTreeHeight();
    
    
    for (int m_level = 0; m_level < max_level; ++m_level) {
        LevelStrategy qs(m_level);
        tree->queryStrategy(qs);
    }
    
    CheckStrategy cs;
    tree->queryStrategy(cs);
    
    boost::thread thd1(&retrive);
    boost::thread thd2(&gen);
    
    thd1.join();
    thd2.join();
    
    //cout << *dynamic_cast<Point*>(rst_vector[0]->query)  <<endl;
    delete tree;
    delete file;
    delete diskfile;
    return 0;
}


int ParsePointFile()
{
    int traj_point_num= -1;
    int traj_id,traj_len;
    
    double x,y,v;
    char datetime[25];
    
    
    FILE *fp = fopen("result.txt","r");
    
    if(!fp){
        printf("Can't open file!");
        fclose(fp);
        return -1;
    }
    
    while(!feof(fp)){
        
        fscanf(fp,"%i %i\n",&traj_id,&traj_len);
        //printf("%i %i\n",traj_id,traj_len);
        
        for(int i=0;i<traj_len;i++){
            fscanf(fp,"%s %lf %lf %lf\n",&datetime,&x,&y,&v);
            //printf("%s %lf %lf %lf\n",datetime,x,y,v);
            traj_point_num++;
            double coor[2] = {x,y};
            Taxi_Point p(coor, 2, traj_point_num, traj_id, v);
            point_set.push_back(p);
        }
        
    }
    fclose(fp);
    
    printf("traj_point_num=%i\ntraj_num=%i\n", point_set.size(),traj_id);
    
    return traj_id;
    
}






















