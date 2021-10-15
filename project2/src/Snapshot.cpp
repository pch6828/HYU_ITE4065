#include "Snapshot.hpp"
#include <valarray>

using namespace std;
//---------------------------------------------------------------------------
int SnapValue::getLabel()
{
    return this->label;
}

int SnapValue::getValue()
{
    return this->value;
}

valarray<int> SnapValue::getSnap()
{
    return this->snap;
}
//---------------------------------------------------------------------------
static valarray<int> getValueFromRegisters(valarray<SnapValue *> &regs)
{
    valarray<int> result(regs.size());

    for (int tid = 0; tid < regs.size(); tid++)
    {
        result[tid] = regs[tid]->getValue();
    }

    return result;
}
//---------------------------------------------------------------------------
valarray<SnapValue *> Snapshot::collect()
{
    return this->regs;
}

valarray<int> Snapshot::scan()
{
    valarray<SnapValue *> oldCopy, newCopy;
    valarray<bool> moved(numThread, false);

    oldCopy = this->collect();

    while (true)
    {
        bool flag = true;
        newCopy = this->collect();
        for (int tid = 0; tid < numThread; tid++)
        {
            if (oldCopy[tid]->getLabel() != newCopy[tid]->getLabel())
            {
                if (moved[tid])
                {
                    return newCopy[tid]->getSnap();
                }
                else
                {
                    moved[tid] = true;
                    oldCopy = newCopy;
                    flag = false;
                    break;
                }
            }
        }
        if (flag)
        {
            break;
        }
    }

    return getValueFromRegisters(newCopy);
}

void Snapshot::update(int tid, int value)
{
    valarray<int> snap = this->scan();
    SnapValue *oldValue = regs[tid];
    SnapValue *newValue = new SnapValue(oldValue->getLabel() + 1, value, snap);
    regs[tid] = newValue;
}
//---------------------------------------------------------------------------