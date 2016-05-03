use std::io::Write;
use bytecode::function::Function;
use bytecode::instruction::Instruction;

#[derive(Debug, Eq, PartialEq)]
pub struct Module {
    pub name: String,
    pub functions: Vec<Function>,
}

impl Module {
    pub fn emit<T: Write>(&self, out: &mut T) {
        let main = format!("{}:{}", self.name, "main");
        Instruction::Call(0, main, 0, Vec::new()).emit(out);
        Instruction::Exit.emit(out);

        for function in self.functions.iter() {
            function.emit(out);
        }
    }

    pub fn emit_human_readable<T: Write>(&self, out: &mut T) {
        out.write(format!("module {}\n", self.name).as_bytes()).unwrap();

        let main = format!("{}:{}", self.name, "main");
        Instruction::Call(0, main, 0, Vec::new()).emit_human_readable(out);
        Instruction::Exit.emit_human_readable(out);

        for function in self.functions.iter() {
            function.emit_human_readable(out);
        }
        out.write("\n".as_bytes()).unwrap();
    }
}

