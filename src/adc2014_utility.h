//
//  utility.h
//  RTreeTest
//
//  Created by Yuxing Han on 8/24/15.
//  Copyright (c) 2015 Yuxing Han. All rights reserved.
//

#ifndef RTreeTest_utility_h
#define RTreeTest_utility_h

#include <queue>
#include <vector>
#include <unordered_set>
#include <spatialindex/SpatialIndex.h>
using namespace SpatialIndex;
using namespace std;

class Taxi_Point;
extern unordered_map<id_type, double> enhanced_mbr;
extern vector<Taxi_Point> point_set;

class Taxi_Point : public Point
{
public:
    Taxi_Point():Point(), m_id(-1), m_speed(0.0){}
    Taxi_Point(const double* pCoords, uint32_t dimension, long id, long t_id, double speed):Point(pCoords, dimension), m_id(id), traj_id(t_id), m_speed(speed){}
    Taxi_Point(const Taxi_Point& p): Point(p), m_id(p.m_id), traj_id(p.traj_id), m_speed(p.m_speed){}
    virtual ~Taxi_Point(){}
    
    friend std::ostream& operator<<(std::ostream& os, const Taxi_Point& tpt)
    {
        const Point &pt = tpt;
        os << pt << "\nspeed: " << tpt.m_speed;
        return os;
    }
    
public:
    long m_id;
    long traj_id;
    double m_speed;
};


//traverse the RTree in DFS manner

class LevelStrategy : public SpatialIndex::IQueryStrategy
{
private:
    stack<id_type> ids;
    unordered_set<id_type> visited; // id_type is RTree's internal own Id
    int m_level;
    
public:
    vector<int> result;
    vector<int> real_data;
public:
    LevelStrategy(int level) : m_level(level) {}
    
    
    void getNextEntry(const IEntry& entry, id_type& nextEntry, bool& hasNext)
    {
        const INode* n = dynamic_cast<const INode*>(&entry);
        
        if (n != 0){
            id_type id = n->getIdentifier();
            
            if (visited.count(id) == 0)
            {
                //                  visit the node
                //                cout << n->getLevel() << " ";
                result.push_back(n->getLevel());
                
                visited.insert(id);
                if (n->getLevel() > 0)
                {
                    for (uint32_t cChild = 0; cChild < n->getChildrenCount(); cChild++)
                    {
                        ids.push(n->getChildIdentifier(cChild));
                    }
                }
                
                double max_speed = -1;
                
                if (n->getLevel() == m_level) {
                
                    if ( m_level == 0)
                    {
                        for (uint32_t cChild = 0; cChild < n->getChildrenCount(); cChild++)
                        {
                            double s = point_set[n->getChildIdentifier(cChild)].m_speed;
                            max_speed = s > max_speed ? s : max_speed;
                        }
                        
                    }
                    else
                    {
                        for (uint32_t cChild = 0; cChild < n->getChildrenCount(); cChild++)
                        {
                            double s =  enhanced_mbr[n->getChildIdentifier(cChild)];
                            max_speed = s > max_speed ? s : max_speed;
                        }
                    }
                    
                    enhanced_mbr[id] = max_speed;
                }
            }
        }
        
        if (!ids.empty()) {
            nextEntry = ids.top();ids.pop();
            hasNext = true;
        }
        else
        {
            hasNext = false;
        }
        
    }
};

class CheckStrategy : public SpatialIndex::IQueryStrategy
{
private:
    queue<id_type> ids;
    
public:
    void getNextEntry(const IEntry& entry, id_type& nextEntry, bool& hasNext)
    {
   
        const INode* n = dynamic_cast<const INode*>(&entry);
        
        // traverse only index nodes at levels 1 and higher.
        if (n != 0 && n->getLevel() > 0)
        {
            for (uint32_t cChild = 0; cChild < n->getChildrenCount(); cChild++)
            {
                id_type childId = n->getChildIdentifier(cChild);
                ids.push(childId);
                assert(enhanced_mbr[n->getIdentifier()] >= enhanced_mbr[childId]);
            }
        }
        
        if (n!=0 && n->getLevel() ==0 ) {
            for (uint32_t cChild = 0; cChild < n->getChildrenCount(); cChild++)
            {
                assert(enhanced_mbr[n->getIdentifier()] >= point_set[n->getChildIdentifier(cChild)].m_speed);
            }
        }
        
        if (! ids.empty())
        {
            nextEntry = ids.front(); ids.pop();
            hasNext = true;
        }
        else
        {
            hasNext = false;
        }
        
    }
};

class MyVisitor : public IVisitor
{
public:
    size_t m_indexIO;
    size_t m_leafIO;
    int m_count;
    
public:
    MyVisitor() : m_indexIO(0), m_leafIO(0), m_count(0) {}
    
    void visitNode(const INode& n)
    {
        if (n.isLeaf()) m_leafIO++;
        else m_indexIO++;
    }
    
    void visitData(const IData& d)
    {
        cout << d.getIdentifier() << " ";
        // the ID of this data entry is an answer to the query. I will just print it to stdout.
        m_count++;
    }
    
    void visitData(std::vector<const IData*>& v)
    {
        cout << v[0]->getIdentifier() << " " << v[1]->getIdentifier() << endl;
    }
};


#endif
