use ast;

use std::collections::HashMap;

mod opcodes;
mod function;
mod module;
mod instruction;

pub use self::instruction::{Bytecode, Instruction, Reg};
pub use self::function::Function;
pub use self::module::Module;

type Signature = (String, u8);

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
            let mut generated = self.generate_expr(expr);
            code.append(&mut generated);
        }

        code.push(Instruction::Mov(0, self.var_count));
        code.push(Instruction::Return);
        code
    }

    fn generate_expr(&mut self, expr: &'a ast::Expr) -> Bytecode {
        match expr {
            &ast::Expr::Apply(ref a) => {
                let mut res = Vec::new();
                for arg in a.args.iter() {
                    res.append(&mut self.generate_expr(arg))
                }
                let mut arg_locations: Vec<Reg> = a.args.iter().map(|_| self.pop()).collect();
                arg_locations.reverse();
                let ret_loc  = self.push();

                let mut me = self.apply_op(a, ret_loc, arg_locations);
                res.append(&mut me);
                res
            },
            &ast::Expr::Int(ref i) => {
                let val = i.value.parse::<u16>().unwrap();
                vec![Instruction::Store(self.push(), val)]
            },
            &ast::Expr::If(ref i) => {
                let mut res = Vec::new();
                let mut else_branch = vec![Instruction::Return];

                match *i.condition {
                    ast::Expr::Apply(ref a) => {
                        match a.name {
                            ">" => {
                                res.append(&mut self.generate_expr(&a.args[0]));
                                res.append(&mut self.generate_expr(&a.args[1]));
                                let arg2 = self.pop();
                                let arg1 = self.pop();
                                let jump = instruction::byte_size_of(&else_branch);
                                res.push(Instruction::TestGt(arg1, arg2, jump));
                            },
                            _ => panic!("Cannot generate if statement")
                        }
                    }
                    _ => panic!("Cannot generate if statement")
                }

                res.append(&mut else_branch);
                for expr in i.body.iter() {
                    res.append(&mut self.generate_expr(expr))
                }
                res
            },
            &ast::Expr::Tuple(ref t) => {
                let mut res = Vec::new();
                for elem in t.elems.iter() {
                    res.append(&mut self.generate_expr(elem))
                }

                let mut elem_locations: Vec<Reg> = t.elems.iter().map(|_| self.pop()).collect();
                elem_locations.reverse();
                let reg = self.push();

                let mut me = vec![Instruction::Tuple(reg, t.elems.len() as u8, elem_locations)];
                res.append(&mut me);
                res
            },
            &ast::Expr::Vector(ref v) => {
                let mut res = Vec::new();
                for elem in v.elems.iter() {
                    res.append(&mut self.generate_expr(elem))
                }

                let mut elem_locations: Vec<Reg> = v.elems.iter().map(|_| self.pop()).collect();
                elem_locations.reverse();
                let reg = self.push();

                let mut me = vec![Instruction::Vector(reg, v.elems.len() as u8, elem_locations)];
                res.append(&mut me);
                res
            },
            &ast::Expr::Ident(ref i) => {
                vec![Instruction::Mov(self.push(), *self.env.get(i.name).unwrap())]
            }
            &ast::Expr::True => {
                vec![Instruction::StoreTrue(self.push())]
            }
            &ast::Expr::False => {
                panic!("False");
            }
        }
    }

    fn apply_op(&self, ap: &ast::Apply, ret_loc: Reg, args: Vec<Reg>) -> Bytecode {
        match ap.name {
            "+" => vec![Instruction::Add(ret_loc, args[0], args[1])],
            "-" => vec![Instruction::Sub(ret_loc, args[0], args[1])],
            "print" => vec![Instruction::Print(args[0])],
            "tuple_nth" => vec![Instruction::TupleNth(ret_loc, args[0], args[1])],
            "assert_eq" => vec![Instruction::AssertEq(args[0], args[1])],
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
