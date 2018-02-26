//
// Created by erow on 18-2-22.
//

#ifndef NODES_RANGETYPE_HPP
#define NODES_RANGETYPE_HPP

#include "../Node.hpp"
#include <stdio.h>
namespace Node
{
template <int lower, int upper>
class RangeType : public NodeType
{
  public:
    int value;
    bool is_legal(const Binary &value) const override
    {
        int v = toInt(value);
        return (lower <= v) && (v <= upper);
    }

    void parse(const Binary &value) override
    {
        this->value = toInt(value);
    }

    String toString() const override
    {
        char tmp[10];
        sprintf(tmp, "%d", value);
        return tmp;
    }
    int toInt(const Binary &value) const
    {
        int v;
        sscanf((const char *)value.data(), "%d", &v);
        return v;
    }
};
}

#endif //NODES_RANGETYPE_HPP
