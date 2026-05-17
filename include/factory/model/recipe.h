#ifndef FACTORY_RECIPE_H
#define FACTORY_RECIPE_H

#include <cstdint>
#include <string>
#include <vector>

//===========================================================================
namespace factory
{

//===========================================================================
enum class ComponentType
{
   Input,
   Output
};

//===========================================================================
struct RecipeInfo
{
   std::string name;
   std::uint32_t duration;

   struct ComponentInfo
   {
      std::string name;
      ComponentType type;
      std::uint32_t count;
   };
   std::vector<ComponentInfo> components;

   bool operator==(const RecipeInfo&) const = default;
};

struct RecipeHash
{
   std::size_t operator()(const RecipeInfo& k) const
   {
      return std::hash<std::string>()(k.name);
   }
};

template <typename... Bases>
struct overload : Bases...
{
   using is_transparent = void;
   using Bases::operator()...;
};

struct char_pointer_hash
{
   auto operator()(const char* ptr) const noexcept
   {
      return std::hash<std::string_view>{}(ptr);
   }
};

using transparent_string_hash = overload<std::hash<std::string>,
                                         std::hash<std::string_view>,
                                         char_pointer_hash>;

} // namespace factory

#endif
