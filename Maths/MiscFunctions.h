#pragma once

namespace JSL
{
	//! A useful function when one knows a value needs to be between 0 and 1 \param a The value to be tested \returns min(max(0,a),1), i.e., if 0<a<1, returns a, otherwise returns either 0 or 1 as appropriate
	inline double FractionBounder(double a)
	{
		return std::min( std::max(0.0,a),1.0);
	};


}
