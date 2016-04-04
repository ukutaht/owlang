extern crate iris_compiler;

use iris_compiler::ast::*;
use iris_compiler::parser;

#[test]
fn parses_integer() {
    let res = parser::parse(b"1");

    assert_eq!(res, Ok(mk_int("1")))
}

#[test]
fn parses_valid_identifiers() {
    let simple = parser::parse(b"a");
    assert_eq!(simple, Ok(mk_ident("a")));

    let underscore = parser::parse(b"a_b");
    assert_eq!(underscore, Ok(mk_ident("a_b")));

    let number = parser::parse(b"a_b1");
    assert_eq!(number, Ok(mk_ident("a_b1")));
}

#[test]
fn parses_simple_function_application() {
    let res = parser::parse(b"function()");
    assert_eq!(res, Ok(mk_apply(None, "function", Vec::new())));
}

#[test]
fn parses_function_with_one_arg() {
    let res = parser::parse(b"function(1)");
    assert_eq!(res, Ok(mk_apply(None, "function", vec![mk_int("1")])));
}

#[test]
fn parses_function_with_two_args() {
    let res = parser::parse(b"function(a, b)");
    assert_eq!(res, Ok(mk_apply(None, "function", vec![mk_ident("a"), mk_ident("b")])));
}

#[test]
fn parses_infix_plus() {
    let res = parser::parse(b"a + b");
    assert_eq!(res, Ok(mk_apply(None, "+", vec![mk_ident("a"), mk_ident("b")])));
}

#[test]
fn parses_infix_minus() {
    let res = parser::parse(b"a - b");
    assert_eq!(res, Ok(mk_apply(None, "-", vec![mk_ident("a"), mk_ident("b")])));
}

#[test]
fn parses_simple_function_definition() {
    let res = parser::parse(b"fn hello() { 1 }");
    assert_eq!(res, Ok(mk_function("hello", Vec::new(), vec![
        mk_int("1")
    ])));
}

#[test]
fn parses_function_def_with_args() {
    let res = parser::parse(b"fn hello(a, b) { a + b }");
    assert_eq!(res, Ok(mk_function("hello", vec![mk_argument("a"), mk_argument("b")], vec![
        mk_apply(None, "+", vec![mk_ident("a"), mk_ident("b")])
    ])));
}
