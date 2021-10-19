#include "Snapshot.hpp"
#include <valarray>
#include <memory>
#include <atomic>

using namespace std;
//---------------------------------------------------------------------------
/* 
 * PAGE 1 : Function Definition for SnapValue
**/
int SnapValue::getLabel()
// Getter for Label
{
    return this->label;
}
int SnapValue::getValue()
// Getter for Thread-Local Value
{
    return this->value;
}
valarray<int> SnapValue::getSnap()
// Getter for Thread-Local Snapshot
{
    return this->snap;
}
//---------------------------------------------------------------------------
/* 
 * PAGE 2 : Helper Functions
**/
static void copyArray(valarray<shared_ptr<SnapValue>> &dest, const valarray<shared_ptr<SnapValue>> &src)
// Copy Array of SnapValue by Atomically load and store each SnapValue
// This function is needed because simple assignment of shared_ptr is not thread-safe.
{
    // for each value in source array,
    for (unsigned int tid = 0; tid < dest.size(); tid++)
    {
        // load its value atomically,
        // and store it atomically to destination array
        atomic_store(&dest[tid], atomic_load(&src[tid]));
    }
}
static valarray<int> getValueFromRegisters(valarray<shared_ptr<SnapValue>> &regs)
// Get Array of Thread-Local Value out of SnapValues
// This function is needed just for readability of code.
{
    valarray<int> result(regs.size());

    // for each Thread-Local Snapshot Value,
    for (unsigned int tid = 0; tid < regs.size(); tid++)
    {
        // store its value to result array
        result[tid] = regs[tid]->getValue();
    }

    return result;
}
//---------------------------------------------------------------------------
/* 
 * PAGE 3 : Function Definition for Snapshot
**/
void Snapshot::collect(valarray<shared_ptr<SnapValue>> &dest)
// Read All Snapshot Values
{
    // for each Thread-Local Snapshot Value,
    for (int i = 0; i < this->numThread; i++)
    {
        // load its value atomically,
        // and store it atomically to destination array
        atomic_store(&dest[i], atomic_load(&regs[i]));
    }
}
valarray<int> Snapshot::scan()
// Get Clean Collect of Snapshot
{
    valarray<shared_ptr<SnapValue>> oldCopy(this->numThread);
    valarray<shared_ptr<SnapValue>> newCopy(this->numThread);
    valarray<bool> moved(false, this->numThread);

    // get first collect
    this->collect(oldCopy);

    while (true)
    {
        // flag for breaking loop
        // if this remains true, it means there are no need to collect again.
        bool flag = true;

        // get next collect
        this->collect(newCopy);

        //for each Thread-Local Snapshot Value,
        for (int tid = 0; tid < numThread; tid++)
        {
            // compare its label,
            // if these two are different, additional check is followed.
            if (oldCopy[tid]->getLabel() != newCopy[tid]->getLabel())
            {
                if (moved[tid])
                {
                    // if this Snapshot Value is moved before,
                    // it means this thread interfered before(which is within the interval of this scan)
                    // so, just return its local Snapshot
                    return newCopy[tid]->getSnap();
                }
                else
                {
                    // if this update interferes for the first time,
                    // mark that this thread interfered.
                    moved[tid] = true;
                    // copy the current collect to old collect
                    copyArray(oldCopy, newCopy);
                    // set the flag false,
                    // so that next collect and compare will follow
                    flag = false;
                    break;
                }
            }
        }
        // if flag is remained true,
        // it means comparement of all Thread-Local Value was succeeded.
        // so, additional collect and compare are unnecessary
        // therefore, break the loop
        if (flag)
        {
            break;
        }
    }

    // extract local values from SnapValues
    // and return it
    return getValueFromRegisters(newCopy);
}

void Snapshot::update(int tid, int value)
// Update Given Thread's Local Value by Given Value
{
    // get clean collect
    valarray<int> snap = this->scan();
    shared_ptr<SnapValue> oldValue, newValue;
    // load old value atomiccaly to get its label
    oldValue = atomic_load(&regs[tid]);
    // make new SnapValue with given argument
    // new SnapValue's label is 1 greater than old value's
    // also add snapshot into SnapValue to help other thread's scan
    newValue = make_shared<SnapValue>(oldValue->getLabel() + 1, value, snap);
    // write new value to register atomiccaly
    atomic_store(&regs[tid], newValue);
}
//---------------------------------------------------------------------------