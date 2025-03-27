#include "TcpConnection.h"
#include "Callbacks.h"

namespace mymuduo{

    namespace net{



        void defaultConnectionCallback(const TcpConnectionPtr& conn){

        }
        
        void defaultMessageCallback(const TcpConnectionPtr& conn,
                                    Buffer* buffer,
                                    Timestamp receiveTime){

                                    }
    }
}
