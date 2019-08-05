//
// Created by 林小兀 on 2019-07-29.
//

#ifndef V8DEMO_SHELL_H
#define V8DEMO_SHELL_H

class Point {
public:
  Point(int x, int y) : x_(x), y_(y) { }
  int x_, y_;


  int multi() {
    return this->x_ * this->y_;
  }
};

#endif //V8DEMO_SHELL_H
