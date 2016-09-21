extern crate owlc;

use owlc::ast::*;
use owlc::bytecode::{Instruction, VarRef};
use owlc::bytecode;

#[test]
fn generates_simple_addition() {
    let ast = mk_function("main", Vec::new(), vec![
        mk_apply(None, "+", vec![mk_int("1"), mk_int("2")])
    ]);

    let res = bytecode::generate_function(&ast);

    assert_eq!(res, bytecode::Function {
        name: "unknown.main".to_string(),
        arity: 0,
        code: vec![
            Instruction::StoreInt(VarRef::Register(1), 1),
            Instruction::StoreInt(VarRef::Register(2), 2),
            Instruction::Add(VarRef::Register(0), VarRef::Register(1), VarRef::Register(2)),
            Instruction::Return
        ]
    })
}

#[test]
fn generates_equality_test() {
    let ast = mk_function("main", Vec::new(), vec![
        mk_apply(None, "==", vec![mk_int("1"), mk_int("2")])
    ]);

    let res = bytecode::generate_function(&ast);

    assert_eq!(res.code, vec![
            Instruction::StoreInt(VarRef::Register(1), 1),
            Instruction::StoreInt(VarRef::Register(2), 2),
            Instruction::Eq(VarRef::Register(0), VarRef::Register(1), VarRef::Register(2)),
            Instruction::Return
        ]
    )
}

#[test]
fn generates_non_equality_test() {
    let ast = mk_function("main", Vec::new(), vec![
        mk_apply(None, "!=", vec![mk_int("1"), mk_int("2")])
    ]);

    let res = bytecode::generate_function(&ast);

    assert_eq!(res.code, vec![
            Instruction::StoreInt(VarRef::Register(1), 1),
            Instruction::StoreInt(VarRef::Register(2), 2),
            Instruction::NotEq(VarRef::Register(0), VarRef::Register(1), VarRef::Register(2)),
            Instruction::Return
        ]
    )
}

#[test]
fn generates_not_op() {
    let ast = mk_function("main", Vec::new(), vec![
        mk_apply(None, "!", vec![mk_true()])
    ]);

    let res = bytecode::generate_function(&ast);

    assert_eq!(res.code, vec![
            Instruction::StoreTrue(VarRef::Register(1)),
            Instruction::Not(VarRef::Register(0), VarRef::Register(1)),
            Instruction::Return
        ]
    )
}

#[test]
fn generates_greater_than_op() {
    let ast = mk_function("main", Vec::new(), vec![
        mk_apply(None, ">", vec![mk_int("1"), mk_int("2")])
    ]);

    let res = bytecode::generate_function(&ast);

    assert_eq!(res.code, vec![
            Instruction::StoreInt(VarRef::Register(1), 1),
            Instruction::StoreInt(VarRef::Register(2), 2),
            Instruction::GreaterThan(VarRef::Register(0), VarRef::Register(1), VarRef::Register(2)),
            Instruction::Return
        ]
    )
}

#[test]
fn generates_system_exit_with_value() {
    let ast = mk_function("main", Vec::new(), vec![
        mk_apply(None, "exit", vec![mk_int("1")])
    ]);

    let res = bytecode::generate_function(&ast);

    assert_eq!(res.code, vec![
            Instruction::StoreInt(VarRef::Register(1), 1),
            Instruction::Exit(VarRef::Register(1)),
            Instruction::Return
        ]
    )
}

#[test]
fn generates_file_pwd() {
    let ast = mk_function("main", Vec::new(), vec![
        mk_apply(None, "file_pwd", vec![])
    ]);

    let res = bytecode::generate_function(&ast);

    assert_eq!(res.code, vec![
            Instruction::FilePwd(VarRef::Register(0)),
            Instruction::Return
        ]
    )
}

#[test]
fn generates_file_ls() {
    let ast = mk_function("main", Vec::new(), vec![
        mk_apply(None, "file_ls", vec![mk_string(".")])
    ]);

    let res = bytecode::generate_function(&ast);

    assert_eq!(res.code, vec![
            Instruction::LoadString(VarRef::Register(1), ".".to_string()),
            Instruction::FileLs(VarRef::Register(0), VarRef::Register(1)),
            Instruction::Return
        ]
    )
}

#[test]
fn generates_concat() {
    let ast = mk_function("main", Vec::new(), vec![
        mk_apply(None, "++", vec![mk_string("a"), mk_string("b")])
    ]);

    let res = bytecode::generate_function(&ast);

    assert_eq!(res.code, vec![
            Instruction::LoadString(VarRef::Register(1), "a".to_string()),
            Instruction::LoadString(VarRef::Register(2), "b".to_string()),
            Instruction::Concat(VarRef::Register(0), VarRef::Register(1), VarRef::Register(2)),
            Instruction::Return
        ]
    )
}

