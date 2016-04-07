use std::io::Write;
use bytecode::function::Function;

#[derive(Debug, Eq, PartialEq)]
pub struct Module<'a> {
    pub name: &'a str,
    pub functions: Vec<Function<'a>>
}

impl<'a> Module<'a> {

    pub fn name_bytes(&self) -> Vec<u8> {
        let mut result = Vec::new();
        let length = self.name.len();

        result.push((length % 250) as u8);
        result.push((length / 250) as u8);
        result.append(&mut self.name.to_string().into_bytes());
        result.push(0 as u8);
        result
    }

    pub fn emit<T: Write>(&self, out: &'a mut T) {
        out.write(&self.name_bytes()).unwrap();

        for function in self.functions.iter() {
            function.emit(out);
        }
    }

    pub fn emit_human_readable<T: Write>(&self, out: &'a mut T) {
        let header = format!("module {}\n\n", self.name);
        out.write(&header.as_bytes()).unwrap();

        for function in self.functions.iter() {
            function.emit_human_readable(out);
        }
    }
}

