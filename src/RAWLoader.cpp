#include "RAWLoader.h"
#include <fstream>
#include <iostream>

namespace CGCP
{
    void RAWLoader::threadLoad(std::string path, ProgressCallback progress)
    {
        // std::ifstream datfile;
        // datfile.open(path);

        // if (!datfile.is_open())
        // {
        //     progress(Error::NO_FILE, nullptr, 0);
        //     return;
        // }

        // datfile.close();

        scan_ = std::make_unique<TomographyScan>(Vec3Ds(10, 10, 10));
        for (int x = 0; x < 10; x++)
        {
            for (int y = 0; y < 10; y++)
            {
                for (int z = 0; z < 10; z++)
                {
                    double xr = x - 5;
                    double yr = y - 5;
                    double zr = z - 5;
                    scan_->at(Vec3Ds(x, y, z)) = xr * xr + yr * yr + zr * zr - 16;
                }
            }
        }

        progress(Error::OK, buildFunction(), 1);
    }
} // namespace CGCP