#[test]
fn generates_nested_arithmetic() {
    let ast = mk_function("main", Vec::new(), vec![
        mk_apply(None, "+", vec![
                 mk_int("1"),
                 mk_apply(None, "-", vec![
                    mk_int("2"),
                    mk_int("3")
                 ])
        ])
    ]);

    let res = bytecode::generate_function(&ast);

    assert_eq!(res, bytecode::Function {
        name: "unknown.main".to_string(),
        arity: 0,
        code: vec![
            Instruction::StoreInt(VarRef::Register(1), 1),
            Instruction::StoreInt(VarRef::Register(3), 2),
            Instruction::StoreInt(VarRef::Register(4), 3),
            Instruction::Sub(VarRef::Register(2), VarRef::Register(3), VarRef::Register(4)),
            Instruction::Add(VarRef::Register(0), VarRef::Register(1), VarRef::Register(2)),
            Instruction::Return,
        ]
    })
}

#[test]
fn generates_print_op() {
    let ast = mk_function("main", Vec::new(), vec![
        mk_apply(None, "print", vec![mk_int("1")])
    ]);

    let res = bytecode::generate_function(&ast);

    assert_eq!(res.code, vec![
            Instruction::StoreInt(VarRef::Register(1), 1),
            Instruction::Print(VarRef::Register(1)),
            Instruction::Return,
        ]
    )
}

#[test]
fn generates_simple_if_statement() {
    let ast = mk_function("main", Vec::new(), vec![
        mk_if(
            mk_true(),
            vec![mk_apply(None, "print", vec![mk_int("1")])],
            Vec::new()
            )
    ]);

    let res = bytecode::generate_function(&ast);

    assert_eq!(res.code, vec![
            Instruction::StoreTrue(VarRef::Register(1)),
            Instruction::Test(VarRef::Register(1), 5),
            Instruction::StoreNil(VarRef::Register(0)),
            Instruction::Jmp(7),
            Instruction::StoreInt(VarRef::Register(1), 1),
            Instruction::Print(VarRef::Register(1)),
            Instruction::Return
        ]
    )
}

#[test]
fn generates_simple_module() {
    let module = mk_module("mod", vec![
        mk_function("main", Vec::new(), vec![mk_int("1")])
    ]);

    let res = bytecode::generate(&module);

    assert_eq!(res.name, "mod");
    assert_eq!(res.functions.len(), 1);
}

#[test]
fn generates_function_call_in_same_module() {
    let main = mk_function("main", Vec::new(), vec![
        mk_apply(None, "wut", Vec::new())
    ]);

    let module = mk_module("mod", vec![main]);

    let res = bytecode::generate(&module);

    assert_eq!(res.functions[0].code, vec![
        Instruction::Call(VarRef::Register(0), "mod.wut".to_string(), 0, Vec::new()),
        Instruction::Return,
    ])
}

#[test]
fn generates_function_call_in_different_module() {
    let main = mk_function("main", Vec::new(), vec![
        mk_apply(Some("other_module"), "wut", Vec::new())
    ]);

    let module = mk_module("mod", vec![main]);

    let res = bytecode::generate(&module);

    assert_eq!(res.functions[0].code, vec![
        Instruction::Call(VarRef::Register(0), "other_module.wut".to_string(), 0, Vec::new()),
        Instruction::Return,
    ])
}

#[test]
fn generates_tuple() {
    let main = mk_function("main", Vec::new(), vec![
        mk_tuple(vec![mk_int("1"), mk_int("2")])
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        Instruction::StoreInt(VarRef::Register(1), 1),
        Instruction::StoreInt(VarRef::Register(2), 2),
        Instruction::Tuple(VarRef::Register(0), 2, vec![
             VarRef::Register(1),
             VarRef::Register(2)
        ]),
        Instruction::Return,
    ])
}

#[test]
fn generates_list() {
    let main = mk_function("main", Vec::new(), vec![
        mk_list(vec![mk_int("1"), mk_int("2")])
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        Instruction::StoreInt(VarRef::Register(1), 1),
        Instruction::StoreInt(VarRef::Register(2), 2),
        Instruction::List(VarRef::Register(0), 2, vec![
             VarRef::Register(1),
             VarRef::Register(2)
        ]),
        Instruction::Return,
    ])
}

#[test]
fn generates_true() {
    let main = mk_function("main", Vec::new(), vec![
        mk_true()
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        Instruction::StoreTrue(VarRef::Register(0)),
        Instruction::Return,
    ])
}

#[test]
fn generates_false() {
    let main = mk_function("main", Vec::new(), vec![
        mk_false()
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        Instruction::StoreFalse(VarRef::Register(0)),
        Instruction::Return,
    ])
}

#[test]
fn generates_nil() {
    let main = mk_function("main", Vec::new(), vec![
        mk_nil()
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        Instruction::StoreNil(VarRef::Register(0)),
        Instruction::Return,
    ])
}

#[test]
fn generates_local_variable() {
    let main = mk_function("main", Vec::new(), vec![
        mk_let(mk_ident("a"), mk_int("1")),
        mk_ident("a")
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        Instruction::StoreInt(VarRef::Register(1), 1),
        Instruction::Mov(VarRef::Register(0), VarRef::Register(1)),
        Instruction::Return,
    ])
}

#[test]
#[should_panic]
fn does_not_allow_rebinding() {
    let main = mk_function("main", Vec::new(), vec![
        mk_let(mk_ident("a"), mk_int("1")),
        mk_let(mk_ident("a"), mk_int("2")),
    ]);

    bytecode::generate_function(&main);
}

