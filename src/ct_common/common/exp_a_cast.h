#ifndef CT_COMMON_EXP_A_CAST_H_
#define CT_COMMON_EXP_A_CAST_H_

#include <ct_common/common/utils.h>
#include <ct_common/common/exp_a_unary.h>

namespace ct {
namespace common {
class DLL_EXPORT Exp_A_Cast : public Exp_A_Unary {
public:
  Exp_A_Cast(void);
  Exp_A_Cast(const Exp_A_Cast &from);
  Exp_A_Cast &operator = (const Exp_A_Cast &right);
  virtual ~Exp_A_Cast(void);

public:
  virtual std::string get_class_name(void) const;
  static std::string class_name(void);
  virtual void dump(std::ostream &os, const std::vector<boost::shared_ptr<ParamSpec> > &param_specs) const;
  virtual std::string get_op_token(void) const;

private:
  virtual double evaluate_double(double val) const;
  virtual int evaluate_int(int val) const;
};
}  // namespace common
}  // namespace ct

#endif  // CT_COMMON_EXP_A_CAST_H_