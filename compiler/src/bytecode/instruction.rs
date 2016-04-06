use bytecode::opcodes;
use std::io::Write;

pub type Reg = u8;
pub type Bytecode = Vec<Instruction>;

#[derive(Debug, Eq, PartialEq)]
pub enum Instruction {
    Exit,
    Add(Reg, Reg, Reg),
    Sub(Reg, Reg, Reg),
    Store(Reg, u16),
    Mov(Reg, Reg),
    Print(Reg),
    TestGt(Reg, Reg, u8),
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
            },
            &Instruction::Print(reg) => {
                out.write(&[opcodes::PRINT, reg]).unwrap();
            },
            &Instruction::TestGt(reg1, reg2, jump) => {
                out.write(&[opcodes::TEST_GT, reg1, reg2, jump]).unwrap();
            }
            &Instruction::Exit => {
                out.write(&[opcodes::EXIT]).unwrap();
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
            &Instruction::Print(reg) => {
                let string = format!("print %{}\n", reg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::TestGt(reg1, reg2, jump) => {
                let string = format!("test_gt %{}, %{}, {}\n", reg1, reg2, jump);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Exit => {
                out.write(b"exit\n").unwrap();
            }
        }
    }

    pub fn byte_size(&self) -> u8 {
        match self {
            &Instruction::Add(_, _, _)    => 4,
            &Instruction::Sub(_, _, _)    => 4,
            &Instruction::Store(_, _)     => 4,
            &Instruction::Mov(_, _)       => 3,
            &Instruction::Print(_)        => 2,
            &Instruction::TestGt(_, _, _) => 4,
            &Instruction::Exit            => 1,
        }
    }
}

pub fn byte_size_of(instructions: &Vec<Instruction>) -> u8 {
    instructions.iter().fold(1, |acc, x| acc + x.byte_size())
}
