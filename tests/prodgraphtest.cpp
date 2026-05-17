#include <factory/model/productiongraph.h>
#include <factory/model/recipe.h>
#include <gtest/gtest.h>

namespace
{
using namespace std::literals;
using namespace factory;

const std::vector<RecipeInfo> kTestRecipes = {
   {.name     = "Copper Ingot (default)"s,
    .duration = 1,
    .components =
       {{.name = "Copper Ore"s, .type = ComponentType::Input, .count = 30},
        {.name = "Copper Ingot"s, .type = ComponentType::Output, .count = 30}}},
   {.name     = "Wire (default)"s,
    .duration = 1,
    .components =
       {{.name = "Copper Ingot"s, .type = ComponentType::Input, .count = 15},
        {.name = "Wire"s, .type = ComponentType::Output, .count = 30}}},
   {.name       = "Cable (default)"s,
    .duration   = 1,
    .components = {{.name = "Wire"s, .type = ComponentType::Input, .count = 60},
                   {.name  = "Cable"s,
                    .type  = ComponentType::Output,
                    .count = 30}}},
   {.name     = "Steel Ingot (default)"s,
    .duration = 1,
    .components =
       {{.name = "Iron Ore"s, .type = ComponentType::Input, .count = 45},
        {.name = "Coal"s, .type = ComponentType::Input, .count = 45},
        {.name = "Steel Ingot"s, .type = ComponentType::Output, .count = 45}}},
   {.name     = "Steel Pipe (default)"s,
    .duration = 1,
    .components =
       {{.name = "Steel Ingot"s, .type = ComponentType::Input, .count = 30},
        {.name = "Steel Pipe"s, .type = ComponentType::Output, .count = 20}}},
   {.name     = "Stator (default)"s,
    .duration = 1,
    .components =
       {{.name = "Steel Pipe"s, .type = ComponentType::Input, .count = 15},
        {.name = "Wire"s, .type = ComponentType::Input, .count = 40},
        {.name = "Stator"s, .type = ComponentType::Output, .count = 5}}},
   {.name       = "Automated Wiring (fictional)"s,
    .duration   = 2,
    .components = {
       {.name = "Stator"s, .type = ComponentType::Input, .count = 5},
       {.name = "Cable"s, .type = ComponentType::Input, .count = 100},
       {.name = "Fakium"s, .type = ComponentType::Input, .count = 1},
       {.name = "Automated Wiring"s, .type = ComponentType::Output, .count = 5},
       {.name = "Fake Thing"s, .type = ComponentType::Output, .count = 1}}}};

const std::vector<RecipeInfo> kTestReachableRecipes = {
   {.name       = "A Recipe"s,
    .duration   = 1,
    .components = {{.name = "A0"s, .type = ComponentType::Input, .count = 1},
                   {.name = "A1"s, .type = ComponentType::Output, .count = 1}}},
   {.name       = "B Recipe"s,
    .duration   = 1,
    .components = {{.name = "B0"s, .type = ComponentType::Input, .count = 1},
                   {.name = "B1"s, .type = ComponentType::Output, .count = 1}}},
   {.name       = "C Recipe"s,
    .duration   = 1,
    .components = {{.name = "C0"s, .type = ComponentType::Input, .count = 1},
                   {.name = "C1"s, .type = ComponentType::Output, .count = 1}}},
   {.name       = "D Recipe"s,
    .duration   = 1,
    .components = {{.name = "D0"s, .type = ComponentType::Input, .count = 1},
                   {.name = "D1"s, .type = ComponentType::Output, .count = 1}}},
   {.name       = "AB Recipe"s,
    .duration   = 1,
    .components = {{.name = "A1"s, .type = ComponentType::Input, .count = 1},
                   {.name = "B1"s, .type = ComponentType::Input, .count = 1},
                   {.name = "AB"s, .type = ComponentType::Output, .count = 1}}},
   {.name       = "ABC Recipe"s,
    .duration   = 1,
    .components = {{.name = "AB"s, .type = ComponentType::Input, .count = 1},
                   {.name = "C1"s, .type = ComponentType::Input, .count = 1},
                   {.name  = "ABC"s,
                    .type  = ComponentType::Output,
                    .count = 1}}},
   {.name       = "BC Recipe"s,
    .duration   = 1,
    .components = {{.name = "B1"s, .type = ComponentType::Input, .count = 1},
                   {.name = "C1"s, .type = ComponentType::Input, .count = 1},
                   {.name = "BC"s, .type = ComponentType::Output, .count = 1}}},
   {.name       = "D2 Recipe"s,
    .duration   = 1,
    .components = {
       {.name = "D1"s, .type = ComponentType::Input, .count = 1},
       {.name = "D2"s, .type = ComponentType::Output, .count = 1}}}};

} // namespace

