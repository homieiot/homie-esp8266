//
// Created by erow on 18-2-22.
//

#ifndef NODES_NODE_HPP
#define NODES_NODE_HPP

#include <utility>
#include <vector>
#include <string>

using Binary = std::vector<unsigned char>;
#define D printf

namespace Node
{
using String = std::string;
enum HandleError
{
  success = 1,
  fail = 0,
  illegal_name = -1,
  not_find = -2,
  illegal_data = -3
};

class NodeType
{
public:
  virtual bool is_legal(const Binary &value) const = 0;

  virtual void parse(const Binary &value) = 0;

  virtual String toString() const = 0;
};

class Node
{
  String name;
  Node *mParent = nullptr;

public:
  void setParent(Node *p) { mParent = p; }
  String getPath()
  {
    if (mParent)
      return mParent->getPath() + "/" + name;
    else
      return name;
  }
  explicit Node(String str) : name(std::move(str)) {}

  virtual int handleMsg(const String &node_name, const Binary &data) = 0;

  virtual int handleMsg(const Binary &data) { return 0; }

  virtual void setup() {}
  virtual void loop() {}
  virtual void onReadyToOperate() {}
  virtual String toString() = 0;

public:
  String getName() { return name; }
};
}

#endif //NODES_NODE_HPP
