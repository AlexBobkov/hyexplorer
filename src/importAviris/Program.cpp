#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QApplication>
#include <QDebug>
#include <QDateTime>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

class CsvRow
{
public:
    CsvRow(const std::string& input)
    {
        std::size_t offset = 0;
        while (offset < input.size())
        {
            std::size_t pos = 0;

            //Все, что внутри двойных кавычек считаем за одно поле
            if (input[offset] == '"')
            {
                pos = input.find('"', offset + 1);
                pos++;
            }
            else
            {
                pos = input.find(',', offset);
            }

            if (pos >= input.size() || pos == std::string::npos)
            {
                break;
            }

            if (input[offset] == '"')
            {
                _tokens.push_back(input.substr(offset + 1, pos - offset - 2));
            }
            else
            {
                _tokens.push_back(input.substr(offset, pos - offset));
            }
            offset = pos + 1;
        }

        if (input[offset] == '"')
        {
            _tokens.push_back(input.substr(offset + 1, input.size() - offset - 2));
        }
        else
        {
            _tokens.push_back(input.substr(offset, input.size() - offset));
        }
    }

    std::size_t size() const
    {
        return _tokens.size();
    }

    template<typename T>
    T as(std::size_t column) const
    {
        assert(column < _tokens.size());
        assert(_tokens[column].size() > 0);

        return boost::lexical_cast<T>(_tokens[column]);
    }
        
    template<>
    std::string as<std::string>(std::size_t column) const
    {
        assert(column < _tokens.size());

        std::string str = _tokens[column];

        //Заменяем одинарные кавычки на двойные
        std::size_t offset = 0;
        std::size_t pos = 0;
        while (offset < str.size() && (pos = str.find('\'', offset)) != std::string::npos)
        {
            str.insert(pos, 1, '\'');
            offset = pos + 2;
        }

        return str;
    }

protected:
    std::vector<std::string> _tokens;
};

const bool doInsert = true;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <csv filename>\n";
        return 1;
    }
    
    QApplication app(argc, argv);

    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
#if 0
    db.setHostName("localhost");
    db.setDatabaseName("GeoPortal");
    db.setUserName("user");
    db.setPassword("user");
#else
    db.setHostName("178.62.140.44");
    db.setDatabaseName("GeoPortal");
    db.setUserName("portal");
    db.setPassword("PortalPass");
#endif

    if (!db.open())
    {
        qDebug() << "Failed to open database:" << db.lastError().text();
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

    std::ostringstream queryStream;
    queryStream << "BEGIN;";

    while (true)
    {
        std::string str;
        std::getline(fin, str);
        if (str.empty())
        {
            break; //end of file
        }

        CsvRow row(str);

        std::string sceneId = row.as<std::string>(0);
        std::string sitename = row.as<std::string>(1);        
        std::string investigator = row.as<std::string>(3);
        std::string comments = row.as<std::string>(4);
                
        std::string rdnver = row.as<std::string>(7);
        std::string geover = row.as<std::string>(9);

        std::string tape = row.as<std::string>(11);
        int flight = row.as<int>(13);
        int run = row.as<int>(14);

        int year = row.as<int>(15);
        int month = row.as<int>(16);
        int day = row.as<int>(17);
        int hour = row.as<int>(18);
        int minute = row.as<int>(19);

        QDateTime sceneTime = QDateTime(QDate(year, month, day), QTime(hour, minute), Qt::UTC);
        
        double pixelSize = row.as<double>(20);
        double rotation = row.as<double>(21);
        
        double sunElevation = row.as<double>(24);
        double sunAzimuth = row.as<double>(25);
                
        double meanSceneElev = row.as<double>(26);
        double minSceneElev = row.as<double>(27);
        double maxSceneElev = row.as<double>(28);
                        
        std::string sceneUrl = row.as<std::string>(38);

        double lon1 = row.as<double>(40);
        double lon2 = row.as<double>(41);
        double lon3 = row.as<double>(42);
        double lon4 = row.as<double>(43);

        double lat1 = row.as<double>(44);
        double lat2 = row.as<double>(45);
        double lat3 = row.as<double>(46);
        double lat4 = row.as<double>(47);        

        std::ostringstream polygonStream;
        polygonStream << std::setprecision(15);
        polygonStream << "ST_GeographyFromText('SRID=4326;POLYGON((";
        polygonStream << lon1 << " " << lat1 << ",";
        polygonStream << lon2 << " " << lat2 << ",";
        polygonStream << lon3 << " " << lat3 << ",";
        polygonStream << lon4 << " " << lat4 << ",";
        polygonStream << lon1 << " " << lat1 << "))')";        

        if (fin.fail())
        {
            std::cout << "Failed to read file " << argv[1] << std::endl;
            return 1;
        }

        queryStream << "WITH ins1 AS(";
        queryStream << "INSERT INTO public.scenes (sensor, sceneid, scenetime, pixelsize, bounds, sunazimuth, sunelevation, hasoverview, hasscene, sceneurl) ";
        queryStream << "VALUES ('AVIRIS', '" << sceneId << "','" << sceneTime.toString(Qt::ISODate).toUtf8().constData() << "'," << pixelSize << "," << polygonStream.str() << "," << sunAzimuth << "," << sunElevation << ",FALSE,FALSE,'" << sceneUrl << "') ";
        queryStream << "RETURNING ogc_fid";
        queryStream << ") ";

        queryStream << "INSERT INTO public.aviris (ogc_fid, sitename, comments, investigator, scenerotation, tape, geover, rdnver, meansceneelev, minsceneelev, maxsceneelev, flight, run)";
        queryStream << "SELECT ogc_fid,'" << sitename << "','" << comments << "','" << investigator << "'," << rotation << ",'" << tape << "','"<< geover << "','" << rdnver << "'," << meanSceneElev << "," << minSceneElev << "," << maxSceneElev << "," << flight << "," << run << " ";
        queryStream << "FROM ins1;";

        counter++;

        if (counter % 10000 == 0)
        {
            std::cout << "Insert counter " << counter << std::endl;

            queryStream << "COMMIT;ANALYZE public.scenes;";

            if (doInsert)
            {
                QSqlQuery query;
                if (!query.exec(queryStream.str().c_str()))
                {
                    qDebug() << "Failed to execute insert query: " << query.lastError().text();
                    return 1;
                }
            }

            queryStream = std::ostringstream();
            queryStream << "BEGIN;";
        }
    }

    std::cout << "Insert counter " << counter << std::endl;

    queryStream << "COMMIT;ANALYZE public.scenes;";

    if (doInsert)
    {
        QSqlQuery query;
        if (!query.exec(queryStream.str().c_str()))
        {
            qDebug() << "Failed to execute insert query: " << query.lastError().text();
            return 1;
        }
    }

    fin.close();
     
    std::cout << "Records " << counter << std::endl;

    return 0;
}
