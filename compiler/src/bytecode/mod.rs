use ast;

use std::collections::HashMap;
use std::cell::RefCell;

mod opcodes;
mod function;
mod module;
mod instruction;

pub use self::instruction::{Bytecode, Instruction, VarRef};
pub use self::function::Function;
pub use self::module::Module;

struct FnGenerator<'a> {
    var_count: u8,
    module_name: &'a str,
    function_name: &'a str,
    env: HashMap<String, VarRef>,
    args: &'a Vec<ast::Argument<'a>>,
    body: &'a Vec<ast::Expr<'a>>,
    parent: Option<&'a FnGenerator<'a>>,
    upvals: RefCell<Vec<VarRef>>
}

impl<'a> FnGenerator<'a> {
    fn new<'b>(module_name: &'b str, function_name: &'b str, args: &'b Vec<ast::Argument>, body: &'b Vec<ast::Expr>, parent: Option<&'b FnGenerator<'b>>) -> FnGenerator<'b> {
        let mut env = HashMap::new();
        let mut var_count: u8 = 0;

        for arg in args.iter() {
            var_count += 1;
            env.insert(arg.name.to_string(), VarRef::Register(var_count));
        }

        FnGenerator {
            var_count: var_count,
            module_name: module_name,
            function_name: function_name,
            env: env,
            args: args,
            body: body,
            parent: parent,
            upvals: RefCell::new(Vec::new())
        }
    }

    fn generate(&mut self) -> Function {
        let code = self.generate_code();
        let name = format!("{}.{}", self.module_name, self.function_name);

        Function {
            name: name,
            arity: self.args.len() as u8,
            code: code
        }
    }

    fn generate_code(&mut self) -> Bytecode {
        let mut code = self.generate_block(VarRef::Register(0), self.body);

        code.push(Instruction::Return);
        code
    }

    fn search_parent_env(&self, identifier: &str) -> Option<VarRef> {
      self.parent
          .map(|parent_fun| parent_fun.search_env(identifier))
          .unwrap_or(None)
    }

    fn push_upval(&self, var_ref: VarRef) -> VarRef {
        let existing = self.upvals.borrow().iter().position(|&u| u == var_ref);

        match existing {
            Some(index) => {
                VarRef::Upvalue((index) as u8)
            },
            None => {
                let upval_index = self.upvals.borrow().len() as u8;
                self.upvals.borrow_mut().push(var_ref);
                VarRef::Upvalue(upval_index)
            }
        }
    }

    /// Searches for an identifier in the current scope. If local variable does not
    /// exist, keeps recursively looking through parent functions hoping to grab an upvalue from
    /// one of the parent environments. If the value is found on a parent, every function along the
    /// way adds the found variable reference to their upvalues so they get copied down when the
    /// function is constructed in the VM.
    fn search_env(&self, identifier: &str) -> Option<VarRef> {
        match self.env.get(identifier) {
            Some(var_ref) => Some(*var_ref),
            None => {
                match self.search_parent_env(identifier) {
                    Some(upval) => {
                        Some(self.push_upval(upval))
                    },
                    None => None
                }
            }
        }
    }

    fn identifier_exists_in_scope(&self, identifier: &str) -> bool {
        match self.env.get(identifier) {
            Some(_) => true,
            None => self.parent.map(|p| p.identifier_exists_in_scope(identifier)).unwrap_or(false)
        }
    }

    fn insert_env(&mut self, identifier: String, reg: VarRef) {
        self.env.insert(identifier, reg);
    }

