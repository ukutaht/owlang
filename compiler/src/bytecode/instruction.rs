use std::io::Write;
use bytecode::opcodes;

pub type Reg = u8;
pub type Bytecode = Vec<Instruction>;

#[derive(Debug, Eq, PartialEq)]
pub enum Instruction {
    Exit,
    Store(Reg, u16),
    Print(Reg),
    TestGt(Reg, Reg, u8),
    Add(Reg, Reg, Reg),
    Sub(Reg, Reg, Reg),
    Call(u8, String, u8, Vec<Reg>),
    Return,
    Mov(Reg, Reg),
    Jmp(u8),
    Tuple(Reg, u8, Vec<Reg>),
    TupleNth(Reg, Reg, u8),
    AssertEq(Reg, Reg),
    Vector(Reg, u8, Vec<Reg>),
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
            },
            &Instruction::Call(ret_loc, ref name, arity, ref regs) => {
                let full_name = format!("{}/{}", name, arity);
                let name_size = full_name.len() as u8;
                out.write(&[opcodes::CALL, ret_loc, name_size + 1]).unwrap(); // +1 accounts for null termination
                out.write(&full_name.as_bytes()).unwrap();
                out.write(&[0]).unwrap(); // Null terminate the string
                out.write(&[arity]).unwrap();

                let copied_regs = regs.clone();
                out.write(&copied_regs).unwrap();
            }
            &Instruction::Jmp(loc) => {
                out.write(&vec![opcodes::JMP, loc]).unwrap();
            }
            &Instruction::Return => {
                out.write(&vec![opcodes::RETURN]).unwrap();
            }
            &Instruction::Tuple(reg, size, ref elems) => {
                let mut res = vec![opcodes::TUPLE, reg, size];
                res.append(&mut elems.clone());

                out.write(&res).unwrap();
            }
            &Instruction::TupleNth(dest, reg, nth) => {
                out.write(&[opcodes::TUPLE_NTH, dest, reg, nth]).unwrap();
            },
            &Instruction::AssertEq(reg1, reg2) => {
                out.write(&[opcodes::ASSERT_EQ, reg1, reg2]).unwrap();
            },
            &Instruction::Vector(reg, size, ref elems) => {
                let mut res = vec![opcodes::VECTOR, reg, size];
                res.append(&mut elems.clone());

                out.write(&res).unwrap();
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
            &Instruction::Call(ret_loc, ref name, arity, ref regs) => {
                let string;

                if regs.len() > 0 {
                    let args: Vec<_> = regs.iter().map(|int| format!("%{}", int)).collect();
                    string = format!("call %{}, {}, %{}, {}\n", ret_loc, name, arity, args.join(", "));
                } else {
                    string = format!("call %{}, {}, {}\n", ret_loc, name, arity);
                }

                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Jmp(loc) => {
                let string = format!("jmp {}\n", loc);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Return => {
                out.write(b"return\n").unwrap();
            }
            &Instruction::Tuple(reg, size, ref regs) => {
                let string;

                if regs.len() > 0 {
                    let elems: Vec<_> = regs.iter().map(|int| format!("%{}", int)).collect();
                    string = format!("tuple %{}, {}, {}\n", reg, size, elems.join(", "));
                } else {
                    string = format!("tuple %{}, {}\n", reg, size);
                }

                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::TupleNth(dest, reg, nth) => {
                let string = format!("tuple_nth %{}, %{}, %{}\n", dest, reg, nth);
                out.write(&string.as_bytes()).unwrap();
            },
            &Instruction::AssertEq(reg1, reg2) => {
                let string = format!("assert_eq %{}, %{}\n", reg1, reg2);
                out.write(&string.as_bytes()).unwrap();
            },
            &Instruction::Vector(reg, size, ref regs) => {
                let string;

                if regs.len() > 0 {
                    let elems: Vec<_> = regs.iter().map(|int| format!("%{}", int)).collect();
                    string = format!("vector %{}, {}, {}\n", reg, size, elems.join(", "));
                } else {
                    string = format!("vector %{}, {}\n", reg, size);
                }

                out.write(&string.as_bytes()).unwrap();
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
            &Instruction::Call(_, ref name, _, ref regs) => 5 + (name.len() as u8) + (regs.len() as u8),
            &Instruction::Jmp(_)          => 2,
            &Instruction::Tuple(_, _, ref regs)   => 3 + regs.len() as u8,
            &Instruction::TupleNth(_, _, _)  => 4,
            &Instruction::Return          => 1,
            &Instruction::AssertEq(_, _)  => 3,
            &Instruction::Vector(_, _, ref regs)   => 3 + regs.len() as u8,
        }
    }
}

pub fn byte_size_of(instructions: &Vec<Instruction>) -> u8 {
    instructions.iter().fold(1, |acc, x| acc + x.byte_size())
}