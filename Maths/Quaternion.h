#pragma once
#include <vector>
#include <stdexcept>
#include <string.h>
#include <iostream>
#include <math.h>
#include "vector.h"
#include "matrix.h"


namespace JSL
{
	
	class Quaternion : public JSL::Vector
	{
		public: 
			
			Quaternion() : JSL::Vector(4) {	};

			Quaternion(const double & scalar, const JSL::Vector & vec) : JSL::Vector(4)
			{
				if (vec.Size() != 3)
				{
					throw std::runtime_error("ERROR in JSL::Quaternion: Quaternions(scalar,vector) initialisation only works if the vector has dimension 3");
				}
				Data[0] = scalar;
				for (int i = 0; i < 3; ++i)
				{
					Data[i+1] = vec[i];
				}
			}
			
			Quaternion(const JSL::Vector & vec4) : JSL::Vector(vec4)
			{
				if (vec4.Size() != 4)
				{
					throw std::runtime_error("ERROR in JSL::Quaternion: Quaternions(Vector) initialisation only works if the vector has dimension 4");
				}
			}
			Quaternion(const std::vector<double> & vec4) : JSL::Vector(vec4)
			{
				if (vec4.size() != 4)
				{
					throw std::runtime_error("ERROR in JSL::Quaternion: Quaternions(Vector) initialisation only works if the vector has dimension 4");
				}
			}
			
			static Quaternion One()
			{
				Quaternion q;
				q[0] = 1;
				return q;
			}
			static Quaternion Zero()
			{
				Quaternion q;
				return q;
			}
			static Quaternion Random()
			{
				Quaternion q;
				
				for (int i = 0; i < 4; ++i)
				{
					q[i] = (double)random() / RAND_MAX;
				}
				
				return q;
			}
			double & Scalar()
			{
				return Data[0];
			}
			double & Vector(int id)
			{
				return Data[id+1];
			}
			const double & Scalar() const
			{
				return Data[0];
			}
			const double & Vector(int id) const
			{
				return Data[id+1];
			}
			JSL::Vector Vector() const
			{
				return JSL::Vector({Data[1],Data[2],Data[3]});
			}
				
			Quaternion Conjugate() const
			{
				return Quaternion(Scalar(), -1*Vector());
			}
			
			JSL::Matrix LeftMultiplicationMatrix() const
			{
				JSL::Matrix M(4,4);
				
				M(0,0) = Data[0];
				for (int i = 1; i < 4; ++i)
				{
					M(0,i) = -Data[i];
					M(i,0) = Data[i];
					M(i,i) = Data[0];
				}
				
				M(1,2) = -Data[3];
				M(2,1) = Data[3];
				M(1,3) = Data[2];
				M(3,1) = -Data[2];
				M(2,3) = -Data[1];
				M(3,2) = Data[1];
				
				return M;
				
			}
			
			JSL::Matrix RightMultiplicationMatrix() const
			{
				JSL::Matrix M(4,4);
				
				M(0,0) = Data[0];
				for (int i = 1; i < 4; ++i)
				{
					M(0,i) = -Data[i];
					M(i,0) = Data[i];
					M(i,i) = Data[0];
				}
				
				M(1,2) = Data[3];
				M(2,1) = -Data[3];
				M(1,3) = -Data[2];
				M(3,1) = Data[2];
				M(2,3) = Data[1];
				M(3,2) = -Data[1];
				
				return M;
				
			}
		protected:

		
			using JSL::Vector::operator[];
	};
	
	inline Quaternion operator*(const Quaternion & lhs, const Quaternion & rhs)
	{
		double scalar = lhs.Scalar() * rhs.Scalar() - lhs.Vector().Dot(rhs.Vector());
		JSL::Vector vec = lhs.Scalar() * rhs.Vector() + rhs.Scalar() * lhs.Vector() + lhs.Vector().Cross(rhs.Vector());
		return Quaternion(scalar,vec);
	}
	
	inline Quaternion operator*(const JSL::Matrix & lhs, const Quaternion & rhs)
	{
		//~ std::vector<double> vTemp = ;
		JSL::Vector v({rhs.Scalar(),rhs.Vector(0),rhs.Vector(1),rhs.Vector(2)});
		
		return Quaternion(lhs * v);
	}
	
	inline Quaternion operator/(const Quaternion & lhs, const Quaternion & rhs)
	{
		double N = rhs.Norm();
		if (N <= 0)
		{
			throw std::runtime_error("Quaternion division is not well defined when the norm is 0");
		}
		
		return 1.0/N * lhs * rhs.Conjugate();
	}
}

