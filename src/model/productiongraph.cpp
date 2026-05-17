#include "factory/model/recipe.h"

#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/visitors.hpp>
#include <cassert>
#include <factory/model/productiongraph.h>
#include <variant>

// helper type for the visitor #4
template <class... Ts>
struct overloaded : Ts...
{
   using Ts::operator()...;
};

//===========================================================================
namespace factory
{

//===========================================================================
ProductionGraph::ProductionGraph(const std::vector<RecipeInfo>& recipes)
{
   ItemMap addedItems;

   for (const auto& recipe : recipes)
   {
      const auto recipeVertex = addRecipeVertex(recipe);
      recipes_.emplace(recipeVertex);

      for (const auto& componentInfo : recipe.components)
      {
         if (componentInfo.type == ComponentType::Input)
         {
            const auto inputVertex =
               addItemVertex(componentInfo.name, addedItems);
            addRecipeComponent(inputVertex, recipeVertex, componentInfo.count);
         }
         else // componentInfo.type == ComponentType::Output
         {
            const auto outputVertex =
               addItemVertex(componentInfo.name, addedItems);
            addRecipeComponent(recipeVertex, outputVertex, componentInfo.count);
         }
      }
   }

   distances_ =
      std::vector<Graph::vertices_size_type>(boost::num_vertices(g_), 0);
   colours_ = std::vector<boost::default_color_type>(boost::num_vertices(g_),
                                                     boost::white_color);
}

//===========================================================================
ProductionGraph::Vertex ProductionGraph::addRecipeVertex(
   const RecipeInfo& recipe)
{
   return boost::add_vertex(
      VertexProperties{
         Recipe{.name = recipe.name, .duration = recipe.duration}},
      g_);
}

//===========================================================================
ProductionGraph::Vertex ProductionGraph::addItemVertex(std::string_view name,
                                                       ItemMap& itemMap)
{
   if (auto it = itemMap.find(name); it != itemMap.end())
   {
      return it->second;
   }

   auto itemVertex =
      boost::add_vertex(VertexProperties{Item{.name = std::string{name}}}, g_);

   itemMap.emplace(name, itemVertex);

   return itemVertex;
}

//===========================================================================
ProductionGraph::Graph::edge_descriptor ProductionGraph::addRecipeComponent(
   Vertex from, Vertex to, std::uint32_t count)
{
   return boost::add_edge(from, to, EdgeProperties{.count = count}, g_).first;
}

//===========================================================================
bool ProductionGraph::empty() const
{
   return boost::num_vertices(g_) == 0;
}

//===========================================================================
std::optional<std::reference_wrapper<const ProductionGraph::Item>>
ProductionGraph::getItem(Vertex v) const
{
   using RetType =
      std::optional<std::reference_wrapper<const ProductionGraph::Item>>;

   return std::visit(overloaded{[](const Item& item) -> RetType {
      return std::ref(item);
   }, [](const Recipe&) -> RetType { return std::nullopt; }},
                     g_[v]);
}

//===========================================================================
std::optional<std::reference_wrapper<const ProductionGraph::Recipe>>
ProductionGraph::getRecipe(Vertex v) const
{

   using RetType =
      std::optional<std::reference_wrapper<const ProductionGraph::Recipe>>;

   return std::visit(overloaded{[](const Item&) -> RetType {
      return std::nullopt;
   }, [](const Recipe& recipe) -> RetType { return std::ref(recipe); }},
                     g_[v]);
}

//===========================================================================
std::vector<ProductionGraph::RecipeComponent> ProductionGraph::
   getRecipeComponents(Vertex v) const
{
   std::vector<RecipeComponent> components;

   if (!std::holds_alternative<Recipe>(g_[v]))
   {
      return components;
   }

   for (auto [ei, end] = boost::in_edges(v, g_); ei != end; ++ei)
   {
      auto e           = *ei;
      auto src         = boost::source(e, g_);
      const Item& item = std::get<Item>(g_[src]);

      components.emplace_back(RecipeComponent{.count = g_[e].count,
                                              .type  = ComponentType::Input,
                                              .item  = std::ref(item)});
   }

   for (auto [ei, end] = boost::out_edges(v, g_); ei != end; ++ei)
   {
      auto e           = *ei;
      auto tgt         = boost::target(e, g_);
      const Item& item = std::get<Item>(g_[tgt]);

      components.emplace_back(RecipeComponent{.count = g_[e].count,
                                              .type  = ComponentType::Output,
                                              .item  = std::ref(item)});
   }

   return components;
}

//===========================================================================
template <typename VertexType>
ProductionGraph::VertexList ProductionGraph::vertices(
   Reachability reachability) const
{
   VertexList vertices;

   for (auto [vi, end] = boost::vertices(g_); vi != end; ++vi)
   {
      const auto v = *vi;

      if (!std::holds_alternative<VertexType>(g_[v]))
      {
         continue;
      }

      bool includeVertex = true;
      if (reachability == Reachability::Source)
      {
         includeVertex &= boost::in_degree(v, g_) == 0;
      }
      else if (reachability == Reachability::Sink)
      {
         includeVertex &= boost::out_degree(v, g_) == 0;
      }

      if (includeVertex)
      {
         vertices.push_back(v);
      }
   }

   return vertices;
}

//===========================================================================
template <typename VertexType>
ProductionGraph::VertexList ProductionGraph::reachableVertices(
   Vertex start, Direction direction, Distance distance) const
{
   VertexList vertices;

   // TODO: Include start vertex in result (to be consistent with
   // reachableVertices(startingVertices))
   if (distance == 0)
   {
      return vertices;
   }

   std::ranges::fill(distances_, 0);

   // TODO: Make a better visitor that terminates early when the given distance
   // is exceeded.
   auto visitor = boost::make_bfs_visitor(
      boost::record_distances(distances_.data(), boost::on_tree_edge()));

   if (direction == Direction::Forward)
   {
      boost::breadth_first_search(g_, start, boost::visitor(visitor));
   }
   else // direction == Direction::Reverse
   {
      boost::breadth_first_search(
         boost::make_reverse_graph(g_), start, boost::visitor(visitor));
   }

   for (auto [vi, end] = boost::vertices(g_); vi != end; ++vi)
   {
      const auto v = *vi;
      if (std::holds_alternative<VertexType>(g_[v]) &&
          (distance == Distance::Any ? distances_[v] != 0 :
                                       distances_[v] == distance))
      {
         vertices.push_back(v);
      }
   }

   return vertices;
}

//===========================================================================
template <typename VertexType>
ProductionGraph::VertexList ProductionGraph::reachableVertices(
   VertexList startingVertices, Direction direction) const
{
   VertexList vertices;

   if (startingVertices.empty())
   {
      return vertices;
   }

   // breadth_first_visit() requires us to make all verticec as WHITE.
   std::ranges::fill(colours_, boost::white_color);

   // We colour all starting vertices GRAY and add to initial queue.
   boost::queue<Vertex> queue;
   for (Vertex v : startingVertices)
   {
      colours_[v] = boost::gray_color;
      queue.push(v);
   }

   auto start = startingVertices[0]; // arbitrary start vertex

   if (direction == Direction::Forward)
   {
      boost::breadth_first_visit(
         g_, start, queue, boost::default_bfs_visitor{}, colours_.data());
   }
   else // direction == Direction::Reverse
   {
      boost::breadth_first_visit(boost::make_reverse_graph(g_),
                                 start,
                                 queue,
                                 boost::default_bfs_visitor{},
                                 colours_.data());
   }

   for (auto [vi, end] = boost::vertices(g_); vi != end; ++vi)
   {

      const auto v = *vi;
      if (std::holds_alternative<VertexType>(g_[v]) &&
          colours_[v] == boost::black_color)
      {
         vertices.push_back(v);
      }
   }

   return vertices;
}

//===========================================================================
template ProductionGraph::VertexList ProductionGraph::vertices<
   ProductionGraph::Recipe>(ProductionGraph::Reachability) const;
template ProductionGraph::VertexList ProductionGraph::vertices<
   ProductionGraph::Item>(ProductionGraph::Reachability) const;

//===========================================================================
template ProductionGraph::VertexList ProductionGraph::reachableVertices<
   ProductionGraph::Recipe>(Vertex, Direction, Distance) const;
template ProductionGraph::VertexList ProductionGraph::reachableVertices<
   ProductionGraph::Item>(Vertex, Direction, Distance) const;

//===========================================================================
template ProductionGraph::VertexList ProductionGraph::reachableVertices<
   ProductionGraph::Recipe>(VertexList, Direction) const;
template ProductionGraph::VertexList ProductionGraph::reachableVertices<
   ProductionGraph::Item>(VertexList, Direction) const;

} // namespace factory
