#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

void Print(const boost::system::error_code &e)
{
    std::cout << "Hello, world!" << std::endl;
}

int main()
{
    boost::asio::io_service io;

    boost::asio::deadline_timer t(io, boost::posix_time::seconds(2));
    t.async_wait(&Print);
    std::cout << "after async_wait" << std::endl;

    io.run();
    std::cout << "after io.run()" << std::endl;

    return 0;
}