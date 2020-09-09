#include "PrioritizedValue.h"
#include <iostream>

using namespace std;

int main() {
    PrioritizedValue pValue;
    pValue.set_value(1, 2);
    cout<<pValue.get_value().get_int()<<endl;
    pValue.set_value(2, 3);
    cout<<pValue.get_value().get_int()<<endl;
    pValue.set_value(3, 5);
    cout<<pValue.get_value().get_int()<<endl;
    pValue.set_value(1, 6);
    cout<<pValue.get_value().get_int()<<endl;
    pValue.unset_value(3);
    cout<<pValue.get_value().get_int()<<endl;
    pValue.unset_value(2);
    cout<<pValue.get_value().get_int()<<endl;
    pValue.set_value(3, 9);
    cout<<pValue.get_value().get_int()<<endl;
    pValue.trigger_value(20);
    cout<<pValue.get_value().get_int()<<endl;
    pValue.set_value(0, 9);
    cout<<pValue.get_value().get_int()<<endl;
}