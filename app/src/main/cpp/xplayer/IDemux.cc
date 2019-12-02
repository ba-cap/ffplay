//
//   author: patrick.dai
// datetime: 2019-12-02 12:12
//

#include "IDemux.h"

void IDemux::main()
{
    while (!mIsExit)
    {
        XData d = Read();
        if(d.size > 0)
        {
            notify(d);
        }

        //if(d.size <= 0)
        //{
        //    break;
        //}
    }
}