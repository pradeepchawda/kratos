from kratos import Generator, signed
from kratos.util import reduce_or, Const, const


def test_expr():
    mod = Generator("module")
    a = mod.var("a", 2)
    b = mod.var("b", 2)
    expr = a + b
    assert str(expr) == "a + b"


def test_slice():
    mod = Generator("module")
    a = mod.var("a", 2)
    b = a[0]
    assert b.width == 1
    b = a[1, 0]
    assert b.width == 2
    assert str(b) == "a[1:0]"


def test_assign():
    mod = Generator("module")
    a = mod.var("a", 2)
    b = mod.var("b", 2)
    assign = a.assign(b)
    assert assign.left == a
    assert assign.right == b


def test_signed():
    mod = Generator("module")
    a = mod.var("a", 2)
    c = signed(a)
    assert str(c) == "$signed(a)"


def test_reduce_or():
    mod = Generator("module")
    a = mod.var("a", 2)
    b = mod.var("b", 2)
    expr = reduce_or(a, b)
    assert str(expr) == "a | b"
    assert str(reduce_or(a)) == "a"


def test_const():
    a = Const[4](2)
    b = const(2, 4)
    assert str(a) == str(b)
    assert str(a) == "4'h2"
