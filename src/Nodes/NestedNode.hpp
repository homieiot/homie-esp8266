//
// Created by erow on 18-2-22.
//

#ifndef NODES_NESTNODE_HPP
#define NODES_NESTNODE_HPP

#include <ArduinoJson.hpp>

#include "Node.hpp"
namespace Node
{
class NestedNode : public Node
{
  protected:
    std::vector<Node *> mNodes;

  public:
    explicit NestedNode(const String &str) : Node(str) {}
    void setup() override
    {
        for (auto p : mNodes)
            p->setup();
    }
    void loop() override
    {
        for (auto p : mNodes)
            p->loop();
    }
    void onReadyToOperate() override
    {
        for (auto p : mNodes)
            p->onReadyToOperate();
    }
    bool addNode(Node *node)
    {
        mNodes.push_back(node);
        node->setParent(this);
    }

    int handleMsg(const String &node_name, const Binary &data) override
    {
        if (node_name.empty())
            return Node::handleMsg(data);
        auto i = node_name.find_first_of('/');
        String name = (i == std::string::npos) ? node_name : String(node_name.begin(), node_name.begin() + i);
        if (i == std::string::npos)
            i = node_name.size() - 1;
        for (auto p : mNodes)
        {
            if (p->getName() == name)
                return p->handleMsg(String(node_name.begin() + i + 1, node_name.end()), data);
        }
        return HandleError::not_find;
    }

    void getProperties(ArduinoJson::JsonObject &json)
    {
        for (auto n : mNodes)
        {
            auto t = n->toString();
            auto name = n->getName().c_str();
            if (t.size() == 0)
            {
                auto &j2 = json.createNestedObject(name);
                static_cast<NestedNode *>(n)->getProperties(j2);
            }
            else
            {
                json[name] = t.c_str();
            }
        }
    }
    String toString() override
    {
        return "";
    }
};
}
#endif //NODES_NESTNODE_HPP
