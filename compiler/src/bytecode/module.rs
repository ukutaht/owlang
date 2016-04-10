use std::io::Write;
use bytecode::function::Function;
use bytecode::instruction::Instruction;

#[derive(Debug, Eq, PartialEq)]
pub struct Module<'a> {
    pub name: &'a str,
    pub functions: Vec<Function<'a>>,
    pub main_location: u8,
}

impl<'a> Module<'a> {
    pub fn emit<T: Write>(&self, out: &'a mut T) {
        Instruction::Jmp(self.main_location).emit(out);

        for function in self.functions.iter() {
            function.emit(out);
        }
    }

    pub fn emit_human_readable<T: Write>(&self, out: &'a mut T) {
        Instruction::Jmp(self.main_location).emit_human_readable(out);

        for function in self.functions.iter() {
            function.emit_human_readable(out);
        }
    }
}

