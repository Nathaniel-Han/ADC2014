//
//  main.cpp
//  ADC2014
//
//  Created by Yuxing Han on 8/23/15.
//  Copyright (c) 2015 Yuxing Han. All rights reserved.
//
#include <string>
#include "global_func.h"

#ifdef DEBUG_TEST
#include <assert.h>
#endif

using namespace std;

int main(int argc, const char * argv[]) {
    ParsePointFile();

    try
    {
        string basename("RTREE.DATA");
        IStorageManager *diskfile = StorageManager::loadDiskStorageManager(basename);
        StorageManager::IBuffer* file = StorageManager::createNewRandomEvictionsBuffer(*diskfile, 10, false);
        ISpatialIndex* tree = RTree::loadRTree(*file, 1);


        if (!tree->isIndexValid()) {
            cout << "Bad Tree" <<endl;
            return -1;
        }

        IStatistics *sts;
        tree->getStatistics(&sts);

        int MAX_LEVEL = dynamic_cast<RTree::Statistics*>(sts)->getTreeHeight();
        //Operation on MBR in RTree level by level
        for (int m_level = 0; m_level < MAX_LEVEL; ++m_level) {
            LevelStrategy ls(m_level);
            tree->queryStrategy(ls);
        }

        delete sts;

    #ifdef DEBUG_TEST
        CheckStrategy cs;
        tree->queryStrategy(cs);
    #endif

    ////========== Start Multi-Treads =========================================
    ////if the number of trajectories(points) is small, thread may be suspended
        boost::thread_group threads;
        for (int i=0; i<QUERY_SIZE; ++i) {
            threads.create_thread(boost::bind(&NN_RST::Put, rst_vector[i],  dynamic_cast<RTree::RTree*>(tree)));
        }

        Taxi_Point p;
        double time;

        for (int i=0; i< QUERY_SIZE; ++i) {
            p = point_set[rst_vector[i]->Get()];
            time = p.getMinimumDistance(*rst_vector[i]->query) /  p.m_speed;
            Q.push(QEntry(i, p.traj_id, time));
        }

    // No need to invoke AllCoverTest in every loop, so check_flag is set
        int check_flag=0;
        while(true)
        {
            QEntry entry = Q.top();Q.pop();

            p = point_set[rst_vector[entry.q_id]->Get()];
            time = ComputePTime(p, *dynamic_cast<Point*>(rst_vector[entry.q_id]->query));
            Q.push(QEntry(entry.q_id, p.traj_id, time));


            if (!CTestHelper[entry.t_id].test(entry.q_id)) {
                if (Candidates.count(entry.t_id) == 0)
                    Candidates[entry.t_id] = time;
                else
                    Candidates[entry.t_id] += time;
            }

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

            ++check_flag;

            if (check_flag > QUERY_K*100) {
                if (AllCoverTest())
                    break;
                else
                    check_flag = 0;

            }
        }

    //  set continued to false to stop all the other threads
        {
            boost::mutex::scoped_lock lock(bool_mutex);
            continued = false;

        }

        for (int i=0; i<QUERY_SIZE; ++i)
            rst_vector[i]->cond.notify_one();

        threads.join_all();

    //======= Multi-Threads are stopped ==================================

        //    cout << Q.size() <<endl;
    //    while (!Q.empty()) {
    //        cout << Q.top().time<<endl;
    //        Q.pop();
    //    }

    //=====   Refine && Verification =====================================

        double LargestK = -1.0;
        int LargestPos = -1;

        ComputeLargestK(LargestK, LargestPos);

        for (int traj_id : partial_covering)
        {
            double time = LowBound(traj_id);
            if (time < LargestK) {
                Candidates[LargestPos] = traj_id;
                ComputeLargestK(LargestK, LargestPos);
            }
        }

        // now all_covering is the final correct answer
        for(int traj_id : all_covering)
            cout << traj_id << endl;


        delete tree;
        delete file;
        delete diskfile;
        return 0;
    }
    catch (Tools::Exception& e)
    {
        cerr << "******ERROR******" << endl;
        std::string s = e.what();
        cerr << s << endl;
        return -1;
    }
    catch (...)
    {
        cerr << "******ERROR******" << endl;
        cerr << "other exception" << endl;
        return -1;
    }
}
