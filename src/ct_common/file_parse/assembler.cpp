//===----- ct_common/file_parse/assembler.cpp -------------------*- C++ -*-===//
//
//                      The ct_common Library
//
// This file is distributed under the MIT license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the function definitions of class Assembler
//
//===----------------------------------------------------------------------===//

#include <algorithm>
#include <exception>
#include <cstdlib>
#include <sstream>
#include <cmath>
#include <ct_common/file_parse/assembler.h>
#include <ct_common/common/seed_tuple.h>
#include <ct_common/common/seed_constraint.h>

using namespace ct::common;

Assembler::Assembler(void) {
  this->default_precision_ = 0;
}

Assembler::Assembler(const Assembler &from) {
  this->default_precision_ = from.default_precision_;
  this->err_logger_ = from.err_logger_;
  this->stored_invalidations_ = from.stored_invalidations_;
}

Assembler::~Assembler(void) {
}

const Assembler &Assembler::operator =(const Assembler &right) {
  this->default_precision_ = right.default_precision_;
  this->err_logger_ = right.err_logger_;
  this->stored_invalidations_ = right.stored_invalidations_;
  return (*this);
}

void Assembler::reportError(const std::string &str) {
  if (this->err_logger_.get()) {
    this->err_logger_->reportError(str);
  }
}

void Assembler::reportWarning(const std::string &str) {
  if (this->err_logger_.get()) {
    this->err_logger_->reportWarning(str);
  }
}

std::size_t Assembler::numErrs(void) const {
  if (this->err_logger_.get()) {
    return this->err_logger_->numErrs();
  }
  return 0;
}

std::size_t Assembler::numWarnings(void) const {
  if (this->err_logger_.get()) {
    return this->err_logger_->numWarnings();
  }
  return 0;
}

ParamSpec *Assembler::asm_paramspec(
    const std::string &type,
    const std::string &identifier,
    const std::vector<boost::shared_ptr<TreeNode> > &vals) {
  ParamSpec *tmp_return;
  if (type == "int") {
    tmp_return = new ParamSpec_Int();
  } else if (type == "double") {
    tmp_return = new ParamSpec_Double();
  } else if (type == "string") {
    tmp_return = new ParamSpec_String();
  } else if (type == "bool") {
    tmp_return = new ParamSpec_Bool();
  } else {
    CT_EXCEPTION("unrecognized parameter type");
    return 0;
  }
  tmp_return->set_param_name(identifier);
  std::vector<std::string> str_vals;
  if (TYPE_CHECK(tmp_return, ParamSpec_Bool*)) {
    if (vals.size() != 0 && (vals.size() != 2 ||
        !TYPE_CHECK(vals[0].get(), Constraint_L_CBool*) ||
        !TYPE_CHECK(vals[1].get(), Constraint_L_CBool*) ||
        dynamic_cast<Constraint_L_CBool*>(vals[0].get())->get_value() != true ||
        dynamic_cast<Constraint_L_CBool*>(vals[1].get())->get_value() != false)) {
      this->reportWarning("values for boolean parameters are neglected, possible format is:");
      this->reportWarning("bool <param>;");
      this->reportWarning("bool <param>: true, false;");
    }
  } else {
    for (std::size_t i = 0; i < vals.size(); ++i) {
      if (TYPE_CHECK(tmp_return, ParamSpec_Int*) &&
          TYPE_CHECK(vals[i].get(), Exp_S_CString*)) {
        this->reportWarning(
            std::string("warning: forcing string value ") + vals[i]->get_str_value()
            + " as integer for parameter " + tmp_return->get_param_name());
      }
      if (TYPE_CHECK(tmp_return, ParamSpec_Int*) &&
          TYPE_CHECK(vals[i].get(), Exp_A_CDouble*)) {
        this->reportWarning(
            std::string("warning: forcing double value ") + vals[i]->get_str_value()
            + " as integer for parameter " + tmp_return->get_param_name());
      }
      if (TYPE_CHECK(tmp_return, ParamSpec_Double*) &&
          TYPE_CHECK(vals[i].get(), Exp_S_CString*)) {
        this->reportWarning(
            std::string("warning: forcing string value ") + vals[i]->get_str_value()
            + " as double for parameter " + tmp_return->get_param_name());
      }
      if (TYPE_CHECK(tmp_return, ParamSpec_String*) &&
          TYPE_CHECK(vals[i].get(), Exp_A_CInt*)) {
        this->reportWarning(
            std::string("warning: forcing integer value ") + vals[i]->get_str_value()
            + " as string for parameter " + tmp_return->get_param_name());
      }
      if (TYPE_CHECK(tmp_return, ParamSpec_String*) &&
          TYPE_CHECK(vals[i].get(), Exp_A_CDouble*)) {
        this->reportWarning(
            std::string("warning: forcing double value ") + vals[i]->get_str_value()
            + " as string for parameter " + tmp_return->get_param_name());
      }
      str_vals.push_back(vals[i]->get_str_value());
    }
  }
  tmp_return->set_values(str_vals);
  return tmp_return;
}

