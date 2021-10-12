#pragma once
#include <vector>
#include <stdexcept>
#include <string.h>
#include <iostream>
#include <math.h>

namespace JSL
{
	//Forward declarations to make some of the weird circular stuff work --> these are defined in full later!
	class Vector;
	inline double VectorDotProduct(const Vector & lhs, const Vector & rhs);
	inline Vector VectorCrossProduct(const Vector & lhs, const Vector & rhs);
	inline double AngleBetweenVectors(const Vector & lhs, const Vector & rhs);
	
	/*!
		* A class implementing basic R^n vector mathematics. Mostly acts as an extention to the basic std::vector object, but with the implicit assumption that the objects should behave like members of a true vector space, can unambiguously overload some operators and add in additional functionality. 
	*/
	class Vector
	{
		public:
			
			//! The dimensionality of the vector. Under no realistic circumstances should this change over time, or differ from the true data length. If it does, I have messed up and this structure becomes borked.
			const int Size;
			
			//!Initialises the vector to a state of length n, populated by zeros \param n The length of the vector to be created
			Vector(int n): Size(n)
			{
				Data = std::vector<double>(n,0.0);
			}
			
			//!Initialises the vector to contain the provided stl vector\param input An std::vector which the new Vector will envelop.
			Vector(std::vector<double> input): Data(input), Size(input.size())
			{
				
			}
			
			//! Overload access operator so can call Vector[0] etc as normal for a vector class. Performs checks on the size so that you cannot over/underflow the memory access
			double & operator[](int idx)
			{
				if (idx < Size && idx >= 0)
				{
					return Data[idx];
				}
				else
				{
					if (idx < 0)
					{
						throw std::runtime_error(negativeIntegerError(idx));
					}
					else
					{
						throw std::runtime_error(outOfBoundsError(idx));
					}
				}
			}
			
			//! Replication of non-const version (annoying) but necessary for good access....
			const double& operator[](int idx) const
			{
				if (idx < Size && idx >= 0)
				{
					return Data[idx];
				}
				else
				{
					if (idx < 0)
					{
						throw std::runtime_error(negativeIntegerError(idx));
					}
					else
					{
						throw std::runtime_error(outOfBoundsError(idx));
					}
				}
			}
		
			//! Provides a member alias for VectorDotProduct(), with the first argument being the current object \param rhs The second object passed to VectorDotProduct \returns The dot product of rhs and the object 
			double Dot(const Vector & rhs) const
			{
				return VectorDotProduct(*this,rhs);
			}
			
			//! Provides a member alias for VectorCrossProduct(), with the first argument being the current object (recall order does matter for cross products!) \param rhs The second object passed to VectorDotProduct \returns The dot product (this x rhs) 
			Vector Cross(const Vector & rhs) const
			{
				return VectorCrossProduct(*this,rhs);
			}
			
			//! The squared-norm of the current object, calculated using Dot(). Probably not as much use as Norm(), but saves time sqrting and then squaring again! \returns this.Dot(this)
			double SqNorm() const
			{
				return VectorDotProduct(*this,*this);
			}
			
			//! The norm of the current object, \returns The square-root of the SqNorm() function.
			double Norm() const
			{
				return sqrt(SqNorm());
			}
		
			//! A member-alias for AngleBetweenVectors(), with the first argument being the current object. \param rhs The second object passed to AngleBetweenVectors() \returns The angle between this object and the provided vector
			double AngleBetween(const Vector & rhs) const
			{
				return AngleBetweenVectors(*this,rhs);
			}
			
			//! Converts the vector into a human-readable string \returns A representation of the vector, such as (1,4.5,3)
			std::string to_string() const
			{
				std::string outString = "(";
				for (int i = 0; i < Size; ++i)
				{
					outString += std::to_string(Data[i]);
					if (i < Size - 1)
					{
						outString += ", ";
					}
				}
				outString += ")";
				return outString;
			}
		
			//! In-place addition of two vectors. Calls Vector operator+(const Vector & lhs, const Vector & rhs) using this object as lhs. \param rhs The vector to be accumulated into the current object. Must be the same Size as the calling object. \returns A reference to the now-modified calling object
			Vector & operator+=(const Vector & rhs)
			{
				if (Size != rhs.Size)
				{
					throw std::runtime_error("Cannot add vectors of different sizes");
				}
				for (int i = 0; i < Size; ++i)
				{
					Data[i] += rhs[i];
				}
				return *this;
			}
			//! In-place subtraction of two vectors. Calls Vector operator-(const Vector & lhs, const Vector & rhs) using this object as lhs. \param rhs The vector to be subtracted from the current object. Must be the same Size as the calling object. \returns A reference to the now-modified calling object
			Vector & operator-=(const Vector & rhs)
			{
				if (Size != rhs.Size)
				{
					throw std::runtime_error("Cannot add vectors of different sizes");
				}
				for (int i = 0; i < Size; ++i)
				{
					Data[i] -= rhs[i];
				}
				return *this;
			}
			
