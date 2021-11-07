#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "RAWLoader.h"
#include "Vec3D.h"

// trim from start (in place)
static inline void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
                                    { return !std::isspace(ch); }));
}

// trim from end (in place)
static inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
                         { return !std::isspace(ch); })
                .base(),
            s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s)
{
    ltrim(s);
    rtrim(s);
}

#define PROGRESS_READ_HEADER 0.1
#define PROGRESS_READ_RAW 0.9

namespace CGCP
{
    void RAWLoader::threadLoad(std::string path, ProgressCallback progress)
    {
        progress(Error::OK, nullptr, 0);
        std::ifstream datfile;
        datfile.open(path);

        if (!datfile.is_open())
        {
            progress(Error::NO_FILE, nullptr, PROGRESS_READ_HEADER);
            return;
        }

        std::string rawfile_filename;
        Vec3Dll resolution;
        Vec3Df scale(1);
        std::size_t bpv = 1;

        std::string line;

        while (std::getline(datfile, line))
        {
            std::istringstream ss_line(line);
            std::string name;

            if (!std::getline(ss_line, name, ':'))
            {
                continue;
            }

            trim(name);

            qDebug() << "name:" << QString::fromStdString(name);

            if (name.compare("ObjectFileName") == 0)
            {
                ss_line >> rawfile_filename;
                trim(rawfile_filename);
                qDebug() << "rawfile_path:" << QString::fromStdString(rawfile_filename);
            }
            else if (name.compare("Resolution") == 0)
            {
                ss_line >> resolution.x() >> resolution.y() >> resolution.z();
                qDebug() << "resolution:" << resolution.x() << resolution.y() << resolution.z();
            }
            else if (name.compare("SliceThickness") == 0)
            {
                ss_line >> scale.x() >> scale.y() >> scale.z();
                qDebug() << "scale:" << scale.x() << scale.y() << scale.z();
            }
            else if (name.compare("Format") == 0)
            {
                std::string format_s;
                ss_line >> format_s;
                trim(format_s);
                qDebug() << "format_s:" << QString::fromStdString(format_s);
                if (format_s.compare("UCHAR") == 0)
                {
                    bpv = 1;
                }
                else if (format_s.compare("USHORT") == 0)
                {
                    bpv = 2;
                }
            }
        }

        datfile.close();

        // TODO: validate parameters

        QFileInfo rawfile_path(QString::fromStdString(path));

        std::ifstream rawfile;
        rawfile.open(
            rawfile_path
                .dir()
                .filePath(QString::fromStdString(rawfile_filename))
                .toStdString(),
            std::ios::in | std::ios::binary);

        qDebug() << "try open" << rawfile_path.dir().filePath(QString::fromStdString(rawfile_filename));

        if (!rawfile.is_open())
        {
            progress(Error::NO_FILE, nullptr, PROGRESS_READ_HEADER);
            return;
        }

        progress(Error::OK, nullptr, PROGRESS_READ_HEADER);

        Vec3Ds dim(resolution);

        auto scan = std::make_shared<TomographyScan>(dim, scale);

        double max_value = (1 << (8 * bpv)) - 1;

        for (std::size_t z = 0; z < scan->shape().z(); z++)
        {
            for (std::size_t y = 0; y < scan->shape().y(); y++)
            {
                for (std::size_t x = 0; x < scan->shape().x(); x++)
                {
                    unsigned char lo = 0;
                    unsigned char hi = 0;

                    rawfile.read(reinterpret_cast<char *>(&lo), 1);
                    if (bpv == 2)
                    {
                        rawfile.read(reinterpret_cast<char *>(&hi), 1);
                    }

                    double v = ((unsigned int)hi << 8) | lo;

                    scan->at(Vec3Ds(x, y, z)) = v / max_value;
                }
            }
            progress(
                Error::OK,
                nullptr,
                PROGRESS_READ_HEADER +
                    PROGRESS_READ_RAW * z / scan->shape().z());
        }

        rawfile.close();

        progress(Error::OK, scan, 1);
    }
} // namespace CGCP
