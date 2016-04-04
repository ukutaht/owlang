use ast;

pub type Reg = u8;

#[derive(Debug, Eq, PartialEq)]
pub enum Instruction {
    Add(Reg, Reg, Reg),
    Store(Reg, u16),
    Mov(Reg, Reg)
}

pub type Bytecode = Vec<Instruction>;

#[derive(Debug, Eq, PartialEq)]
pub struct Function<'a> {
    pub name: &'a str,
    pub arity: u8,
    pub code: Bytecode
}

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
                let mut arg1 = self.generate_expr(&a.args[0]);
                let mut arg2 = self.generate_expr(&a.args[1]);
                let arg2_loc = self.pop();
                let arg1_loc = self.pop();
                let ret_loc  = self.push();

                let mut me = vec![Instruction::Add(ret_loc, arg1_loc, arg2_loc)];
                arg1.append(&mut arg2);
                arg1.append(&mut me);
                arg1
            },
            &ast::Expr::Int(ref i) => {
                let val = i.value.parse::<u16>().unwrap();
                vec![Instruction::Store(self.push(), val)]
            },
            _ => panic!("WAT")
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

pub fn generate<'a>(ast: &'a ast::Expr) -> Function<'a> {
    match ast {
        &ast::Expr::Function(ref f) => GenContext::new(f).generate(),
        _ => panic!("Need function here")
    }
}
