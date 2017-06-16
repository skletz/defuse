/** DeXtraction (Version 2.0) ***************************
 * ******************************************************
 *       _    _      ()_()
 *      | |  | |    |(o o)
 *   ___| | _| | ooO--`o'--Ooo
 *  / __| |/ / |/ _ \ __|_  /
 *  \__ \   <| |  __/ |_ / /
 *  |___/_|\_\_|\___|\__/___|
 *
 * ******************************************************
 * Purpose: Extract a specific descriptor from a set of videos or fram a specific video file
 * Input/Output:
 *	-arg1: Name of the dataset
 *	-arg2: Type of Input
 *		0 = Path to a specific video file
 *		1 = Path to a directory of video files*
 *	-arg3: Path to a directory of video segments ..\\data\\segments OR to a specific video file ..\\data\\segments\\1\\test.mp4
 *	-arg4: Path to the output directory ..\\data\\features
 *	-arg5: Type of Descriptor <INT>
 *		0 = static feature signatures (variant 1)
 *		1 = dynamic feature signatures (variant 1) //trajectories
 *		2 = dynamic feature signatures (variant 2) //optical-flow
 *		3 = motion histogram (variant 1)
 *		4 = cnn (fc7 layer)
 *	-args > 5 depends on the type of the choosen descriptor
 *		***********************************************************
 *		TYPE: 0 = static feature signatures variant 1
 *		***********************************************************
 *		-arg6: Number of Samplepoints
 *		-arg7: Distribution
 *			0 = RANDOM
 *			1 = REGULAR
 *			2 = GAUSSIAN
 *		-arg8: Number of initial centroids <INT>
 *		-arg9: Path to a directory of samplepoint files, if distribution is RANDOM ..\\data\\samplepoints
 *		-arg10: Number of iterations adjustable clustering - Default: 5 <INT>
 *		-arg11: Minimal weight adjustable clustering - Default: 2 <INT>
 *		-arg12: Minimal distance adjustable clustering - Default: 0.01 <FLOAT>
 *		-arg13: Keyframe selection
 *			0 = use middle frame of each segment
 *			1 = use first frame of each segment
 *			2 = use last frame of each segment
  *		***********************************************************
 *		TYPE: 1 = dynamic feature signatures variant 1 using trajectories
 *		***********************************************************
 *		-arg6: Number of Samplepoints
 *		-arg7: Distribution
 *			0 = RANDOM
 *			1 = REGULAR
 *			2 = GAUSSIAN
 *		-arg8: Number of initial centroids <INT>
 *		-arg9: Path to a directory of samplepoint files, if distribution is RANDOM ..\\data\\samplepoints
 *		-arg10: Number of iterations adjustable clustering - Default: 5 <INT>
 *		-arg11: Minimal weight adjustable clustering - Default: 2 <INT>
 *		-arg12: Minimal distance adjustable clustering - Default: 0.01 <FLOAT>
 *		-arg13: Frame selection
 *			0 = use fix number of frames per segment (v1)
 *			1 = use fix number per second
 *			2 = use all frames
 *			3 = use fix number of frames per segment (v2)
 *		-arg14: Number of frames per segment or second, if keyframe selection type is 1,2 (fix number)
 *		***********************************************************
 *		TYPE: 2 = dynamic feature signatures variant 2 using optical-flow
 *		***********************************************************
 *		-arg6: Number of Samplepoints
 *		-arg7: Distribution
 *			0 = RANDOM
 *			1 = REGULAR
 *			2 = GAUSSIAN
 *		-arg8: Number of initial centroids <INT>
 *		-arg9: Path to a directory of samplepoint files, if distribution is RANDOM ..\\data\\samplepoints
 *		-arg10: Number of iterations adjustable clustering - Default: 5 <INT>
 *		-arg11: Minimal weight adjustable clustering - Default: 2 <INT>
 *		-arg12: Minimal distance adjustable clustering - Default: 0.01 <FLOAT>
 *		-arg13: Frame selection
 *			0 = use fix number of frames per segment (v1)
 *			1 = use fix number of frames per segment (v2)
 *			2 = use fix number per second
 *			3 = use all frames
 *		-arg14: Number of frames per segment or second, if keyframe selection type is 1,2 (fix number)
 *		***********************************************************
 *		TYPE: 3 = motion histogram variant 1
 *		***********************************************************
 *		-arg6: Number of samplepoints
 *		-arg7: Distribution
 *			0 = RANDOM
 *			1 = REGULAR
 *			2 = GAUSSIAN
  *		-arg8: Path to a directory of samplepoint files, if distribution is RANDOM ..\\data\\samplepoints
 *		-arg9: Frame selection
 *			0 = use fix number of frames per segment (v1)
 *			1 = use fix number of frames per segment (v2)
 *			2 = use fix number per second
 *			3 = use all frames
 *		-arg10: Number of frames per segment or second, if keyframe selection type is 1,2 (fix number)
 *		-arg11: samplex
 *		-arg12: sampley
 *		***********************************************************
 *		Last parameter visualize extraction without saving it
 *		***********************************************************
 *		-argsLAST: 0 false, 1 true
 * @author skletz
 * @version 2.0 13/03/17
 *
**/

