//
// Created by erow on 18-2-22.
//

#ifndef NODES_EXECUTENODE_HPP
#define NODES_EXECUTENODE_HPP

#include <functional>
#include "Node.hpp"

namespace Node
{
template <class T>
class ExecuteNode : public Node
{
  public:
    typedef std::function<bool(const T &)> NodeHandler;

    explicit ExecuteNode(const String &str, NodeHandler h = nullptr) : Node(str)
    {
        mType = new T();
        mHandler = h;
    }

    ~ExecuteNode() { delete (mType); }

  public:
    void setHandler(NodeHandler h)
    {
        mHandler = h;
    }

    int handleMsg(const Binary &data) override
    {
        if (mType->is_legal(data))
        {
            mType->parse(data);
            return mHandler(*mType);
        }
        return HandleError::illegal_data;
    }

    int handleMsg(const String &node_name, const Binary &data) final
    {
        if (node_name.empty())
            return handleMsg(data);
        D("don't support nested msg in executeNode!\n");
        return HandleError ::fail;
    }
    String toString() override
    {
        return mType->toString();
    }

  private:
    NodeHandler mHandler = [](const T &property) { return false; };
    T *mType;
};
}
#endif //NODES_EXECUTENODE_HPP
