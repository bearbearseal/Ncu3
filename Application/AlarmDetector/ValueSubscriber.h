//Open and maintain a socket
//Use the socket to listen for values from multiple channel.
#include <string>
#include "../../../MyLib/Basic/HashKey.h"

class ValueSubscriber
{
public:
    class Listener
    {
    public:
        virtual void catch_value_update(const );
    };
    ValueSubscriber();
    virtual ~ValueSubscriber();
};