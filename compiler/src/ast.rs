#[derive(Debug, Eq, PartialEq)]
pub struct Int<'a> {
    pub value: &'a str,
}

#[derive(Debug, Eq, PartialEq)]
pub struct Let<'a> {
    pub left: Ident<'a>,
    pub right: Box<Expr<'a>>
}

#[derive(Debug, Eq, PartialEq)]
pub struct Ident<'a> {
    pub name: &'a str,
}

#[derive(Debug, Eq, PartialEq)]
pub struct Apply<'a> {
    pub module: Option<&'a str>,
    pub name: &'a str,
    pub args: Vec<Expr<'a>>,
}

impl<'a> Apply<'a> {
    pub fn arity(&self) -> u8 {
        self.args.len() as u8
    }
}

#[derive(Debug, Eq, PartialEq)]
pub struct If<'a> {
    pub condition: Box<Expr<'a>>,
    pub body: Vec<Expr<'a>>,
    pub else_body: Vec<Expr<'a>>,
}

#[derive(Debug, Eq, PartialEq)]
pub struct Function<'a> {
    pub name: &'a str,
    pub args: Vec<Argument<'a>>,
    pub body: Vec<Expr<'a>>,
}

impl<'a> Function<'a> {
    pub fn arity(&self) -> u8 {
        self.args.len() as u8
    }
}

#[derive(Debug, Eq, PartialEq)]
pub struct Argument<'a> {
    pub name: &'a str,
}

#[derive(Debug, Eq, PartialEq)]
pub struct Tuple<'a> {
    pub elems: Vec<Expr<'a>>,
}

#[derive(Debug, Eq, PartialEq)]
pub struct List<'a> {
    pub elems: Vec<Expr<'a>>,
}

#[derive(Debug, Eq, PartialEq)]
pub struct Str<'a> {
    pub value: &'a str,
}

#[derive(Debug, Eq, PartialEq)]
pub enum Expr<'a> {
    Int(Int<'a>),
    True,
    False,
    Nil,
    Let(Let<'a>),
    Ident(Ident<'a>),
    Apply(Apply<'a>),
    If(If<'a>),
    Tuple(Tuple<'a>),
    List(List<'a>),
    Str(Str<'a>),
}

#[derive(Debug, Eq, PartialEq)]
pub struct Module<'a> {
    pub name: &'a str,
    pub functions: Vec<Function<'a>>,
}



pub fn mk_int(val: &str) -> Expr {
    Expr::Int(Int {value: val})
}

pub fn mk_true<'a>() -> Expr<'a> {
    Expr::True
}

pub fn mk_false<'a>() -> Expr<'a> {
    Expr::False
}

pub fn mk_nil<'a>() -> Expr<'a> {
    Expr::Nil
}

pub fn mk_ident(name: &str) -> Expr {
    Expr::Ident(Ident {name: name})
}

pub fn mk_apply<'a>(module: Option<&'a str>, name: &'a str, args: Vec<Expr<'a>>) -> Expr<'a> {
    Expr::Apply(Apply {module: module, name: name, args: args})
}

pub fn mk_function<'a>(name: &'a str, args: Vec<Argument<'a>>, body: Vec<Expr<'a>>) -> Function<'a> {
    Function {name: name, args: args, body: body}
}

pub fn mk_argument(name: &str) -> Argument {
    Argument {name: name}
}

pub fn mk_if<'a>(condition: Expr<'a>, body: Vec<Expr<'a>>, else_body: Vec<Expr<'a>>) -> Expr<'a> {
    Expr::If(If{condition: Box::new(condition), body: body, else_body: else_body})
}

pub fn mk_tuple<'a>(elems: Vec<Expr<'a>>) -> Expr<'a> {
    Expr::Tuple(Tuple{elems: elems})
}

pub fn mk_list<'a>(elems: Vec<Expr<'a>>) -> Expr<'a> {
    Expr::List(List{elems: elems})
}

pub fn mk_module<'a>(name: &'a str, fns: Vec<Function<'a>>) -> Module<'a> {
    Module { name: name, functions: fns  }
}

pub fn mk_string(string: &str) -> Expr {
    Expr::Str(Str {value: string})
}

pub fn mk_let<'a>(left: Expr<'a>, right: Expr<'a>) -> Expr<'a> {
    let ident = match left {
        Expr::Ident(i) => i,
        _ => panic!("Expected ident")
    };
    Expr::Let(Let {left: ident, right: Box::new(right)})
}
