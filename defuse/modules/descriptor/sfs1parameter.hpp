#ifndef _DEFUSE_SFS1PARAMETER_HPP_
#define _DEFUSE_SFS1PARAMETER_HPP_
#ifdef __cplusplus

#include <cplusutil.h>
#include "../parameter.hpp"

namespace defuse {

	class SFS1Parameter : public Parameter
	{
	public:
		int samplepoints = 40000;			//The number of sample points
		bool distribution = 0;	//The distribution of sample points	0 = random, 2 = regular
		int initialCentroids = 40;			//The number of first centroids used for clustering
		std::string samplepointsfile = "";

		int iterations = 5;			//The number of maximal iterations
		int minimalWeight = 2;		//Minimum cluster size
		float minimalDistance = 0.01;	//Minimum distance between two centroids

		int keyframeSelection = 0; //The keyframeSelection position 0 = middle, > 0 select

		std::string get() override
		{
			std::stringstream st;
			st << "Samplepoints: " << samplepoints;
			st << "; Distribution: ";
			if (distribution == 0)
			{
				st << "random";
			}
			else
			{
				st << "regular";
			}

			st << "; Init Centroids: " << initialCentroids;
			st << "; Samplepointfile: " << samplepointsfile;
			st << "; Iterations: " << iterations;
			st << "; Minimal Weight: " << minimalWeight;
			st << "; Minimal Distance: " << minimalDistance;

			return st.str();
		}

		std::string getFilename() override
		{
			std::stringstream st;
			st << "SFS_";
			st << samplepoints;
			st << "_";
			if (distribution == 0)
			{
				st << "random";
			}
			else
			{
				st << "regular";
			}

			st << "_" << initialCentroids;
			st << "_" << iterations;
			st << "_" << minimalWeight;
			st << "_" << minimalDistance;

			return st.str();
		}
	};

}
#endif
#endif