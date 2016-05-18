extern crate owlc;

use owlc::ast::*;
use owlc::parser;

#[test]
fn parses_integer() {
    let res = parser::parse_expr(b"1");

    assert_eq!(res, Ok(mk_int("1")))
}

#[test]
fn parses_valid_identifiers() {
    let simple = parser::parse_expr(b"a");
    assert_eq!(simple, Ok(mk_ident("a")));

    let underscore = parser::parse_expr(b"a_b");
    assert_eq!(underscore, Ok(mk_ident("a_b")));

    let number = parser::parse_expr(b"a_b1");
    assert_eq!(number, Ok(mk_ident("a_b1")));
}

#[test]
fn parses_true() {
    let res = parser::parse_expr(b"true");
    assert_eq!(res, Ok(mk_true()));
}

#[test]
fn parses_false() {
    let res = parser::parse_expr(b"false");
    assert_eq!(res, Ok(mk_false()));
}

#[test]
fn parses_nil() {
    let res = parser::parse_expr(b"nil");
    assert_eq!(res, Ok(mk_nil()));
}

#[test]
fn parses_unary_not() {
    let res = parser::parse_expr(b"!false");
    assert_eq!(res, Ok(mk_apply(None, "!", vec![mk_false()])));
}

#[test]
fn parses_simple_function_application() {
    let res = parser::parse_expr(b"function()");
    assert_eq!(res, Ok(mk_apply(None, "function", Vec::new())));
}

#[test]
fn parses_simple_function_application_with_module() {
    let res = parser::parse_expr(b"module:function()");
    assert_eq!(res, Ok(mk_apply(Some("module"), "function", Vec::new())));
}

#[test]
fn parses_function_with_one_arg() {
    let res = parser::parse_expr(b"function(1)");
    assert_eq!(res, Ok(mk_apply(None, "function", vec![mk_int("1")])));
}

#[test]
fn parses_function_with_two_args() {
    let res = parser::parse_expr(b"function(a, b)");
    assert_eq!(res, Ok(mk_apply(None, "function", vec![mk_ident("a"), mk_ident("b")])));
}

#[test]
fn parses_infix_plus() {
    let res = parser::parse_expr(b"a + b");
    assert_eq!(res, Ok(mk_apply(None, "+", vec![mk_ident("a"), mk_ident("b")])));
}

#[test]
fn parses_infix_minus() {
    let res = parser::parse_expr(b"a - b");
    assert_eq!(res, Ok(mk_apply(None, "-", vec![mk_ident("a"), mk_ident("b")])));
}

#[test]
fn parses_infix_equals() {
    let res = parser::parse_expr(b"a == b");
    assert_eq!(res, Ok(mk_apply(None, "==", vec![mk_ident("a"), mk_ident("b")])));
}

#[test]
fn parses_infix_not_equals() {
    let res = parser::parse_expr(b"a != b");
    assert_eq!(res, Ok(mk_apply(None, "!=", vec![mk_ident("a"), mk_ident("b")])));
}

#[test]
fn parses_simple_function_definition() {
    let res = parser::parse_fn(b"fn hello() { 1 }");
    assert_eq!(res, Ok(mk_function("hello", Vec::new(), vec![
        mk_int("1")
    ])));
}

#[test]
fn parses_function_def_with_args() {
    let res = parser::parse_fn(b"fn hello(a, b) { a + b }");
    assert_eq!(res, Ok(mk_function("hello", vec![mk_argument("a"), mk_argument("b")], vec![
        mk_apply(None, "+", vec![mk_ident("a"), mk_ident("b")])
    ])));
}

#[test]
fn parses_greater_than_comp() {
    let res = parser::parse_expr(b"a > b");
    assert_eq!(res, Ok(mk_apply(None, ">", vec![mk_ident("a"), mk_ident("b")])));
}

#[test]
fn parses_greater_than_equal_comp() {
    let res = parser::parse_expr(b"a >= b");
    assert_eq!(res, Ok(mk_apply(None, ">=", vec![mk_ident("a"), mk_ident("b")])));
}

#[test]
fn parses_less_than_comp() {
    let res = parser::parse_expr(b"a < b");
    assert_eq!(res, Ok(mk_apply(None, "<", vec![mk_ident("a"), mk_ident("b")])));
}

#[test]
fn parses_less_than_or_equal_comp() {
    let res = parser::parse_expr(b"a <= b");
    assert_eq!(res, Ok(mk_apply(None, "<=", vec![mk_ident("a"), mk_ident("b")])));
}

#[test]
fn parses_if_statement() {
    let res = parser::parse_expr(b"if a { 1 }");
    assert_eq!(res, Ok(mk_if(mk_ident("a"), vec![mk_int("1")], Vec::new())));
}

#[test]
fn parses_else_branch() {
    let res = parser::parse_expr(b"if a { 1 } else { 2 }");
    assert_eq!(res, Ok(mk_if(mk_ident("a"), vec![mk_int("1")], vec![mk_int("2")])));
}

#[test]
fn parses_simple_module_definition() {
    let res = parser::parse_module(b"module a { fn b() { 1 } }");
    assert_eq!(res, Ok(mk_module("a", vec![
        mk_function("b", Vec::new(), vec![mk_int("1")])
    ])));
}

#[test]
fn parses_simple_tuple() {
    let res = parser::parse_expr(b"(1, 2)");
    assert_eq!(res, Ok(mk_tuple(vec![
        mk_int("1"), mk_int("2")
    ])));
}

#[test]
fn parses_simple_vector() {
    let res = parser::parse_expr(b"[1, 2]");
    assert_eq!(res, Ok(mk_vector(vec![
        mk_int("1"), mk_int("2")
    ])));
}

#[test]
fn parses_simple_let() {
    let res = parser::parse_expr(b"let a = 1");
    assert_eq!(res, Ok(mk_let(mk_ident("a"), mk_int("1"))));
}

#[test]
fn parses_let_with_functions() {
    let res = parser::parse_expr(b"let a = 2 + wut(1)");
    assert_eq!(res, Ok(mk_let(mk_ident("a"),
        mk_apply(None, "+", vec![
                 mk_int("2"),
                 mk_apply(None, "wut", vec![mk_int("1")]),
        ])
    )));
}

#[test]
fn parses_let_with_vector() {
    let res = parser::parse_expr(b"let a = [1, 2]");
    assert_eq!(res, Ok(mk_let(mk_ident("a"), mk_vector(vec![mk_int("1"), mk_int("2")]))));
}
