//
// Created by nw on 24.03.17.
//
/*!
 * \file DataStreamInterface.hpp
 * \brief Defines interface for data access
 */
#ifndef LIGHTLDA_DATASTREAMINTERFACE_HPP
#define LIGHTLDA_DATASTREAMINTERFACE_HPP

namespace multiverso
{
    namespace lightlda
    {
        namespace dev
        {
            /*! \brief interface of data stream */
            template <typename T>
            class DataStreamInterface : private T
            {
                    typedef DataStreamInterface<T> self_type;
                    typedef T impl_type;
                public:
                    /*! \brief Constructor with variadic types and arguments */
                    template <typename... Args>
                    DataStreamInterface(Args&&... args) : impl_type(std::forward<Args>(args)...) {}

                     /*! \brief destructor */
                    virtual ~DataStreamInterface() {}

                    /*! \brief Should call this method before access a data block */
                    void BeforeDataAccess()
                    {
                        impl_type::BeforeDataAccess();
                    }

                    /*! \brief Should call this method after access a data block */
                    void EndDataAccess()
                    {
                        impl_type::EndDataAccess();
                    }

                    /*!
                    * \brief Get one data block
                    * \return reference to data block
                    */
                    //Remark: this-> needed for gcc 4.9 (c++11). Not needed for gcc>=5.2
                    auto CurrDataBlock() -> decltype(this->impl_type::CurrDataBlock())
                    {
                        return impl_type::CurrDataBlock();
                    }
                private:
                    // No copying allowed
                    DataStreamInterface(const DataStreamInterface&) {}
                    void operator=(const DataStreamInterface&) {}
            };


/*
        // Dummy example
        class myImpl
        {
        public:
            myImpl(){data = new double(111);}
            myImpl(double d){data = new double(d);}
            virtual ~myImpl(){delete data;}
            void BeforeDataAccess() {std::cout<<"start \n";}
            void EndDataAccess() {std::cout<<"end \n";}
            double& CurrDataBlock() {
                std::cout<<"data = "<< *data << " \n";
                return *data;}
        private:
            double* data;

        };

        typedef DataStreamInterface<myImpl> myDataStream;

        int main()
        {
            myDataStream a;
            a.BeforeDataAccess();
            a.CurrDataBlock();
            a.EndDataAccess();

            myDataStream b(333);
            b.BeforeDataAccess();
            double& valb = b.CurrDataBlock();
            b.EndDataAccess();
            std::cout<<"valb = "<< valb << " \n";

            return 0;
        }
*/
        } // namespace dev
    } // namespace lightlda
} // namespace multiverso
#endif //LIGHTLDA_DATASTREAMINTERFACE_HPP
