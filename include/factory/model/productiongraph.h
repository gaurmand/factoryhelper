#ifndef FACTORY_NODE_H
#define FACTORY_NODE_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <cstdint>
#include <factory/model/recipe.h>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <variant>

//===========================================================================
namespace factory
{

//===========================================================================
class ProductionGraph
{
public:
   struct Item
   {
      std::string name;
   };

   struct Recipe
   {
      std::string name;
      std::uint32_t duration;
   };

   struct RecipeComponent
   {
      std::uint32_t count;
      ComponentType type;
      std::reference_wrapper<const Item> item;
   };

private:
   using VertexProperties = std::variant<Item, Recipe>;

   struct EdgeProperties
   {
      std::uint32_t count;
   };

   using Graph = boost::adjacency_list<boost::vecS,
                                       boost::vecS,
                                       boost::bidirectionalS,
                                       VertexProperties,
                                       EdgeProperties>;

public:
   using Vertex     = Graph::vertex_descriptor;
   using VertexList = std::vector<Vertex>;

public:
   ProductionGraph() = default;
   ProductionGraph(const std::vector<RecipeInfo>& recipes);

   bool empty() const;

   std::optional<std::reference_wrapper<const Item>> getItem(Vertex v) const;
   std::optional<std::reference_wrapper<const Recipe>> getRecipe(
      Vertex v) const;
   std::vector<RecipeComponent> getRecipeComponents(Vertex v) const;

   enum class Reachability
   {
      Any,
      Source,
      Sink
   };

   template <typename VertexType>
   VertexList vertices(Reachability reachability = Reachability::Any) const;

   enum Distance : std::uint32_t
   {
      Adjacent = 1,
      Any      = std::numeric_limits<std::uint32_t>::max()
   };
   enum class Direction
   {
      Forward,
      Reverse
   };

   template <typename VertexType>
   VertexList reachableVertices(Vertex start,
                                Direction direction,
                                Distance distance = Distance::Any) const;

   template <typename VertexType>
   VertexList reachableVertices(VertexList startingVertices,
                                Direction direction) const;

private:
   Vertex addRecipeVertex(const RecipeInfo& recipe);
   using ItemMap = std::unordered_map<std::string_view,
                                      Vertex,
                                      transparent_string_hash,
                                      std::equal_to<>>;
   Vertex addItemVertex(std::string_view itemName, ItemMap& itemMap);
   Graph::edge_descriptor addRecipeComponent(Vertex from,
                                             Vertex to,
                                             std::uint32_t count);

private:
   Graph g_;
   mutable std::vector<Graph::vertices_size_type> distances_;
   mutable std::vector<boost::default_color_type> colours_;

   std::unordered_set<Vertex> recipes_;
};

} // namespace factory

#endif
