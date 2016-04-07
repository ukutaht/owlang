use ast;

mod opcodes;
mod function;
mod module;
mod instruction;

pub use self::instruction::{Bytecode, Instruction, Reg};
pub use self::function::Function;
pub use self::module::Module;


pub struct GenContext<'a> {
    var_count: u8,
    function: &'a ast::Function<'a>
}

impl<'a> GenContext<'a> {
    fn new<'b>(f: &'b ast::Function) -> GenContext<'b> {
        GenContext {
            function: f,
            var_count: f.arity()
        }
    }

    fn generate(&mut self) -> Function<'a> {
        let code = self.generate_code();

        Function {
            name: self.function.name,
            arity: self.function.arity(),
            code: code
        }
    }

    fn generate_code(&mut self) -> Bytecode {
        let mut code = Vec::new();

        for expr in self.function.body.iter() {
            let mut generated = self.generate_expr(expr);
            code.append(&mut generated);
            code.push(Instruction::Mov(0, self.var_count))
        }

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

                let mut me = self.apply_op(a.name, ret_loc, arg_locations);
                res.append(&mut me);
                res
            },
            &ast::Expr::Int(ref i) => {
                let val = i.value.parse::<u16>().unwrap();
                vec![Instruction::Store(self.push(), val)]
            },
            &ast::Expr::If(ref i) => {
                let mut res = Vec::new();
                let mut else_branch = vec![Instruction::Exit];

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
            _ => panic!("WAT")
        }
    }

    fn apply_op(&self, name: &'a str, ret_loc: Reg, args: Vec<Reg>) -> Bytecode {
        match name {
            "+" => vec![Instruction::Add(ret_loc, args[0], args[1])],
            "-" => vec![Instruction::Sub(ret_loc, args[0], args[1])],
            "print" => vec![Instruction::Print(args[0])],
            _   => panic!("Unknown function {}", name)
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
    GenContext::new(f).generate()
}

pub fn generate<'a>(module: &'a ast::Module) -> Module<'a> {
    let mut functions = Vec::new();

    for f in module.functions.iter() {
        functions.push(GenContext::new(f).generate())
    }

    Module {
        name: module.name,
        functions: functions
    }
}
