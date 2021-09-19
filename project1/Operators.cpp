#include <Operators.hpp>
#include <cassert>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <pthread.h>
#include <stdio.h>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
// #define THREAD_CONTROL
// preprocessor for control # of active thread
// #define PRINT_THREAD
// preprocessor to see thread's enter and exit
//---------------------------------------------------------------------------
int num_of_thread = 0;
mutex mtx;
condition_variable cv;
//---------------------------------------------------------------------------
template <typename Function>
void *thread_func(void *arg)
// function template of thread function
// by using arg, which should be lambda function, it runs given function
// then it returns given function's return value
{
#ifdef THREAD_CONTROL
  unique_lock<mutex> enter_lock(mtx);
  cv.wait(enter_lock, [&]
          { return num_of_thread < 4; });
  num_of_thread++;
  enter_lock.unlock();
  cv.notify_all();
#endif
#ifdef PRINT_THREAD
  printf("Enter %ld, Active Thread : %d\n", pthread_self(), num_of_thread);
#endif

  Function *f = (Function *)arg;
  void *ret = (void *)(*f)();

#ifdef THREAD_CONTROL
  unique_lock<mutex> exit_lock(mtx);
  cv.wait(exit_lock, [&]
          { return num_of_thread > 0; });
  num_of_thread--;
  exit_lock.unlock();
  cv.notify_all();
#endif
#ifdef PRINT_THREAD
  printf("Exit %ld, Active Thread : %d\n", pthread_self(), num_of_thread);
#endif

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
  ++resultSize;
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

#ifdef THREAD_CONTROL
  unique_lock<mutex> wait_lock(mtx);
  cv.wait(wait_lock, [&]
          { return num_of_thread > 0; });
  num_of_thread--;
  wait_lock.unlock();
  cv.notify_all();

#ifdef PRINT_THREAD
  // printf("Wait %ld, Active Thread : %d\n", pthread_self(), num_of_thread);
#endif
#endif

  // wait for child threads
  // it does not use return value.
  void *ret;
  pthread_join(left_thread, &ret);
  pthread_join(right_thread, &ret);

#ifdef THREAD_CONTROL
  unique_lock<mutex> reenter_lock(mtx);
  cv.wait(reenter_lock, [&]
          { return num_of_thread < 4; });
  num_of_thread++;
  reenter_lock.unlock();
  cv.notify_all();

#ifdef PRINT_THREAD
  // printf("ReEnter %ld, Active Thread : %d\n", pthread_self(), num_of_thread);
#endif
#endif

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
}
//---------------------------------------------------------------------------
void SelfJoin::copy2Result(uint64_t id)
// Copy to result
{
  for (unsigned cId = 0; cId < copyData.size(); ++cId)
    tmpResults[cId].push_back(copyData[cId][id]);
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
  // function for getting input of Checksum
  // at now, I used original code for thread function
  // Since Checksum has only one input, thread is not necessary.
  // But I used thread for compatibility when using control of number of active thread
  auto run_join = [&]()
  {
    for (auto &sInfo : colInfo)
    {
      input->require(sInfo);
    }
    input->run();
    return nullptr;
  };

  pthread_t join_thread;

  // run child thread
  if (pthread_create(&join_thread, NULL, thread_func<decltype(run_join)>, &run_join) < 0)
  {
    cerr << "[Error] Checksum failed" << endl;
    exit(-1);
  }

  // wait for child threads
  // it does not use return value.
  void *ret;
  pthread_join(join_thread, &ret);

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
