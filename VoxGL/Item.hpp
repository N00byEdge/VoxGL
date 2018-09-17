#pragma once

#include <optional>
#include <utility>

struct Item {
  virtual ~Item() { }
  virtual void operator+=(Item &) { }
  virtual operator bool() { return false; }
};

template<typename BlockT, int maxStack = 64>
struct BlockItem: Item {
  void operator+=(Item &item) override {
    auto other = dynamic_cast<BlockItem &>(item);
    if (other) {
      if (auto sum = num + other.num; sum > maxStack)
        other.num = maxStack - num, num = maxStack;
      else num = sum;
    }
  }
  operator bool() override { return num; }
  int num;
};
