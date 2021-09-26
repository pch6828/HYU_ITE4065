#include <Operators.hpp>
#include <cassert>
#include <iostream>
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
  unique_lock<mutex> lock(mtx);
  cv.wait(lock, [this]
          { return !(this->process_finished); });
  resultSize = relation.size;
  process_finished = true;
  lock.unlock();
  cv.notify_all();
}
//---------------------------------------------------------------------------
vector<uint64_t *> Scan::getResults()
// Get materialized results
{
  unique_lock<mutex> lock(mtx);
  cv.wait(lock, [this]
          { return (this->process_finished); });
  flush_finished = process_finished;
  nowResultSize = resultSize;
  lock.unlock();
  cv.notify_all();
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
    tmpResults[0].emplace_back();
    tmpResults[1].emplace_back();
    unsigned colId = tmpResults[0].size() - 1;
    select2ResultColId[info] = colId;
  }
  return true;
}
//---------------------------------------------------------------------------
void FilterScan::copy2Result(uint64_t id)
// Copy to result
{
  unique_lock<mutex> lock(mtx);
  for (unsigned cId = 0; cId < inputData.size(); ++cId)
    tmpResults[buffer_idx][cId].push_back(inputData[cId][id]);
  ++resultSize;
  ++flushSize;
  cv.notify_all();
  lock.unlock();
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
  process_finished = true;
  cv.notify_all();
}
//---------------------------------------------------------------------------
vector<uint64_t *> Operator::getResults()
// Get materialized results
{
  unique_lock<mutex> lock(mtx);

  cv.wait(lock, [this]
          { return this->process_finished || flushSize >= MIN_FLUSH_SIZE; });

  uint64_t flush_idx = buffer_idx;
  buffer_idx++;
  buffer_idx %= 2;

  flush_finished = process_finished;
  nowResultSize = resultSize;
  flushSize = 0;
  tmpResults[buffer_idx].clear();
  for (uint64_t i = 0; i < tmpResults[flush_idx].size(); i++)
  {
    tmpResults[buffer_idx].emplace_back();
  }
  lock.unlock();
  cv.notify_all();

  vector<uint64_t *> resultVector;
  for (auto &c : tmpResults[flush_idx])
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

    tmpResults[0].emplace_back();
    tmpResults[1].emplace_back();
    requestedColumns.emplace(info);
  }
  return true;
}
//---------------------------------------------------------------------------
void Join::copy2Result(uint64_t leftId, uint64_t rightId)
// Copy to result
{
  unique_lock<mutex> lock(mtx);
  unsigned relColId = 0;
  for (unsigned cId = 0; cId < copyLeftData.size(); ++cId)
    tmpResults[buffer_idx][relColId++].push_back(copyLeftData[cId][leftId]);

  for (unsigned cId = 0; cId < copyRightData.size(); ++cId)
    tmpResults[buffer_idx][relColId++].push_back(copyRightData[cId][rightId]);
  ++resultSize;
  ++flushSize;
  cv.notify_all();
  lock.unlock();
}
//---------------------------------------------------------------------------
void Join::run()
// Run
{
  // function for getting left input of join
  // at now, I used original code for thread function
  auto run_left = [&]()
  {
    left->require(pInfo.left);
    left->run();
    return nullptr;
  };

  // function for getting right input of join
  // at now, I used original code for thread function
  auto run_right = [&]()
  {
    right->require(pInfo.right);
    right->run();
    return nullptr;
  };

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

  // Build phase
  auto leftKeyColumn = leftInputData[leftColId];
  hashTable.reserve(left->resultSize * 2);
  for (uint64_t i = 0, limit = i + left->resultSize; i != limit; ++i)
  {
    hashTable.emplace(leftKeyColumn[i], i);
  }
  // Probe phase
  auto rightKeyColumn = rightInputData[rightColId];
  for (uint64_t i = 0, limit = i + right->resultSize; i != limit; ++i)
  {
    auto rightKey = rightKeyColumn[i];
    auto range = hashTable.equal_range(rightKey);
    for (auto iter = range.first; iter != range.second; ++iter)
    {
      copy2Result(iter->second, i);
    }
  }
  process_finished = true;
  cv.notify_all();
}
//---------------------------------------------------------------------------
void SelfJoin::copy2Result(uint64_t id)
// Copy to result
{
  unique_lock<mutex> lock(mtx);
  for (unsigned cId = 0; cId < copyData.size(); ++cId)
  {
    tmpResults[buffer_idx][cId].push_back(copyData[cId][id]);
  }
  ++resultSize;
  ++flushSize;
  cv.notify_all();
  lock.unlock();
}
//---------------------------------------------------------------------------
bool SelfJoin::require(SelectInfo info)
// Require a column and add it to results
{
  if (requiredIUs.count(info))
    return true;
  if (input->require(info))
  {
    tmpResults[0].emplace_back();
    tmpResults[1].emplace_back();
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
  auto run_input = [&]()
  {
    input->run();
    return nullptr;
  };

  pthread_t input_thread;

  // run child thread
  if (pthread_create(&input_thread, NULL, thread_func<decltype(run_input)>, &run_input) < 0)
  {
    cerr << "[Error] SelfJoin failed" << endl;
    exit(-1);
  }

  uint64_t prevResultSize = 0;
  while (!(input->flush_finished))
  {
    inputData = input->getResults();
    if (input->nowResultSize == 0)
    {
      continue;
    }
    copyData.clear();
    for (auto &iu : requiredIUs)
    {
      auto id = input->resolve(iu);
      copyData.push_back(inputData[id]);
      select2ResultColId.emplace(iu, copyData.size() - 1);
    }

    auto leftColId = input->resolve(pInfo.left);
    auto rightColId = input->resolve(pInfo.right);

    auto leftCol = inputData[leftColId];
    auto rightCol = inputData[rightColId];
    for (uint64_t i = 0; i < input->nowResultSize - prevResultSize; ++i)
    {
      if (leftCol[i] == rightCol[i])
      {
        copy2Result(i);
      }
    }
    prevResultSize = input->nowResultSize;
  }
  // wait for child threads
  // it does not use return value.
  void *ret;
  pthread_join(input_thread, &ret);
  process_finished = true;
  cv.notify_all();
}
//---------------------------------------------------------------------------
void Checksum::run()
// Run
{
  // function for getting input of Checksum
  // at now, I used original code for thread function
  // Since Checksum has only one input, thread is not necessary.
  // But I used thread for compatibility when using control of number of active thread
  auto run_input = [&]()
  {
    for (auto &sInfo : colInfo)
    {
      input->require(sInfo);
    }
    input->run();
    return nullptr;
  };

  pthread_t input_thread;

  // run child thread
  if (pthread_create(&input_thread, NULL, thread_func<decltype(run_input)>, &run_input) < 0)
  {
    cerr << "[Error] Checksum failed" << endl;
    exit(-1);
  }

  // wait for child threads
  // it does not use return value.
  void *ret;
  pthread_join(input_thread, &ret);

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
