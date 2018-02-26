//
// Created by erow on 18-2-22.
//

#ifndef NODES_STRINGTYPE_HPP
#define NODES_STRINGTYPE_HPP


#include "../Node.hpp"
namespace Node{
//    class Type : public NodeType {
//    public:
//        bool is_legal(const Binary &value) const override {
//            return true;
//        }
//        void parse(const Binary &value) override {
//
//        }
//        String toString() const override {
//            return "";
//        }
//    };
    class StringType : public NodeType,String {
    public:
        bool is_legal(const Binary &value) const override {
            return true;
        }
        void parse(const Binary &value) override {
            this->clear();
            for(auto c:value)
                this->push_back(c);
        }
        String toString() const override {
            return (*this);
        }
    };
}


#endif //NODES_STRINGTYPE_HPP
