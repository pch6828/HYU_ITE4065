#include "Snapshot.hpp"
#include <valarray>
#include <memory>
#include <atomic>

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
static valarray<int> getValueFromRegisters(valarray<shared_ptr<SnapValue>> &regs)
{
    valarray<int> result(regs.size());

    for (unsigned int tid = 0; tid < regs.size(); tid++)
    {
        result[tid] = regs[tid]->getValue();
    }

    return result;
}

static void copyArray(valarray<shared_ptr<SnapValue>> &dest, const valarray<shared_ptr<SnapValue>> &src)
{
    for (unsigned int tid = 0; tid < dest.size(); tid++)
    {
        atomic_store(&dest[tid], atomic_load(&src[tid]));
    }
}
//---------------------------------------------------------------------------
void Snapshot::collect(valarray<shared_ptr<SnapValue>> &dest)
{
    for (int i = 0; i < this->numThread; i++)
    {
        atomic_store(&dest[i], atomic_load(&regs[i]));
    }
}

valarray<int> Snapshot::scan()
{
    valarray<shared_ptr<SnapValue>> oldCopy(this->numThread);
    valarray<shared_ptr<SnapValue>> newCopy(this->numThread);
    valarray<bool> moved(false, this->numThread);

    this->collect(oldCopy);

    while (true)
    {
        bool flag = true;
        this->collect(newCopy);
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
                    copyArray(oldCopy, newCopy);
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
    shared_ptr<SnapValue> oldValue, newValue;
    oldValue = atomic_load(&regs[tid]);
    newValue = make_shared<SnapValue>(oldValue->getLabel() + 1, value, snap);
    atomic_store(&regs[tid], newValue);
}
//---------------------------------------------------------------------------