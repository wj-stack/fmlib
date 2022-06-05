//
// Created by Administrator on 2022/4/26.
//

#include "IgnoreSigPipe.h"

IgnoreSigPipe::IgnoreSigPipe(){
    ::signal(SIGPIPE, SIG_IGN);
}
