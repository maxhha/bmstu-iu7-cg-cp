#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "ContinuesFunction.h"
#include "Exception.h"

namespace CGCP
{
    class FunctionCreator
    {
    private:
        using FunctionPtr = std::shared_ptr<ContinuesFunction>;
        using Config = std::map<std::string, std::string>;
        using Type = std::string;
        using TypeFunc = std::function<FunctionPtr(const Config &, FunctionPtr)>;
        using TypeMap = std::map<Type, TypeFunc>;
        using TypeMapPtr = std::shared_ptr<TypeMap>;

        TypeMapPtr type_funcs_;
        FunctionPtr func_;

    public:
        FunctionCreator()
            : type_funcs_(std::make_shared<TypeMap>()){};

        FunctionCreator(TypeMapPtr type_funcs, FunctionPtr func)
            : type_funcs_(type_funcs),
              func_(func){};

        FunctionCreator &addType(const Type &type, TypeFunc creator)
        {
            type_funcs_->insert({type, creator});
            return *this;
        }

        FunctionCreator &set(FunctionPtr func)
        {
            func_ = func;
            return *this;
        }

        FunctionCreator add(const Type &type, const Config &config)
        {
            if (type_funcs_->count(type) == 0)
            {
                THROW_EXCEPTION("Unknown function type {}", type);
            }

            return FunctionCreator(
                type_funcs_,
                func_ ? type_funcs_->at(type)(config, func_) : func_);
        }

        FunctionPtr result()
        {
            return func_;
        };

        ~FunctionCreator() = default;
    };
} // namespace CGCP
