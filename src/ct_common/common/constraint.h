#ifndef CT_COMMON_CONSTRAINT_H_
#define CT_COMMON_CONSTRAINT_H_

#include <ct_common/common/utils.h>
#include <vector>
#include <string>
#include <ct_common/common/pvpair.h>
#include <ct_common/common/assignment.h>
#include <ct_common/common/paramspec.h>
#include <boost/shared_ptr.hpp>
#include <ct_common/common/tree_node.h>
#include <ct_common/common/eval_type_bool.h>

namespace ct {
namespace common {
// Base constraint class
class DLL_EXPORT Constraint : public TreeNode {
public:
  Constraint(void);
  // This class is not supposed to be copied and assigned
  // only the reference is copied
  Constraint(const Constraint &from);
  Constraint &operator = (const Constraint &right);
  virtual ~Constraint(void) = 0;
  
  virtual std::string get_class_name(void) const;
  static std::string class_name(void);

public:
  virtual EvalType_Bool Evaluate(const std::vector<boost::shared_ptr<ParamSpec> > &param_specs,
                        const Assignment &assignment) const = 0;

};
}  // namespace common
}  // namespace ct

#endif  // CT_COMMON_CONSTRAINT_H_
