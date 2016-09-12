use std::io::Write;
use bytecode::instruction::Bytecode;
use bytecode::opcodes;

#[derive(Debug, Eq, PartialEq)]
pub struct Function {
    pub name: String,
    pub arity: u8,
    pub code: Bytecode
}

impl Function {
    pub fn emit<T: Write>(&self, out: &mut T) {
        let full_name = format!("{}\\{}", self.name, self.arity);
        let name_size = full_name.len() as u8;
        out.write(&[opcodes::PUB_FN, name_size + 1]).unwrap(); // +1 accounts for null termination
        out.write(&full_name.as_bytes()).unwrap();
        out.write(&[0]).unwrap(); // Null terminate the string

        for instr in self.code.iter() {
            instr.emit(out);
        }
    }

    pub fn emit_human_readable<T: Write>(&self, out: &mut T) {
        let header = format!("{}\\{}:\n", self.name, self.arity);
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

