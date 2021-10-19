#pragma once
#include <valarray>
#include <memory>
//---------------------------------------------------------------------------
class SnapValue
{
    /// Definition for Thread-Local Snapshot Value
private:
    /// Label to Indicate # of Thread-Local Updates.
    int label;
    /// Thread-Local Value
    int value;
    /// Thread-Local Snapshot to Help Other Threads' Snapshot::scan()
    std::valarray<int> snap;

public:
    /// Default Constructor
    /// it is used to initialize Snapshot object
    SnapValue(int numThread) : label(0),
                               value(0),
                               snap(std::valarray<int>(numThread))
    {
        // Do Nothing
    }
    /// Constructor for New Snapshot
    /// it is used to make new SnapValue made in Snapshot::update()
    SnapValue(int label, int value, std::valarray<int> snap) : label(label),
                                                               value(value),
                                                               snap(snap)
    {
        // Do Nothing
    }
    /// Basic getter function for SnapValue's field
    /// since SnapValue itself is immutable, there are no setter function.
    /// in Snapshot::update(), updating SnapValue is done by assigning new SnapValue object into Snapshot.
    int getLabel();               // Getter for Label
    int getValue();               // Getter for Thread-Local Value
    std::valarray<int> getSnap(); // Getter for Thread-Local Snapshot
};
//---------------------------------------------------------------------------
class Snapshot
{
    /// Definition for Wait-Free Atomic Snapshot
private:
    /// # of Threads Using Snapshot
    int numThread;
    /// Register for Each Thread's Thread-Local Snapshot Value.
    std::valarray<std::shared_ptr<SnapValue>> regs;

    /// Read All Snapshot Values
    void collect(std::valarray<std::shared_ptr<SnapValue>> &dest);

public:
    /// Constructor
    Snapshot(int numThread = 0) : numThread(numThread),
                                  regs(std::valarray<std::shared_ptr<SnapValue>>(numThread))
    {
        // initialize each register with default SnapValue object
        for (auto &reg : regs)
        {
            reg = std::make_shared<SnapValue>(numThread);
        }
    }

    /// Get Clean Collect of Snapshot
    std::valarray<int> scan();
    /// Update Given Thread's Local Value by Given Value
    void update(int tid, int value);
};
//---------------------------------------------------------------------------