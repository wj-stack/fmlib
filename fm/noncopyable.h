//
// Created by Administrator on 2022/4/23.
//

#ifndef FM_NONCOPYABLE_H
#define FM_NONCOPYABLE_H


class noncopyable {
public:
    noncopyable() = default;
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};


#endif //FM_NONCOPYABLE_H