TEST(ProductionGraph, defaultConstuctor)
{
   ProductionGraph graph;
   EXPECT_TRUE(graph.empty());
}

TEST(ProductionGraph, recipesConstructor)
{
   ProductionGraph graph(kTestRecipes);
   EXPECT_FALSE(graph.empty());
}

TEST(ProductionGraph, vertices)
{
   ProductionGraph graph(kTestRecipes);
   EXPECT_EQ(graph.vertices<ProductionGraph::Recipe>().size(), 7);
   EXPECT_EQ(graph.vertices<ProductionGraph::Item>().size(), 12);

   EXPECT_EQ(
      graph
         .vertices<ProductionGraph::Item>(ProductionGraph::Reachability::Source)
         .size(),
      4);
   EXPECT_EQ(
      graph.vertices<ProductionGraph::Item>(ProductionGraph::Reachability::Sink)
         .size(),
      2);

   EXPECT_EQ(graph
                .vertices<ProductionGraph::Recipe>(
                   ProductionGraph::Reachability::Source)
                .size(),
             0);
   EXPECT_EQ(
      graph
         .vertices<ProductionGraph::Recipe>(ProductionGraph::Reachability::Sink)
         .size(),
      0);
}

TEST(ProductionGraph, getAllItems)
{
   ProductionGraph graph(kTestRecipes);
   auto items = graph.vertices<ProductionGraph::Item>();
   ASSERT_EQ(items.size(), 12);

   std::optional<std::reference_wrapper<const ProductionGraph::Item>> item;

   item = graph.getItem(items[0]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Copper Ore"s);

   item = graph.getItem(items[1]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Copper Ingot"s);

   item = graph.getItem(items[2]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Wire"s);

   item = graph.getItem(items[3]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Cable"s);

   item = graph.getItem(items[4]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Iron Ore"s);

   item = graph.getItem(items[5]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Coal"s);

   item = graph.getItem(items[6]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Steel Ingot"s);

   item = graph.getItem(items[7]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Steel Pipe"s);

   item = graph.getItem(items[8]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Stator"s);

   item = graph.getItem(items[9]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Fakium"s);

   item = graph.getItem(items[10]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Automated Wiring"s);

   item = graph.getItem(items[11]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Fake Thing"s);
}

TEST(ProductionGraph, getSourceItems)
{
   ProductionGraph graph(kTestRecipes);
   auto items = graph.vertices<ProductionGraph::Item>(
      ProductionGraph::Reachability::Source);
   ASSERT_EQ(items.size(), 4);

   std::optional<std::reference_wrapper<const ProductionGraph::Item>> item;

   item = graph.getItem(items[0]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Copper Ore"s);

   item = graph.getItem(items[1]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Iron Ore"s);

   item = graph.getItem(items[2]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Coal"s);

   item = graph.getItem(items[3]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Fakium"s);
}

TEST(ProductionGraph, getSinkItems)
{
   ProductionGraph graph(kTestRecipes);
   auto items = graph.vertices<ProductionGraph::Item>(
      ProductionGraph::Reachability::Sink);
   ASSERT_EQ(items.size(), 2);

   std::optional<std::reference_wrapper<const ProductionGraph::Item>> item;

   item = graph.getItem(items[0]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Automated Wiring"s);

   item = graph.getItem(items[1]);
   ASSERT_TRUE(item.has_value());
   EXPECT_EQ(item.value().get().name, "Fake Thing"s);
}

TEST(ProductionGraph, getRecipes)
{
   ProductionGraph graph(kTestRecipes);
   auto recipes = graph.vertices<ProductionGraph::Recipe>();
   ASSERT_EQ(recipes.size(), 7);

   std::optional<std::reference_wrapper<const ProductionGraph::Recipe>> recipe;

   recipe = graph.getRecipe(recipes[0]);
   ASSERT_TRUE(recipe.has_value());
   EXPECT_EQ(recipe.value().get().name, "Copper Ingot (default)"s);
   EXPECT_EQ(recipe.value().get().duration, 1);

   recipe = graph.getRecipe(recipes[1]);
   ASSERT_TRUE(recipe.has_value());
   EXPECT_EQ(recipe.value().get().name, "Wire (default)"s);
   EXPECT_EQ(recipe.value().get().duration, 1);

   recipe = graph.getRecipe(recipes[2]);
   ASSERT_TRUE(recipe.has_value());
   EXPECT_EQ(recipe.value().get().name, "Cable (default)"s);
   EXPECT_EQ(recipe.value().get().duration, 1);

   recipe = graph.getRecipe(recipes[3]);
   ASSERT_TRUE(recipe.has_value());
   EXPECT_EQ(recipe.value().get().name, "Steel Ingot (default)"s);
   EXPECT_EQ(recipe.value().get().duration, 1);

   recipe = graph.getRecipe(recipes[4]);
   ASSERT_TRUE(recipe.has_value());
   EXPECT_EQ(recipe.value().get().name, "Steel Pipe (default)"s);
   EXPECT_EQ(recipe.value().get().duration, 1);

   recipe = graph.getRecipe(recipes[5]);
   ASSERT_TRUE(recipe.has_value());
   EXPECT_EQ(recipe.value().get().name, "Stator (default)"s);
   EXPECT_EQ(recipe.value().get().duration, 1);

   recipe = graph.getRecipe(recipes[6]);
   ASSERT_TRUE(recipe.has_value());
   EXPECT_EQ(recipe.value().get().name, "Automated Wiring (fictional)"s);
   EXPECT_EQ(recipe.value().get().duration, 2);
}

TEST(ProductionGraph, getRecipeComponents)
{
   ProductionGraph graph(kTestRecipes);
   auto recipes = graph.vertices<ProductionGraph::Recipe>();
   ASSERT_EQ(recipes.size(), 7);

   std::vector<ProductionGraph::RecipeComponent> components;

   components = graph.getRecipeComponents(recipes[0]);
   ASSERT_EQ(components.size(), 2);
   EXPECT_EQ(components[0].type, ComponentType::Input);
   EXPECT_EQ(components[0].item.get().name, "Copper Ore"s);
   EXPECT_EQ(components[0].count, 30);
   EXPECT_EQ(components[1].type, ComponentType::Output);
   EXPECT_EQ(components[1].item.get().name, "Copper Ingot"s);
   EXPECT_EQ(components[1].count, 30);

   components = graph.getRecipeComponents(recipes[1]);
   ASSERT_EQ(components.size(), 2);
   EXPECT_EQ(components[0].type, ComponentType::Input);
   EXPECT_EQ(components[0].item.get().name, "Copper Ingot"s);
   EXPECT_EQ(components[0].count, 15);
   EXPECT_EQ(components[1].type, ComponentType::Output);
   EXPECT_EQ(components[1].item.get().name, "Wire"s);
   EXPECT_EQ(components[1].count, 30);

   components = graph.getRecipeComponents(recipes[2]);
   ASSERT_EQ(components.size(), 2);
   EXPECT_EQ(components[0].type, ComponentType::Input);
   EXPECT_EQ(components[0].item.get().name, "Wire"s);
   EXPECT_EQ(components[0].count, 60);
   EXPECT_EQ(components[1].type, ComponentType::Output);
   EXPECT_EQ(components[1].item.get().name, "Cable"s);
   EXPECT_EQ(components[1].count, 30);

   components = graph.getRecipeComponents(recipes[3]);
   ASSERT_EQ(components.size(), 3);
   EXPECT_EQ(components[0].type, ComponentType::Input);
   EXPECT_EQ(components[0].item.get().name, "Iron Ore"s);
   EXPECT_EQ(components[0].count, 45);
   EXPECT_EQ(components[1].type, ComponentType::Input);
   EXPECT_EQ(components[1].item.get().name, "Coal"s);
   EXPECT_EQ(components[1].count, 45);
   EXPECT_EQ(components[2].type, ComponentType::Output);
   EXPECT_EQ(components[2].item.get().name, "Steel Ingot"s);
   EXPECT_EQ(components[2].count, 45);

   components = graph.getRecipeComponents(recipes[4]);
   ASSERT_EQ(components.size(), 2);
   EXPECT_EQ(components[0].type, ComponentType::Input);
   EXPECT_EQ(components[0].item.get().name, "Steel Ingot"s);
   EXPECT_EQ(components[0].count, 30);
   EXPECT_EQ(components[1].type, ComponentType::Output);
   EXPECT_EQ(components[1].item.get().name, "Steel Pipe"s);
   EXPECT_EQ(components[1].count, 20);

   components = graph.getRecipeComponents(recipes[5]);
   ASSERT_EQ(components.size(), 3);
   EXPECT_EQ(components[0].type, ComponentType::Input);
   EXPECT_EQ(components[0].item.get().name, "Steel Pipe"s);
   EXPECT_EQ(components[0].count, 15);
   EXPECT_EQ(components[1].type, ComponentType::Input);
   EXPECT_EQ(components[1].item.get().name, "Wire"s);
   EXPECT_EQ(components[1].count, 40);
   EXPECT_EQ(components[2].type, ComponentType::Output);
   EXPECT_EQ(components[2].item.get().name, "Stator"s);
   EXPECT_EQ(components[2].count, 5);

   components = graph.getRecipeComponents(recipes[6]);
   ASSERT_EQ(components.size(), 5);
   EXPECT_EQ(components[0].type, ComponentType::Input);
   EXPECT_EQ(components[0].item.get().name, "Stator"s);
   EXPECT_EQ(components[0].count, 5);
   EXPECT_EQ(components[1].type, ComponentType::Input);
   EXPECT_EQ(components[1].item.get().name, "Cable"s);
   EXPECT_EQ(components[1].count, 100);
   EXPECT_EQ(components[2].type, ComponentType::Input);
   EXPECT_EQ(components[2].item.get().name, "Fakium"s);
   EXPECT_EQ(components[2].count, 1);
   EXPECT_EQ(components[3].type, ComponentType::Output);
   EXPECT_EQ(components[3].item.get().name, "Automated Wiring"s);
   EXPECT_EQ(components[3].count, 5);
   EXPECT_EQ(components[4].type, ComponentType::Output);
   EXPECT_EQ(components[4].item.get().name, "Fake Thing"s);
   EXPECT_EQ(components[4].count, 1);
}

TEST(ProductionGraph, getWrongVertexType)
{
   ProductionGraph graph(kTestRecipes);
   auto items   = graph.vertices<ProductionGraph::Item>();
   auto recipes = graph.vertices<ProductionGraph::Recipe>();

   ASSERT_FALSE(items.empty());
   ASSERT_FALSE(recipes.empty());

   EXPECT_FALSE(graph.getItem(recipes[0]).has_value());
   EXPECT_FALSE(graph.getRecipe(items[0]).has_value());
   EXPECT_TRUE(graph.getRecipeComponents(items[0]).empty());
}

TEST(ProductionGraph, reachableVertices)
{
   ProductionGraph graph(kTestReachableRecipes);
   auto items   = graph.vertices<ProductionGraph::Item>();
   auto recipes = graph.vertices<ProductionGraph::Recipe>();

   ASSERT_EQ(items.size(), 12);
   ASSERT_EQ(recipes.size(), 8);

   ProductionGraph::VertexList vertices;

   {
      // A0
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[0], ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 3);

      // A1
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[1], ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 2);

      // B0
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[2], ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 4);

      // B1
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[3], ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 3);

      // C0
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[4], ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 3);

      // C1
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[5], ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 2);

      // D0
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[6], ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 2);

      // D1
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[7], ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 1);

      // AB
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[8], ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 1);

      // ABC
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[9], ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 0);

      // BC
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[10], ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 0);

      // D2
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[11], ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 0);
   }

   {
      // A0
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[0], ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 0);

      // A1
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[1], ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 1);

      // B0
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[2], ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 0);

      // B1
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[3], ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 1);

      // C0
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[4], ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 0);

      // C1
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[5], ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 1);

      // D0
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[6], ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 0);

      // D1
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[7], ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 1);

      // AB
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[8], ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 4);

      // ABC
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[9], ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 7);

      // BC
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[10], ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 4);

      // D2
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[11], ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 2);
   }

   {
      // A1 Recipe
      vertices = graph.reachableVertices<ProductionGraph::Recipe>(
         recipes[0], ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 2);

      vertices = graph.reachableVertices<ProductionGraph::Recipe>(
         recipes[0], ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 0);

      // AB Recipe
      vertices = graph.reachableVertices<ProductionGraph::Recipe>(
         recipes[4], ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 1);

      vertices = graph.reachableVertices<ProductionGraph::Recipe>(
         recipes[4], ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 2);
   }
}

