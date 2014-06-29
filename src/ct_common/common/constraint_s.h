#ifndef CT_COMMON_CONSTRAINT_S_H_
#define CT_COMMON_CONSTRAINT_S_H_

#include <ct_common/common/utils.h>
#include <ct_common/common/constraint.h>

namespace ct {
namespace common {
class DLL_EXPORT Constraint_S : public Constraint {
public:
  Constraint_S(void);
  Constraint_S(const Constraint_S &from);
  Constraint_S &operator = (const Constraint_S &right);
  virtual ~Constraint_S(void);

  virtual std::string get_class_name(void) const;
  static std::string class_name(void);
};
}  // namespace common
}  // namespace ct

#endif  // CT_COMMON_CONSTRAINT_S_H_
