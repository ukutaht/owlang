use std::io::Write;
use bytecode::instruction::Bytecode;

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

    pub fn byte_size(&self) -> u8 {
        self.code.iter().fold(0, |acc, instr| acc + instr.byte_size())
    }
}

