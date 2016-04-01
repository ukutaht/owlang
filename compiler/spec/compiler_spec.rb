require 'spec_helper'
require 'opcodes'
require 'compiler'

RSpec.describe Compiler do
  def compile(code)
    out = StringIO.new
    Compiler.new(code, out).compile
    out.rewind
    out.string.unpack('C*')
  end

  it 'compiles an EXIT instruction' do
    expect(compile("exit 0")).to eq([OpCodes::EXIT, 0])
  end

  it 'ignores whitespace on either side of the line' do
    expect(compile("  exit  1")).to eq([OpCodes::EXIT, 1])
  end

  it 'compiles an INT_STORE instruction' do
    expect(compile("store #1, 33")).to eq([OpCodes::INT_STORE, 1, 33, 0])
  end

  it 'compiles an INT_PRINT instruction' do
    expect(compile("int_print #1")).to eq([OpCodes::INT_PRINT, 1])
  end
end
