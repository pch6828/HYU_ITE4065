#pragma once
#include <vector>
#include <cstdint>
#include <unordered_map>
#include "Operators.hpp"
#include "Relation.hpp"
#include "Parser.hpp"
//---------------------------------------------------------------------------
/// each join tree node has its own Disjoint_Set_Element
using Disjoint_Set_Element = struct
{
  unsigned root_id;               // root's number of subtree which given node belongs.
  std::unique_ptr<Operator> tree; // Operator subtree which root is given node.
};
/// Disjoint Set type built by hash table
using DisjointSet = std::unordered_map<unsigned, Disjoint_Set_Element>;
//---------------------------------------------------------------------------
class Joiner
{
  /// Add scan to query
  void addScan(DisjointSet &disjoint_set, SelectInfo &info, QueryInfo &query);

public:
  /// The relations that might be joined
  std::vector<Relation> relations;
  /// Add relation
  void addRelation(const char *fileName);
  /// Get relation
  Relation &getRelation(unsigned id);
  /// Joins a given set of relations
  std::string join(QueryInfo &i);
};
//---------------------------------------------------------------------------
