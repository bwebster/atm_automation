#pragma once
#include <Arduino.h>

template <size_t CAPACITY>
class StringFifo {
public:
  bool  push(const String& s) {            // returns false if full
    if (full()) return false;
    _buf[_tail] = s;
    _tail = (_tail + 1) % CAPACITY;
    ++_count;
    return true;
  }

  bool  pop(String& out) {                 // returns false if empty
    if (empty()) return false;
    out = _buf[_head];
    _head = (_head + 1) % CAPACITY;
    --_count;
    return true;
  }

  bool drop() {                 // true on success
    if (empty()) return false;
    _head = (_head + 1) % CAPACITY;
    --_count;
    return true;
  }

 bool contains(const String& needle) const
  {
    for (size_t i = 0, idx = _head; i < _count; ++i, idx = (idx + 1) % CAPACITY) {
      if (needle == _buf[idx])
        return true;
    }
    return false;
  }

  bool  empty() const { return _count == 0; }
  bool  full () const { return _count == CAPACITY; }
  size_t size() const { return _count; }
  size_t cap() const { return CAPACITY; }

private:
  String _buf[CAPACITY];
  size_t _head = 0, _tail = 0, _count = 0;
};