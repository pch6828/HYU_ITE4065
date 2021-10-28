#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#define NUM_THREAD_IN_POOL 4

void Print()
{
    std::cout << "Hi, I'm Thread " << boost::this_thread::get_id() << std::endl;
}

int main()
{
    boost::asio::io_service io;
    boost::thread_group threadpool;
    boost::asio::io_service::work *work = new boost::asio::io_service::work(io);

    for (int i = 0; i < NUM_THREAD_IN_POOL; i++)
    {
        threadpool.create_thread(boost::bind(&boost::asio::io_service::run, &io));
    }

    for (int i = 0; i < 10; i++)
    {
        io.post(Print);
        sleep(1);
    }

    threadpool.join_all();
    io.stop();

    return 0;
}