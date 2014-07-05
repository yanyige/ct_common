#include <ct_common/common/exp.h>

using namespace ct::common;

Exp::Exp(void)
    : TreeNode() {
}

Exp::Exp(const Exp &from)
    : TreeNode(from) {
}

Exp& Exp::operator = (const Exp &right) {
  TreeNode::operator = (right);
  return *this;
}

Exp::~Exp(void) {
}

std::string Exp::get_class_name(void) const {
  return Exp::class_name();
}

std::string Exp::class_name(void) {
  return "Exp";
}
