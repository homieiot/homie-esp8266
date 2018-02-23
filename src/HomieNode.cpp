#include "HomieNode.hpp"
#include "Homie.hpp"

using namespace HomieInternals;

HomieNode::HomieNode(const char *id, const char *type)
    : NestedNode(id), _type(type)
{
  if (strlen(id) + 1 > MAX_NODE_ID_LENGTH || strlen(type) + 1 > MAX_NODE_TYPE_LENGTH)
  {
    Helpers::abort(F("✖ HomieNode(): either the id or type string is too long"));
    return; // never reached, here for clarity
  }
  Homie._checkBeforeSetup(F("HomieNode::HomieNode"));
}

HomieNode::~HomieNode()
{
  Helpers::abort(F("✖✖ ~HomieNode(): Destruction of HomieNode object not possible\n  Hint: Don't create HomieNode objects as a local variable (e.g. in setup())"));
  return; // never reached, here for clarity
}