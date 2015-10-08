#include <boost/lexical_cast.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <csv filename>\n";
        return 1;
    }

    std::ifstream fin(argv[1]);
    if (!fin)
    {
        std::cerr << "Failed to open file " << argv[1] << std::endl;
        return 1;
    }

    std::string str;
    std::getline(fin, str); //first line
        
    int counter = 0;

    while (true)
    {
        std::string sceneId;
        std::getline(fin, sceneId, ',');

        if (sceneId.empty())
        {
            break; //end of file
        }

        std::string acquisitionDateStr;
        std::getline(fin, acquisitionDateStr, ',');

        std::string cloudStr;
        std::getline(fin, cloudStr, ',');

        std::getline(fin, str, ',');
        int orbitPath = boost::lexical_cast<int>(str);

        std::getline(fin, str, ',');
        int orbitRow = boost::lexical_cast<int>(str);

        std::getline(fin, str, ',');
        int targetPath = boost::lexical_cast<int>(str);

        std::getline(fin, str, ',');
        int targetRow = boost::lexical_cast<int>(str);

        std::string station;
        std::getline(fin, station, ',');

        std::string processingLevel;
        std::getline(fin, processingLevel, ',');

        std::string sceneStartTimeStr;
        std::getline(fin, sceneStartTimeStr, ',');

        std::string sceneStopTimeStr;
        std::getline(fin, sceneStopTimeStr, ',');

        std::getline(fin, str, ',');
        double sunAzimuth = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double sunElevation = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double satelliteInclination = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double lookAngle = boost::lexical_cast<double>(str);

        std::string dateEnteredStr;
        std::getline(fin, dateEnteredStr, ',');

        std::string centerLatStr;
        std::getline(fin, centerLatStr, ',');

        std::string centerLonStr;
        std::getline(fin, centerLonStr, ',');

        std::string nwCornerLatStr;
        std::getline(fin, nwCornerLatStr, ',');

        std::string nwCornerLonStr;
        std::getline(fin, nwCornerLonStr, ',');

        std::string neCornerLatStr;
        std::getline(fin, neCornerLatStr, ',');

        std::string neCornerLonStr;
        std::getline(fin, neCornerLonStr, ',');

        std::string seCornerLatStr;
        std::getline(fin, seCornerLatStr, ',');

        std::string seCornerLonStr;
        std::getline(fin, seCornerLonStr, ',');

        std::string swCornerLatStr;
        std::getline(fin, swCornerLatStr, ',');

        std::string swCornerLonStr;
        std::getline(fin, swCornerLonStr, ',');

        std::getline(fin, str, ',');
        double centerLat = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double centerLon = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double nwCornerLat = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double nwCornerLon = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double neCornerLat = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double neCornerLon = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double seCornerLat = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double seCornerLon = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double swCornerLat = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double swCornerLon = boost::lexical_cast<double>(str);

        std::string displayId;
        std::getline(fin, displayId, ',');

        std::string orderingId;
        std::getline(fin, orderingId, ',');

        std::string downloadLink;
        std::getline(fin, downloadLink);

        if (fin.fail())
        {
            std::cout << "Failed to read file " << argv[1] << std::endl;
            return 1;
        }

        //std::cout << "Name " << sceneId << " " << orbitPath << " " << processingLevel << " " << sunElevation << " " << centerLat << " " << centerLon << " " << downloadLink << std::endl << std::endl;

        counter++;
    }

    std::cout << "Records " << counter << std::endl;

    fin.close();

    return 0;
}
