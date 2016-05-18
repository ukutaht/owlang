use ast;

use std::collections::HashMap;

mod opcodes;
mod function;
mod module;
mod instruction;

pub use self::instruction::{Bytecode, Instruction, Reg};
pub use self::function::Function;
pub use self::module::Module;

struct FnGenerator<'a> {
    var_count: u8,
    function: &'a ast::Function<'a>,
    module_name: &'a str,
    env: HashMap<String, Reg>,
}

impl<'a> FnGenerator<'a> {
    fn new<'b>(module_name: &'b str, f: &'b ast::Function) -> FnGenerator<'b> {
        let mut env = HashMap::new();
        let mut var_count: u8 = 0;

        for arg in f.args.iter() {
            var_count += 1;
            env.insert(arg.name.to_string(), var_count);
        }

        FnGenerator {
            function: f,
            var_count: var_count,
            module_name: module_name,
            env: env
        }
    }

    fn generate(&mut self) -> Function {
        let code = self.generate_code();
        let name = format!("{}:{}", self.module_name, self.function.name);

        Function {
            name: name,
            arity: self.function.arity(),
            code: code
        }
    }

    fn generate_code(&mut self) -> Bytecode {
        let mut code = Vec::new();

        for expr in self.function.body.iter() {
            let mut generated = self.generate_expr(0, expr);
            code.append(&mut generated);
        }

        code.push(Instruction::Return);
        code
    }

    fn generate_expr(&mut self, out: Reg, expr: &'a ast::Expr) -> Bytecode {
        match expr {
            &ast::Expr::Apply(ref a) => {
                let mut res = Vec::new();
                for arg in a.args.iter() {
                    let arg_out = self.push();
                    res.append(&mut self.generate_expr(arg_out, arg))
                }
                let mut arg_locations: Vec<Reg> = a.args.iter().map(|_| self.pop()).collect();
                arg_locations.reverse();
                let mut me = self.apply_op(a, out, arg_locations);
                res.append(&mut me);
                res
            },
            &ast::Expr::Int(ref i) => {
                let val = i.value.parse::<u16>().unwrap();
                vec![Instruction::StoreInt(out, val)]
            },
            &ast::Expr::If(ref i) => {
                let mut res = Vec::new();
                let mut then_branch = Vec::new();
                let mut else_branch = Vec::new();

                if i.body.len() > 0 {
                    for expr in i.body.iter() {
                        then_branch.append(&mut self.generate_expr(out, expr))
                    }
                } else {
                    then_branch.push(Instruction::StoreInt(out, 0));
                }

                if i.else_body.len() > 0 {
                    for expr in i.else_body.iter() {
                        else_branch.append(&mut self.generate_expr(out, expr))
                    }
                } else {
                    else_branch.push(Instruction::StoreInt(out, 0));
                }

                let then_size = instruction::byte_size_of(&then_branch);
                else_branch.push(Instruction::Jmp(then_size + 1));
                let else_size = instruction::byte_size_of(&else_branch);
                let condition_out = self.push();
                res.append(&mut self.generate_expr(condition_out, &(*i.condition)));
                res.push(Instruction::Test(self.pop(), else_size + 1));

                res.append(&mut else_branch);
                res.append(&mut then_branch);
                res
            },
            &ast::Expr::Tuple(ref t) => {
                let mut res = Vec::new();
                for elem in t.elems.iter() {
                    let elem_out = self.push();
                    res.append(&mut self.generate_expr(elem_out, elem))
                }

                let mut elem_locations: Vec<Reg> = t.elems.iter().map(|_| self.pop()).collect();
                elem_locations.reverse();
                let mut me = vec![Instruction::Tuple(out, t.elems.len() as u8, elem_locations)];
                res.append(&mut me);
                res
            },
            &ast::Expr::Vector(ref v) => {
                let mut res = Vec::new();
                for elem in v.elems.iter() {
                    let elem_out = self.push();
                    res.append(&mut self.generate_expr(elem_out, elem))
                }

                let mut elem_locations: Vec<Reg> = v.elems.iter().map(|_| self.pop()).collect();
                elem_locations.reverse();
                let mut me = vec![Instruction::Vector(out, v.elems.len() as u8, elem_locations)];
                res.append(&mut me);
                res
            },
            &ast::Expr::Ident(ref i) => {
                vec![Instruction::Mov(out, *self.env.get(i.name).unwrap())]
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
            &ast::Expr::Let(ref l) => {
                if self.env.contains_key(l.left.name) {
                    panic!("Not allowed to rebind variable: {}", l.left.name);
                } else {
                    let var = self.push();
                    self.env.insert(l.left.name.to_string(), var);
                    self.generate_expr(var, &(*l.right))
                }
            }
        }
    }

    fn apply_op(&self, ap: &ast::Apply, ret_loc: Reg, args: Vec<Reg>) -> Bytecode {
        match ap.name {
            "+" => vec![Instruction::Add(ret_loc, args[0], args[1])],
            "-" => vec![Instruction::Sub(ret_loc, args[0], args[1])],
            "==" => vec![Instruction::Eq(ret_loc, args[0], args[1])],
            "!=" => vec![Instruction::NotEq(ret_loc, args[0], args[1])],
            "!" => vec![Instruction::Not(ret_loc, args[0])],
            "exit" => vec![Instruction::Exit(args[0])],
            "print" => vec![Instruction::Print(args[0])],
            "tuple_nth" => vec![Instruction::TupleNth(ret_loc, args[0], args[1])],
            _   => {
                let module = ap.module.unwrap_or(self.module_name);
                let name = format!("{}:{}", module, ap.name);
                vec![Instruction::Call(ret_loc, name, ap.arity(), args)]
            }
        }
    }

    fn push(&mut self) -> u8 {
        self.var_count += 1;
        self.var_count
    }

    fn pop(&mut self) -> u8 {
        let val = self.var_count;
        self.var_count -= 1;
        val
    }
}

pub fn generate_function(f: &ast::Function) -> Function {
    FnGenerator::new("unknown", f).generate()
}

pub fn generate(module: &ast::Module) -> Module {
    let mut functions = Vec::new();

    for f in module.functions.iter() {
        let generated = FnGenerator::new(module.name, f).generate();

        functions.push(generated);
    }

    Module {
        name: module.name.to_string(),
        functions: functions,
    }
}
