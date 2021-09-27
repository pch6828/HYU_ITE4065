#include "Operators.hpp"
#include "ConcurrentQueue.hpp"
#include <cassert>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <pthread.h>
#include <stdio.h>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
template <typename Function>
void *thread_func(void *arg)
// function template of thread function
// by using arg, which should be lambda function, it runs given function
// then it returns given function's return value
{
  Function *f = (Function *)arg;
  void *ret = (void *)(*f)();

  pthread_exit(ret);
}
//---------------------------------------------------------------------------
bool Scan::require(SelectInfo info)
// Require a column and add it to results
{
  if (info.binding != relationBinding)
    return false;
  assert(info.colId < relation.columns.size());
  resultColumns.push_back(relation.columns[info.colId]);
  select2ResultColId[info] = resultColumns.size() - 1;
  return true;
}
//---------------------------------------------------------------------------
void Scan::run()
// Run
{
  // Nothing to do
  resultSize = relation.size;

  for (uint64_t i = 0; i < resultSize; i++)
  {
    for (auto &iter : partitions)
    {
      const SelectInfo &info = iter.first;
      Partition &partition = iter.second;
      uint64_t colValue = resultColumns[select2ResultColId[info]][i];
      partition[colValue % PARTITION_SIZE].emplace_back(i);
    }
  }
}
//---------------------------------------------------------------------------
vector<uint64_t *> Scan::getResults()
// Get materialized results
{
  return resultColumns;
}
//---------------------------------------------------------------------------
bool FilterScan::require(SelectInfo info)
// Require a column and add it to results
{
  if (info.binding != relationBinding)
    return false;
  assert(info.colId < relation.columns.size());
  if (select2ResultColId.find(info) == select2ResultColId.end())
  {
    // Add to results
    inputData.push_back(relation.columns[info.colId]);
    tmpResults.emplace_back();
    unsigned colId = tmpResults.size() - 1;
    select2ResultColId[info] = colId;
  }
  return true;
}
//---------------------------------------------------------------------------
void FilterScan::copy2Result(uint64_t id)
// Copy to result
{
  for (unsigned cId = 0; cId < inputData.size(); ++cId)
    tmpResults[cId].push_back(inputData[cId][id]);
  for (auto &iter : partitions)
  {
    const SelectInfo &info = iter.first;
    Partition &partition = iter.second;
    uint64_t colValue = tmpResults[select2ResultColId[info]][resultSize];
    partition[colValue % PARTITION_SIZE].emplace_back(resultSize);
  }
  ++resultSize;
}
//---------------------------------------------------------------------------
bool FilterScan::applyFilter(uint64_t i, FilterInfo &f)
// Apply filter
{
  auto compareCol = relation.columns[f.filterColumn.colId];
  auto constant = f.constant;
  switch (f.comparison)
  {
  case FilterInfo::Comparison::Equal:
    return compareCol[i] == constant;
  case FilterInfo::Comparison::Greater:
    return compareCol[i] > constant;
  case FilterInfo::Comparison::Less:
    return compareCol[i] < constant;
  };
  return false;
}
//---------------------------------------------------------------------------
void FilterScan::run()
// Run
{
  for (uint64_t i = 0; i < relation.size; ++i)
  {
    bool pass = true;
    for (auto &f : filters)
    {
      pass &= applyFilter(i, f);
    }
    if (pass)
      copy2Result(i);
  }
}
//---------------------------------------------------------------------------
vector<uint64_t *> Operator::getResults()
// Get materialized results
{
  vector<uint64_t *> resultVector;
  for (auto &c : tmpResults)
  {
    resultVector.push_back(c.data());
  }
  return resultVector;
}
//---------------------------------------------------------------------------
bool Join::require(SelectInfo info)
// Require a column and add it to results
{
  if (requestedColumns.count(info) == 0)
  {
    bool success = false;
    if (left->require(info))
    {
      requestedColumnsLeft.emplace_back(info);
      success = true;
    }
    else if (right->require(info))
    {
      success = true;
      requestedColumnsRight.emplace_back(info);
    }
    if (!success)
      return false;

    tmpResults.emplace_back();
    requestedColumns.emplace(info);
  }
  return true;
}
//---------------------------------------------------------------------------
void Join::copy2Result(uint64_t leftId, uint64_t rightId)
// Copy to result
{
  unsigned relColId = 0;
  for (unsigned cId = 0; cId < copyLeftData.size(); ++cId)
    tmpResults[relColId++].push_back(copyLeftData[cId][leftId]);

  for (unsigned cId = 0; cId < copyRightData.size(); ++cId)
    tmpResults[relColId++].push_back(copyRightData[cId][rightId]);
  for (auto &iter : partitions)
  {
    const SelectInfo &info = iter.first;
    Partition &partition = iter.second;
    uint64_t colValue = tmpResults[select2ResultColId[info]][resultSize];
    partition[colValue % PARTITION_SIZE].emplace_back(resultSize);
  }
  ++resultSize;
}
//---------------------------------------------------------------------------

