#include <iostream>
#include <string>
#include <cmath>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#define NUM_THREAD_IN_POOL 4

boost::asio::io_service ioCounter;
boost::asio::io_service ioPrinter;

bool isPrime(int n)
{
    if (n < 2)
    {
        return false;
    }

    for (int i = 2; i <= sqrt(n); i++)
    {
        if (n % i == 0)
        {
            return false;
        }
    }
    return true;
}

void printResult(int sequenceNumber, int rangeStart, int rangeEnd, int result)
{
    std::cout << "(" << sequenceNumber << ")number of primes in " << rangeStart << " ~ " << rangeEnd << " is " << result << std::endl;
}

void countPrime(int sequenceNumber, int rangeStart, int rangeEnd)
{
    int result = 0;
    for (int i = rangeStart; i <= rangeEnd; i++)
    {
        if (isPrime(i))
        {
            result++;
        }
    }
    ioPrinter.post(boost::bind(printResult, sequenceNumber, rangeStart, rangeEnd, result));
}

int main()
{
    boost::thread_group counterPool;
    boost::thread_group printerPool;
    boost::asio::io_service::work *counterWork = new boost::asio::io_service::work(ioCounter);
    boost::asio::io_service::work *printerWork = new boost::asio::io_service::work(ioPrinter);

    int rangeStart, rangeEnd, sequenceNumber = 0;

    for (int i = 0; i < NUM_THREAD_IN_POOL; i++)
    {
        counterPool.create_thread(boost::bind(&boost::asio::io_service::run, &ioCounter));
    }

    printerPool.create_thread(boost::bind(&boost::asio::io_service::run, &ioPrinter));

    while (true)
    {
        std::cin >> rangeStart;
        if (rangeStart == -1)
        {
            break;
        }
        std::cin >> rangeEnd;

        ioCounter.post(boost::bind(countPrime, sequenceNumber, rangeStart, rangeEnd));

        sequenceNumber++;
    }

    delete counterWork;
    counterPool.join_all();
    ioCounter.stop();

    delete printerWork;
    printerPool.join_all();
    ioPrinter.stop();

    return 0;
}