ParamSpec *Assembler::asm_paramspec(
    const std::string &type,
    const std::string &identifier,
    const std::vector<std::pair<boost::shared_ptr<TreeNode>, boost::shared_ptr<TreeNode> > > &auto_value_specs) {
  ParamSpec *tmp_return;
  if (type == "int") {
    tmp_return = new ParamSpec_Int();
  } else if (type == "double") {
    tmp_return = new ParamSpec_Double();
  } else if (type == "string") {
    tmp_return = new ParamSpec_String();
  } else if (type == "bool") {
    tmp_return = new ParamSpec_Bool();
  } else {
    CT_EXCEPTION("unrecognized parameter type");
    return 0;
  }
  if (auto_value_specs.size() == 0) {
    this->reportWarning(std::string("the auto value specs for parameter ") + identifier + " is empty");
  } else {
    boost::shared_ptr<Constraint_L_CBool> last_cond = boost::dynamic_pointer_cast<Constraint_L_CBool>(auto_value_specs.back().first);
    if (!last_cond || last_cond->get_value() != true) {
      this->reportWarning(std::string("the last condition for auto parameter ") + identifier + " is not a constant true. This may cause errors if the specified conditions do not cover all the cases. Consider using a \"default\" or \"true\".");
    }
  }
  
  tmp_return->set_auto(true);
  tmp_return->set_param_name(identifier);
  tmp_return->auto_value_specs() = auto_value_specs;
  return tmp_return;
}


void attach_strengths_core(
    const std::vector<std::size_t> &pid_list,
    std::vector<std::vector<std::size_t> > &strengths,
    std::vector<std::size_t> &stack,
    std::size_t strength,
    std::size_t depth) {
  if (depth == strength) {
    std::vector<std::size_t> new_strength;
    for (std::size_t i = 0; i < stack.size(); ++i) {
      new_strength.push_back(pid_list[stack[i]]);
    }
    strengths.push_back(new_strength);
    return;
  }
  std::size_t start_point = 0;
  if (depth > 0) {
    start_point = stack[depth-1]+1;
  }
  for (std::size_t i = start_point; i < pid_list.size(); ++i) {
    stack[depth] = i;
    attach_strengths_core(pid_list, strengths, stack, strength, depth+1);
  }
}

void Assembler::attach_default_strengths(
    const std::vector<boost::shared_ptr<ParamSpec> > &param_specs,
    std::vector<ct::common::Strength> &strengths,
    std::size_t strength) {
  // compute the parameter id list
  std::vector<std::size_t> pid_list;
  for (std::size_t i = 0; i < param_specs.size(); ++i) {
    if (!param_specs[i]->is_auto() && !param_specs[i]->is_aux()) {
      pid_list.push_back(i);
    }
  }
  // generate the strengths
  for (std::size_t i = 0; i < pid_list.size(); ++i) {
    if (param_specs[pid_list[i]]->is_auto()) {
      CT_EXCEPTION("auto parameters should never be included in any covering strength");
      return;
    }
  }
  if (pid_list.size() < strength) {
    CT_EXCEPTION("the parameter list is smaller than the strength");
    return;
  }
  Strength new_strength;
  new_strength.first = pid_list;
  new_strength.second = strength;
  strengths.push_back(new_strength);
}

