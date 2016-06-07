use std::io::Write;
use bytecode::opcodes;

pub type Reg = u8;
pub type Length = u8;
pub type Arity = u8;
pub type Jump = u8;
pub type Bytecode = Vec<Instruction>;

#[derive(Debug, Eq, PartialEq)]
pub enum Instruction {
    Exit(Reg),
    StoreInt(Reg, u16),
    Print(Reg),
    Test(Reg, Jump),
    Add(Reg, Reg, Reg),
    Sub(Reg, Reg, Reg),
    Call(Reg, String, Arity, Vec<Reg>),
    Return,
    Mov(Reg, Reg),
    Jmp(Jump),
    Tuple(Reg, Length, Vec<Reg>),
    TupleNth(Reg, Reg, Reg),
    List(Reg, Length, Vec<Reg>),
    ListNth(Reg, Reg, Reg),
    StoreTrue(Reg),
    StoreFalse(Reg),
    StoreNil(Reg),
    Eq(Reg, Reg, Reg),
    NotEq(Reg, Reg, Reg),
    Not(Reg, Reg),
    GreaterThan(Reg, Reg, Reg),
    LoadString(Reg, String),
    FilePwd(Reg),
    FileLs(Reg, Reg),
    Concat(Reg, Reg, Reg),
    Capture(Reg, String, Arity),
    CallLocal(Reg, Reg, Vec<Reg>),
    ListCount(Reg, Reg),
    ListSlice(Reg, Reg, Reg, Reg),
    StringSlice(Reg, Reg, Reg, Reg),
    CodeLoad(Reg, Reg),
    CallByName(Reg, Reg, Reg),
    StringCount(Reg, Reg),
    StringContains(Reg, Reg, Reg),
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
            &Instruction::StoreInt(to, val) => {
                let first = val % 250;
                let second = val / 250;
                out.write(&[opcodes::STORE_INT, to, first as u8, second as u8]).unwrap();
            }
            &Instruction::Mov(to, from) => {
                out.write(&[opcodes::MOV, to, from]).unwrap();
            },
            &Instruction::Concat(to, left, right) => {
                out.write(&[opcodes::CONCAT, to, left, right]).unwrap();
            },
            &Instruction::CallByName(result, name, args) => {
                out.write(&[opcodes::CALL_BY_NAME, result, name, args]).unwrap();
            },
            &Instruction::Print(reg) => {
                out.write(&[opcodes::PRINT, reg]).unwrap();
            },
            &Instruction::Test(reg, jump) => {
                out.write(&[opcodes::TEST, reg, jump]).unwrap();
            }
            &Instruction::Exit(reg) => {
                out.write(&[opcodes::EXIT, reg]).unwrap();
            },
            &Instruction::FilePwd(reg) => {
                out.write(&[opcodes::FILE_PWD, reg]).unwrap();
            },
            &Instruction::FileLs(reg, path) => {
                out.write(&[opcodes::FILE_LS, reg, path]).unwrap();
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
            &Instruction::CallLocal(ret_loc, func_loc, ref regs) => {
                out.write(&[opcodes::CALL_LOCAL, ret_loc, func_loc]).unwrap();
                out.write(&[regs.len() as u8]).unwrap();

                let copied_regs = regs.clone();
                out.write(&copied_regs).unwrap();
            }
            &Instruction::Capture(ret_loc, ref name, arity) => {
                let full_name = format!("{}/{}", name, arity);
                let name_size = full_name.len() as u8;
                out.write(&[opcodes::CAPTURE, ret_loc, name_size + 1]).unwrap(); // +1 accounts for null termination
                out.write(&full_name.as_bytes()).unwrap();
                out.write(&[0]).unwrap(); // Null terminate the string
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
            &Instruction::ListNth(dest, reg1, reg2) => {
                out.write(&[opcodes::LIST_NTH, dest, reg1, reg2]).unwrap();
            },
            &Instruction::List(reg, size, ref elems) => {
                let mut res = vec![opcodes::LIST, reg, size];
                res.append(&mut elems.clone());

                out.write(&res).unwrap();
            }
            &Instruction::StoreTrue(reg) => {
                out.write(&[opcodes::STORE_TRUE, reg]).unwrap();
            }
            &Instruction::StoreFalse(reg) => {
                out.write(&[opcodes::STORE_FALSE, reg]).unwrap();
            }
            &Instruction::StoreNil(reg) => {
                out.write(&[opcodes::STORE_NIL, reg]).unwrap();
            }
            &Instruction::Eq(to, reg1, reg2) => {
                out.write(&[opcodes::EQ, to, reg1, reg2]).unwrap();
            }
            &Instruction::NotEq(to, reg1, reg2) => {
                out.write(&[opcodes::NOT_EQ, to, reg1, reg2]).unwrap();
            }
            &Instruction::Not(to, reg) => {
                out.write(&[opcodes::NOT, to, reg]).unwrap();
            }
            &Instruction::ListCount(to, reg) => {
                out.write(&[opcodes::LIST_COUNT, to, reg]).unwrap();
            }
            &Instruction::StringCount(to, reg) => {
                out.write(&[opcodes::STRING_COUNT, to, reg]).unwrap();
            }
            &Instruction::StringContains(to, string, substr) => {
                out.write(&[opcodes::STRING_CONTAINS, to, string, substr]).unwrap();
            }
            &Instruction::CodeLoad(to, reg) => {
                out.write(&[opcodes::CODE_LOAD, to, reg]).unwrap();
            }
            &Instruction::ListSlice(ret, reg, from, to) => {
                out.write(&[opcodes::LIST_SLICE, ret, reg, from, to]).unwrap();
            }
            &Instruction::StringSlice(ret, reg, from, to) => {
                out.write(&[opcodes::STRING_SLICE, ret, reg, from, to]).unwrap();
            }
            &Instruction::LoadString(to, ref content) => {
                let content_size = content.len() as u8;
                out.write(&[opcodes::LOAD_STRING, to, content_size + 1]).unwrap();; // +1 accounts for null termination
                out.write(&content.as_bytes()).unwrap();
                out.write(&[0]).unwrap(); // Null terminate the string
            }
            &Instruction::GreaterThan(to, arg1, arg2) => {
                out.write(&[opcodes::GREATER_THAN, to, arg1, arg2]).unwrap();
            },
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
            &Instruction::StoreInt(to, val) => {
                let string = format!("store_int %{}, {}\n", to, val);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Mov(to, from) => {
                let string = format!("mov %{}, %{}\n", to, from);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Concat(to, left, right) => {
                let string = format!("concat %{}, %{}, %{}\n", to, left, right);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::CallByName(to, name, args) => {
                let string = format!("call_by_name %{}, %{}, %{}\n", to, name, args);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::StringContains(to, string, substr) => {
                let string = format!("string_contains %{}, %{}, %{}\n", to, string, substr);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Print(reg) => {
                let string = format!("print %{}\n", reg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Test(reg, jump) => {
                let string = format!("test %{}, {}\n", reg, jump);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Exit(reg) => {
                let string = format!("exit %{}\n", reg);
                out.write(string.as_bytes()).unwrap();
            }
            &Instruction::FilePwd(reg) => {
                let string = format!("file_pwd %{}\n", reg);
                out.write(string.as_bytes()).unwrap();
            }
            &Instruction::FileLs(reg, path) => {
                let string = format!("file_ls %{}, %{}\n", reg, path);
                out.write(string.as_bytes()).unwrap();
            }
            &Instruction::Call(ret_loc, ref name, arity, ref regs) => {
                let string;

                if regs.len() > 0 {
                    let args: Vec<_> = regs.iter().map(|int| format!("%{}", int)).collect();
                    string = format!("call %{}, {}/{}, {}\n", ret_loc, name, arity, args.join(", "));
                } else {
                    string = format!("call %{}, {}/{}\n", ret_loc, name, arity);
                }

                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::CallLocal(ret_loc, func_loc, ref regs) => {
                let string;

                if regs.len() > 0 {
                    let args: Vec<_> = regs.iter().map(|int| format!("%{}", int)).collect();
                    string = format!("call_local %{}, %{}, {}\n", ret_loc, func_loc, args.join(", "));
                } else {
                    string = format!("call_local %{}, %{}\n", ret_loc, func_loc);
                }

                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Capture(ret_loc, ref name, arity) => {
                let string = format!("capture %{}, {}/{}\n", ret_loc, name, arity);
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
            &Instruction::ListNth(dest, reg1, reg2) => {
                let string = format!("list_nth %{}, %{}, %{}\n", dest, reg1, reg2);
                out.write(&string.as_bytes()).unwrap();
            },
            &Instruction::List(reg, size, ref regs) => {
                let string;

                if regs.len() > 0 {
                    let elems: Vec<_> = regs.iter().map(|int| format!("%{}", int)).collect();
                    string = format!("list %{}, {}, {}\n", reg, size, elems.join(", "));
                } else {
                    string = format!("list %{}, {}\n", reg, size);
                }

                out.write(&string.as_bytes()).unwrap();
            },
            &Instruction::StoreTrue(reg) => {
                let string = format!("store_true %{}\n", reg);
                out.write(&string.as_bytes()).unwrap();
            },
            &Instruction::StoreFalse(reg) => {
                let string = format!("store_false %{}\n", reg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::StoreNil(reg) => {
                let string = format!("store_nil %{}\n", reg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Eq(to, reg1, reg2) => {
                let string = format!("eq %{}, %{}, %{}\n", to, reg1, reg2);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::NotEq(to, reg1, reg2) => {
                let string = format!("not_eq %{}, %{}, %{}\n", to, reg1, reg2);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Not(to, reg) => {
                let string = format!("not %{}, %{}\n", to, reg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::ListCount(to, reg) => {
                let string = format!("list_count %{}, %{}\n", to, reg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::StringCount(to, reg) => {
                let string = format!("string_count %{}, %{}\n", to, reg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::CodeLoad(to, reg) => {
                let string = format!("code_load %{}, %{}\n", to, reg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::ListSlice(ret, reg, from, to) => {
                let string = format!("list_slice %{}, %{}, %{}, %{}\n", ret, reg, from, to);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::StringSlice(ret, reg, from, to) => {
                let string = format!("string_slice %{}, %{}, %{}, %{}\n", ret, reg, from, to);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::GreaterThan(to, arg1, arg2) => {
                let string = format!("greater_than %{}, %{}, %{}\n", to, arg1, arg2);
                out.write(&string.as_bytes()).unwrap();
            },
            &Instruction::LoadString(to, ref content) => {
                let string = format!("load_string %{}, \"{}\"\n", to, content);
                out.write(&string.as_bytes()).unwrap();
            }
        }
    }

    pub fn byte_size(&self) -> u8 {
        match self {
            &Instruction::Add(_, _, _)          => 4,
            &Instruction::Sub(_, _, _)          => 4,
            &Instruction::Concat(_, _, _)       => 4,
            &Instruction::CallByName(_, _, _)   => 4,
            &Instruction::StoreInt(_, _)        => 4,
            &Instruction::Mov(_, _)             => 3,
            &Instruction::Print(_)              => 2,
            &Instruction::Test(_, _)            => 3,
            &Instruction::Exit(_)               => 2,
            &Instruction::FilePwd(_)            => 2,
            &Instruction::FileLs(_, _)          => 3,
            &Instruction::Call(_, _, _, ref regs) => 4 + (regs.len() as u8), // Name only counts for 1 byte because it is interned at load-time
            &Instruction::Capture(_, _, _)      => 4, // Name only counts for 1 byte because it is interned at load-time
            &Instruction::CallLocal(_, _, ref regs) => 4 + (regs.len() as u8), // Name only counts for 1 byte because it is interned at load-time
            &Instruction::Jmp(_)                => 2,
            &Instruction::Tuple(_, _, ref regs) => 3 + regs.len() as u8,
            &Instruction::TupleNth(_, _, _)     => 4,
            &Instruction::ListNth(_, _, _)      => 4,
            &Instruction::Return                => 1,
            &Instruction::List(_, _, ref regs)  => 3 + regs.len() as u8,
            &Instruction::StoreTrue(_)          => 2,
            &Instruction::StoreFalse(_)         => 2,
            &Instruction::StoreNil(_)           => 2,
            &Instruction::Eq(_, _, _)           => 4,
            &Instruction::NotEq(_, _, _)        => 4,
            &Instruction::Not(_, _)             => 3,
            &Instruction::ListCount(_, _)       => 3,
            &Instruction::StringCount(_, _)     => 3,
            &Instruction::StringContains(_, _, _)  => 4,
            &Instruction::CodeLoad(_, _)        => 3,
            &Instruction::ListSlice(_, _, _, _) => 5,
            &Instruction::StringSlice(_, _, _, _) => 5,
            &Instruction::GreaterThan(_, _, _)  => 4,
            &Instruction::LoadString(_, _)      => 3, // Content only counts for 1 byte because it is interned at load-time
        }
    }
}

pub fn byte_size_of(instructions: &Vec<Instruction>) -> u8 {
    instructions.iter().fold(0, |acc, x| acc + x.byte_size())
}
