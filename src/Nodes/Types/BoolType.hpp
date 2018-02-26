//
// Created by erow on 18-2-23.
//

#ifndef NODES_BOOLTYPE_HPP
#define NODES_BOOLTYPE_HPP
#include "../Node.hpp"
namespace Node
{
class BoolType : public NodeType
{
  public:
    bool value;
    bool is_legal(const Binary &value) const override
    {
        String t;
        for (char c : value)
            t.push_back(c);
        return (t == "true") or (t == "false");
    }

    void parse(const Binary &value) override
    {
        String t;
        for (char c : value)
            t.push_back(c);
        this->value = (t == "true");
    }

    String toString() const override
    {
        return value ? "true" : "false";
    }
};
}
#endif