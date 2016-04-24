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
        name: "main",
        arity: 0,
        code: vec![
            bytecode::Instruction::Store(1, 1),
            bytecode::Instruction::Store(2, 2),
            bytecode::Instruction::Add(1, 1, 2),
            bytecode::Instruction::Mov(0, 1),
            bytecode::Instruction::Exit
        ]
    })
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
        name: "main",
        arity: 0,
        code: vec![
            bytecode::Instruction::Store(1, 1),
            bytecode::Instruction::Store(2, 2),
            bytecode::Instruction::Store(3, 3),
            bytecode::Instruction::Sub(2, 2, 3),
            bytecode::Instruction::Add(1, 1, 2),
            bytecode::Instruction::Mov(0, 1),
            bytecode::Instruction::Exit,
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
            bytecode::Instruction::Store(1, 1),
            bytecode::Instruction::Print(1),
            bytecode::Instruction::Mov(0, 1),
            bytecode::Instruction::Exit,
        ]
    )
}

#[test]
fn generates_simple_if_statement() {
    let ast = mk_function("main", Vec::new(), vec![
        mk_if(
            mk_apply(None, ">", vec![mk_int("1"), mk_int("2")]),
            vec![mk_apply(None, "print", vec![mk_int("1")])]
            )
    ]);

    let res = bytecode::generate_function(&ast);

    assert_eq!(res.code, vec![
            bytecode::Instruction::Store(1, 1),
            bytecode::Instruction::Store(2, 2),
            bytecode::Instruction::TestGt(1, 2, 2),
            bytecode::Instruction::Exit,
            bytecode::Instruction::Store(1, 1),
            bytecode::Instruction::Print(1),
            bytecode::Instruction::Mov(0, 1),
            bytecode::Instruction::Exit
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
    let wut = mk_function("wut", Vec::new(), vec![
        mk_apply(None, "print", vec![mk_int("1")])
    ]);

    let main = mk_function("main", Vec::new(), vec![
        mk_apply(None, "wut", Vec::new())
    ]);

    let module = mk_module("mod", vec![wut, main]);

    let res = bytecode::generate(&module);


    assert_eq!(res.functions[1].code, vec![
        bytecode::Instruction::Call(1, 2, 0, Vec::new()),
        bytecode::Instruction::Mov(0, 1),
        bytecode::Instruction::Exit,
    ])
}

#[test]
fn generates_tuple() {
    let main = mk_function("main", Vec::new(), vec![
        mk_tuple(vec![mk_int("1"), mk_int("2")])
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        bytecode::Instruction::Store(1, 1),
        bytecode::Instruction::Store(2, 2),
        bytecode::Instruction::Tuple(1, 2, vec![1, 2]),
        bytecode::Instruction::Mov(0, 1),
        bytecode::Instruction::Exit,
    ])
}

#[test]
fn generates_vector() {
    let main = mk_function("main", Vec::new(), vec![
        mk_vector(vec![mk_int("1"), mk_int("2")])
    ]);

    let res = bytecode::generate_function(&main);

    assert_eq!(res.code, vec![
        bytecode::Instruction::Store(1, 1),
        bytecode::Instruction::Store(2, 2),
        bytecode::Instruction::Vector(1, 2, vec![1, 2]),
        bytecode::Instruction::Mov(0, 1),
        bytecode::Instruction::Exit,
    ])
}