void Assembler::attach_strengths(
    const std::vector<boost::shared_ptr<ParamSpec> > &param_specs,
    const std::vector<std::string> &identifiers,
    std::vector<ct::common::Strength> &strengths,
    std::size_t strength) {
  // compute the parameter id list
  std::vector<std::size_t> pid_list;
  for (std::size_t i = 0; i < identifiers.size(); ++i) {
    if (identifiers[i] == "default") {
      for (std::size_t j = 0; j < param_specs.size(); ++j) {
        if (!param_specs[j]->is_aux() && !param_specs[j]->is_auto()){
          pid_list.push_back(j);
        }
      }
      continue;
    }
    std::size_t id = find_param_id(param_specs, identifiers[i]);
    if (id == PID_BOUND) {
      CT_EXCEPTION((std::string("parameter ") + identifiers[i] + " not found").c_str());
    }
    if (param_specs[id]->is_auto()) {
      CT_EXCEPTION((std::string("parameter ") + identifiers[i] +
                    " is a auto parameter, and cannot be added to a strength").c_str());
    }
    pid_list.push_back(id);
  }
  // make unique
  std::sort(pid_list.begin(), pid_list.end());
  std::vector<std::size_t> unique_pid_list;
  for (std::size_t i = 0; i < pid_list.size(); i++) {
    if (unique_pid_list.size() == 0 ||
        pid_list[i] > unique_pid_list.back()) {
      unique_pid_list.push_back(pid_list[i]);
    }
  }
  // generate the strengths
  if (unique_pid_list.size() < strength) {
    CT_EXCEPTION("the parameter list is smaller than the strength");
  }
  Strength new_strength;
  new_strength.first = unique_pid_list;
  new_strength.second = strength;
  strengths.push_back(new_strength);
}

PVPair *Assembler::asm_pvpair(
    const std::vector<boost::shared_ptr<ParamSpec> > &param_specs,
    const std::string &identifier,
    const TreeNode *value_exp) {
  if (value_exp == NULL) {
    CT_EXCEPTION("encountered null value expression");
    return 0;
  }
  if (value_exp->get_str_value() == "") {
    CT_EXCEPTION("not a valid value");
    return 0;
  }
  PVPair *tmp_return = new PVPair();
  std::size_t pid = find_param_id(param_specs, identifier);
  if (pid == PID_BOUND) {
    // CT_EXCEPTION((std::string("parameter ") + identifier + " not found").c_str()); // no exception now
    return 0;
  }
  if (param_specs[pid]->is_auto()) {
    CT_EXCEPTION((std::string("cannot assemble pvpair for auto parameter") + identifier).c_str());
  }
  std::size_t vid = param_specs[pid]->query_value_id(value_exp->get_str_value());
  if (vid == VID_BOUND) {
    CT_EXCEPTION((std::string("value not found:<")+identifier+","+value_exp->get_str_value()+">").c_str());
    return 0;
  }
  tmp_return->pid_ = pid;
  tmp_return->vid_ = vid;
  return tmp_return;
}

TreeNode *Assembler::asm_param(const std::vector<boost::shared_ptr<ParamSpec> > &param_specs, const std::string &identifier) {
  std::size_t pid = find_param_id(param_specs, identifier);
  if (pid == PID_BOUND) {
    CT_EXCEPTION((std::string("parameter ") + identifier + " not found").c_str());
    return 0;
  }
  TreeNode *tmp_return = 0;
  if (TYPE_CHECK(param_specs[pid].get(), ParamSpec_String*)) {
    Exp_S_Param *exp = new Exp_S_Param();
    exp->set_pid(pid);
    tmp_return = exp;
  } else if (TYPE_CHECK(param_specs[pid].get(), ParamSpec_Int*)) {
    Exp_A_Param *exp = new Exp_A_Param();
    exp->set_pid(pid);
    exp->set_type(EAT_INT);
    tmp_return = exp;
  } else if (TYPE_CHECK(param_specs[pid].get(), ParamSpec_Double*)) {
    Exp_A_Param *exp = new Exp_A_Param();
    exp->set_pid(pid);
    exp->set_type(EAT_DOUBLE);
    tmp_return = exp;
  } else if (TYPE_CHECK(param_specs[pid].get(), ParamSpec_Bool*)) {
    Constraint_L_Param *constr = new Constraint_L_Param();
    constr->set_pid(pid);
    tmp_return = constr;
  } else {
    CT_EXCEPTION("unrecognized parameter type");
  }
  return tmp_return;
}

