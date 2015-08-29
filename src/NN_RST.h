
//  NN_RST.h
//  ADC2014
//
//  Created by Yuxing Han on 8/25/15.
//  Copyright (c) 2015 Yuxing Han. All rights reserved.
//

#ifndef NN_RST_h
#define NN_RST_h

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <spatialindex/rtree/RTree.h>
#include "adc2014_utility.h"

using namespace SpatialIndex;
extern SpatialIndex::ISpatialIndex* tree;
extern boost::mutex io_mutex;
extern boost::mutex bool_mutex;
extern boost::mutex disk_mutex;
extern bool continued;


//maintain result of a certain query point 
class NN_RST
{
public:
    typedef boost::mutex::scoped_lock scoped_lock;
    
    NN_RST(IShape *q): query(q){}
    
    int Get(){
        scoped_lock lock(m_mutex);
        
        if (qs.empty())
        {
//            {
//                scoped_lock lock(io_mutex);
//                cout << "thread (Get) : queue is EMPTY!!" <<endl;
//            }
            
            while (qs.empty())
            {
                cond.wait(lock);
            }
        }
        
        int d = qs.front();qs.pop();
        cond.notify_one();
        return d;
    }
    
    
    // After While loop is done, this procedure will produce all the point in the ascending order.
    // Of Course, We don't want to see this when the data is very large.
    void Put()
    {
        typedef RTree::RTree::NNEntry NNEntry;
        RTree::RTree::NNComparator nnc;
        
#ifdef HAVE_PTHREAD_H
//        Tools::LockGuard lock(&dynamic_cast<RTree::RTree*>(tree)->m_lock);
#endif
        
        priority_queue<NNEntry*, std::vector<NNEntry*>, NNEntry::ascending> queue;
        queue.push(new NNEntry(dynamic_cast<RTree::RTree*>(tree)->m_rootID, 0, 0.0));
        
        double knearest = 0.0;
        while (! queue.empty()) {
            NNEntry* pFirst = queue.top();
            queue.pop();
            
            scoped_lock lock(disk_mutex);
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
                // different placement of lock can achieve better efficiency 
                scoped_lock lock(m_mutex);
                if (qs.size() >= 100)
                {
//                    {
//                        scoped_lock lock(io_mutex);
//                        cout << "thread (Put): " << qs.size() << endl;
//                    }
                    while (qs.size() >= 100)
                    {
                        {
                            scoped_lock lock(bool_mutex);
                            if (!continued)
                                break;
                        }
                        
                        cond.wait(lock);
                    }
                }
                
                qs.push(static_cast<IData*>(pFirst->m_pEntry)->getIdentifier());
                knearest = pFirst->m_minDist;
                delete pFirst->m_pEntry;
                cond.notify_one();
            }
            
            delete pFirst;
            
            {
                scoped_lock lock(bool_mutex);
                if (!continued)
                    break;
            }
        }
        
        while (! queue.empty())
        {
            NNEntry* e = queue.top(); queue.pop();
            if (e->m_pEntry != 0) delete e->m_pEntry;
            delete e;
        }
        
    
        return;
    }
    
public:
    boost::mutex m_mutex;
    boost::condition cond;
    IShape *query;
    queue<int> qs;
};

#endif