TEST(ProductionGraph, reachableVerticesDistance)
{
   ProductionGraph graph(kTestReachableRecipes);
   auto items   = graph.vertices<ProductionGraph::Item>();
   auto recipes = graph.vertices<ProductionGraph::Recipe>();

   ASSERT_EQ(items.size(), 12);
   ASSERT_EQ(recipes.size(), 8);

   ProductionGraph::VertexList vertices;

   {
      // B0
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[2],
         ProductionGraph::Direction::Forward,
         ProductionGraph::Distance{0});
      EXPECT_EQ(vertices.size(), 0);

      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[2],
         ProductionGraph::Direction::Forward,
         ProductionGraph::Distance{1});
      EXPECT_EQ(vertices.size(), 0);

      vertices = graph.reachableVertices<ProductionGraph::Recipe>(
         items[2],
         ProductionGraph::Direction::Forward,
         ProductionGraph::Distance{1});
      EXPECT_EQ(vertices.size(), 1);

      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[2],
         ProductionGraph::Direction::Forward,
         ProductionGraph::Distance{2});
      EXPECT_EQ(vertices.size(), 1);

      vertices = graph.reachableVertices<ProductionGraph::Recipe>(
         items[2],
         ProductionGraph::Direction::Forward,
         ProductionGraph::Distance{2});
      EXPECT_EQ(vertices.size(), 0);

      vertices = graph.reachableVertices<ProductionGraph::Recipe>(
         items[2],
         ProductionGraph::Direction::Forward,
         ProductionGraph::Distance{3});
      EXPECT_EQ(vertices.size(), 2);

      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[2],
         ProductionGraph::Direction::Forward,
         ProductionGraph::Distance{4});
      EXPECT_EQ(vertices.size(), 2);

      vertices = graph.reachableVertices<ProductionGraph::Recipe>(
         items[2],
         ProductionGraph::Direction::Forward,
         ProductionGraph::Distance{5});
      EXPECT_EQ(vertices.size(), 1);

      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[2],
         ProductionGraph::Direction::Forward,
         ProductionGraph::Distance{6});
      EXPECT_EQ(vertices.size(), 1);
   }

   {
      // ABC
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[9],
         ProductionGraph::Direction::Reverse,
         ProductionGraph::Distance{0});
      EXPECT_EQ(vertices.size(), 0);

      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[9],
         ProductionGraph::Direction::Reverse,
         ProductionGraph::Distance{1});
      EXPECT_EQ(vertices.size(), 0);

      vertices = graph.reachableVertices<ProductionGraph::Recipe>(
         items[9],
         ProductionGraph::Direction::Reverse,
         ProductionGraph::Distance{1});
      EXPECT_EQ(vertices.size(), 1);

      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[9],
         ProductionGraph::Direction::Reverse,
         ProductionGraph::Distance{2});
      EXPECT_EQ(vertices.size(), 2);

      vertices = graph.reachableVertices<ProductionGraph::Recipe>(
         items[9],
         ProductionGraph::Direction::Reverse,
         ProductionGraph::Distance{2});
      EXPECT_EQ(vertices.size(), 0);

      vertices = graph.reachableVertices<ProductionGraph::Recipe>(
         items[9],
         ProductionGraph::Direction::Reverse,
         ProductionGraph::Distance{3});
      EXPECT_EQ(vertices.size(), 2);

      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[9],
         ProductionGraph::Direction::Reverse,
         ProductionGraph::Distance{4});
      EXPECT_EQ(vertices.size(), 3);

      vertices = graph.reachableVertices<ProductionGraph::Recipe>(
         items[9],
         ProductionGraph::Direction::Reverse,
         ProductionGraph::Distance{5});
      EXPECT_EQ(vertices.size(), 2);

      vertices = graph.reachableVertices<ProductionGraph::Item>(
         items[9],
         ProductionGraph::Direction::Reverse,
         ProductionGraph::Distance{6});
      EXPECT_EQ(vertices.size(), 2);
   }
}

