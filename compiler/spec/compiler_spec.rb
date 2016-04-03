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

  it 'compiles a TEST_EQ instruction' do
    expect(compile("print:\ntest_eq %1, %2, print")).to eq([OpCodes::TEST_EQ, 1, 2, 0])
  end

  it 'compiles a TEST_GT instruction' do
    expect(compile("print:\ntest_gt %1, %2, print")).to eq([OpCodes::TEST_GT, 1, 2, 0])
  end

  it 'compiles a TEST_GTE instruction' do
    expect(compile("print:\ntest_gte %1, %2, print")).to eq([OpCodes::TEST_GTE, 1, 2, 0])
  end

  it 'compiles a TEST_LT instruction' do
    expect(compile("print:\ntest_lt %1, %2, print")).to eq([OpCodes::TEST_LT, 1, 2, 0])
  end

  it 'compiles a TEST_LTE instruction' do
    expect(compile("print:\ntest_lte %1, %2, print")).to eq([OpCodes::TEST_LTE, 1, 2, 0])
  end

  it 'compiles an ADD instruction' do
    expect(compile("add %3, %1, %2")).to eq([OpCodes::ADD, 3, 1, 2])
  end

  it 'compiles a SUB instruction' do
    expect(compile("sub %3, %1, %2")).to eq([OpCodes::SUB, 3, 1, 2])
  end

  it 'compiles a MOV instruction' do
    expect(compile("mov %3, %1")).to eq([OpCodes::MOV, 3, 1])
  end

  it 'compiles a RESTORE instruction' do
    expect(compile("restore %3")).to eq([OpCodes::RESTORE, 3])
  end

  it 'compiles a CALL instruction' do
    expect(compile("call main/1, %1\nfn main/1:")).to eq([OpCodes::CALL, 4, 1, 1])
  end

  it 'compiles a RETURN instruction' do
    expect(compile("return")).to eq([OpCodes::RETURN])
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
      expect(compile("store %1, 33\nprint:\ntest_eq %1, %2, print")).to eq([
        OpCodes::INT_STORE, 1, 33, 0,
        OpCodes::TEST_EQ, 1, 2, 4
      ])
    end

    it 'can translate forward addresses' do
      expect(compile("store %1, 33\ntest_eq %1, %2, print\nprint:\nint_print %1")).to eq([
        OpCodes::INT_STORE, 1, 33, 0,
        OpCodes::TEST_EQ, 1, 2, 8,
        OpCodes::INT_PRINT, 1
      ])
    end
  end
end
