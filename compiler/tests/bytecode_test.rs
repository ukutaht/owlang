extern crate owlc;

use owlc::ast::*;
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
            bytecode::Instruction::StoreInt(1, 1),
            bytecode::Instruction::StoreInt(2, 2),
            bytecode::Instruction::Add(0, 1, 2),
            bytecode::Instruction::Return
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
            bytecode::Instruction::StoreInt(1, 1),
            bytecode::Instruction::StoreInt(2, 2),
            bytecode::Instruction::Eq(0, 1, 2),
            bytecode::Instruction::Return
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
            bytecode::Instruction::StoreInt(1, 1),
            bytecode::Instruction::StoreInt(2, 2),
            bytecode::Instruction::NotEq(0, 1, 2),
            bytecode::Instruction::Return
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
            bytecode::Instruction::StoreTrue(1),
            bytecode::Instruction::Not(0, 1),
            bytecode::Instruction::Return
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
            bytecode::Instruction::StoreInt(1, 1),
            bytecode::Instruction::StoreInt(2, 2),
            bytecode::Instruction::GreaterThan(0, 1, 2),
            bytecode::Instruction::Return
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
            bytecode::Instruction::StoreInt(1, 1),
            bytecode::Instruction::Exit(1),
            bytecode::Instruction::Return
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
            bytecode::Instruction::FilePwd(0),
            bytecode::Instruction::Return
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
            bytecode::Instruction::LoadString(1, ".".to_string()),
            bytecode::Instruction::FileLs(0, 1),
            bytecode::Instruction::Return
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
            bytecode::Instruction::LoadString(1, "a".to_string()),
            bytecode::Instruction::LoadString(2, "b".to_string()),
            bytecode::Instruction::Concat(0, 1, 2),
            bytecode::Instruction::Return
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
            bytecode::Instruction::StoreInt(1, 1),
            bytecode::Instruction::StoreInt(3, 2),
            bytecode::Instruction::StoreInt(4, 3),
            bytecode::Instruction::Sub(2, 3, 4),
            bytecode::Instruction::Add(0, 1, 2),
            bytecode::Instruction::Return,
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
            bytecode::Instruction::StoreInt(1, 1),
            bytecode::Instruction::Print(1),
            bytecode::Instruction::Return,
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
            bytecode::Instruction::StoreTrue(1),
            bytecode::Instruction::Test(1, 5),
            bytecode::Instruction::StoreNil(0),
            bytecode::Instruction::Jmp(7),
            bytecode::Instruction::StoreInt(1, 1),
            bytecode::Instruction::Print(1),
            bytecode::Instruction::Return
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
        bytecode::Instruction::Call(0, "mod.wut".to_string(), 0, Vec::new()),
        bytecode::Instruction::Return,
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
        bytecode::Instruction::Call(0, "other_module.wut".to_string(), 0, Vec::new()),
        bytecode::Instruction::Return,
    ])
}

#[test]
fn generates_tuple() {
    let main = mk_function("main", Vec::new(), vec![
        mk_tuple(vec![mk_int("1"), mk_int("2")])
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        bytecode::Instruction::StoreInt(1, 1),
        bytecode::Instruction::StoreInt(2, 2),
        bytecode::Instruction::Tuple(0, 2, vec![1, 2]),
        bytecode::Instruction::Return,
    ])
}

#[test]
fn generates_list() {
    let main = mk_function("main", Vec::new(), vec![
        mk_list(vec![mk_int("1"), mk_int("2")])
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        bytecode::Instruction::StoreInt(1, 1),
        bytecode::Instruction::StoreInt(2, 2),
        bytecode::Instruction::List(0, 2, vec![1, 2]),
        bytecode::Instruction::Return,
    ])
}

#[test]
fn generates_true() {
    let main = mk_function("main", Vec::new(), vec![
        mk_true()
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        bytecode::Instruction::StoreTrue(0),
        bytecode::Instruction::Return,
    ])
}

#[test]
fn generates_false() {
    let main = mk_function("main", Vec::new(), vec![
        mk_false()
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        bytecode::Instruction::StoreFalse(0),
        bytecode::Instruction::Return,
    ])
}

#[test]
fn generates_nil() {
    let main = mk_function("main", Vec::new(), vec![
        mk_nil()
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        bytecode::Instruction::StoreNil(0),
        bytecode::Instruction::Return,
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
        bytecode::Instruction::StoreInt(1, 1),
        bytecode::Instruction::Mov(0, 1),
        bytecode::Instruction::Return,
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
        bytecode::Instruction::StoreNil(0),
        bytecode::Instruction::Return,
    ])
}

#[test]
fn generates_and_and() {
    let main = mk_function("main", Vec::new(), vec![
        mk_apply(None, "&&", vec![mk_true(), mk_false()])
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        bytecode::Instruction::StoreTrue(0),
        bytecode::Instruction::Test(0, 3),
        bytecode::Instruction::Jmp(3),
        bytecode::Instruction::StoreFalse(0),
        bytecode::Instruction::Return,
    ])
}

#[test]
fn generates_or_or() {
    let main = mk_function("main", Vec::new(), vec![
        mk_apply(None, "||", vec![mk_true(), mk_false()])
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        bytecode::Instruction::StoreTrue(0),
        bytecode::Instruction::Test(0, 3),
        bytecode::Instruction::StoreFalse(0),
        bytecode::Instruction::Return,
    ])
}

#[test]
fn generates_interned_string() {
    let main = mk_function("main", Vec::new(), vec![
        mk_string("Hello")
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        bytecode::Instruction::LoadString(0, "Hello".to_string()),
        bytecode::Instruction::Return,
    ])
}

#[test]
fn generates_function_capture() {
    let main = mk_function("main", Vec::new(), vec![
        mk_capture(None, "some_function", 0)
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        bytecode::Instruction::Capture(0, "unknown.some_function".to_string(), 0),
        bytecode::Instruction::Return,
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
        bytecode::Instruction::Capture(1, "unknown.some_function".to_string(), 0),
        bytecode::Instruction::CallLocal(0, 1, Vec::new()),
        bytecode::Instruction::Return,
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
        bytecode::Instruction::Capture(1, "unknown.some_function".to_string(), 0),
        bytecode::Instruction::Call(0, "Module.captured".to_string(), 0, Vec::new()),
        bytecode::Instruction::Return,
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
        bytecode::Instruction::AnonFn(0, 14, 0),
        bytecode::Instruction::StoreInt(1, 1),
        bytecode::Instruction::StoreInt(2, 1),
        bytecode::Instruction::Add(0, 1, 2),
        bytecode::Instruction::Return,
        bytecode::Instruction::Return
    ])
}

#[test]
fn generates_anonymous_function_with_arguments() {
    let main = mk_function("main", Vec::new(), vec![
        mk_anon_fn(vec![mk_argument("a"), mk_argument("b")], vec![mk_apply(None, "+", vec![mk_ident("a"), mk_ident("b")])])
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        bytecode::Instruction::AnonFn(0, 12, 2),
        bytecode::Instruction::Mov(3, 1),
        bytecode::Instruction::Mov(4, 2),
        bytecode::Instruction::Add(0, 3, 4),
        bytecode::Instruction::Return,
        bytecode::Instruction::Return
    ])
}
