use ast;

mod opcodes;
mod function;
mod module;
mod instruction;

pub use self::instruction::{Bytecode, Instruction, Reg};
pub use self::function::Function;
pub use self::module::Module;

use std::collections::HashMap;

type Signature = (String, u8);
type Location = u8;

pub struct GenContext {
    function_defs: HashMap<Signature, Location>,
}

impl GenContext {
    fn new() -> GenContext {
        GenContext {
            function_defs: HashMap::new(),
        }
    }

    fn add_def(&mut self, def: Signature, location: u8) {
        self.function_defs.insert(def, location);
    }

    fn find_location(&self, def: &Signature) -> Option<&u8> {
        self.function_defs.get(def)
    }
}


struct FnGenerator<'a> {
    var_count: u8,
    function: &'a ast::Function<'a>,
}

impl<'a> FnGenerator<'a> {
    fn new<'b>(f: &'b ast::Function) -> FnGenerator<'b> {
        FnGenerator {
            function: f,
            var_count: f.arity()
        }
    }

    fn generate(&mut self, gen_ctx: &mut GenContext) -> Function<'a> {
        let code = self.generate_code(gen_ctx);

        Function {
            name: self.function.name,
            arity: self.function.arity(),
            code: code
        }
    }

    fn generate_code(&mut self, gen_ctx: &mut GenContext) -> Bytecode {
        let mut code = Vec::new();

        for expr in self.function.body.iter() {
            let mut generated = self.generate_expr(expr, gen_ctx);
            code.append(&mut generated);
            code.push(Instruction::Mov(0, self.var_count));
            code.push(self.return_op());
        }

        code
    }

    fn generate_expr(&mut self, expr: &'a ast::Expr, gen_ctx: &mut GenContext) -> Bytecode {
        match expr {
            &ast::Expr::Apply(ref a) => {
                let mut res = Vec::new();
                for arg in a.args.iter() {
                    res.append(&mut self.generate_expr(arg, gen_ctx))
                }
                let mut arg_locations: Vec<Reg> = a.args.iter().map(|_| self.pop()).collect();
                arg_locations.reverse();
                let ret_loc  = self.push();

                let mut me = self.apply_op(a.name, a.arity(), ret_loc, arg_locations, gen_ctx);
                res.append(&mut me);
                res
            },
            &ast::Expr::Int(ref i) => {
                let val = i.value.parse::<u16>().unwrap();
                vec![Instruction::Store(self.push(), val)]
            },
            &ast::Expr::If(ref i) => {
                let mut res = Vec::new();
                let mut else_branch = vec![self.return_op()];

                match *i.condition {
                    ast::Expr::Apply(ref a) => {
                        match a.name {
                            ">" => {
                                res.append(&mut self.generate_expr(&a.args[0], gen_ctx));
                                res.append(&mut self.generate_expr(&a.args[1], gen_ctx));
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
                    res.append(&mut self.generate_expr(expr, gen_ctx))
                }
                res
            },
            _ => panic!("WAT")
        }
    }

    fn return_op(&self) -> Instruction {
        if self.function.name == "main" {
            Instruction::Exit
        } else {
            Instruction::Return
        }
    }

    fn apply_op(&self, name: &'a str, arity: u8, ret_loc: Reg, args: Vec<Reg>, gen_ctx: &mut GenContext) -> Bytecode {
        match name {
            "+" => vec![Instruction::Add(ret_loc, args[0], args[1])],
            "-" => vec![Instruction::Sub(ret_loc, args[0], args[1])],
            "print" => vec![Instruction::Print(args[0])],
            _   => {
                match gen_ctx.find_location(&(name.to_string(), arity)) {
                    Some(loc) => vec![Instruction::Call(ret_loc, *loc, arity, args)],
                    _ => panic!("Function not found: {}\\{}", name, arity)
                }
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

pub fn generate_function<'a>(f: &'a ast::Function) -> Function<'a> {
    FnGenerator::new(f).generate(&mut GenContext::new())
}

pub fn generate<'a>(module: &'a ast::Module) -> Module<'a> {
    let mut functions = Vec::new();
    let mut gen_ctx   = GenContext::new();
    let mut location  = 2;
    let mut main_loc  = None;

    for f in module.functions.iter() {
        gen_ctx.add_def((f.name.to_string(), f.arity()), location);
        let generated = FnGenerator::new(f).generate(&mut gen_ctx);

        if f.name == "main" {
            main_loc = Some(location);
        }

        location += generated.byte_size();
        functions.push(generated);
    }

    Module {
        name: module.name,
        functions: functions,
        main_location: main_loc.expect("No main module defined")
    }
}
