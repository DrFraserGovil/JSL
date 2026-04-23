#pragma once
#include <math.h> 
#include <cmath>

/* This is the algebra of the dual field RxR used for automatic differentiation. 
    The templating is designed to minimise the need for dual-promotion of scalars, whilst allowing an intuitive algebra (i.e. 2 * dual)
*/

class Dual
{
    public:
        double Scalar = 0;
        double Infinitesimal = 0;

        //some basic constructors
        Dual(double scalar,double infinitesimal) : Scalar(scalar), Infinitesimal(infinitesimal){}
        Dual() : Dual(0,0){};
        Dual(double x): Dual(x,0){};


        //Now define the in-place operations...

        //ADDITION
            void operator+=(const Dual x)
            {
                Scalar += x.Scalar;
                Infinitesimal += x.Infinitesimal;
            }
            template<class T> void operator+=(const T x)
            {
                Scalar += x;
            }
        //SUBTRACTION
            void operator-=(const Dual x)
            {
                Scalar -= x.Scalar;
                Infinitesimal -= x.Infinitesimal;
            }
            template<class T> void operator-=(const T x)
            {
                Scalar -= x;
            }

        //MULTIPLICATION
            void operator*=(const Dual x)
            {
                Infinitesimal = Scalar*x.Infinitesimal + Infinitesimal * x.Scalar;
                Scalar *= x.Scalar; //update second!
            }
            template<class T> void operator*=(const T x)
            {
                Scalar *= x;
                Infinitesimal *= x;
            }
        //DIVISION
            void operator/=(const Dual x)
            {
                auto xs = x.Scalar;
                Infinitesimal = (Infinitesimal * xs- x.Infinitesimal * Scalar)/(xs*xs); 
                Scalar /= xs;
            }
            template<class T> void operator/=(const T x)
            {
                Scalar /= x;
                Infinitesimal /= x;
            }
            
        //LOGIC (only checks scalar components)
            //spaceship operator generates <,<=, > etc. 
            auto operator<=>(const Dual compare) const
            {
                return Scalar <=> compare.Scalar;
            }
            template<class T> bool operator<=>(const T compare) const
            {
                return Scalar <=> compare;
            }
};

/*
    Dual functional library

    This is the magic which makes the automatic differentiation work. 
    By defining dual-overloads of common functions, we enable the duals to automatically accumulate their derivatives without the author having to know if they're using duals or not. 

*/

//ADDITION 
    Dual operator+(const Dual  x, const Dual  y)
    {
        return Dual(x.Scalar + y.Scalar,x.Infinitesimal + y.Infinitesimal);
    }
    template<class T> Dual operator+(const Dual  x, T y)
    {
        return Dual(x.Scalar + y,x.Infinitesimal);
    }
    template<class T> Dual operator+(T y, const Dual  x)
    {
        return Dual(x.Scalar + y,x.Infinitesimal);
    }
//SUBTRACTION
    Dual operator-(Dual  x)
    {
        return Dual(-x.Scalar,-x.Infinitesimal);
    }
    Dual operator-(const Dual  x, const Dual  y)
    {
        return Dual(x.Scalar - y.Scalar,x.Infinitesimal - y.Infinitesimal);
    }
    template<class T> Dual operator-(T y, const Dual  x)
    {
        return Dual(y - x.Scalar,-1*x.Infinitesimal);
    }
    template<class T> Dual operator-(const Dual  x, T y)
    {
        return Dual(x.Scalar - y,x.Infinitesimal);
    }
//MULTIPLICATION
    Dual operator*(const Dual  x, const Dual  y)
    {
        return Dual(x.Scalar * y.Scalar, x.Scalar * y.Infinitesimal + x.Infinitesimal * y.Scalar);
    }
    template<class T> Dual operator*(const Dual  x, T y)
    {
        return Dual(x.Scalar*y,x.Infinitesimal*y);
    }
    template<class T> Dual operator*(T y,const Dual  x)
    {
        return Dual(x.Scalar*y,x.Infinitesimal*y);
    }
//DIVISION
    Dual operator/(const Dual  x, const Dual  y)
    {
        return Dual(x.Scalar / y.Scalar, (x.Infinitesimal * y.Scalar - x.Scalar * y.Infinitesimal)/(y.Scalar * y.Scalar));
    }
    template<class T> Dual operator/(const Dual  x, T y)
    {
        return Dual(x.Scalar / y, x.Infinitesimal/y);
    }
    template<class T> Dual operator/(T y,const Dual  x)
    {
        return Dual(y/x.Scalar, -y*x.Infinitesimal/(x.Scalar * x.Scalar));
    }
