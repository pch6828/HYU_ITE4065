#pragma once
#include <valarray>
#include <memory>
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
    std::valarray<std::shared_ptr<SnapValue>> regs;

    void collect(std::valarray<std::shared_ptr<SnapValue>> &dest);

public:
    Snapshot(int numThread = 0) : numThread(numThread),
                                  regs(std::valarray<std::shared_ptr<SnapValue>>(numThread))
    {
        for (auto &reg : regs)
        {
            reg = std::make_shared<SnapValue>();
        }
    }

    std::valarray<int> scan();
    void update(int tid, int value);
};
//---------------------------------------------------------------------------