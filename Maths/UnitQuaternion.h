#pragma once
#include <vector>
#include <stdexcept>
#include <string.h>
#include <iostream>
#include <math.h>
#include "Quaternion.h"


namespace JSL
{
	
	class UnitQuaternion : public Quaternion
	{
		public:
			
			
			UnitQuaternion() : Quaternion
			{
				Data[0] = 1;
			}
		
			UnitQuaternion(const double & scalar, const JSL::Vector & vec): Quaternion(scalar,vec)
			{
				
			}
	
			
	
	
			JSL::Vector Rotate(const JSL::Vector & vec) const
			{
				if (vec.Size() != 3)
				{
					throw std::runtime_error("ERROR in JSL::Quaternion: Cna only rotate vectors if they have dimension 3");
				}
				
				Quaternion q = (*this) * Quaternion(0,vec) * Conjugate();
				return q.Vector();
			}
			
		
			bool CheckNormalisation(bool throwError)
			{
				double NormalisationTolerance = 1e-5;
				bool isNormalised = true;
				if ( abs(Norm() -1) < NormalisationTolerance)
				{
					isNormalised = false;
					
					if (throwError)
					{
						throw std::runtime_error("ERROR in JSL::UnitQuaternion: You have attempted to create a UnitQuaternion with norm " + std::to_string(Norm()) + ", this exceeds the stated tolerances");
					}
				}
				return isNormalised;
			}
	}
}
