//
//  NN_RST_Driver.h
//  ADC2014
//
//  Created by Yuxing Han on 8/28/15.
//  Copyright (c) 2015 Yuxing Han. All rights reserved.
//

#ifndef NN_RST_Driver_h
#define NN_RST_Driver_h

#include "config.h"
#include "NN_RST.h"
#include <iostream>

class NN_RST_Driver
{
public:
    NN_RST_Driver(NN_RST* n_rst):rst(n_rst), continued(true){}
    
    void retrive()
    {
        for (int i=0; i< POINT_NUM; i++)
        {
            p_id = rst->Get();
            {
                boost::mutex::scoped_lock lock(io_mutex);
                cout << i << ": " << p_id << endl;
            }
        }
        {
//                    boost::mutex::scoped_lock lock(bool_mutex);
            //        when procedure executed here, thread(gen) is wait for the condition, so 'continued' is safe
            continued = false;
//            wake thread(gen) so that it can stop
            rst->cond.notify_one();
        }
    }
    
    void retrive1()
    {
        for (int i =0; i< POINT_NUM; ++i)
        {
            p_id = rst->Get();
            
            {
                boost::mutex::scoped_lock lock(io_mutex);
                cout << i << ": " << p_id << endl;
            }
            
            
            {
                boost::mutex::scoped_lock lock(bool_mutex);
                if (!continued)
                {
//                    boost::this_thread::sleep(boost::posix_time::seconds(3));
                    rst->cond.notify_one();
                    return;
                }
            }
        }
    }
    
    
    void gen()
    {
        rst->Put();
    }
    
    void run()
    {
        thd1 = new boost::thread(boost::bind(&NN_RST_Driver::retrive1, this));
        thd2 = new boost::thread(boost::bind(&NN_RST_Driver::gen, this));
    }
    
    void thd_join()
    {
        if (!thd1)
            thd1->join();
        if (!thd2)
            thd2->join();
        
    }
    
    ~NN_RST_Driver()
    {
        if (!thd1)
            delete thd1;
        if (!thd2)
            delete thd2;
    }
    
public:
    int p_id;
    bool continued;
    boost::thread* thd1;
    boost::thread* thd2;
    
private:
    NN_RST *rst;
    

};

#endif
