#pragma once
#include <vector>
#include <cstdint>
#include <unordered_map>
#include "Operators.hpp"
#include "Relation.hpp"
#include "Parser.hpp"
//---------------------------------------------------------------------------
using Disjoint_Set_Element = struct
{
  unsigned root_id;
  std::unique_ptr<Operator> tree;
};
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
