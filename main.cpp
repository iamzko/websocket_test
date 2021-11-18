#include <iostream>
#include <string>
#include <queue>
#include <thread>
#include <chrono>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
std::queue<int> data_queue;
void do_session(boost::asio::ip::tcp::socket socket)
{
    try
    {
        boost::beast::websocket::stream<boost::asio::ip::tcp::socket> websocket_stream{std::move(socket)};
        websocket_stream.set_option(boost::beast::websocket::stream_base::decorator([]
        (boost::beast::websocket::response_type& res){
            res.set(boost::beast::http::field::server,std::string(BOOST_BEAST_VERSION_STRING)+" "
                                                                                              "websocket-server-sync") ;
        }));
        websocket_stream.accept();
        boost::beast::flat_buffer buffer(10);
        buffer.max_size(100);
        auto buf = buffer.prepare(5);
        while (true)
        {
            if(!data_queue.empty())
            {
                auto data = data_queue.front();
                auto data_str = std::to_string(data);
                data_queue.pop();
                char * const p = static_cast<char * const>(buf.data());
                std::memcpy(p,data_str.c_str(),data_str.size());
                buffer.commit(data_str.size());
                websocket_stream.write(buffer.data());
                std::cout << "get data " << data << std::endl;
            }
        }
    }
    catch (const boost::beast::system_error &e)
    {
        std::cerr << "beast error: " << e.code().message() << std::endl;
    }
    catch (const std::exception e)
    {
        std::cerr << "do_session error: " << e.what() << std::endl;
    }
}
void do_push_data()
{
    int i   = 0;
    while (true)
    {
        data_queue.push(i++);
        std::this_thread::sleep_for(std::chrono::seconds{1});
    }
}

int main() {
    try
    {
        std::thread(&do_push_data).detach();
        auto address = boost::asio::ip::make_address("0.0.0.0");
        unsigned short port = 9999;
        boost::asio::io_context ioc{1};
        boost::asio::ip::tcp::acceptor acceptor{ioc,{address,port}};
        while (true)
        {
            boost::asio::ip::tcp::socket socket(ioc);
            acceptor.accept(socket);
            std::thread(&do_session,std::move(socket)).detach();
        }

    }
    catch (const std::exception &e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
