#include "interface.hpp"
using namespace std;

namespace scheme {

class self_eval_exec : public executor {
private:
  sc_obj obj;
public:
  self_eval_exec(sc_obj obj): obj {obj} {}

  sc_obj
  eval(environment* env) const {
    return obj;
  }
};

class var_exec : public executor {
private:
  symbol sym;
public:
  var_exec(symbol sym): sym {sym} {}

  sc_obj
  eval(environment* env) const {
    return env->lookup(sym);
  }
};

class set_exec : public executor {
private:
  symbol var;
  executor *vproc;
public:
  set_exec(symbol var, executor *exec): var {var}, vproc {exec} {}

  sc_obj
  eval(environment *env) const {
    env->set_variable(var, vproc->eval(env));
    return "ok";
  }
};

class def_exec : public executor {
private:
  symbol var;
  executor *vproc;
public:
  def_exec(symbol v1, executor *v2): var {v1}, vproc {v2} {}

  sc_obj
  eval(environment *env) const {
    env->define_variable(var, vproc->eval(env));
    return "ok";
  }
};

class if_exec : public executor {
private:
  executor *pproc, *cproc, *aproc;

public:
  if_exec(executor *pp, executor *cp, executor *ap): pproc {pp}, cproc {cp}, aproc {ap} {}

  sc_obj
  eval(environment *env) const {
    if (is_true(pproc->eval(env)))
      return cproc->eval(env);
    else
      return aproc->eval(env);
  }
};

class lambda_exec : public executor {
private:
  vector<symbol> vars;
  executor *bproc;
public:
  lambda_exec(const vector<symbol>& v, executor *b): vars {v}, bproc {b} {}

  sc_obj
  eval(environment *env) const {
    return new procedure(vars, bproc, env);
  } 
};

class begin_exec : public executor {
private:
  vector<executor*> execs;

public:
  begin_exec(const vector<executor*>& v): execs {v} {}

  sc_obj
  eval(environment *env) const {
    sc_obj ret;
    for (let ex : execs) {
      ret = ex->eval(env);
    }
    return ret;
  }
};

class apply_exec : public executor {
private:
  executor *fproc;
  vector<executor*> aprocs;
public:
  apply_exec(executor *fp, const vector<executor*>& aps): fproc {fp}, aprocs {aps} {}

  sc_obj
  eval(environment *env) const {
    vector<sc_obj> args {};
    for (let aproc : aprocs) {
      args.push_back(aproc->eval(env));
    }
    return scheme::apply(
      fproc->eval(env), 
      args
    );
  }
};

class let_exec : public executor {
private:
  map<symbol, executor*> pseudoframe;
  executor *bproc;

  environment*
  get_frame(environment *env) const {
    auto ret = new environment(env);
    for (let& p : pseudoframe) {
      ret->define_variable(
        p.first,
        p.second->eval(env)
      );
    }
    return ret;
  }

public:
  let_exec(const map<symbol, executor*> mp, executor* bp): pseudoframe {mp}, bproc {bp} {}

  sc_obj
  eval(environment *env) const {
    let env2 = get_frame(env);
    return bproc->eval(env2);
  } 
};

class and_exec : public executor {
private:
  vector<executor*> execs;

public:
  and_exec(const vector<executor*>& v): execs {v} {}

  sc_obj
  eval(environment *env) const {
    for (let ex : execs) {
      if (is_false(ex->eval(env)))
        return false;
    }
    return true;
  }
};

class or_exec : public executor {
private:
  vector<executor*> execs;

public:
  or_exec(const vector<executor*>& v): execs {v} {}

  sc_obj
  eval(environment *env) const {
    for (let ex : execs) {
      if (is_true(ex->eval(env)))
        return true;
    }
    return false;
  }
};

class cons_set_exec : public executor {
private:
  symbol var;
  executor *vproc;
  bool side;
public:
  cons_set_exec(symbol var, executor *exec, bool side): 
    var {var}, vproc {exec}, side {side} {}

  sc_obj
  eval(environment *env) const {
    let edit = vproc->eval(env);
    auto found = (env->lookup(var));
    if (!holds_alternative<cons*>(found)) {
      throw runtime_error("cons_set_exec::eval: type error");
    }
    let thing = get<cons*>(found);
    switch (side) {
      case 0:
        thing->car = edit;
        break;
      case 1:
        thing->cdr = edit;
        break;   
    }
    return thing;
  }
};

class cxr_exec : public executor {
private:
  string word;
  executor *exec;
public:
  cxr_exec(string s, executor *e): word {s}, exec {e} {}

  sc_obj
  eval(environment *env) const {
    let val = exec->eval(env);
    if (!holds_alternative<cons*>(val)) {
      throw runtime_error("cxr_exec::eval: type error");
    }
    auto found = get<cons*>(val);
    return found->at(word);
  }
};

}