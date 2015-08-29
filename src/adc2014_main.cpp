//
//  main.cpp
//  ADC2014
//
//  Created by Yuxing Han on 8/23/15.
//  Copyright (c) 2015 Yuxing Han. All rights reserved.
//
#include <string>
#include "global.h"

using namespace std;

int ParsePointFile();
void Incremental_Search(IShape* query);
bool AllCoverTest();

//queue<int> qs;

int main(int argc, const char * argv[]) {
    string basename("RTREE.DATA");
    
    IStorageManager *diskfile = StorageManager::loadDiskStorageManager(basename);
    StorageManager::IBuffer* file = StorageManager::createNewRandomEvictionsBuffer(*diskfile, 10, false);
    tree = RTree::loadRTree(*file, 1);
    
    
     ParsePointFile();
//    Usage of IStatistics
    IStatistics *sts;
    tree->getStatistics(&sts);
    int max_level = dynamic_cast<RTree::Statistics*>(sts)->getTreeHeight();
    //Operation on MBR in RTree level by level
    for (int m_level = 0; m_level < max_level; ++m_level) {
        LevelStrategy qs(m_level);
        tree->queryStrategy(qs);
    }
    CheckStrategy cs;
    tree->queryStrategy(cs);
    
    boost::thread_group threads;
    for (int i=0; i<QUERY_SIZE; ++i) {
        threads.create_thread(boost::bind(&NN_RST::Put, rst_vector[i]));
    }

    Taxi_Point p;
    double time;
    
    for (int i=0; i< QUERY_SIZE; ++i) {
        p = point_set[rst_vector[i]->Get()];
        time = p.getMinimumDistance(*rst_vector[i]->query) /  p.m_speed;
        Q.push(QEntry(i, p.traj_id, time));
    }
    
    
    int check_flag=0;
    while(true)
    {
        QEntry entry = Q.top();Q.pop();
//        C.push_back(entry.t_id);
        if (CTestHelper.count(entry.t_id) == 0)
        {
            bitset<QUERY_SIZE> bt;
            bt.set(entry.q_id);
            CTestHelper[entry.t_id] = bt;
        }
        else
        {
            CTestHelper[entry.t_id].set(entry.q_id);
        }
        
        
        p = point_set[rst_vector[entry.q_id]->Get()];
        time = p.getMinimumDistance(*rst_vector[entry.q_id]->query) / p.m_speed;
        Q.push(QEntry(entry.q_id, p.traj_id, time));
        
        ++check_flag;
        
        if (check_flag > QUERY_K*100) {
            if (AllCoverTest())
                break;
            else
                check_flag = 0;
            
        }
        
        
    }

    {
        boost::mutex::scoped_lock lock(bool_mutex);
        continued = false;
        
    }

    for (int i=0; i<QUERY_SIZE; ++i)
        rst_vector[i]->cond.notify_one();
    
    threads.join_all();
    
//    cout << Q.size() <<endl;
    while (!Q.empty()) {
        cout << Q.top().time<<endl;
        Q.pop();
    }

    
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


//void Incremental_Search(IShape* query)
//{
//    typedef RTree::RTree::NNEntry NNEntry;
//    RTree::RTree::NNComparator nnc;
//    
//    priority_queue<NNEntry*, std::vector<NNEntry*>, NNEntry::ascending> queue;
//    queue.push(new NNEntry(dynamic_cast<RTree::RTree*>(tree)->m_rootID, 0, 0.0));
//    
//    double knearest = 0.0;
//    while (! queue.empty()) {
//        NNEntry* pFirst = queue.top();
//        queue.pop();
//        
//        boost::mutex::scoped_lock lock(disk_mutex);
//        if (pFirst->m_pEntry == 0)
//        {
//            RTree::NodePtr n = dynamic_cast<RTree::RTree*>(tree)->readNode(pFirst->m_id);
//            
//            for (uint32_t cChild=0; cChild < n->m_children; ++cChild) {
//                if (n->m_level == 0) {
//                    RTree::Data* e = new RTree::Data(n->m_pDataLength[cChild], n->m_pData[cChild], *(n->m_ptrMBR[cChild]), n->m_pIdentifier[cChild]);
//                    queue.push(new NNEntry(n->m_pIdentifier[cChild], e, nnc.getMinimumDistance(*query, *e) / point_set[n->m_pIdentifier[cChild]].m_speed));
//                }
//                else
//                {
//                    queue.push(new NNEntry(n->m_pIdentifier[cChild], 0, nnc.getMinimumDistance(*query, *(n->m_ptrMBR[cChild]))/enhanced_mbr[n->m_pIdentifier[cChild]]));
//                }
//            }
//        }
//        else
//        {
//            qs.push(static_cast<IData*>(pFirst->m_pEntry)->getIdentifier());
//            knearest = pFirst->m_minDist;
//            delete pFirst->m_pEntry;
//        }
//        
//        delete pFirst;
//    }
//    
//    while (! queue.empty())
//    {
//        NNEntry* e = queue.top(); queue.pop();
//        if (e->m_pEntry != 0) delete e->m_pEntry;
//        delete e;
//    }
//    
//    
//    return;
//}

bool AllCoverTest()
{
    int count = 0;
    for (unordered_map<int, bitset<QUERY_SIZE>>::iterator iter = CTestHelper.begin(); iter != CTestHelper.end(); ++iter) {
        if (iter->second.all()) {
            ++count;
        }
    }
    if (count >= QUERY_K) {
        return true;
    }
    return false;
}



















