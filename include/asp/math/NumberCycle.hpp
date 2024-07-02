#pragma once

namespace asp {

class NumberCycle {
public:
    NumberCycle(int from, int to);
    NumberCycle(int val, int from, int to);

    NumberCycle(const NumberCycle& other) = default;
    NumberCycle(NumberCycle&& other) = default;

    NumberCycle& operator=(const NumberCycle& other) = default;
    NumberCycle& operator=(NumberCycle&& other) = default;

    operator int() const;
    int get() const;
    void set(int val);

    int operator++();
    int operator--();

    int increment();
    int decrement();

    int operator+(int) const;
    int operator-(int) const;

private:
    int from, to, current;
};

}