use std::io::Write;
use std::fmt;
use bytecode::opcodes;

pub type Length = u8;
pub type Arity = u8;
pub type Jump = u8;
pub type Bytecode = Vec<Instruction>;

#[derive(Debug, Eq, PartialEq, Clone, Copy)]
pub enum VarRef {
    Register(u8),
    Upvalue(u8)
}

impl VarRef {
    /// The most significant bit represents whether we want to access vm registers or
    /// upvalues of the currently executing function. Adding 128 flips the first bit
    /// which the VM uses to distinguish between these different access methods.
    pub fn byte(&self) -> u8 {
        match self {
            &VarRef::Register(reg) => reg,
            &VarRef::Upvalue(idx) => idx + 128,
        }
    }
}

impl fmt::Display for VarRef {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            &VarRef::Register(reg) => write!(f, "R{}", reg),
            &VarRef::Upvalue(idx) => write!(f, "U{}", idx)
        }
    }
}

#[derive(Debug, Eq, PartialEq)]
pub enum Instruction {
    Exit(VarRef),
    StoreInt(VarRef, u16),
    Print(VarRef),
    Test(VarRef, Jump),
    Add(VarRef, VarRef, VarRef),
    Sub(VarRef, VarRef, VarRef),
    Call(VarRef, String, Arity, Vec<VarRef>),
    Return,
    Mov(VarRef, VarRef),
    Jmp(Jump),
    Tuple(VarRef, Length, Vec<VarRef>),
    TupleNth(VarRef, VarRef, VarRef),
    List(VarRef, Length, Vec<VarRef>),
    ListNth(VarRef, VarRef, VarRef),
    StoreTrue(VarRef),
    StoreFalse(VarRef),
    StoreNil(VarRef),
    Eq(VarRef, VarRef, VarRef),
    NotEq(VarRef, VarRef, VarRef),
    Not(VarRef, VarRef),
    GreaterThan(VarRef, VarRef, VarRef),
    LoadString(VarRef, String),
    FilePwd(VarRef),
    FileLs(VarRef, VarRef),
    Concat(VarRef, VarRef, VarRef),
    Capture(VarRef, String, Arity),
    CallLocal(VarRef, VarRef, Vec<VarRef>),
    ListCount(VarRef, VarRef),
    ListSlice(VarRef, VarRef, VarRef, VarRef),
    StringSlice(VarRef, VarRef, VarRef, VarRef),
    CodeLoad(VarRef, VarRef),
    FunctionName(VarRef, VarRef),
    StringCount(VarRef, VarRef),
    StringContains(VarRef, VarRef, VarRef),
    ToString(VarRef, VarRef),
    GetUpval(VarRef, VarRef),
    AnonFn(VarRef, Jump, Arity, Vec<VarRef>),
}