			//! In-place addition of a scalar onto the callign object. Calls Vector operator+(const Vector & lhs, const double & scalar) using this object as lhs. \param scalar The double to be accumulated into the current object. \returns A reference to the now-modified calling object
			Vector & operator+=(const double & scalar)
			{
				for (int i = 0; i < Size; ++i)
				{
					Data[i] += scalar;
				}
				return *this;
			}
			
			//! In-place subtraction of a scalar onto the callign object. Calls Vector operator-(const Vector & lhs, const double & scalar) using this object as lhs. \param scalar The double to be subtracted from the current object. \returns A reference to the now-modified calling object
			Vector & operator-=(const double & scalar)
			{
				for (int i = 0; i < Size; ++i)
				{
					Data[i] -= scalar;
				}
				return *this;
			}
			
			//! In-place multiplication of a scalar with the calling object. Calls Vector operator*(const Vector & lhs, const double & scalar) using this object as lhs. \param scalar The double to be accumulated into the current object. \returns A reference to the now-modified calling object
			Vector & operator*=(const double & scalar)
			{
				for (int i = 0; i < Size; ++i)
				{
					Data[i] *= scalar;
				}
				return *this;
			}
			
			//! In-place division of the calling object with a scalar. Calls Vector operator/(const Vector & lhs, const double & scalar) using this object as lhs. \param scalar The double to be accumulated into the current object. \returns A reference to the now-modified calling object
			Vector & operator/=(const double & scalar)
			{
				for (int i = 0; i < Size; ++i)
				{
					Data[i] /= scalar;
				}
				return *this;
			}
		private:
			std::vector<double> Data;
						
			std::string inline negativeIntegerError(int idx) const
			{
				return "JSL::Vector Error: Cannot access negative indices (" + std::to_string(idx) + ").";
			}
			std::string inline outOfBoundsError(int idx) const
			{
				return "JSL::Vector Error: " + std::to_string(idx) +" exceeds the length of this vector (" + std::to_string(Size) + ").";
			}
	};
	
	//Overloaded operators
	
	inline bool operator==(const Vector & lhs, const Vector & rhs)
	{
		if (lhs.Size != rhs.Size)
		{
			return false;
		}
		
		for (int i = 0; i < lhs.Size;++i)
		{
			if (lhs[i] != rhs[i])
			{
				return false;
			}
		}
		return true;
	}
	
	inline bool operator!=(const Vector & lhs, const Vector & rhs)
	{
		return !(lhs == rhs);
	}

	//! Performs obvious vector addition (a+b)_i = a_i + b_i. Throws an error if the vectors are not the same size. \param lhs The first vector to be summed \param rhs The second vector to be summed (order is irrelevant) \return The vector lhs + rhs
	inline Vector operator+(const Vector & lhs, const Vector & rhs)
	{
		if (rhs.Size != lhs.Size)
		{
			throw std::runtime_error("JSL::Vector Error: Cannot peform + operation on vectors of different sizes");
		}
		
		JSL::Vector output = lhs;
		
		for (int i = 0; i < output.Size; ++i)
		{
			output[i] += rhs[i];
		}
		return output;
	};
	//! Performs obvious vector subtraction (a-b)_i = a_i - b_i. Throws an error if the vectors are not the same size. \param lhs The base vector \param rhs The vector to be subtracted from the base vector (order does matter!) \return The vector lhs - rhs.
	inline Vector operator-(const Vector & lhs, const Vector & rhs)
	{
		if (rhs.Size != lhs.Size)
		{
			throw std::runtime_error("JSL::Vector Error: Cannot peform - operation on vectors of different sizes");
		}
		
		JSL::Vector output = lhs;
		
		for (int i = 0; i < output.Size; ++i)
		{
			output[i] -= rhs[i];
		}
		return output;
	};

	//! Adds the value of scalar to every element in the vector \param lhs The vector to be summed \param scalar The scalar to be added element-wise \return The vector lhs + scalar
	inline Vector operator+(const Vector & lhs, const double & scalar)
	{

		JSL::Vector output = lhs;
		
		for (int i = 0; i < output.Size; ++i)
		{
			output[i] += scalar;
		}
		return output;
	};
	
	//! Exactly equivalent to JSL::operator+(const Vector &lhs, const double &scalar), just swapped around. \param scalar The scalar to be added element-wise \param rhs The vector to be summed \return The scalar + rhs
	inline Vector operator+(const double & scalar, const Vector & rhs)
	{
		return rhs + scalar;
	}
	//! Subtracts the value of scalar to every element in the vector \param lhs The base vector \param scalar The scalar to be subtracted from the base vector element wise \return The vector lhs - scalar.
	inline Vector operator-(const Vector & lhs, const double & scalar)
	{
		return lhs + -1*scalar;
	}
	
