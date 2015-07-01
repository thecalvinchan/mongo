#include "mongo/scripting/tinyjs/node.h"

namespace mongo {
namespace tinyjs {

Node::Node(std::string name) {
    this->name = name;
}

Node::~Node() {
    for (std::vector<std::unique_ptr<Node> >::iterator it = this->children.begin(); it != this->children.end(); it++) {
        (*it).reset();
    }
    this->children.clear();
}

void Node::addChild(std::unique_ptr<Node> child) {
    if (child) {
        this->children.push_back(std::move(child));
    }
}

std::vector<std::unique_ptr<Node> > Node::getChildren() {
    return std::move(this->children);
}

std::string Node::getName() {
    return this->name;
}

}
}