Constraint_A *Assembler::asm_constraint_a(Exp_A *oprd1, Exp_A *oprd2, eOPERATOR op, double precision) {
  if (oprd1 == NULL || oprd2 == NULL) {
    CT_EXCEPTION("encountered null oprands");
    return 0;
  }
  Constraint_A_Binary *tmp_return = 0;
  switch (op) {
  case OP_EQ:
    tmp_return = new Constraint_A_EQ();
    break;
  case OP_NE:
    tmp_return = new Constraint_A_NE();
    break;
  case OP_GT:
    tmp_return = new Constraint_A_GT();
    break;
  case OP_GE:
    tmp_return = new Constraint_A_GE();
    break;
  case OP_LT:
    tmp_return = new Constraint_A_LT();
    break;
  case OP_LE:
    tmp_return = new Constraint_A_LE();
    break;
  default:
    CT_EXCEPTION("unexpected arithmetic constraint type");
    return 0;
    break;
  }
  boost::shared_ptr<Exp_A> tmp_oprd1(oprd1);
  boost::shared_ptr<Exp_A> tmp_oprd2(oprd2);
  tmp_return->set_loprd(tmp_oprd1);
  tmp_return->set_roprd(tmp_oprd2);
  tmp_return->set_precision(precision);
  return tmp_return;
}

Constraint_S *Assembler::asm_constraint_s(Exp_S *oprd1, Exp_S *oprd2, eOPERATOR op) {
  if (oprd1 == NULL || oprd2 == NULL) {
    CT_EXCEPTION("encountered null oprands");
    return 0;
  }
  Constraint_S_Binary *tmp_return = 0;
  switch (op) {
  case OP_EQ:
    tmp_return = new Constraint_S_EQ();
    break;
  case OP_NE:
    tmp_return = new Constraint_S_NE();
    break;
  default:
    CT_EXCEPTION("unexpected string constraint type");
    return 0;
    break;
  }
  boost::shared_ptr<Exp_S> tmp_oprd1(oprd1);
  boost::shared_ptr<Exp_S> tmp_oprd2(oprd2);
  tmp_return->set_loprd(tmp_oprd1);
  tmp_return->set_roprd(tmp_oprd2);
  return tmp_return;
}

Constraint *Assembler::asm_constraint_asb(TreeNode *oprd1, TreeNode *oprd2, eOPERATOR op, const std::string &str) {
  if (oprd1 == NULL || oprd2 == NULL) {
    CT_EXCEPTION("encountered null oprands");
    return 0;
  }
  double precision = this->default_precision_;
  bool is_precision_set = false;
  std::size_t pos_lbracket = str.find('[');
  std::size_t pos_rbracket = str.find(']');
  if (pos_lbracket != std::string::npos && pos_rbracket != std::string::npos && pos_rbracket > pos_lbracket+1) {
    is_precision_set = true;
    std::string str_precision = str.substr(pos_lbracket+1, pos_rbracket-pos_lbracket-1);
    std::stringstream ss(str_precision);
    ss >> precision;
    precision = fabs(precision);
  }
  if (TYPE_CHECK(oprd1, Exp_A*) && TYPE_CHECK(oprd2, Exp_A*)) {
    Exp_A *exp1 = dynamic_cast<Exp_A *>(oprd1);
    Exp_A *exp2 = dynamic_cast<Exp_A *>(oprd2);
    return this->asm_constraint_a(exp1, exp2, op, precision);
  }
  if (TYPE_CHECK(oprd1, Exp_S*) && TYPE_CHECK(oprd2, Exp_S*)) {
    if (is_precision_set) {
      this->reportWarning("ignoring precision for string relation");
    }
    Exp_S *exp1 = dynamic_cast<Exp_S *>(oprd1);
    Exp_S *exp2 = dynamic_cast<Exp_S *>(oprd2);
    return this->asm_constraint_s(exp1, exp2, op);
  }
  if (TYPE_CHECK(oprd1, Constraint*) && TYPE_CHECK(oprd2, Constraint*)) {
    if (is_precision_set) {
      this->reportWarning("ignoring precision for bool relation");
    }
    Constraint *constr1 = dynamic_cast<Constraint *>(oprd1);
    Constraint *constr2 = dynamic_cast<Constraint *>(oprd2);
    return this->asm_constraint_l(constr1, constr2, op);
  }
  CT_EXCEPTION((std::string("operands mismatch: ")+oprd1->get_class_name()+ " " + oprd2->get_class_name()).c_str());
  return 0;
}