	//! A slightly odd operation (included for completeness) - adds the value of scalar to the negative of the elements of the vector \param scalar The value which acts as a base \param rhs The vector which will be subtracted elementwise from the base scalar \return The vector scalar - rhs
	inline Vector operator-(const double & scalar, const Vector & rhs)
	{
		//To do it the `clever' way would be to redirect this to return scalar + -1*rhs, but this would involve two separate loops over the array. Annoyingly, turns out to be better to just copy the loop code and do it only once.
		JSL::Vector output(rhs.Size);
		for (int i = 0; i < output.Size; ++i)
		{
			output[i] = scalar - rhs[i];
		}
		return output;
	}
	
	
	
	
	
	
	//! Naive element-wise scalar multiplication. \param scalar The value to multiply elements by \param rhs The vector to multiply \returns The pointwise product of the elements of rhs and the scalar
	inline Vector operator*(const double & scalar, const Vector & rhs)
	{
		JSL::Vector output = rhs;
		for (int i = 0; i < output.Size; ++i)
		{
			output[i] *= scalar;
		}
		return output;
	}
	
	//! Alias of JSL::operator+(const double &scalar,const Vector &rhs) with the operation order swapped around  \param lhs The vector to multiply \param scalar The value to multiply elements by \returns The pointwise product of the elements of lhs and the scalar
	inline Vector operator*(const Vector & lhs, const double & scalar)
	{
		return scalar * lhs;
	}
	
	//! Essentially an alias for JSL::operator+(const double &scalar,const Vector &rhs) with the scalar set to one-over itself, i.e. pointwise division of the provided vector. \param lhs The vector to divide \param scalar The value to divide elements by \returns The pointwise divisor of the elements of lhs and the scalar
	inline Vector operator/(const Vector & lhs, const double & scalar)
	{
		if (scalar == 0)
		{
			throw std::runtime_error("Division by zero results in undefined behaviour");
		}
		return 1.0/scalar * lhs;
	}
	
	//! Executes the pointwise (Hadamard) product of two vectors \param lhs The first vector to be multiplied \param rhs The second vector to be multiplied (order is irrelevant) \return The vector (lhs * rhs)_i = lhs_i * rhs_i
	inline Vector operator*(const Vector & lhs, const Vector & rhs)
	{
		JSL::Vector output = rhs;
		for (int i = 0; i < output.Size; ++i)
		{
			output[i] *= rhs[i];
		}
		return output;
	}
	
	//! The standard dot product on R^n \param lhs Vector 1, \param rhs Vector 2 (order irrelevant) \returns The sum (lhs_i * rhs_i)
	inline double VectorDotProduct(const Vector & lhs, const Vector & rhs)
	{
		double sum =0;
		if (lhs.Size != rhs.Size)
		{
			throw std::runtime_error("Cannot take the dot product of vectors of different sizes");
		}
		for (int i = 0; i < lhs.Size; ++i)
		{
			sum += lhs[i] * rhs[i];
		}
		return sum;
	}
	
	//! The standard cross product -- only defined on R^3 (throws an error else) \param lhs Vector 1 \param rhs Vector 2 (order relevant - using standard conventions Vector 1 x Vector 2) \returns Vector cross product of inputs
	inline Vector VectorCrossProduct(const Vector & lhs, const Vector & rhs)
	{
		bool correctSize = (lhs.Size == 3) && (rhs.Size == 3);
		if (!correctSize)
		{
			throw std::runtime_error("Cannot compute the cross product for vectors not on R^3");
		}
		
		JSL::Vector output(3);
		
		output[0] = lhs[1]*rhs[2] - rhs[1]*lhs[2];
		output[1] = lhs[2]*rhs[0] - rhs[2]*lhs[0];
		output[2] = lhs[0]*rhs[1] - rhs[0]*lhs[1];
		
		return output;
	}
	
	//! Uses Vector::Norm() and VectorDotProduct() to extract an angle between the vectors. \param lhs Vector 1, \param rhs Vector 2 (order irrelevant) \returns The angle between the two vectors (between 0 and M_PI)
	inline double AngleBetweenVectors(const Vector & lhs, const Vector & rhs)
	{
		double aDotb = lhs.Dot(rhs);
		double norms = lhs.Norm() * rhs.Norm();
		if (norms == 0)
		{
			throw std::runtime_error("The angle between a vector of length zero is undefined");
		}
		
		double cosTheta = std::min(1.0,aDotb /norms);
		
		return acos(cosTheta);
		
	}

	//! Calls JSL::Vector::to_string() and then passes it to the provided stream, enabling sweet, smooth output such as std::cout << v1 << std::endl. \param os An output stream capable of parsing strings \param obj A vector object to be inserted into the stream for output \returns A reference to the modified stream 
	inline std::ostream& operator<<(std::ostream& os, const Vector & obj)
	{
		os << obj.to_string();
		return os;
	}
	
}
