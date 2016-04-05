use std::str;
use chomp::{Input, U8Result, ParseError, Error, parse_only, take_while1, take_while};
use chomp::{token, string};
use chomp::parsers::{satisfy, peek_next};
use chomp::ascii::{is_digit, is_alpha, is_lowercase, skip_whitespace, is_end_of_line, is_whitespace};
use chomp::combinators::{or, sep_by, option};

use ast::*;

pub fn parse(input: &[u8]) -> Result<Expr, ParseError<u8, Error<u8>>> {
    parse_only(expr, input)
}

pub fn expr(i: Input<u8>) -> U8Result<Expr> {
    parse!{i;
        function() <|> apply() <|> infix() <|> ident() <|> int()
    }
}

pub fn function(i: Input<u8>) -> U8Result<Expr> {
    parse!{i;
        string(b"fn");
        satisfy(|i| is_whitespace(i));
        let name = identifier();
        token(b'(');
        skip_whitespace();
        let args: Vec<_> = sep_by(argument, comma);
        skip_whitespace();
        token(b')');
        skip_newline_and_whitespace();
        token(b'{');
        skip_newline_and_whitespace();
        let body: Vec<_> = sep_by(expr, skip_newline_and_whitespace);
        skip_newline_and_whitespace();
        token(b'}');
        skip_newline_and_whitespace();

        ret mk_function(name, args, body)
    }
}

fn apply(i: Input<u8>) -> U8Result<Expr> {
    parse!{i;
        let module = option(module_prefix, None);
        let name = identifier();
        token(b'(');
        skip_whitespace();
        let args: Vec<_> = sep_by(expr, comma);
        skip_whitespace();
        token(b')');

        ret mk_apply(module, name, args)
    }
}

fn module_prefix(i: Input<u8>) -> U8Result<Option<&str>> {
    parse!{i;
        let name = identifier();
        string(b"::");
        ret Some(name)
    }
}

fn int(i: Input<u8>) -> U8Result<Expr> {
    take_while1(i, is_digit)
        .map(|utf8|
            mk_int(unsafe { str::from_utf8_unchecked(utf8) })
        )
}

fn ident(i: Input<u8>) -> U8Result<Expr> {
    parse!{i;
        let name = identifier();

        ret mk_ident(name)
    }
}

fn infix(i: Input<u8>) -> U8Result<Expr> {
    parse!{i;
        let lhs = int() <|> ident();
        skip_whitespace();
        let op = infix_op();
        skip_whitespace();
        let rhs = expr();

        ret Expr::Apply(Apply {
            module: None,
            name: op,
            args: vec![lhs, rhs]
        })
    }
}

fn infix_op(i: Input<u8>) -> U8Result<&str> {
    or(i, |i| string(i, b"+"),
          |i| string(i, b"-"))
        .map(|op| unsafe { str::from_utf8_unchecked(op) })
}

fn argument(i: Input<u8>) -> U8Result<Argument> {
    parse!{i;
        let name = identifier();

        ret mk_argument(name)
    }
}

fn identifier(i: Input<u8>) -> U8Result<&str> {
    peek_next(i).bind(|i, first_char| {
        if is_lowercase(first_char) {
            any_case_ident(i)
        } else {
            i.err(Error::Unexpected)
        }
    })
}

fn any_case_ident(i: Input<u8>) -> U8Result<&str> {
    take_while1(i, |i| is_alpha(i) || is_allowed_special_character(i) || is_digit(i))
        .map(|utf8| unsafe { str::from_utf8_unchecked(utf8) })
}

fn is_allowed_special_character(c: u8) -> bool {
    c == b'_'
}

fn comma(i: Input<u8>) -> U8Result<()> {
    parse!(i;
           skip_whitespace();
           token(b',');
           skip_whitespace();
    )
}

pub fn skip_newline_and_whitespace(i: Input<u8>) -> U8Result<()> {
    take_while(i, |i| is_end_of_line(i) || is_whitespace(i)).map(|_| ())
}