#include <future>
#include <cpluslogger.hpp>
#include <cplusutil.hpp>
#include <defuse.hpp>

using namespace defuse;

//Default Settings
static int descriptortype = 0;
static std::string descriptorname = "static feature signatures variant 1";
static std::string descriptorshort = "SFS1";

static Parameter* paramter = new SFS1Parameter();
static std::string extractionsettings = ""; //is generated at runtime to use it for filenames, depends on the given settings

static std::string videopath = "..\\data\\segments\\";
static std::string outputpath = "..\\data\\features\\";
static std::string samplepointspath = "..\\data\\samplepoints\\";

static std::string dataset = "data_test-v1_0";
static bool videopathIsDirectory = true;

static Directory* clipsdir = nullptr;
static File* videoFile = nullptr; //In the case of the input is only one video file
static Directory* outputdir = nullptr;

void processVideoDirectory(Xtractor* xtractor);
void processVideoFile(Xtractor* xtractor);

Features* xtract(VideoBase* _videobase, Xtractor* xtractor);
bool serialize(Features* _features, File _file);
Features* extractVideo(Directory* dir, File* videofile, Xtractor* xtractor);

static bool display = false;

std::mutex m;
std::unique_lock<std::mutex> f() {
	std::unique_lock<std::mutex> guard(m);
	return std::move(guard);
}