struct join_thread_args
{
  void *func;
  uint64_t partition_id;
};
template <typename Function>
void *join_thread_func(void *arg)
// function template of thread function
// by using arg, which should be lambda function, it runs given function
// then it returns given function's return value
{
  Function *f = ((Function *)((join_thread_args *)arg)->func);
  uint64_t partition_id = ((join_thread_args *)arg)->partition_id;

  void *ret = (void *)(*f)(partition_id);

  pthread_exit(ret);
}
//---------------------------------------------------------------------------
void Join::run()
// Run
{
  // function for getting left input of join
  // at now, I used original code for thread function
  auto run_left = [&]()
  {
    left->run();
    return nullptr;
  };

  // function for getting right input of join
  // at now, I used original code for thread function
  auto run_right = [&]()
  {
    right->run();
    return nullptr;
  };

  left->require(pInfo.left);
  right->require(pInfo.right);
  left->requirePartition(pInfo.left);
  right->requirePartition(pInfo.right);
  // run each thread
  pthread_t left_thread, right_thread;

  if (pthread_create(&left_thread, NULL, thread_func<decltype(run_left)>, &run_left) < 0)
  {
    cerr << "[Error]" << pInfo.left.dumpText() << " failed" << endl;
    exit(-1);
  }
  if (pthread_create(&right_thread, NULL, thread_func<decltype(run_right)>, &run_right) < 0)
  {
    cerr << "[Error]" << pInfo.right.dumpText() << " failed" << endl;
    exit(-1);
  }

  // wait for child threads
  // it does not use return value.
  void *ret;
  pthread_join(left_thread, &ret);
  pthread_join(right_thread, &ret);

  // Use smaller input for build
  if (left->resultSize > right->resultSize)
  {
    swap(left, right);
    swap(pInfo.left, pInfo.right);
    swap(requestedColumnsLeft, requestedColumnsRight);
  }

  auto leftInputData = left->getResults();
  auto rightInputData = right->getResults();
  auto leftPartition = left->getPartition(pInfo.left);
  auto rightPartition = right->getPartition(pInfo.right);

  // Resolve the input columns
  unsigned resColId = 0;
  for (auto &info : requestedColumnsLeft)
  {
    copyLeftData.push_back(leftInputData[left->resolve(info)]);
    select2ResultColId[info] = resColId++;
  }
  for (auto &info : requestedColumnsRight)
  {
    copyRightData.push_back(rightInputData[right->resolve(info)]);
    select2ResultColId[info] = resColId++;
  }

  auto leftColId = left->resolve(pInfo.left);
  auto rightColId = right->resolve(pInfo.right);
  auto leftKeyColumn = leftInputData[leftColId];
  auto rightKeyColumn = rightInputData[rightColId];

  ConcurrentQueue<pair<uint64_t, uint64_t>> cq;

  auto join_on_partition = [&](uint64_t partition_id)
  {
    auto &leftIdSet = leftPartition[partition_id];
    auto &rightIdSet = rightPartition[partition_id];
    HT hashTable;
    // Build phase
    for (auto id : leftIdSet)
    {
      hashTable.emplace(leftKeyColumn[id], id);
    }

    // Probe phase
    for (auto id : rightIdSet)
    {
      auto rightKey = rightKeyColumn[id];
      auto range = hashTable.equal_range(rightKey);
      for (auto iter = range.first; iter != range.second; ++iter)
      {
        cq.push({iter->second, id});
      }
    }

    return nullptr;
  };

  vector<pthread_t *> threads;
  for (uint64_t i = 0; i < PARTITION_SIZE; i++)
  {
    pthread_t *thread = new pthread_t();
    threads.push_back(thread);
    join_thread_args *args = new join_thread_args();
    args->func = &join_on_partition;
    args->partition_id = i;
    if (pthread_create(thread, NULL, join_thread_func<decltype(join_on_partition)>, (void *)args) < 0)
    {
      exit(-1);
    }
  }
  for (auto &thread : threads)
  {
    void *ret;
    pthread_join(*thread, &ret);
  }

  pair<uint64_t, uint64_t> dest;
  while (cq.pop(dest))
  {
    copy2Result(dest.first, dest.second);
  }
}
//---------------------------------------------------------------------------
void SelfJoin::copy2Result(uint64_t id)
// Copy to result
{
  for (unsigned cId = 0; cId < copyData.size(); ++cId)
    tmpResults[cId].push_back(copyData[cId][id]);
  for (auto &iter : partitions)
  {
    const SelectInfo &info = iter.first;
    Partition &partition = iter.second;
    uint64_t colValue = tmpResults[select2ResultColId[info]][resultSize];
    partition[colValue % PARTITION_SIZE].emplace_back(resultSize);
  }
  ++resultSize;
}
//---------------------------------------------------------------------------
bool SelfJoin::require(SelectInfo info)
// Require a column and add it to results
{
  if (requiredIUs.count(info))
    return true;
  if (input->require(info))
  {
    tmpResults.emplace_back();
    requiredIUs.emplace(info);
    return true;
  }
  return false;
}
//---------------------------------------------------------------------------
void SelfJoin::run()
// Run
{
  input->require(pInfo.left);
  input->require(pInfo.right);
  input->requirePartition(pInfo.left);
  input->requirePartition(pInfo.right);
  input->run();
  inputData = input->getResults();

  for (auto &iu : requiredIUs)
  {
    auto id = input->resolve(iu);
    copyData.emplace_back(inputData[id]);
    select2ResultColId.emplace(iu, copyData.size() - 1);
  }

  auto leftColId = input->resolve(pInfo.left);
  auto rightColId = input->resolve(pInfo.right);

  auto leftCol = inputData[leftColId];
  auto rightCol = inputData[rightColId];
  for (uint64_t i = 0; i < input->resultSize; ++i)
  {
    if (leftCol[i] == rightCol[i])
      copy2Result(i);
  }
}
//---------------------------------------------------------------------------
void Checksum::run()
// Run
{
  for (auto &sInfo : colInfo)
  {
    input->require(sInfo);
  }
  input->run();

  auto results = input->getResults();

  for (auto &sInfo : colInfo)
  {
    auto colId = input->resolve(sInfo);
    auto resultCol = results[colId];
    uint64_t sum = 0;
    resultSize = input->resultSize;
    for (auto iter = resultCol, limit = iter + input->resultSize; iter != limit; ++iter)
      sum += *iter;
    checkSums.push_back(sum);
  }
}
//---------------------------------------------------------------------------
