#pragma once

#include <functional>
#include <memory>

#include "CFLoader.h"
#include "MeshDrawer.h"
#include "Polygonizer.h"
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

    using CFLoaderSolution = Solution<CFLoader>;
    using DrawerSolution = Solution<MeshDrawer>;
    using PolygonizerSolution = Solution<Polygonizer>;
} // namespace CGCP
