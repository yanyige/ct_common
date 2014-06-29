#ifndef CT_COMMON_EXP_H_
#define CT_COMMON_EXP_H_

#include <map>
#include <string>
#include <ct_common/common/utils.h>
#include <ct_common/common/tuple.h>
#include <ct_common/common/pvpair.h>
#include <ct_common/common/tree_node.h>

namespace ct {
namespace common {
class DLL_EXPORT Exp : public TreeNode {
public:
  Exp(void);
  // This class is not supposed to be copied and assigned
  // Only reference is copied
  // Except for terms, whose values are copied
  Exp(const Exp &from);
  Exp &operator = (const Exp &right);
  virtual ~Exp(void) = 0;

public:
  virtual std::string get_class_name(void) const;
  static std::string class_name(void);
};
}  // namespace common
}  // namespace ct

#endif  // CT_COMMON_EXP_H_
