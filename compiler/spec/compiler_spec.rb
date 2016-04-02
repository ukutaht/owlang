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

  it 'compiles an INT_STORE instruction' do
    expect(compile("store %1, 33")).to eq([OpCodes::INT_STORE, 1, 33, 0])
  end

  it 'compiles an INT_PRINT instruction' do
    expect(compile("int_print %1")).to eq([OpCodes::INT_PRINT, 1])
  end

  it 'compiles a CMP instruction' do
    expect(compile("cmp %1, %2")).to eq([OpCodes::CMP, 1, 2])
  end

  it 'compiles a JUMPZ instruction' do
    expect(compile("print:\njmpz print")).to eq([OpCodes::JMPZ, 0])
  end

  it 'compiles a CALL instruction' do
    expect(compile("call main/0\nfn main/0:")).to eq([OpCodes::CALL, 3, 0])
  end

  describe 'formatting' do
    it 'ignores whitespace on either side of the line' do
      expect(compile("  exit  1")).to eq([OpCodes::EXIT, 1])
    end

    it 'ignores empty lines' do
      expect(compile("\n")).to eq([])
    end

    it 'ignores comments' do
      expect(compile("exit 1; here be dragons")).to eq([OpCodes::EXIT, 1])
    end

    it 'raises when not enough arguments are given' do
      expect { compile("exit") }.to raise_error(CompileError)
    end

    it 'raises when bad arguments are given' do
      expect { compile("exit %2") }.to raise_error(CompileError)
      expect { compile("store 2") }.to raise_error(CompileError)
      expect { compile("store 2, 2") }.to raise_error(CompileError)
    end
  end

  describe 'label -> address translation' do
    it 'does not output anything for labels' do
      expect(compile("label:")).to eq([])
    end

    it 'translates label to the addres of instruction' do
      expect(compile("store %1, 33\nprint:\njmpz print")).to eq([
        OpCodes::INT_STORE, 1, 33, 0,
        OpCodes::JMPZ, 4
      ])
    end

    it 'can translate forward addresses' do
      expect(compile("store %1, 33\njmpz print\nprint:\nint_print %1")).to eq([
        OpCodes::INT_STORE, 1, 33, 0,
        OpCodes::JMPZ, 6,
        OpCodes::INT_PRINT, 1
      ])
    end

    it 'labels do not collide with opcodes' do
      expect(compile("int_print %1\njmpz print\nprint:\nint_print %1")).to eq([
        OpCodes::INT_PRINT, 1,
        OpCodes::JMPZ, 4,
        OpCodes::INT_PRINT, 1
      ])
    end
  end
end
