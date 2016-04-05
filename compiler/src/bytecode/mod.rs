use ast;

mod opcodes;
use std::io::Write;

pub type Reg = u8;

#[derive(Debug, Eq, PartialEq)]
pub enum Instruction {
    Add(Reg, Reg, Reg),
    Sub(Reg, Reg, Reg),
    Store(Reg, u16),
    Mov(Reg, Reg),
}

impl Instruction {
    pub fn emit<'a, T: Write>(&self, out: &'a mut T) {
        match self {
            &Instruction::Add(to, arg1, arg2) => {
                out.write(&[opcodes::ADD, to, arg1, arg2]).unwrap();
            },
            &Instruction::Sub(to, arg1, arg2) => {
                out.write(&[opcodes::SUB, to, arg1, arg2]).unwrap();
            },
            &Instruction::Store(to, val) => {
                let first = val % 250;
                let second = val / 250;
                out.write(&[opcodes::STORE, to, first as u8, second as u8]).unwrap();
            }
            &Instruction::Mov(to, from) => {
                out.write(&[opcodes::MOV, to, from]).unwrap();
            }
        }
    }

    pub fn emit_human_readable<'a, T: Write>(&self, out: &'a mut T) {
        match self {
            &Instruction::Add(to, arg1, arg2) => {
                let string = format!("add %{}, %{}, %{}\n", to, arg1, arg2);
                out.write(&string.as_bytes()).unwrap();
            },
            &Instruction::Sub(to, arg1, arg2) => {
                let string = format!("sub %{}, %{}, %{}\n", to, arg1, arg2);
                out.write(&string.as_bytes()).unwrap();
            },
            &Instruction::Store(to, val) => {
                let string = format!("store %{}, {}\n", to, val);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Mov(to, from) => {
                let string = format!("mov %{}, %{}\n", to, from);
                out.write(&string.as_bytes()).unwrap();
            }
        }
    }
}

pub type Bytecode = Vec<Instruction>;

#[derive(Debug, Eq, PartialEq)]
pub struct Function<'a> {
    pub name: &'a str,
    pub arity: u8,
    pub code: Bytecode
}

impl<'a> Function<'a> {
    pub fn emit<T: Write>(&self, out: &'a mut T) {
        for instr in self.code.iter() {
            instr.emit(out);
        }
    }

    pub fn emit_human_readable<T: Write>(&self, out: &'a mut T) {
        let header = format!("{}/{}:\n", self.name, self.arity);
        out.write(&header.as_bytes()).unwrap();

        for instr in self.code.iter() {
            out.write(b"  ").unwrap();
            instr.emit_human_readable(out);
        }
    }
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

                let mut me = self.apply_op(a.name, ret_loc, arg1_loc, arg2_loc);
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

    fn apply_op(&self, name: &'a str, ret_loc: Reg, arg1_loc: Reg, arg2_loc: Reg) -> Bytecode {
        match name {
            "+" => vec![Instruction::Add(ret_loc, arg1_loc, arg2_loc)],
            "-" => vec![Instruction::Sub(ret_loc, arg1_loc, arg2_loc)],
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

pub fn generate<'a>(ast: &'a ast::Expr) -> Function<'a> {
    match ast {
        &ast::Expr::Function(ref f) => GenContext::new(f).generate(),
        _ => panic!("Need function here")
    }
}
