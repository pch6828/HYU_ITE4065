#include "Joiner.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <set>
#include <sstream>
#include <vector>
#include "Parser.hpp"
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
void Joiner::addRelation(const char *fileName)
// Loads a relation from disk
{
  relations.emplace_back(fileName);
}
//---------------------------------------------------------------------------
Relation &Joiner::getRelation(unsigned relationId)
// Loads a relation from disk
{
  if (relationId >= relations.size())
  {
    cerr << "Relation with id: " << relationId << " does not exist" << endl;
    throw;
  }
  return relations[relationId];
}
//---------------------------------------------------------------------------
void Joiner::addScan(DisjointSet &disjoint_set, SelectInfo &info, QueryInfo &query)
// Add scan to query
// and add selected relation into disjoint set
// At first, scan operator instance made in this function is treated as new join tree.
{
  vector<FilterInfo> filters;
  // find filters that are applied to selected relation
  for (auto &f : query.filters)
  {
    if (f.filterColumn.binding == info.binding)
    {
      filters.emplace_back(f);
    }
  }

  // add selected relation into disjoint set
  // set root id as info.binding
  disjoint_set[info.binding].root_id = info.binding;
  // make scan operator instance and add it into disjoint set
  disjoint_set[info.binding].tree = filters.size() ? make_unique<FilterScan>(getRelation(info.relId), filters) : make_unique<Scan>(getRelation(info.relId), info.binding);
}
//---------------------------------------------------------------------------
static unsigned getRootOfJoinTree(DisjointSet &disjoint_set, unsigned selected_id)
// using disjoint set structure
// find root id of join tree
// for optimization, this function apply path compression
// because of this path compression, time complexity of next call of this function will be O(C)
// if given selected_id is in the same tree of previous call.
{
  // parent is root id of given relation's join tree
  unsigned parent = disjoint_set[selected_id].root_id;

  // if selected relation is root of join tree it belongs to,
  // return its id
  if (parent == selected_id)
  {
    return parent;
  }

  // if selected relation is not root id, then find the root of its parent
  // also, apply path compression at this step for optimization
  return disjoint_set[selected_id].root_id = getRootOfJoinTree(disjoint_set, parent);
}
//---------------------------------------------------------------------------
static unsigned unionTwoJoinTrees(DisjointSet &disjoint_set, PredicateInfo &pInfo, unsigned left_id, unsigned right_id)
// using disjoint set structure
// connect two separate join trees into single join tree
// and return root id of created join tree
{
  unsigned root;

  // find root id of each join trees.
  unsigned left_root = getRootOfJoinTree(disjoint_set, left_id);
  unsigned right_root = getRootOfJoinTree(disjoint_set, right_id);

  // get each join trees by using its root id;
  auto left_tree = move(disjoint_set[left_root].tree);
  auto right_tree = move(disjoint_set[right_root].tree);

  // root id for new join tree is right root id
  root = right_root;
  // set left tree's root id to right root id;
  disjoint_set[left_root].root_id = root;
  // create new join tree
  disjoint_set[right_root].tree = make_unique<Join>(move(left_tree), move(right_tree), pInfo);

  return root;
}
//---------------------------------------------------------------------------
enum QueryGraphProvides
{
  AddScanLeft,     // Disjoint Set does not have node of given leftInfo
                   // And these nodes need connection
  AddScanRight,    // Disjoint Set does not have node of given rightInfo
                   // And these nodes need connection
  AddScanBoth,     // Disjoint Set does not have nodes of given leftInfo and rightInfo
                   // And these nodes need connection
  NeedConnect,     // Both Nodes are already in disjoint set
                   // But these nodes need connection
  AlreadyConnected // Both Nodes are already in disjoint set
                   // And these nodes are already connected
};
//---------------------------------------------------------------------------
static QueryGraphProvides analyzeInputOfJoin(DisjointSet &disjoint_set, SelectInfo &leftInfo, SelectInfo &rightInfo)
// Analyzes inputs of join
{
  bool usedLeft = disjoint_set.count(leftInfo.binding);
  bool usedRight = disjoint_set.count(rightInfo.binding);

  if (usedLeft ^ usedRight)
    return usedLeft ? QueryGraphProvides::AddScanRight : QueryGraphProvides::AddScanLeft;
  if (!(usedLeft && usedRight))
    return QueryGraphProvides::AddScanBoth;

  unsigned left_root = getRootOfJoinTree(disjoint_set, leftInfo.binding);
  unsigned right_root = getRootOfJoinTree(disjoint_set, rightInfo.binding);
  return left_root == right_root ? QueryGraphProvides::AlreadyConnected : QueryGraphProvides::NeedConnect;
}
//---------------------------------------------------------------------------
string Joiner::join(QueryInfo &query)
// Executes a join query
{
  //cerr << query.dumpText() << endl;
  DisjointSet disjoint_set;

  unsigned root;

  for (unsigned i = 0; i < query.predicates.size(); ++i)
  {
    auto &pInfo = query.predicates[i];
    auto &leftInfo = pInfo.left;
    auto &rightInfo = pInfo.right;
    unique_ptr<Operator> left, right;
    switch (analyzeInputOfJoin(disjoint_set, leftInfo, rightInfo))
    {
    case QueryGraphProvides::AddScanLeft:
      // add left relation into disjoint set
      addScan(disjoint_set, leftInfo, query);

      // make new join tree by connecting left and right join tree
      root = unionTwoJoinTrees(disjoint_set, pInfo, leftInfo.binding, rightInfo.binding);
      break;
    case QueryGraphProvides::AddScanRight:
      // add right relation into disjoint set
      addScan(disjoint_set, rightInfo, query);

      // make new join tree by connecting left and right join tree
      root = unionTwoJoinTrees(disjoint_set, pInfo, leftInfo.binding, rightInfo.binding);
      break;
    case QueryGraphProvides::AddScanBoth:
      // add both(left and right) relation into disjoint set
      addScan(disjoint_set, leftInfo, query);
      addScan(disjoint_set, rightInfo, query);

      // make new join tree by connecting left and right join tree
      root = unionTwoJoinTrees(disjoint_set, pInfo, leftInfo.binding, rightInfo.binding);
      break;
    case QueryGraphProvides::NeedConnect:
      // just connect left and right join tree to make new join tree
      // because given input's relations are already used in other join trees
      root = unionTwoJoinTrees(disjoint_set, pInfo, leftInfo.binding, rightInfo.binding);
      break;
    case QueryGraphProvides::AlreadyConnected:
      // All relations of this join are already belongs to same join tree.
      // Thus, we have either a cycle in our join graph or more than one join predicate per join.
      root = getRootOfJoinTree(disjoint_set, leftInfo.binding);
      auto temp = move(disjoint_set[root].tree);
      disjoint_set[root].tree = make_unique<SelfJoin>(move(temp), pInfo);
      break;
    };
  }

  Checksum checkSum(move(disjoint_set[root].tree), query.selections);
  checkSum.run();
  stringstream out;
  auto &results = checkSum.checkSums;
  for (unsigned i = 0; i < results.size(); ++i)
  {
    out << (checkSum.resultSize == 0 ? "NULL" : to_string(results[i]));
    if (i < results.size() - 1)
      out << " ";
  }
  out << "\n";
  return out.str();
}
//---------------------------------------------------------------------------
