#pragma once
#define AUTODIFF_MEMPOOL
#include <cstddef>
#include <math.h> 
#include <cmath>
#include <cstring>
#ifdef AUTODIFF_MEMPOOL
    #include "mempool.h"
    using vectype = double*;
    #define INITIALISE_VECTOR DenseGrad = mempool().allocate();
    #define RELEASE_VECTOR mempool().free(DenseGrad);
#else
    #include <vector>
    using vectype = std::vector<double>
    #define INITIALISE_VECTOR DenseGrad.resize(Dimension);
    #define RELEASE_VECTOR //no need for anything here. vector mode is deleted by default
#endif

#include "expressions.h"
class VectorDual
{
    protected:
        template<bool initialiser=true>
        inline void denseOp(const VectorDual & x, const VectorDual & y, double weight_x, double weight_y)
        {
            weight_x *= x.ScaleFactor;
            weight_y *= y.ScaleFactor;
            if constexpr (initialiser){INITIALISE_VECTOR;}
            for (int i = 0; i <Dimension; ++i)
            {
                DenseGrad[i] = (weight_x * x.DenseGrad[i]) + (weight_y * y.DenseGrad[i]);
            }
            ScaleFactor = 1;
            IsDense = true;
        }
        template<bool initialiser=true>
        inline void mixedOp(const VectorDual & dense, const VectorDual & sparse,double weight_d, double weight_s)
        {
            if constexpr (initialiser){INITIALISE_VECTOR;}
            weight_d *= dense.ScaleFactor;
            weight_s *= sparse.ScaleFactor;
          
            for (int i = 0; i <Dimension; ++i)
            {
                DenseGrad[i] = weight_d*dense.DenseGrad[i]; 
            }

            for (size_t i = 0; i < sparse.ActiveCount; ++i)
            {
                auto & element = sparse.SparseGrad[i];
                DenseGrad[element.Index] += element.Value * weight_s;
            }
            IsDense = true;
            ScaleFactor = 1;
        }
        inline void sparseOp(const VectorDual & x, const VectorDual & y, double weight_x, double weight_y)
        {
            SparseElement staging[2*SGO_THRESHOLD];
            weight_x *= x.ScaleFactor;
            weight_y *= y.ScaleFactor;
            const int initialMix = x.ActiveCount;
            for (int i = 0; i < initialMix; ++i)
            {
                staging[i] = {x.SparseGrad[i].Index,x.SparseGrad[i].Value * x.ScaleFactor};
            }
            int mixCount = initialMix;
            for (int i = 0; i < y.ActiveCount; ++i)
            {
                bool found = false;
                auto idx = y.SparseGrad[i].Index;
                double dy =y.SparseGrad[i].Value * weight_y;
                for (int j = 0; j < initialMix; ++j)
                {
                    if (idx == staging[j].Index)
                    {
                        staging[j].Value += dy;
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    staging[mixCount] = {idx,dy};
                    ++mixCount;
                }
            }

            if (mixCount <= SGO_THRESHOLD)
            {
                IsDense = false;
                ActiveCount = mixCount;
                ScaleFactor = 1;
                for (int i = 0; i < mixCount; ++i) SparseGrad[i] = staging[i];
            }
            else
            {
                INITIALISE_VECTOR;
                IsDense = true;
                ScaleFactor = 1;
                for (int i = 0; i < mixCount; ++i) DenseGrad[staging[i].Index] = staging[i].Value;
            }
        }

        template<bool initialiser=true>
        inline void dispatchOp(const VectorDual & x, const VectorDual & y, double weight_x, double weight_y)
        {
            if (x.IsDense && y.IsDense)
            {
                denseOp<initialiser>(x,y,weight_x,weight_y);
                return;
            }
            if (x.IsDense && !y.IsDense)
            {
                mixedOp<initialiser>(x,y,weight_x,weight_y);
                return;
            }
            if (!x.IsDense && y.IsDense)
            {
                mixedOp<initialiser>(y,x,weight_y,weight_x);
                return;
            }
            if (!x.IsDense && !y.IsDense)
            {
                sparseOp(x,y,weight_x,weight_y);
                return;
            }
        }
        union
        {
            vectype DenseGrad;
            SparseElement SparseGrad[SGO_THRESHOLD];
        };

        double Scalar;
        double ScaleFactor;
        size_t ActiveCount;
        bool IsDense; //equivalent to (ActiveCount >= SGO_THRESHOLD), but cached for quick comparison
    public:
        const size_t Dimension;


        explicit VectorDual(size_t d) : VectorDual(0,d){};
        VectorDual(double scalar, int d) : Dimension(d)
        {
            Scalar = scalar;
            ScaleFactor = 1;
            ActiveCount = 0;
            IsDense = false;
        }
        VectorDual(const VectorDual & x) : Dimension(x.Dimension), IsDense(x.IsDense), ScaleFactor(x.ScaleFactor), Scalar(x.Scalar)
        {
            if (IsDense)
            {
                //this is a deliberate shallow copy. It makes it super quick to do.
                DenseGrad = x.DenseGrad; 
                mempool().touch(DenseGrad);
            }
            else
            {
                ActiveCount = x.ActiveCount;
                std::memcpy(SparseGrad,x.SparseGrad,ActiveCount * sizeof(SparseElement));
            }
        }
        VectorDual(const VectorDual & x,double newScalar) : VectorDual(x)
        {
            Scalar = newScalar;
        }

        ~VectorDual()
        {
            if (IsDense)
            {
                RELEASE_VECTOR;
            }
        }
        double GetScalar()
        {
            return Scalar;
        }
        void MakeConstant()
        {
            ActiveCount = 0;
        }
        void MakeParameter(int index,double value=1)
        {
            ActiveCount = 1;
            SparseGrad[0].Index = index;
            SparseGrad[0].Value = value;
        }
        //LOGIC
            auto operator<=>(const VectorDual & compare) const
            {
                return Scalar <=> compare.Scalar;
            }
            template<class T> bool operator<=>(const T compare) const
            {
                return Scalar <=> compare;
            }
        //ASSIGNMENT
            void operator=(const VectorDual & x)
            {
                double * freeptr = nullptr;
                if (IsDense)
                {
                    freeptr = DenseGrad;
                }
                if (x.IsDense){
                    DenseGrad = x.DenseGrad;
                    mempool().touch(DenseGrad);
                } 
                else 
                {
                    ActiveCount = x.ActiveCount;
                    std::memcpy(SparseGrad,x.SparseGrad,ActiveCount * sizeof(SparseElement));
                }
                if (freeptr)
                {
                    mempool().free(freeptr); //free after the assignment so there's not a gap where it's not assigned to
                }
                Scalar = x.Scalar;
                ScaleFactor = x.ScaleFactor;
            }
        //ADDITION
            friend VectorDual operator+(const VectorDual & x, const VectorDual & y)
            {
                VectorDual out(x.Scalar + y.Scalar,x.Dimension);

                out.dispatchOp(x,y,1,1);
                return out;
            }
            template<class T> 
            friend VectorDual operator+(const VectorDual & x, T y)
            {
                return VectorDual(x,x.Scalar + y);
            }
            template<class T> 
            friend VectorDual operator+(T y, const VectorDual & x)
            {
                return VectorDual(x,x.Scalar + y);
            }
            void operator+=(const VectorDual & x)
            {  
                #ifdef AUTODIFF_MEMPOOL
                    if (!IsDense || mempool().can_mutate(DenseGrad))
                    {
                        this->dispatchOp<false>(*this,x,1,1);
                    }
                    else
                    {
                        *this = (*this) + x; //lose in-place efficiency, but ensure that directions on the mempool don't change
                    }
                #else
                    Scalar += x.Scalar;
                    for (int i = 0; i < Dimension; ++i){Infinitesimal[i] = ScaleFactor * Infinitesimal[i] + x.ScaleFactor * x.Infinitesimal[i];}
                    ScaleFactor = 1;
                #endif
            }
            template<class T> void operator+=(const T x)
            {
                Scalar += x;
            }
        //Subtraction
            friend VectorDual operator-(VectorDual x)
            {
                x.Scalar = -x.Scalar;
                x.ScaleFactor *= -1;
                return x;
            }
            friend VectorDual operator-(const VectorDual & x, const VectorDual & y)
            {
                VectorDual out(x.Scalar - y.Scalar,x.Dimension);

                out.dispatchOp(x,y,1,-1);
                return out;
            }
            template<class T>
            friend VectorDual operator-(const VectorDual & x, T y)
            {
                return VectorDual(x,x.Scalar - y);
            }
            template<class T>
            friend VectorDual operator-(T y, const VectorDual &  x)
            {
                return VectorDual(-x,y - x.Scalar);
            }

            void operator-=(const VectorDual x)
             {  
                #ifdef AUTODIFF_MEMPOOL
                     if (!IsDense || mempool().can_mutate(DenseGrad))
                        {
                            this->dispatchOp<false>(*this,x,1,-1);
                        }
                        else
                        {
                            *this = (*this) - x; //lose in-place efficiency, but ensure that directions on the mempool don't change
                        }
                #else
                    Scalar -= x.Scalar;
                    for (int i = 0; i < Dimension; ++i){Infinitesimal[i] = ScaleFactor * Infinitesimal[i] - x.ScaleFactor * x.Infinitesimal[i];}
                    ScaleFactor = 1;
                #endif
            }
            template<class T> void operator-=(const T x)
            {
                Scalar -= x;
            }
        //MULTIPLICATION
            friend VectorDual operator*(const VectorDual & x, const VectorDual & y)
            {
                VectorDual out(x.Scalar*y.Scalar,x.Dimension);
                out.dispatchOp(x,y,y.Scalar,x.Scalar);
                return out;
            }
            template<class T> friend VectorDual operator*(const VectorDual & x, const T y)
            {
                VectorDual out(x);
                out.Scalar *= y;
                out.ScaleFactor *= y;
                return out;
            }
            template<class T> friend VectorDual operator*(const T y, const VectorDual & x)
            {
                VectorDual out(x);
                out.Scalar *= y;
                out.ScaleFactor *= y;
                return out;
            }
            void operator*=(const VectorDual & x)
            {  
                if (!IsDense || mempool().can_mutate(DenseGrad))
                {
                    this->dispatchOp<false>(*this,x,x.Scalar,Scalar);
                }
                else
                {
                    *this = (*this) * x; //lose in-place efficiency, but ensure that directions on the mempool don't change
                }
            }
            template<class T> void operator*=(const T x)
            {
                Scalar *= x;
                ScaleFactor *= x;
            }
        //DIVISION
            friend VectorDual operator/(const VectorDual & x, const VectorDual & y)
            {
                VectorDual out(x.Scalar/y.Scalar,x.Dimension);
                auto ysSq = y.Scalar * y.Scalar;
                out.dispatchOp(x,y,1.0/y.Scalar,-x.Scalar/ysSq);
                return out;
            }
            template<class T>
            friend VectorDual operator/(const VectorDual & x, T y)
            {
                VectorDual out(x);
                out.Scalar /= y;
                out.ScaleFactor /= y;
                return out;
            }
            template<class T>
            friend VectorDual operator/(T x, const VectorDual & y)
            {
                VectorDual out(y);
                out.ScaleFactor *= -x/(y.Scalar * y.Scalar);
                out.Scalar = x/y.Scalar;
                return out;
            }
            void operator/=(const VectorDual & x)
            {
                if (!IsDense || mempool().can_mutate(DenseGrad))
                {
                    auto ysSq = x.Scalar * x.Scalar;
                    this->dispatchOp<false>(*this,x,1.0/x.Scalar,-Scalar/ysSq);
                }
                else
                {
                    *this = (*this) / x; //lose in-place efficiency, but ensure that directions on the mempool don't change
                }
            }
            template<class T>
            void operator/=(const T y)
            {
                Scalar/=y;
                ScaleFactor /= y;
            }
        //EXPONENTIATION & LOGS
            friend VectorDual exp(const VectorDual & x)
            {
                VectorDual out(x);
                out.Scalar = std::exp(x.Scalar);
                out.ScaleFactor *= out.Scalar;
                return out;
            }
            friend VectorDual log(const VectorDual & x)
            {
                VectorDual out(x);
                out.ScaleFactor /= x.Scalar;
                out.Scalar = std::log(x.Scalar);
                return out;
            }
            friend VectorDual log(const VectorDual & x,double base)
            {
                 VectorDual out(x);
                auto lb = std::log(base);
                out.ScaleFactor /= (x.Scalar * lb);
                out.Scalar = std::log(x.Scalar)/lb;
                return out;
            }
        //POWERS
            friend VectorDual pow(const VectorDual & u, const VectorDual & v)
            {
                double val = std::pow(u.Scalar, v.Scalar);
                VectorDual out(val, u.Dimension);

                double weightU = v.Scalar * (val / u.Scalar); // v * u^(v-1)
                double weightV = val * std::log(u.Scalar);    // u^v * ln(u)

                out.dispatchOp(u, v, weightU, weightV);
                return out;
            }
            template <class T> friend VectorDual pow(const VectorDual & x, T power)
            {
                if (power == 0){ return VectorDual(1.0,x.Dimension);} //x^0 = 1 for all x
                
                VectorDual out(x);
                auto xPminus = std::pow(x.Scalar,power-1);
                out.Scalar = xPminus * x.Scalar;
                out.ScaleFactor *= xPminus * power;
                return out;
            }
            friend VectorDual sqrt(const VectorDual & x)
            {
                VectorDual out(x);
                auto sq = std::sqrt(x.Scalar);
                out.ScaleFactor /= (2*sq);
                out.Scalar = sq;
                return out;
            }
            friend VectorDual abs(const VectorDual & x)
            {
                VectorDual out(x);
                out.Scalar = std::abs(x.Scalar);
                out.ScaleFactor *=  (x.Scalar >= 0 ? 1 : -1);
                return out;
            }
        //Trigonometry
            friend VectorDual sin(const VectorDual & x)
            {
                VectorDual out(x);
                auto theta = x.Scalar;
                out.Scalar = std::sin(theta);
                out.ScaleFactor *= std::cos(theta);
                return out;
            }
            friend VectorDual cos(const VectorDual & x)
            {
                VectorDual out(x);
                auto theta = x.Scalar;
                out.Scalar = std::cos(theta);
                out.ScaleFactor *= -std::sin(theta);
                return out;
            }
            
            friend VectorDual tan(const VectorDual & x)
            {
                VectorDual out(x);
                auto theta = x.Scalar;
                out.Scalar = std::tan(theta);
                double cx = std::cos(theta);
                out.ScaleFactor /= (cx*cx);
                return out;
            }
            friend VectorDual asin(const VectorDual & x)
            {
                VectorDual out(x);
                auto theta = x.Scalar;
                out.Scalar = std::asin(theta);
                out.ScaleFactor /= std::sqrt(1.0 - theta*theta);
                return out;
            }
            friend VectorDual acos(const VectorDual & x)
            {
                VectorDual out(x);
                auto theta = x.Scalar;
                out.Scalar = std::acos(theta);
                out.ScaleFactor /= -std::sqrt(1.0 - theta*theta);
                return out;
            }
            friend VectorDual atan(const VectorDual & x)
            {
                VectorDual out(x);
                auto theta = x.Scalar;
                out.Scalar = std::atan(theta);
                out.ScaleFactor /= (1 + theta * theta);
                return out;
            }
            friend VectorDual atan2(const VectorDual & y, const VectorDual & x)
            {
                VectorDual out( std::atan2(y.Scalar,x.Scalar),x.Dimension);

                auto inv = 1.0/(y.Scalar*y.Scalar + x.Scalar * x.Scalar);
                out.dispatchOp(x,y,-y.Scalar*inv,x.Scalar*inv);
                return out;
            }
            friend VectorDual hypot(const VectorDual & x, const VectorDual & y)
            {  
                VectorDual out(std::hypot(x.Scalar,y.Scalar),x.Dimension); 
                if (out.Scalar == 0){out.MakeConstant(); return out;}

                out.dispatchOp(x,y,x.Scalar/out.Scalar,y.Scalar/out.Scalar);
                return out;
            }
            
            friend VectorDual LogSumExp(const VectorDual & x, const VectorDual & y)
            {
                VectorDual out(x.Dimension);
                if (x > y)
                {
                    double l1p =  std::log1p(std::exp(y.Scalar - x.Scalar));
                    double f = x.Scalar +l1p;
                    out.Scalar = f;
                    double dfdx = std::exp(-l1p);
                    double dfdy = std::exp(y.Scalar - f);

                    out.dispatchOp(x,y,dfdx,dfdy);
                    return out;
                }
                else
                {
                    return LogSumExp(y,x);
                }
            }

};