//EXPONENTIATION & LOGS
    Dual exp(const Dual  x)
    {
        double ex = std::exp(x.Scalar);
        return  Dual(ex,ex*x.Infinitesimal);
    }
    Dual log(const Dual  x)
    {
        return Dual(std::log(x.Scalar),x.Infinitesimal/x.Scalar);
    }
    Dual log10(const Dual  x)
    {
        return Dual(std::log10(x.Scalar),x.Infinitesimal/(std::log(10)*x.Scalar));
    }
    Dual exp1m(const Dual x)
    {
        double ex = std::expm1(x.Scalar);
        return Dual(ex,x.Infinitesimal*(ex+1));
    }
    Dual log1p(const Dual x)
    {
        return Dual(std::log1p(x.Scalar),x.Infinitesimal/(1.0 + x.Scalar));
    } 
// POWERS
    template<class T> Dual pow(const Dual x, const T power)
    {
        if (power == 0)
        {
            return Dual(1,0);
        }
        auto oneBelow = std::pow(x.Scalar,power-1);
        return Dual(oneBelow*x.Scalar,power * oneBelow * x.Infinitesimal);
    }
    Dual pow(const Dual x, const Dual power)
    {
        return exp(power * log(x));
    }
    Dual sqrt(const Dual x)
    {
        auto sq = std::sqrt(x.Scalar);
        return Dual(sq,x.Infinitesimal * 0.5/sq);
    }
    Dual abs(const Dual x)
    {
        return Dual(std::abs(x.Scalar),x.Infinitesimal * (x.Scalar >= 0 ? 1 : -1));
    }
//Trigonometry
    Dual sin(const Dual  x)
    {
        return Dual(std::sin(x.Scalar),x.Infinitesimal * std::cos(x.Scalar));
    }
    Dual cos(const Dual  x)
    {
        return Dual(std::cos(x.Scalar),x.Infinitesimal * -std::sin(x.Scalar));
    }
    Dual tan(const Dual  x)
    {
        double cx = std::cos(x.Scalar);
        return Dual(std::tan(x.Scalar),x.Infinitesimal/(cx*cx));
    }
    Dual asin(const Dual x)
    {
        return Dual(std::asin(x.Scalar),x.Infinitesimal/std::sqrt(1.0 - x.Scalar * x.Scalar));
    }
    Dual acos(const Dual x)
    {
        return Dual(std::acos(x.Scalar),-x.Infinitesimal/std::sqrt(1.0 - x.Scalar * x.Scalar));
    }
    Dual atan(const Dual x)
    {
        return Dual(std::atan(x.Scalar),x.Infinitesimal/(1.0 + x.Scalar * x.Scalar));
    }
    Dual atan2(const Dual y, const Dual x)
    {
        auto inv = 1.0/(y.Scalar*y.Scalar + x.Scalar * x.Scalar);
        return Dual(std::atan2(y.Scalar,x.Scalar),x.Infinitesimal * -y.Scalar*inv + y.Infinitesimal * x.Scalar *inv);
    }
    Dual hypot(const Dual x, const Dual y)
    {
        double h = std::hypot(x.Scalar,y.Scalar);
        if (h == 0){return 0;};
        return Dual(h,(x.Infinitesimal * x.Scalar + y.Infinitesimal * y.Scalar) / h);
    }

// DISCONTINUOUS -- these are a bit of a mess when x=y, but they're here for completeness
    Dual max(const Dual x, const Dual y)
    {
        if ( x > y)
        {
            return x;
        }
        else
        {
            return y;
        }
    }
    template<class T> Dual max(const Dual x, const T y)
    {
        if ( x.Scalar > y)
        {
            return x;
        }
        else
        {
            return Dual(y,0);
        }
    }
    template<class T> Dual max(const T x, const Dual y)
    {
        if ( x > y.Scalar)
        {
            return Dual(x,0);
        }
        else
        {
            return y;
        }
    }
     Dual min(const Dual x, const Dual y)
    {
        if ( x <= y)
        {
            return x;
        }
        else
        {
            return y;
        }
    }
    template<class T> Dual min(const Dual x, const T y)
    {
        if ( x.Scalar <= y)
        {
            return x;
        }
        else
        {
            return Dual(y,0);
        }
    }
    template<class T> Dual min(const T x, const Dual y)
    {
        if ( x <= y.Scalar)
        {
            return Dual(x,0);
        }
        else
        {
            return y;
        }
    }