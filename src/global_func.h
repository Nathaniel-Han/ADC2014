//
//  global_func.h
//  ADC2014
//
//  Created by Yuxing Han on 8/29/15.
//  Copyright (c) 2015 Yuxing Han. All rights reserved.
//

#ifndef global_func_h
#define global_func_h

#include "global_value.h"
#include <math.h>

bool AllCoverTest()
{
    int count = 0;
    for (auto iter = CTestHelper.begin(); iter != CTestHelper.end(); ++iter) {
        if (iter->second.all()) {
            all_covering[count] = iter->first;
            ++count;
        }
    }
    if (count >= QUERY_K) {
        for (auto iter = CTestHelper.begin(); iter != CTestHelper.end(); ++iter){
            if(!iter->second.all())
                partial_covering.push_back(iter->first);
        }
        return true;
    }
    return false;
}

inline double ComputePTime(Taxi_Point& p, Point& q)
{
    return sqrt(pow((p.m_pCoords[0] - q.m_pCoords[0]), 2)+ pow((p.m_pCoords[1]-q.m_pCoords[1]), 2))/p.m_speed;
}


double ComputeTTime(int traj_id)
{
    double time = Candidates[traj_id];
    //traverse this trajectory to find the least reachtime for its uncovered query points
    return time;
}

void ComputeLargestK(double& LargestK, int& LargestPos)
{
#ifdef DEBUG_TEST
    assert(all_covering.size() == QUERY_K);
#endif
    for(int index=0; index < QUERY_K; ++index)
    {
        double distance = Candidates[all_covering[index]];
        if (distance > LargestK)
        {
            LargestK = distance;
            LargestPos = index;
        }
    }
}

double LowBound(int traj_id)
{
    double time = Candidates[traj_id];
    for (int i=0; i<QUERY_SIZE; ++i) {
        if (!CTestHelper[traj_id].test(i)) {
            time += lastRetrieved[i];
        }
    }
    return time;
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


#ifdef DEBUG_TEST
// Single-Thread Version of NN Search for a certain query point
void Incremental_Search(ISpatialIndex* tree, IShape* query)
{
    typedef RTree::RTree::NNEntry NNEntry;
    RTree::RTree::NNComparator nnc;

    priority_queue<NNEntry*, std::vector<NNEntry*>, NNEntry::ascending> queue;
    queue.push(new NNEntry(dynamic_cast<RTree::RTree*>(tree)->m_rootID, 0, 0.0));

    double knearest = 0.0;
    while (! queue.empty()) {
        NNEntry* pFirst = queue.top();
        queue.pop();

//        boost::mutex::scoped_lock lock(disk_mutex);
        if (pFirst->m_pEntry == 0)
        {
            RTree::NodePtr n = dynamic_cast<RTree::RTree*>(tree)->readNode(pFirst->m_id);

            for (uint32_t cChild=0; cChild < n->m_children; ++cChild) {
                if (n->m_level == 0) {
                    RTree::Data* e = new RTree::Data(n->m_pDataLength[cChild], n->m_pData[cChild], *(n->m_ptrMBR[cChild]), n->m_pIdentifier[cChild]);
                    queue.push(new NNEntry(n->m_pIdentifier[cChild], e, nnc.getMinimumDistance(*query, *e) / point_set[n->m_pIdentifier[cChild]].m_speed));
                }
                else
                {
                    queue.push(new NNEntry(n->m_pIdentifier[cChild], 0, nnc.getMinimumDistance(*query, *(n->m_ptrMBR[cChild]))/enhanced_mbr[n->m_pIdentifier[cChild]]));
                }
            }
        }
        else
        {
            qs.push(static_cast<IData*>(pFirst->m_pEntry)->getIdentifier());
            knearest = pFirst->m_minDist;
            delete pFirst->m_pEntry;
        }

        delete pFirst;
    }

    while (! queue.empty())
    {
        NNEntry* e = queue.top(); queue.pop();
        if (e->m_pEntry != 0) delete e->m_pEntry;
        delete e;
    }


    return;
}
#endif


#endif
