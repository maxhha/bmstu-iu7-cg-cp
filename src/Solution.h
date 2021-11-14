#pragma once

#include <functional>
#include <memory>

#include "MeshDrawer.h"
#include "MeshSaver.h"
#include "Polygonizer.h"
#include "TomographyLoader.h"

namespace CGCP
{
    template <class T>
    class Solution
    {
        using TPtr = std::unique_ptr<T>;
        using Creator = std::function<TPtr(void)>;

    private:
        std::map<std::string, TPtr> items_;
        std::map<std::string, Creator> creators_;

    public:
        Solution &add(std::string key, Creator creator)
        {
            creators_[key] = creator;
            return *this;
        };

        T &get(std::string key)
        {
            if (items_.count(key) == 0)
            {
                items_[key] = creators_[key]();
            }

            return *items_[key];
        };

        ~Solution() = default;
    };

    using TomographyLoaderSolution = Solution<TomographyLoader>;
    using DrawerSolution = Solution<MeshDrawer>;
    using PolygonizerSolution = Solution<Polygonizer>;
    using MeshSaverSolution = Solution<MeshSaver>;
} // namespace CGCP
