extern crate iris_compiler;

use iris_compiler::ast::*;
use iris_compiler::bytecode;

#[test]
fn generates_simple_addition() {
    let ast = mk_function("main", Vec::new(), vec![
        mk_apply(None, "+", vec![mk_int("1"), mk_int("2")])
    ]);

    let res = bytecode::generate(&ast);

    assert_eq!(res, bytecode::Function {
        name: "main",
        arity: 0,
        code: vec![
            bytecode::Instruction::Store(1, 1),
            bytecode::Instruction::Store(2, 2),
            bytecode::Instruction::Add(1, 1, 2),
            bytecode::Instruction::Mov(0, 1),
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

    let res = bytecode::generate(&ast);

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
        ]
    })
}

#[test]
fn generates_print_op() {
    let ast = mk_function("main", Vec::new(), vec![
        mk_apply(None, "print", vec![mk_int("1")])
    ]);

    let res = bytecode::generate(&ast);

    assert_eq!(res.code, vec![
            bytecode::Instruction::Store(1, 1),
            bytecode::Instruction::Print(1),
            bytecode::Instruction::Mov(0, 1),
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

    let res = bytecode::generate(&ast);

    assert_eq!(res.code, vec![
            bytecode::Instruction::Store(1, 1),
            bytecode::Instruction::Store(2, 2),
            bytecode::Instruction::TestGt(1, 2, 2),
            bytecode::Instruction::Exit,
            bytecode::Instruction::Store(1, 1),
            bytecode::Instruction::Print(1),
            bytecode::Instruction::Mov(0, 1),
        ]
    )
}