    fn generate_expr(&mut self, out: VarRef, expr: &'a ast::Expr) -> Bytecode {
        match expr {
            &ast::Expr::Apply(ref a) => {
                if a.name == "&&" {
                    self.generate_and_and(out, &a.args[0], &a.args[1])
                } else if a.name == "||" {
                    self.generate_or_or(out, &a.args[0], &a.args[1])
                } else {
                    let mut res = Vec::new();
                    for arg in a.args.iter() {
                        let arg_out = self.push();
                        res.append(&mut self.generate_expr(arg_out, arg))
                    }
                    let mut arg_locations: Vec<VarRef> = a.args.iter().map(|_| self.pop()).collect();
                    arg_locations.reverse();
                    let mut me = self.apply_op(a, out, arg_locations);
                    res.append(&mut me);
                    res
                }
            },
            &ast::Expr::Int(ref i) => {
                let val = i.value.parse::<u16>().unwrap();
                vec![Instruction::StoreInt(out, val)]
            },
            &ast::Expr::If(ref i) => {
                let mut res = Vec::new();
                let mut then_branch = self.generate_block(out, &i.body);
                let mut else_branch = self.generate_block(out, &i.else_body);

                let condition_out = self.push();
                res.append(&mut self.generate_expr(condition_out, &(*i.condition)));
                self.gen_branch_into(&mut res, condition_out, &mut then_branch, &mut else_branch);
                res
            },
            &ast::Expr::Tuple(ref t) => {
                let mut res = Vec::new();
                for elem in t.elems.iter() {
                    let elem_out = self.push();
                    res.append(&mut self.generate_expr(elem_out, elem))
                }

                let mut elem_locations: Vec<VarRef> = t.elems.iter().map(|_| self.pop()).collect();
                elem_locations.reverse();
                let mut me = vec![Instruction::Tuple(out, t.elems.len() as u8, elem_locations)];
                res.append(&mut me);
                res
            },
            &ast::Expr::List(ref v) => {
                let mut res = Vec::new();
                for elem in v.elems.iter() {
                    let elem_out = self.push();
                    res.append(&mut self.generate_expr(elem_out, elem))
                }

                let mut elem_locations: Vec<VarRef> = v.elems.iter().map(|_| self.pop()).collect();
                elem_locations.reverse();
                let mut me = vec![Instruction::List(out, v.elems.len() as u8, elem_locations)];
                res.append(&mut me);
                res
            },
            &ast::Expr::Ident(ref i) => {
                match self.search_env(i.name) {
                    Some(reg) => vec![Instruction::Mov(out, reg)],
                    None => panic!("Undefined variable `{}`", i.name),
                }
            }
            &ast::Expr::True => {
                vec![Instruction::StoreTrue(out)]
            }
            &ast::Expr::False => {
                vec![Instruction::StoreFalse(out)]
            }
            &ast::Expr::Nil => {
                vec![Instruction::StoreNil(out)]
            }
            &ast::Expr::Str(ref string) => {
                vec![Instruction::LoadString(out, string.value.to_string())]
            }
            &ast::Expr::Capture(ref capture) => {
                let module = capture.module.unwrap_or(self.module_name);
                let name = format!("{}.{}", module, capture.name);
                vec![Instruction::Capture(out, name, capture.arity)]
            }
            &ast::Expr::Let(ref l) => {
                if self.identifier_exists_in_scope(l.left.name) {
                    panic!("Not allowed to rebind variable: {}", l.left.name);
                } else {
                    let var = self.push();
                    let generated = self.generate_expr(var, &(*l.right));
                    self.insert_env(l.left.name.to_string(), var);
                    generated
                }
            }
            &ast::Expr::AnonFn(ref anon) => {
                let mut function = FnGenerator::new(self.module_name, "anon", &anon.args, &anon.body, Some(self));
                let mut code = function.generate_code();

                let jmp = instruction::byte_size_of(&code) + 1;
                let mut instruction = vec![Instruction::AnonFn(out, jmp, anon.args.len() as u8, function.upvals.into_inner())];
                instruction.append(&mut code);
                instruction
            }
        }
    }