impl Instruction {
    #[allow(unused_must_use)]
    pub fn emit<'a, T: Write>(&self, out: &'a mut T) {
        match self {
            &Instruction::Add(to, arg1, arg2) => {
                out.write(&[opcodes::ADD, to.byte(), arg1.byte(), arg2.byte()]).unwrap();
            },
            &Instruction::Sub(to, arg1, arg2) => {
                out.write(&[opcodes::SUB, to.byte(), arg1.byte(), arg2.byte()]).unwrap();
            },
            &Instruction::StoreInt(to, val) => {
                let first = val % 250;
                let second = val / 250;
                out.write(&[opcodes::STORE_INT, to.byte(), first as u8, second as u8]).unwrap();
            }
            &Instruction::Mov(to, from) => {
                out.write(&[opcodes::MOV, to.byte(), from.byte()]).unwrap();
            },
            &Instruction::GetUpval(to, address) => {
                out.write(&[opcodes::GET_UPVAL, to.byte(), address.byte()]).unwrap();
            },
            &Instruction::Concat(to, left, right) => {
                out.write(&[opcodes::CONCAT, to.byte(), left.byte(), right.byte()]).unwrap();
            },
            &Instruction::FunctionName(result, arg) => {
                out.write(&[opcodes::FUNCTION_NAME, result.byte(), arg.byte()]).unwrap();
            },
            &Instruction::Print(reg) => {
                out.write(&[opcodes::PRINT, reg.byte()]).unwrap();
            },
            &Instruction::Test(reg, jump) => {
                out.write(&[opcodes::TEST, reg.byte(), jump]).unwrap();
            }
            &Instruction::Exit(reg) => {
                out.write(&[opcodes::EXIT, reg.byte()]).unwrap();
            },
            &Instruction::FilePwd(reg) => {
                out.write(&[opcodes::FILE_PWD, reg.byte()]).unwrap();
            },
            &Instruction::FileLs(reg, path) => {
                out.write(&[opcodes::FILE_LS, reg.byte(), path.byte()]).unwrap();
            },
            &Instruction::Call(ref ret_loc, ref name, arity, ref regs) => {
                let full_name = format!("{}\\{}", name, arity);
                let name_size = full_name.len() as u8;
                out.write(&[opcodes::CALL, ret_loc.byte(), name_size + 1]).unwrap(); // +1 accounts for null termination
                out.write(&full_name.as_bytes()).unwrap();
                out.write(&[0]).unwrap(); // Null terminate the string
                out.write(&[arity]).unwrap();

                for reg in regs {
                    out.write(&[reg.byte()]);
                }
            }
            &Instruction::CallLocal(ref ret_loc, ref func_loc, ref regs) => {
                out.write(&[opcodes::CALL_LOCAL, ret_loc.byte(), func_loc.byte()]).unwrap();
                out.write(&[regs.len() as u8]).unwrap();

                for reg in regs {
                    out.write(&[reg.byte()]);
                }
            }
            &Instruction::Capture(ref ret_loc, ref name, arity) => {
                let full_name = format!("{}\\{}", name, arity);
                let name_size = full_name.len() as u8;
                out.write(&[opcodes::CAPTURE, ret_loc.byte(), name_size + 1]).unwrap(); // +1 accounts for null termination
                out.write(&full_name.as_bytes()).unwrap();
                out.write(&[0]).unwrap(); // Null terminate the string
            }
            &Instruction::Jmp(loc) => {
                out.write(&vec![opcodes::JMP, loc]).unwrap();
            }
            &Instruction::Return => {
                out.write(&vec![opcodes::RETURN]).unwrap();
            }
            &Instruction::Tuple(ref reg, size, ref elems) => {
                out.write(&vec![opcodes::TUPLE, reg.byte(), size]);

                for reg in elems {
                    out.write(&[reg.byte()]);
                }
            }
            &Instruction::TupleNth(dest, reg, nth) => {
                out.write(&[opcodes::TUPLE_NTH, dest.byte(), reg.byte(), nth.byte()]).unwrap();
            },
            &Instruction::ListNth(dest, reg1, reg2) => {
                out.write(&[opcodes::LIST_NTH, dest.byte(), reg1.byte(), reg2.byte()]).unwrap();
            },
            &Instruction::List(ref reg, size, ref elems) => {
                out.write(&vec![opcodes::LIST, reg.byte(), size]);

                for reg in elems {
                    out.write(&[reg.byte()]);
                }
            }
            &Instruction::StoreTrue(reg) => {
                out.write(&[opcodes::STORE_TRUE, reg.byte()]).unwrap();
            }
            &Instruction::StoreFalse(reg) => {
                out.write(&[opcodes::STORE_FALSE, reg.byte()]).unwrap();
            }
            &Instruction::StoreNil(reg) => {
                out.write(&[opcodes::STORE_NIL, reg.byte()]).unwrap();
            }
            &Instruction::Eq(to, reg1, reg2) => {
                out.write(&[opcodes::EQ, to.byte(), reg1.byte(), reg2.byte()]).unwrap();
            }
            &Instruction::NotEq(to, reg1, reg2) => {
                out.write(&[opcodes::NOT_EQ, to.byte(), reg1.byte(), reg2.byte()]).unwrap();
            }
            &Instruction::Not(to, reg) => {
                out.write(&[opcodes::NOT, to.byte(), reg.byte()]).unwrap();
            }
            &Instruction::ListCount(to, reg) => {
                out.write(&[opcodes::LIST_COUNT, to.byte(), reg.byte()]).unwrap();
            }
            &Instruction::StringCount(to, reg) => {
                out.write(&[opcodes::STRING_COUNT, to.byte(), reg.byte()]).unwrap();
            }
            &Instruction::StringContains(to, string, substr) => {
                out.write(&[opcodes::STRING_CONTAINS, to.byte(), string.byte(), substr.byte()]).unwrap();
            }
            &Instruction::CodeLoad(to, reg) => {
                out.write(&[opcodes::CODE_LOAD, to.byte(), reg.byte()]).unwrap();
            }
            &Instruction::ListSlice(ret, reg, from, to) => {
                out.write(&[opcodes::LIST_SLICE, ret.byte(), reg.byte(), from.byte(), to.byte()]).unwrap();
            }
            &Instruction::StringSlice(ret, reg, from, to) => {
                out.write(&[opcodes::STRING_SLICE, ret.byte(), reg.byte(), from.byte(), to.byte()]).unwrap();
            }
            &Instruction::LoadString(ref to, ref content) => {
                let content_size = content.len() as u8;
                out.write(&[opcodes::LOAD_STRING, to.byte(), content_size + 1]).unwrap();; // +1 accounts for null termination
                out.write(&content.as_bytes()).unwrap();
                out.write(&[0]).unwrap(); // Null terminate the string
            }
            &Instruction::GreaterThan(to, arg1, arg2) => {
                out.write(&[opcodes::GREATER_THAN, to.byte(), arg1.byte(), arg2.byte()]).unwrap();
            },
            &Instruction::ToString(to, reg) => {
                out.write(&[opcodes::TO_STRING, to.byte(), reg.byte()]).unwrap();
            }
            &Instruction::AnonFn(ref to, jmp, arity, ref upvals) => {
                out.write(&[opcodes::ANON_FN, to.byte(), jmp, arity]).unwrap();

                out.write(&[upvals.len() as u8]).unwrap();
                for reg in upvals {
                    out.write(&[reg.byte()]);
                }
            }
        }
    }

    pub fn emit_human_readable<'a, T: Write>(&self, out: &'a mut T) {
        match self {
            &Instruction::Add(to, arg1, arg2) => {
                let string = format!("{} = add {}, {}\n", to, arg1, arg2);
                out.write(&string.as_bytes()).unwrap();
            },
            &Instruction::Sub(to, arg1, arg2) => {
                let string = format!("{} = sub {}, {}\n", to, arg1, arg2);
                out.write(&string.as_bytes()).unwrap();
            },
            &Instruction::StoreInt(to, val) => {
                let string = format!("{} = store_int {}\n", to, val);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Mov(to, from) => {
                let string = format!("{} = mov {}\n", to, from);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::GetUpval(to, address) => {
                let string = format!("{} = get_upval {}\n", to, address);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Concat(to, left, right) => {
                let string = format!("{} = concat {}, {}\n", to, left, right);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::FunctionName(to, arg) => {
                let string = format!("{} = function_name {}\n", to, arg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::StringContains(to, string, substr) => {
                let string = format!("{} = string_contains {}, {}\n", to, string, substr);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Print(reg) => {
                let string = format!("print {}\n", reg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Test(reg, jump) => {
                let string = format!("test {}, {}\n", reg, jump);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Exit(reg) => {
                let string = format!("exit {}\n", reg);
                out.write(string.as_bytes()).unwrap();
            }
            &Instruction::FilePwd(reg) => {
                let string = format!("{} = file_pwd\n", reg);
                out.write(string.as_bytes()).unwrap();
            }
            &Instruction::FileLs(reg, path) => {
                let string = format!("{} = file_ls {}\n", reg, path);
                out.write(string.as_bytes()).unwrap();
            }
            &Instruction::Call(ref ret_loc, ref name, arity, ref regs) => {
                let string;

                if regs.len() > 0 {
                    let args: Vec<_> = regs.iter().map(|int| format!("{}", int)).collect();
                    string = format!("{} = call {}\\{}, {}\n", ret_loc, name, arity, args.join(", "));
                } else {
                    string = format!("{} = call {}\\{}\n", ret_loc, name, arity);
                }

                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::CallLocal(ref ret_loc, ref func_loc, ref regs) => {
                let args: Vec<_> = regs.iter().map(|int| format!("{}", int)).collect();
                let string = format!("{} = call_local {}, [{}; {}]\n", ret_loc, func_loc, args.len() as u8, args.join(", "));

                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Capture(ref ret_loc, ref name, arity) => {
                let string = format!("{} = capture {}/{}\n", ret_loc, name, arity);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Jmp(loc) => {
                let string = format!("jmp {}\n", loc);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Return => {
                out.write(b"return\n").unwrap();
            }
            &Instruction::Tuple(ref reg, size, ref regs) => {
                let elems: Vec<_> = regs.iter().map(|int| format!("{}", int)).collect();
                let string = format!("{} = tuple [{}; {}]\n", reg, size, elems.join(", "));

                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::TupleNth(dest, reg, nth) => {
                let string = format!("{} = tuple_nth {}, {}\n", dest, reg, nth);
                out.write(&string.as_bytes()).unwrap();
            },
            &Instruction::ListNth(dest, reg1, reg2) => {
                let string = format!("{} = list_nth {}, {}\n", dest, reg1, reg2);
                out.write(&string.as_bytes()).unwrap();
            },
            &Instruction::List(ref reg, size, ref regs) => {
                let elems: Vec<_> = regs.iter().map(|int| format!("{}", int)).collect();
                let string = format!("{} = list [{}; {}]\n", reg, size, elems.join(", "));
                out.write(&string.as_bytes()).unwrap();
            },
            &Instruction::StoreTrue(reg) => {
                let string = format!("{} = store_true\n", reg);
                out.write(&string.as_bytes()).unwrap();
            },
            &Instruction::StoreFalse(reg) => {
                let string = format!("{} = store_false\n", reg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::StoreNil(reg) => {
                let string = format!("{} = store_nil\n", reg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Eq(to, reg1, reg2) => {
                let string = format!("{} = eq {}, {}\n", to, reg1, reg2);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::NotEq(to, reg1, reg2) => {
                let string = format!("{} = not_eq {}, {}\n", to, reg1, reg2);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::Not(to, reg) => {
                let string = format!("{} = not {}\n", to, reg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::ListCount(to, reg) => {
                let string = format!("{} = list_count {}\n", to, reg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::StringCount(to, reg) => {
                let string = format!("{} = string_count {}\n", to, reg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::CodeLoad(to, reg) => {
                let string = format!("{} = code_load {}\n", to, reg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::ListSlice(ret, reg, from, to) => {
                let string = format!("{} = list_slice {}, {}, {}\n", ret, reg, from, to);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::StringSlice(ret, reg, from, to) => {
                let string = format!("{} = string_slice {}, {}, {}\n", ret, reg, from, to);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::GreaterThan(to, arg1, arg2) => {
                let string = format!("{} = greater_than {}, {}\n", to, arg1, arg2);
                out.write(&string.as_bytes()).unwrap();
            },
            &Instruction::LoadString(ref to, ref content) => {
                let string = format!("{} = load_string \"{}\"\n", to, content);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::ToString(to, reg) => {
                let string = format!("{} = to_string {}\n", to, reg);
                out.write(&string.as_bytes()).unwrap();
            }
            &Instruction::AnonFn(ref to, jmp, arity, ref upvals) => {
                let formatted_upvals: Vec<_> = upvals.iter().map(|int| format!("{}", int)).collect();
                let string = format!("{} = anon_fn {}, {}, [{}; {}]\n", to, jmp, arity, upvals.len(), formatted_upvals.join(", "));
                out.write(&string.as_bytes()).unwrap();
            }
        }
    }

    pub fn byte_size(&self) -> u8 {
        match self {
            &Instruction::Add(_, _, _)          => 4,
            &Instruction::Sub(_, _, _)          => 4,
            &Instruction::Concat(_, _, _)       => 4,
            &Instruction::FunctionName(_, _)    => 3,
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
            &Instruction::ToString(_, _)        => 3,
            &Instruction::AnonFn(_, _, _, ref upvals) => 5 + (upvals.len() as u8),
            &Instruction::GetUpval(_, _)        => 3,
        }
    }
}

pub fn byte_size_of(instructions: &Vec<Instruction>) -> u8 {
    instructions.iter().fold(0, |acc, x| acc + x.byte_size())
}