#[test]
fn empty_function_returns_nil() {
    let main = mk_function("main", Vec::new(), vec![]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        Instruction::StoreNil(VarRef::Register(0)),
        Instruction::Return,
    ])
}

#[test]
fn generates_and_and() {
    let main = mk_function("main", Vec::new(), vec![
        mk_apply(None, "&&", vec![mk_true(), mk_false()])
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        Instruction::StoreTrue(VarRef::Register(0)),
        Instruction::Test(VarRef::Register(0), 3),
        Instruction::Jmp(3),
        Instruction::StoreFalse(VarRef::Register(0)),
        Instruction::Return,
    ])
}

#[test]
fn generates_or_or() {
    let main = mk_function("main", Vec::new(), vec![
        mk_apply(None, "||", vec![mk_true(), mk_false()])
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        Instruction::StoreTrue(VarRef::Register(0)),
        Instruction::Test(VarRef::Register(0), 3),
        Instruction::StoreFalse(VarRef::Register(0)),
        Instruction::Return,
    ])
}

#[test]
fn generates_interned_string() {
    let main = mk_function("main", Vec::new(), vec![
        mk_string("Hello")
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        Instruction::LoadString(VarRef::Register(0), "Hello".to_string()),
        Instruction::Return,
    ])
}

#[test]
fn generates_function_capture() {
    let main = mk_function("main", Vec::new(), vec![
        mk_capture(None, "some_function", 0)
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        Instruction::Capture(VarRef::Register(0), "unknown.some_function".to_string(), 0),
        Instruction::Return,
    ])
}

#[test]
fn generates_calling_function_indirectly() {
    let main = mk_function("main", Vec::new(), vec![
        mk_let(mk_ident("captured"), mk_capture(None, "some_function", 0)),
        mk_apply(None, "captured", Vec::new())
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        Instruction::Capture(VarRef::Register(1), "unknown.some_function".to_string(), 0),
        Instruction::CallLocal(VarRef::Register(0), VarRef::Register(1), Vec::new()),
        Instruction::Return,
    ])
}

#[test]
fn does_not_call_locally_when_module_is_provided() {
    let main = mk_function("main", Vec::new(), vec![
        mk_let(mk_ident("captured"), mk_capture(None, "some_function", 0)),
        mk_apply(Some("Module"), "captured", Vec::new())
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        Instruction::Capture(VarRef::Register(1), "unknown.some_function".to_string(), 0),
        Instruction::Call(VarRef::Register(0), "Module.captured".to_string(), 0, Vec::new()),
        Instruction::Return,
    ])
}

#[test]
#[should_panic]
fn does_not_insert_var_in_env_during_binding() {
    let main = mk_function("main", Vec::new(), vec![
        mk_let(mk_ident("a"), mk_ident("a")),
    ]);

    bytecode::generate_function(&main);
}


#[test]
fn generates_anonymous_function() {
    let main = mk_function("main", Vec::new(), vec![
        mk_anon_fn(vec![], vec![mk_apply(None, "+", vec![mk_int("1"), mk_int("1")])])
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        Instruction::AnonFn(VarRef::Register(0), 14, 0, vec![]),
        Instruction::StoreInt(VarRef::Register(1), 1),
        Instruction::StoreInt(VarRef::Register(2), 1),
        Instruction::Add(VarRef::Register(0), VarRef::Register(1), VarRef::Register(2)),
        Instruction::Return,
        Instruction::Return
    ])
}

#[test]
fn generates_anonymous_function_with_arguments() {
    let main = mk_function("main", Vec::new(), vec![
        mk_anon_fn(vec![mk_argument("a"), mk_argument("b")], vec![mk_apply(None, "+", vec![mk_ident("a"), mk_ident("b")])])
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        Instruction::AnonFn(VarRef::Register(0), 12, 2, vec![]),
        Instruction::Mov(VarRef::Register(3), VarRef::Register(1)),
        Instruction::Mov(VarRef::Register(4), VarRef::Register(2)),
        Instruction::Add(VarRef::Register(0), VarRef::Register(3), VarRef::Register(4)),
        Instruction::Return,
        Instruction::Return
    ])
}

#[test]
fn generates_anonymous_function_with_upvalues() {
    let main = mk_function("main", vec![mk_argument("a")], vec![
        mk_let(mk_ident("b"), mk_int("1")),
        mk_anon_fn(vec![], vec![mk_apply(None, "+", vec![mk_ident("a"), mk_ident("b")])])
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        Instruction::StoreInt(VarRef::Register(2), 1),
        Instruction::AnonFn(VarRef::Register(0), 12, 0, vec![VarRef::Register(1), VarRef::Register(2)]),
        Instruction::Mov(VarRef::Register(1), VarRef::Upvalue(1)),
        Instruction::Mov(VarRef::Register(2), VarRef::Upvalue(2)),
        Instruction::Add(VarRef::Register(0), VarRef::Register(1), VarRef::Register(2)),
        Instruction::Return,
        Instruction::Return
    ])
}
