#pragma once
#include <vector>
#include <stdexcept>
#include <iostream>

// #define ACTIVE_DUAL_COUNT 100
class Mempool
{
    std::vector<double> Buffer;
    std::vector<int> References;
    int MaxDuals;
    size_t offset=0;
    int L;
    bool hasWarned = false;
    public:
        Mempool(int bufferSize) : Buffer(bufferSize){reset(0);};
        double * allocate()
        {
            if (hasWarned)
            {
                int orig = offset;
                //we hope and pray that there's approximately a 1-in-1-out policy, which means the tail empties out as the head moves. 
                //i.e. if early-used blocks expire, then this is just another stepwise assignment with +1 check
                while(References[offset] > 0)
                {
                    offset = (offset + 1) % MaxDuals;
                    if (offset == orig)
                    {
                        throw std::runtime_error("Pool Exhausted: too much memory consumed. This requires recompiling with a higher buffer size.");
                    }
                }
            }
            double * ptr = &Buffer[offset*L];
            ++References[offset];
            ++offset;

            if (offset >= MaxDuals)
            {
                offset = 0;
                if (!hasWarned)
                {
                    hasWarned = true;
                    std::cout << "\n DUAL WARNING: Contiguous memory-reserve depleted at dimension " << L << ". Code may experience slowdown.";
                }
            }

            return ptr;
        }
    
        void free(double * ref)
        {
            //doesn't do anything aside from update the referemces, indicating to the allocator that the block is /(might be) free for reallocation 
            size_t pos = (ref - &Buffer[0])/L;
            --References[pos];
        }
        void touch(double * ref)
        {
            //called when a copy is made of a vector, indicating that it can't be deleted
            size_t pos = (ref - &Buffer[0])/L;
            ++References[pos];
        }
        bool can_mutate(double * ref)
        {
            // allow in-place modifcation of arrays only if no other duals are using this memory
            size_t pos = (ref - &Buffer[0])/L;
            return (References[pos] == 1);
        }

        void reset(size_t paramsPerDual)
        {
            L = paramsPerDual;
            MaxDuals = Buffer.size() / paramsPerDual;
            if (MaxDuals == 0)
            {
                  throw std::runtime_error("Dual-dimension is larger than the pool allocation.");
            }
            References.assign(MaxDuals,0); 
            offset = 0;
        }
        size_t size()
        {
            return L;
        }

};

#ifndef MEMPOOL_BUFFER_SIZE_MB
    #define MEMPOOL_BUFFER_SIZE_MB 100
#endif

inline Mempool & mempool()
{
    static thread_local Mempool pool(1024 * 1024  / sizeof(double) * MEMPOOL_BUFFER_SIZE_MB); 
    return pool;
}