TEST(ProductionGraph, reachableVerticesMultipleStarts)
{
   ProductionGraph graph(kTestReachableRecipes);
   auto items   = graph.vertices<ProductionGraph::Item>();
   auto recipes = graph.vertices<ProductionGraph::Recipe>();

   ASSERT_EQ(items.size(), 12);
   ASSERT_EQ(recipes.size(), 8);

   ProductionGraph::VertexList vertices;

   {
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         ProductionGraph::VertexList{}, ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 0);

      // A0, B0
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         {items[0], items[2]}, ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 7);

      // B0, C0
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         {items[2], items[4]}, ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 7);

      // A0, C0
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         {items[0], items[4]}, ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 7);

      // A0, B0, C0
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         {items[0], items[2], items[4]}, ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 9);

      // A0, B0, C0, D0
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         {items[0], items[2], items[4], items[6]},
         ProductionGraph::Direction::Forward);
      EXPECT_EQ(vertices.size(), 12);
   }

   {
      // A1, B1
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         {items[1], items[3]}, ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 4);

      // B1, C1
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         {items[3], items[5]}, ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 4);

      // AB, ABC
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         {items[8], items[9]}, ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 8);

      // AB, ABC, BC
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         {items[8], items[9], items[10]}, ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 9);

      // AB, ABC, BC, A0, B0
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         {items[8], items[9], items[10], items[0], items[2]},
         ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 9);

      // ABC, BC, D2
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         {items[9], items[10], items[11]}, ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 12);

      // ABC, BC, D2, B1, D1
      vertices = graph.reachableVertices<ProductionGraph::Item>(
         {items[9], items[10], items[11], items[3], items[7]},
         ProductionGraph::Direction::Reverse);
      EXPECT_EQ(vertices.size(), 12);
   }
}
