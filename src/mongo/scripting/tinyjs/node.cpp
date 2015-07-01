#pragma once

#include "mongo/scripting/tinyjs/node.h"

Node::Node(Token token) {
    this->type = token.type;
    this->value = token.value;
}

Node::~Node() {
    for (std::vector<std::unique_ptr<Node> >::iterator it = this->children.begin(); it != this->children.end(); i++) {
        delete (*it);
    }
    this->children.clear();
}

void Node::addChild(std::unique_ptr<Node> child) {
    if (child) {
        this->children.push_back(child);
    }
}
