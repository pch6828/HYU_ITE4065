#pragma once
#include <valarray>
//---------------------------------------------------------------------------
class SnapValue
{
private:
    int label;
    int value;
    std::valarray<int> snap;

public:
    SnapValue() : label(0),
                  value(0),
                  snap(std::valarray<int>())
    {
        //Do Nothing
    }
    SnapValue(int label, int value, std::valarray<int> snap) : label(label),
                                                               value(value),
                                                               snap(snap)
    {
        //Do Nothing
    }

    int getLabel();
    int getValue();
    std::valarray<int> getSnap();
};
//---------------------------------------------------------------------------
class Snapshot
{
private:
    int numThread;
    std::valarray<SnapValue *> regs;

    std::valarray<SnapValue *> collect();

public:
    Snapshot(int numThread) : numThread(numThread),
                              regs(std::valarray<SnapValue *>(numThread))
    {
        for (auto &reg : regs)
        {
            reg = new SnapValue();
        }
    }

    std::valarray<int> scan();
    void update(int tid, int value);
};
//---------------------------------------------------------------------------