#pragma once
#include <cstddef>
#include <cstring>
#include "mempool.h"
struct SparseElement
{
    size_t Index;
    double Value;
};
#ifndef SGO_THRESHOLD
    #define SGO_THRESHOLD 4
#endif

struct GradientHolder
{
    union
    {
        double * DenseGrad;
        SparseElement SparseGrad[SGO_THRESHOLD];
    };
    bool IsDense;
    size_t ActiveCount;
    double ScaleFactor;
    
    void Promote()
    {
        auto & mp = mempool();
        DenseGrad =mp.allocate();
        int n = mp.size();
        for (int i = 0; i < n; ++i)
        {
            DenseGrad[i] = 0;
        }
    }
    void Promote(SparseElement storage[2*SGO_THRESHOLD], int n)
    {
        Promote();
        for (int i =0; i < n; ++i)
        {
            DenseGrad[storage[i].Index] = storage[i].Value;
        }
    }
};

template<typename T>
struct DualExpression
{
    inline const T & self() const
    {
        return static_cast<const T&>(*this);
    }
    inline double GetScalar() const
    {
        return self().scalar();
    }
    inline void accumulate(double * out, double weight) const
    {
        self().accumulate(out,weight);
    }
};

template<typename L, typename R>
struct BinaryExpression : DualExpression<BinaryExpression>
{
    const L LHS;
    const R RHS;
    double w_l;
    double w_r;
    BinaryExpression(const L & lhs, const R & rhs) : LHS(lhs), RHS(rhs){}

    
    inline GradientHolder GetGrad() const
    {
        return w_l * LHS.GetGrad() + w_r * RHS.GetGrad();
    }
};

template<typename L, typename R>
struct DualAdd : BinaryExpression<L,R>
{
    DualAdd(const L & lhs, const R & rhs) : BinaryExpression(lhs,rhs,1,1) {};
    inline double GetScalar() const{
        return lhs.GetScalar() + rhs.GetScalar();
    }
};