    fn apply_op(&mut self, ap: &ast::Apply, ret_loc: VarRef, args: Vec<VarRef>) -> Bytecode {
        match ap.name {
            "+" => vec![Instruction::Add(ret_loc, args[0], args[1])],
            "++" => vec![Instruction::Concat(ret_loc, args[0], args[1])],
            "-" => vec![Instruction::Sub(ret_loc, args[0], args[1])],
            "==" => vec![Instruction::Eq(ret_loc, args[0], args[1])],
            "!=" => vec![Instruction::NotEq(ret_loc, args[0], args[1])],
            "!" => vec![Instruction::Not(ret_loc, args[0])],
            ">" => vec![Instruction::GreaterThan(ret_loc, args[0], args[1])],
            "exit" => vec![Instruction::Exit(args[0])],
            "print" => vec![Instruction::Print(args[0])],
            "file_pwd" => vec![Instruction::FilePwd(ret_loc)],
            "file_ls" => vec![Instruction::FileLs(ret_loc, args[0])],
            "tuple_nth" => vec![Instruction::TupleNth(ret_loc, args[0], args[1])],
            "list_nth" => vec![Instruction::ListNth(ret_loc, args[0], args[1])],
            "list_count" => vec![Instruction::ListCount(ret_loc, args[0])],
            "list_slice" => vec![Instruction::ListSlice(ret_loc, args[0], args[1], args[2])],
            "string_slice" => vec![Instruction::StringSlice(ret_loc, args[0], args[1], args[2])],
            "string_count" => vec![Instruction::StringCount(ret_loc, args[0])],
            "code_load" => vec![Instruction::CodeLoad(ret_loc, args[0])],
            "function_name" => vec![Instruction::FunctionName(ret_loc, args[0])],
            "string_contains" => vec![Instruction::StringContains(ret_loc, args[0], args[1])],
            "term_to_string" => vec![Instruction::ToString(ret_loc, args[0])],
            "gc_collect" => vec![Instruction::GcCollect(ret_loc)],
            _   => {
                self.generic_apply(ap, ret_loc, args)
            }
        }
    }

    fn generic_apply(&mut self, ap: &ast::Apply, ret_loc: VarRef, args: Vec<VarRef>) -> Bytecode {
        match (ap.module, self.search_env(ap.name)) {
            (None, Some(var_ref)) => {
                vec![Instruction::CallLocal(ret_loc, var_ref, args)]
            },
            _ => {
                let module = ap.module.unwrap_or(self.module_name);
                let name = format!("{}.{}", module, ap.name);
                vec![Instruction::Call(ret_loc, name, ap.arity(), args)]
            }
        }
    }


    fn generate_and_and(&mut self, out: VarRef, left: &'a ast::Expr<'a>, right: &'a ast::Expr<'a>) -> Bytecode {
        let mut res = Vec::new();
        res.append(&mut self.generate_expr(out, left));

        let mut then_branch = self.generate_expr(out, right);
        let mut else_branch = Vec::new();

        self.gen_branch_into(&mut res, out, &mut then_branch, &mut else_branch);
        res
    }

    fn generate_or_or(&mut self, out: VarRef, left: &'a ast::Expr<'a>, right: &'a ast::Expr<'a>) -> Bytecode {
        let mut res = Vec::new();
        res.append(&mut self.generate_expr(out, left));

        let mut then_branch = Vec::new();
        let mut else_branch = self.generate_expr(out, right);

        self.gen_branch_into(&mut res, out, &mut then_branch, &mut else_branch);
        res
    }

    fn gen_branch_into(&mut self, code: &mut Bytecode, reg: VarRef, then_branch: &mut Bytecode, else_branch: &mut Bytecode) {
        let then_size = instruction::byte_size_of(&then_branch);
        if then_size > 0 {
            else_branch.push(Instruction::Jmp(then_size + 1));
        }

        let else_size = instruction::byte_size_of(&else_branch);

        code.push(Instruction::Test(reg, else_size + 1));
        code.append(else_branch);
        code.append(then_branch);
    }


    fn generate_block(&mut self, out: VarRef, block: &'a Vec<ast::Expr<'a>>) -> Bytecode {
        let mut buffer = Vec::new();

        if block.len() > 0 {
            for expr in block.iter() {
                buffer.append(&mut self.generate_expr(out, expr))
            }
        } else {
            buffer.push(Instruction::StoreNil(out));
        }

        buffer
    }

    fn push(&mut self) -> VarRef {
        self.var_count += 1;
        VarRef::Register(self.var_count)
    }

    fn pop(&mut self) -> VarRef {
        let val = self.var_count;
        self.var_count -= 1;
        VarRef::Register(val)
    }
}

pub fn generate_function(f: &ast::Function) -> Function {
    FnGenerator::new("unknown", f.name, &f.args, &f.body, None).generate()
}

pub fn generate(module: &ast::Module) -> Module {
    let mut functions = Vec::new();

    for f in module.functions.iter() {
        let generated = FnGenerator::new(module.name, f.name, &f.args, &f.body, None).generate();

        functions.push(generated);
    }

    Module {
        name: module.name.to_string(),
        functions: functions,
    }
}