Constraint_L *Assembler::asm_constraint_l(TreeNode *oprd1, TreeNode *oprd2, eOPERATOR op) {
  if (oprd1 == NULL || oprd2 == NULL) {
    CT_EXCEPTION("encountered null oprands");
    return 0;
  }
  TYPE_ASSERT(oprd1, Constraint*);
  TYPE_ASSERT(oprd2, Constraint*);
  Constraint_L_Binary *tmp_return = 0;
  switch (op) {
  case OP_AND:
    tmp_return = new Constraint_L_And();
    break;
  case OP_OR:
    tmp_return = new Constraint_L_Or();
    break;
  case OP_XOR:
  case OP_NE:
    tmp_return = new Constraint_L_Xor();
    break;
  case OP_IMPLY:
    tmp_return = new Constraint_L_Imply();
    break;
  case OP_IFF:
  case OP_EQ:
    tmp_return = new Constraint_L_Iff();
    break;
  default:
    CT_EXCEPTION("unhandled l constraint");
    return 0;
    break;
  }
  Constraint *constr1 = dynamic_cast<Constraint *>(oprd1);
  Constraint *constr2 = dynamic_cast<Constraint *>(oprd2);
  tmp_return->set_loprd(boost::shared_ptr<Constraint>(constr1));
  tmp_return->set_roprd(boost::shared_ptr<Constraint>(constr2));
  return tmp_return;
}

Constraint_L *Assembler::asm_constraint_l(TreeNode *oprd, eOPERATOR op) {
  if (oprd == NULL) {
    CT_EXCEPTION("encountered null oprand");
    return 0;
  }
  TYPE_ASSERT(oprd, Constraint*);
  Constraint_L_Unary *tmp_return = 0;
  switch (op) {
  case OP_NOT:
    tmp_return = new Constraint_L_Not();
    break;
  default:
    CT_EXCEPTION("unhandled l constraint");
    return 0;
    break;
  }
  Constraint *constr = dynamic_cast<Constraint *>(oprd);
  tmp_return->set_oprd(boost::shared_ptr<Constraint>(constr));
  return tmp_return;
}

Constraint_L_CBool *Assembler::asm_constraint_l_cbool(const std::string &val) {
  Constraint_L_CBool *constr = new Constraint_L_CBool();
  if (val == "true") {
    constr->set_value(true);
  } else if (val == "false") {
    constr->set_value(false);
  } else {
    CT_EXCEPTION("unrecognized boolean value");
  }
  return constr;
}