int main(int argc, char **argv)
{

	if (argc < 3)
	{
		LOG_ERROR("xtraction: too few arguments!");
		return EXIT_SUCCESS;
	}
	else
	{
		dataset = argv[1];
		videopathIsDirectory = std::atoi(argv[2]);
		videopath = argv[3];
		outputpath = argv[4];
		descriptortype = std::atoi(argv[5]);


		if (descriptortype == 0) //Default static feature signatures variant 1
		{
			static_cast<SFS1Parameter *>(paramter)->samplepoints = std::atoi(argv[6]);
			static_cast<SFS1Parameter *>(paramter)->distribution = std::atoi(argv[7]);
			static_cast<SFS1Parameter *>(paramter)->initialCentroids = std::atoi(argv[8]);
			static_cast<SFS1Parameter *>(paramter)->samplepointsfile = argv[9];
			static_cast<SFS1Parameter *>(paramter)->iterations = std::atoi(argv[10]);
			static_cast<SFS1Parameter *>(paramter)->minimalWeight = std::atoi(argv[11]);
			static_cast<SFS1Parameter *>(paramter)->minimalDistance = std::stof(argv[12]);
			static_cast<SFS1Parameter *>(paramter)->keyframeSelection = std::atoi(argv[13]);
			display = std::atoi(argv[14]);

		}
		else if (descriptortype == 1)
		{
			descriptorname = "dynamic feature signatures variant 1 using trajectories";
			descriptorshort = "DFS1";

			paramter = new DFS1Parameter();
			static_cast<DFS1Parameter *>(paramter)->staticparamter.samplepoints = std::atoi(argv[6]);
			static_cast<DFS1Parameter *>(paramter)->staticparamter.distribution = std::atoi(argv[7]);
			static_cast<DFS1Parameter *>(paramter)->staticparamter.initialCentroids = std::atoi(argv[8]);
			static_cast<DFS1Parameter *>(paramter)->staticparamter.samplepointsfile = argv[9];
			static_cast<DFS1Parameter *>(paramter)->staticparamter.iterations = std::atoi(argv[10]);
			static_cast<DFS1Parameter *>(paramter)->staticparamter.minimalWeight = std::atoi(argv[11]);
			static_cast<DFS1Parameter *>(paramter)->staticparamter.minimalDistance = std::stof(argv[12]);

			LOG_INFO("Frame Selection " << std::atoi(argv[13]));
			static_cast<DFS1Parameter *>(paramter)->frameSelection = std::atoi(argv[13]);
			static_cast<DFS1Parameter *>(paramter)->frames = std::atoi(argv[14]);
			display = std::atoi(argv[15]);
		}
		else if (descriptortype == 2)
		{
			descriptorname = "dynamic feature signatures variant 2 using optical-flow";
			descriptorshort = "DFS2";

			paramter = new DFS2Parameter();
			static_cast<DFS2Parameter *>(paramter)->staticparamter.samplepoints = std::atoi(argv[6]);
			static_cast<DFS2Parameter *>(paramter)->staticparamter.distribution = std::atoi(argv[7]);
			static_cast<DFS2Parameter *>(paramter)->staticparamter.initialCentroids = std::atoi(argv[8]);
			static_cast<DFS2Parameter *>(paramter)->staticparamter.samplepointsfile = argv[9];
			static_cast<DFS2Parameter *>(paramter)->staticparamter.iterations = std::atoi(argv[10]);
			static_cast<DFS2Parameter *>(paramter)->staticparamter.minimalWeight = std::atoi(argv[11]);
			static_cast<DFS2Parameter *>(paramter)->staticparamter.minimalDistance = std::stof(argv[12]);

			static_cast<DFS2Parameter *>(paramter)->frameSelection = std::atoi(argv[13]);
			static_cast<DFS2Parameter *>(paramter)->frames = std::atoi(argv[14]);
			display = std::atoi(argv[15]);
		}
		else if (descriptortype == 3)
		{
			descriptorname = "motion histogram variant 1";
			descriptorshort = "MoHist1";

			paramter = new MoHist1Parameter();
			static_cast<MoHist1Parameter *>(paramter)->samplepoints = std::atoi(argv[6]);
			static_cast<MoHist1Parameter *>(paramter)->distribution = std::atoi(argv[7]);
			static_cast<MoHist1Parameter *>(paramter)->samplepointsfile = argv[8];

			static_cast<MoHist1Parameter *>(paramter)->frameSelection = std::atoi(argv[9]);
			static_cast<MoHist1Parameter *>(paramter)->frames = std::atoi(argv[10]);
			static_cast<MoHist1Parameter *>(paramter)->samplex = std::atoi(argv[11]);
			static_cast<MoHist1Parameter *>(paramter)->sampley = std::atoi(argv[12]);
			display = std::atoi(argv[13]);

			Directory tmp(outputpath);
			tmp.addDirectory("images");
			static_cast<MoHist1Parameter *>(paramter)->imgOutputPath = tmp.getPath();
		}

		//generate filename depending on the descriptor
		extractionsettings = paramter->getFilename();

	}


	//Init output directory
	outputdir = new Directory(outputpath);

	//Logfile, init with current timestamp
	time_t now = time(0);
	static char name[20];
	strftime(name, sizeof(name), "%Y%m%d_%H%M%S", localtime(&now));

	std::stringstream logfile;
	logfile << name << "_" << extractionsettings << "_Extraction" << ".log";
	Directory workingdir(".");
	File log(workingdir.getPath(), logfile.str());
	log.addDirectoryToPath("logs");

	//Log input paramters for later usages
	LOG_INFO("Input Parameter");

	Xtractor* xtractor = nullptr;

	if (descriptortype == 0)
	{
		xtractor = new SFS1Xtractor(paramter);
	}
	else if (descriptortype == 1)
	{
		xtractor = new DFS1Xtractor(paramter);
	}
	else if (descriptortype == 2)
	{
		xtractor = new DFS2Xtractor(paramter);
	}
	else if (descriptortype == 3)
	{
		xtractor = new MoHist1Xtractor(paramter);
	}
	else
	{
		LOG_FATAL("Extraction Method not implemented " << descriptortype);
	}

	xtractor->display = display;

	std::string cmd = "";
	for (int i = 0; i < argc; i++)
	{
		cmd += argv[i];
		cmd += " ";

	}


	if (videopathIsDirectory)
	{
		clipsdir = new Directory(videopath);
		cpluslogger::Logger::get()->logfile(log.getFile());
		cpluslogger::Logger::get()->filelogging(true);
		cpluslogger::Logger::get()->perfmonitoring(true);
	}

	LOG_INFO(cmd);

	LOG_INFO("************************************************");
	LOG_INFO("DeXtraction-v2.0");
	LOG_INFO("Used Feature: " + descriptorname + "\n");
	LOG_INFO("Input Directory: " << videopath);
	LOG_INFO("Output Directory: " << outputpath);
	LOG_INFO("Logging: " << log.getFile().c_str());
	LOG_INFO(paramter->get());
	LOG_INFO("************************************************\n");

	if (videopathIsDirectory)
	{
		processVideoDirectory(xtractor);
	}
	else
	{
		videoFile = new File(videopath);
		clipsdir = new Directory(videoFile->getFile().substr(0, videoFile->getFile().find_last_of("/\\")));
		processVideoFile(xtractor);
	}



	delete xtractor;
	delete paramter;
	delete outputdir;

	if (videoFile != nullptr) delete videoFile; //if the input is a directory, videoFile will never be initialized
	delete clipsdir;

	return EXIT_SUCCESS;
}

