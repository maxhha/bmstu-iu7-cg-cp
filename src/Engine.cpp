#include "Engine.h"

namespace CGCP
{
    using Config = std::map<std::string, std::string>;
    using ScanPtr = std::shared_ptr<CGCP::TomographyScan>;
    using Type = std::string;
    using ProgressCallback = std::function<void(Error, ScanPtr, double progress)>;
    using TypeProgressCallback = std::function<bool(Error, double progress)>;
    using TypeFunc = std::function<ScanPtr(const Config &, ScanPtr, TypeProgressCallback)>;

    ScanPtr averageTomographyPreprocess(
        const Config &config,
        ScanPtr s,
        TypeProgressCallback progress_receiver)
    {
        int size = std::stoi(config.at("size"));
        auto shape = s->shape();

        auto s2 = std::make_shared<TomographyScan>(shape, s->scale());

        if (progress_receiver(Error::OK, 0))
        {
            return nullptr;
        }

        for (long long z = 0; z < shape.z(); ++z)
        {
            for (long long y = 0; y < shape.y(); ++y)
            {
                for (long long x = 0; x < shape.x(); ++x)
                {
                    double sum = 0;
                    int n = 0;

                    long long x1 = std::max<long long>({x - size, 0});
                    long long x2 = std::min<long long>({x + size, (long long)shape.x() - 1});
                    long long y1 = std::max<long long>({y - size, 0});
                    long long y2 = std::min<long long>({y + size, (long long)shape.y() - 1});
                    long long z1 = std::max<long long>({z - size, 0});
                    long long z2 = std::min<long long>({z + size, (long long)shape.z() - 1});

                    for (long long z_ = z1; z_ <= z2; ++z_)
                    {
                        for (long long y_ = y1; y_ <= y2; ++y_)
                        {
                            for (long long x_ = x1; x_ <= x2; ++x_)
                            {
                                sum += s->at(x_, y_, z_);
                                ++n;
                            }
                        }
                    }

                    s2->at(x, y, z) = sum / n;
                }
            }

            if (progress_receiver(Error::OK, (double)z / shape.z()))
            {
                return nullptr;
            }
        }

        return s2;
    }

    ScanPtr offsetTomographyPreprocess(
        const Config &config,
        ScanPtr s,
        TypeProgressCallback progress_receiver)
    {
        double offset = std::stod(config.at("value"));

        if (progress_receiver(Error::OK, 0))
        {
            return nullptr;
        }

        for (std::size_t z = 0; z < s->shape().z(); ++z)
        {
            for (std::size_t y = 0; y < s->shape().y(); ++y)
            {
                for (std::size_t x = 0; x < s->shape().x(); ++x)
                {
                    s->at(x, y, z) += offset;
                }
            }

            if (progress_receiver(Error::OK, (double)z / s->shape().z()))
            {
                return nullptr;
            }
        }

        return s;
    }

    Engine::Engine()
        : loader_(std::make_unique<CGCP::TomographyLoaderSolution>()),
          drawer_(std::make_unique<CGCP::DrawerSolution>()),
          polygonizer_(std::make_unique<CGCP::PolygonizerSolution>()),
          preprocessor_(std::make_unique<CGCP::TomographyPreprocessor>())
    {
        preprocessor()
            .add("average", averageTomographyPreprocess)
            .add("offset", offsetTomographyPreprocess);
    };
} // namespace CGCP
