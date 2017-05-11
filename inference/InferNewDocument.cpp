//
// Created by nw on 11.05.17.
//


#include "inferWrapper.hpp"

int main(int argc, char** argv)
{
    try
    {
        multiverso::lightlda::Config::inference = true;
        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();
        {
            multiverso::lightlda::inferWrapper inferNewDoc;
            inferNewDoc.Run(argc, argv);
        }
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_time = end-start;
        std::cout << "total time for LightLDA computation: " << elapsed_time.count() << "s\n";
    }
    catch(mongocxx::exception& e)
    {
        std::cout << "[mongocxx::exception] exception caught: " << e.what();
    }
    catch(bsoncxx::exception& e)
    {
        std::cout << "[bsoncxx::exception] exception caught: " << e.what();
    }
    catch(std::exception& e) {
        std::cout << "[std::exception] exception caught: " << e.what();
    }

    return 0;
}