Exp_A *Assembler::asm_exp_a(TreeNode *oprd1, TreeNode *oprd2, eOPERATOR op) {
  if (oprd1 == NULL || oprd2 == NULL) {
    CT_EXCEPTION("encountered null oprands");
    return 0;
  }
  TYPE_ASSERT(oprd1, Exp_A*);
  TYPE_ASSERT(oprd2, Exp_A*);
  Exp_A *c_oprd1 = dynamic_cast<Exp_A *>(oprd1);
  Exp_A *c_oprd2 = dynamic_cast<Exp_A *>(oprd2);
  Exp_A_Binary *tmp_return = 0;
  switch (op) {
  case OP_ADD:
    tmp_return = new Exp_A_Add();
    break;
  case OP_DIV:
    tmp_return = new Exp_A_Div();
    break;
  case OP_MOD:
    if (c_oprd1->get_type() != EAT_INT ||
        c_oprd2->get_type() != EAT_INT) {
      CT_EXCEPTION("the operands of \'%\' must be integers");
    }
    tmp_return = new Exp_A_Mod();
    break;
  case OP_MULT:
    tmp_return = new Exp_A_Mult();
    break;
  case OP_SUB:
    tmp_return = new Exp_A_Sub();
    break;
  default:
    CT_EXCEPTION("unhandled exp type");
    return 0;
    break;
  }
  tmp_return->set_loprd(boost::shared_ptr<TreeNode>(c_oprd1));
  tmp_return->set_roprd(boost::shared_ptr<TreeNode>(c_oprd2));
  if (c_oprd1->get_type() == EAT_INT &&
      c_oprd2->get_type() == EAT_INT) {
    tmp_return->set_type(EAT_INT);
  } else if (c_oprd1->get_type() == EAT_DOUBLE ||
      c_oprd2->get_type() == EAT_DOUBLE) {
    tmp_return->set_type(EAT_DOUBLE);
  } else {
    CT_EXCEPTION("cannot determine the parent expression type");
  }
  return tmp_return;
}

Exp_A *Assembler::asm_exp_a(TreeNode *oprd, eOPERATOR op) {
  if (oprd == NULL) {
    CT_EXCEPTION("encountered null oprand");
    return 0;
  }
  TYPE_ASSERT(oprd, Exp_A*);
  Exp_A *c_oprd = dynamic_cast<Exp_A*>(oprd);
  switch (op) {
  case OP_NEG: {
    Exp_A_Unary *tmp_return = 0;
    tmp_return = new Exp_A_Neg();
    tmp_return->set_oprd(boost::shared_ptr<TreeNode>(c_oprd));
    tmp_return->set_type(c_oprd->get_type());
    return tmp_return;
    break;
  }
  case OP_UPLUS:
    return c_oprd;
    break;
  default:
    CT_EXCEPTION("unhandled exp type");
    return 0;
    break;
  }
}

Exp_A *Assembler::asm_exp_a_cast(TreeNode *oprd, const std::string &type) {
  if (oprd == NULL) {
    CT_EXCEPTION("encountered null oprand");
    return 0;
  }
  if (TYPE_CHECK(oprd, Exp_A*)) {
    Exp_A *exp = dynamic_cast<Exp_A *>(oprd);
    Exp_A_Cast *tmp_return = new Exp_A_Cast();
    if (type == "int") {
      tmp_return->set_type(EAT_INT);
    } else if (type == "double") {
      tmp_return->set_type(EAT_DOUBLE);
    } else {
      CT_EXCEPTION((std::string("cannot cast to type ") + type).c_str());
      return 0;
    }
    tmp_return->set_oprd(boost::shared_ptr<TreeNode>(exp));
    return tmp_return;
  } else if (TYPE_CHECK(oprd, Constraint*)) {
    Constraint *constr = dynamic_cast<Constraint *>(oprd);
    Exp_A_ConstraintCast *tmp_return = new Exp_A_ConstraintCast();
    if (type == "int") {
      tmp_return->set_type(EAT_INT);
    } else if (type == "double") {
      tmp_return->set_type(EAT_DOUBLE);
    } else {
      CT_EXCEPTION((std::string("cannot cast to type ") + type).c_str());
      return 0;
    }
    tmp_return->set_oprd(boost::shared_ptr<TreeNode>(constr));
    return tmp_return;    
  }
  CT_EXCEPTION((std::string("cannot cast from ") + oprd->get_class_name()).c_str());
  return 0;
}

Constraint *Assembler::asm_constraint_invalid(
    const std::vector<boost::shared_ptr<ParamSpec> > &param_specs,
    const std::string &identifier) {
  std::size_t pid = find_param_id(param_specs, identifier);
  if (pid == PID_BOUND) {
    this->reportWarning(std::string("cannot find parameter ") + identifier + " when assembling parameter invalidation constraints, neglecting");
    return 0;
  }
  // FIXME: the semantics for invalidating auto parameters are not defined
  //if (param_specs[pid]->is_auto()) {
  //  this->reportWarning(std::string("parameter ") + identifier + " cannot be invalidated since it is aux or auto");
  //  return 0;
  //}
  Constraint_L_IVLD *constr = new Constraint_L_IVLD();
  constr->set_pid(pid);
  return constr;
}

