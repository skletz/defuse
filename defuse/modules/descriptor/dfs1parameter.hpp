#ifndef _DEFUSE_DFS1PARAMETER_HPP_
#define _DEFUSE_DFS1PARAMETER_HPP_
#ifdef __cplusplus

#include <cplusutil.h>
#include "../parameter.hpp"
#include "sfs1parameter.hpp"

namespace defuse {

	class DFS1Parameter : public Parameter
	{
	public:
		SFS1Parameter staticparamter;
		int frames = 5;		//The number of frames per segment
		int frameSelection = 1;

		std::string get() override
		{
			std::stringstream st;
			st << staticparamter.get();
			st << "; Frams per segment: " << frames;
			return st.str();
		}

		std::string getFilename() override
		{
			std::stringstream st;
			st << "DFS1_";
			st << staticparamter.getFilename();
			st << "_" << frames;
			return st.str();
		}
	};

}
#endif
#endif