void processVideoDirectory(Xtractor* xtractor)
{
	std::vector<std::string> entries = cplusutil::FileIO::getFileListFromDirectory(clipsdir->getPath());
	int numClips = 0;
	float computationTimes = 0.0;
	LOG_PERFMON(PINTERIM, "Groups:\t" << entries.size());
	for (int iEntry = 0; iEntry < entries.size(); iEntry++)
	{
		std::string entry = entries.at(iEntry);

		std::vector<std::string> clippathes = cplusutil::FileIO::getFileListFromDirectory(entry);

		std::vector<std::future<Features*>> pendingFutures;

		numClips += static_cast<int>(clippathes.size());

		for (int iClipFile = 0; iClipFile < clippathes.size(); iClipFile++)
		{

			boost::filesystem::path isClipFileDir{ clippathes.at(iClipFile) };
			if(boost::filesystem::is_directory(isClipFileDir))
			{
				std::vector<std::string> files = cplusutil::FileIO::getFileListFromDirectory(isClipFileDir.string());
				for(int iFile = 0; iFile < files.size(); iFile++)
				{
					File* file = new File(files.at(iFile));
					std::string filename = file->getFilename();
					std::string parentpath = clippathes.at(iClipFile).substr(0, clippathes.at(iClipFile).find_last_of("/\\"));
					Directory parentdirectory(parentpath);

					std::vector<std::string> parts;
					cplusutil::String::split(filename, '_', parts);

					std::vector<std::string> videoparts;
					cplusutil::String::split(parts.at(0), '.', videoparts);

					int videoID = cplusutil::String::extractIntFromString(videoparts.at(0));

					int startFrameNr;
					{

							std::vector<std::string> p;
							cplusutil::String::split(parts.at(parts.size()-1), '-', p);
							if(p.size() > 1)
							{
								std::string st = p.at(0) + "";
								st += p.at(1);
								std::istringstream reader(st);
								reader >> startFrameNr;
							}else
							{
								std::istringstream reader(parts.at(parts.size()-1)); //the last one is the framenumber
								reader >> startFrameNr;

							}
					}

					int clazz;
					{
							std::istringstream reader(parentdirectory.mDirName);
							reader >> clazz;
					}

					VideoBase* videoclip = new VideoBase(file, videoID, startFrameNr, clazz);

					if(!display)
						pendingFutures.push_back(std::async(std::launch::async, &xtract, videoclip, xtractor));
					else
						xtract(videoclip, xtractor);
				}
			}
			else
			{
				File* file = new File(clippathes.at(iClipFile));

				std::string filename = file->getFilename();
				std::string parentpath = clippathes.at(iClipFile).substr(0, clippathes.at(iClipFile).find_last_of("/\\"));
				Directory parentdirectory(parentpath);

				std::vector<std::string> parts;
				cplusutil::String::split(filename, '_', parts);

				std::vector<std::string> videoparts;
				cplusutil::String::split(parts.at(0), '.', videoparts);

				int videoID = cplusutil::String::extractIntFromString(videoparts.at(0));

				unsigned int startFrameNr;
				{
					std::istringstream reader(parts.at(1));
					reader >> startFrameNr;
				}

				int clazz;
				{
					std::istringstream reader(parentdirectory.mDirName);
					reader >> clazz;
				}

				VideoBase* videoclip = new VideoBase(file, videoID, startFrameNr, clazz);

				if (!display)
					pendingFutures.push_back(std::async(std::launch::async, &xtract, videoclip, xtractor));
				else
					xtract(videoclip, xtractor);

			}
		}


		LOG_DEBUG("Waiting of pending values ...");
		LOG_INFO("Current Directoy:\t" << entries.at(iEntry) << "\tSize:\t" << clippathes.size());

		std::string name = "Extraction ";
		name += std::to_string(pendingFutures.size());

		float computationTime = 0.0;
		for (int i = 0; i < pendingFutures.size(); i++)
		{
			Features* features = pendingFutures.at(i).get();
			computationTime += features->mExtractionTime;

			std::stringstream stream;
			if (descriptortype < 3)
			{
				stream << static_cast<FeatureSignatures *>(features)->mVideoID << "_" << static_cast<FeatureSignatures *>(features)->mStartFrameNr << "_" << static_cast<FeatureSignatures *>(features)->mVideoFileName;
			}
			else if (descriptortype == 3)
			{
				stream << static_cast<MotionHistogram *>(features)->mVideoID << "_" << static_cast<MotionHistogram *>(features)->mStartFrameNr << static_cast<MotionHistogram *>(features)->mVideoFileName;
			}
			else
			{
				LOG_FATAL("Descriptor " << descriptortype << " not implemented!");
			}

			File output(outputdir->getPath(), stream.str(), ".yml");
			output.addDirectoryToPath(descriptorshort);
			output.addDirectoryToPath(extractionsettings);

			std::unique_lock<std::mutex> guard(f());
			cplusutil::Terminal::showProgress(name, i + 1, pendingFutures.size());
			guard.unlock();
			serialize(features, output);
			delete features;
		}

		computationTime /= float(pendingFutures.size());
		computationTimes += computationTime;

		LOG_PERFMON(PTIME, "Average Computation-Time: Extracting (secs) per Group Count: \t" << clippathes.size() << "\t" << computationTime);
		LOG_DEBUG("End of Calculation: \t" << "\tSize:\t" << clippathes.size());

	}

	LOG_PERFMON(PINTERIM, "Video Segments:\t" << numClips);

	computationTimes /= float(entries.size());
	LOG_PERFMON(PTIME, "Computation-Time: Extracting (secs) \t" << entries.size() << "\t" << extractionsettings << "\t" << computationTimes);

	if(display) getchar();
}