void Assembler::store_invalidation(
    const std::vector<boost::shared_ptr<ParamSpec> > &param_specs,
    const std::vector<std::string> &identifiers,
    const boost::shared_ptr<TreeNode> &precond) {
  for (std::size_t i = 0; i < identifiers.size(); ++i) {
    std::size_t pid = find_param_id(param_specs, identifiers[i]);
    if (pid == PID_BOUND) {
      this->reportWarning(std::string("cannot find parameter ") + identifiers[i] + " when assembling parameter invalidation constraints, neglecting");
      continue;
    }
    this->stored_invalidations_[pid].push_back(precond);
  }
}

std::vector<boost::shared_ptr<Constraint> > Assembler::dump_invalidations(const std::vector<boost::shared_ptr<ParamSpec> > &param_specs) {
  std::vector<boost::shared_ptr<Constraint> > tmp_return;
  for (std::size_t i = 0; i < param_specs.size(); ++i) {
    std::size_t pid = i;
    // FIXME: the semantics for invalidating auto parameters are not defined
    //if (param_specs[pid]->is_auto()) {
    //  // cannot proceed with auto parameters
    //  continue;
    //}
    const std::vector<boost::shared_ptr<TreeNode> > &constrs = this->stored_invalidations_[pid];
    boost::shared_ptr<Constraint_L_IVLD> constr_ivld(new Constraint_L_IVLD());
    constr_ivld->set_pid(pid);
    {
      if (constrs.size() == 0) {
        boost::shared_ptr<Constraint_L_Not> constr_nivld(new Constraint_L_Not());
        constr_nivld->set_oprd(constr_ivld);
        tmp_return.push_back(constr_nivld);
      } else {
        boost::shared_ptr<Constraint> constr_conjunction;
        for (std::size_t i = 0; i < constrs.size(); ++i) {
          boost::shared_ptr<Constraint> this_constr = boost::dynamic_pointer_cast<Constraint>(constrs[i]);
          if (!constrs[i]) {
            std::stringstream ss;
            ss << "condition #" << i+1 << " for invalidating parameter " << param_specs[pid]->get_param_name() << " is not a condition";
            this->reportError(ss.str());
            return tmp_return;
          }
          if (i == 0) {
            constr_conjunction = this_constr;
          } else {
            boost::shared_ptr<Constraint_L_Or> constr_or(new Constraint_L_Or());
            constr_or->set_loprd(constr_conjunction);
            constr_or->set_roprd(this_constr);
            constr_conjunction = constr_or;
          }
        }
        boost::shared_ptr<Constraint_L_Iff> constr_to_add(new Constraint_L_Iff());
        constr_to_add->set_loprd(constr_conjunction);
        constr_to_add->set_roprd(constr_ivld);
        tmp_return.push_back(constr_to_add);
      }
    }
  }
  return tmp_return;
}

Seed *Assembler::asm_seed(std::size_t id, const ct::common::Tuple &tuple) {
  Seed_Tuple *seed = new Seed_Tuple();
  seed->set_id(id);
  seed->the_tuple() = tuple;
  return seed;
}

Seed *Assembler::asm_seed(std::size_t id, Constraint *constr) {
  Seed_Constraint *seed = new Seed_Constraint();
  seed->set_id(id);
  seed->the_constraint().reset(constr);
  return seed;
}

void Assembler::set_option(const std::string &identifier, const ct::common::TreeNode *value) {
  if (identifier == "default_precision") {
    if (dynamic_cast<const Exp_A_CInt*>(value)) {
      this->default_precision_ = dynamic_cast<const Exp_A_CInt*>(value)->get_value();
    } else if (dynamic_cast<const Exp_A_CDouble*>(value)) {
      this->default_precision_ = dynamic_cast<const Exp_A_CDouble*>(value)->get_value();
    } else {
      CT_EXCEPTION(std::string("invalid value for option ")+identifier);
    }
  } else {
    CT_EXCEPTION("unhandled option");
  }
}