void processVideoFile(Xtractor* xtractor)
{
	Features* features = extractVideo(clipsdir, videoFile, xtractor);
	std::stringstream stream;
	if (descriptortype < 3)
	{
		stream << static_cast<FeatureSignatures *>(features)->mVideoID << "_" << static_cast<FeatureSignatures *>(features)->mStartFrameNr;
	}
	else if (descriptortype == 3)
	{
		stream << static_cast<MotionHistogram *>(features)->mVideoID << "_" << static_cast<MotionHistogram *>(features)->mStartFrameNr;
	}
	else
	{
		LOG_FATAL("Extraction Method not implemented " << descriptortype);
	}


	File output(outputdir->getPath(), stream.str(), ".yml");
	output.addDirectoryToPath(descriptorshort);
	output.addDirectoryToPath(extractionsettings);

	serialize(features, output);

	//delete output;
	delete features;
}

Features* xtract(VideoBase* _videobase, Xtractor* xtractor)
{
	size_t e1_start = double(cv::getTickCount());

	Features* features = xtractor->xtract(_videobase);

	size_t e1_end = double(cv::getTickCount());

	double elapsed_secs = (e1_end - e1_start) / double(cv::getTickFrequency());
	features->mExtractionTime = elapsed_secs;
	//std::unique_lock<std::mutex> guard(f());
	//LOG_PERFMON(PTIME, "Computation-Time (secs): Extaction \t" << extractionsettings << "\t" << elapsed_secs);
	//guard.unlock();

	return features;
}

bool serialize(Features* _features, File _file)
{
	int featurecount = 1;
	cv::FileStorage fs(_file.getFile(), cv::FileStorage::WRITE);

	if (!fs.isOpened()) {
		LOG_ERROR("Error in serialization of Video: Invalid file " << "");
		return false;
	}

	fs << "Dataset" << dataset;
	fs << "Model" << extractionsettings;
	fs << "FeatureCount" << static_cast<int>(featurecount);
	fs << "Data" << "[";

	if (descriptortype < 3)
	{
		static_cast<FeatureSignatures *>(_features)->write(fs);
	}
	else if (descriptortype == 3)
	{
		static_cast<MotionHistogram *>(_features)->write(fs);
	}
	else
	{
		LOG_FATAL("Extraction Method not implemented " << descriptortype);
	}

	fs << "]";
	fs.release();

	return true;
}

Features* extractVideo(Directory* dir, File* videofile, Xtractor* xtractor)
{
	File* file = videofile;
	std::string filename = file->getFilename();

	std::vector<std::string> parts;
	cplusutil::String::split(videofile->getFilename(), '_', parts);

	std::vector<std::string> videoparts;
	cplusutil::String::split(parts.at(0), '.', videoparts);

	int videoID = cplusutil::String::extractIntFromString(videoparts.at(0));

	unsigned int startFrameNr;
	{
		std::istringstream reader(parts.at(1));
		reader >> startFrameNr;
	}

	int clazz;
	{
		std::istringstream reader(dir->mDirName);
		reader >> clazz;
	}

	VideoBase* videoclip = new VideoBase(file, videoID, startFrameNr, clazz);

	Features* feature = xtract(videoclip, xtractor);

	return feature;
}
