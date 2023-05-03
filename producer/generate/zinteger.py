# Copyright (c) 2023 Chubak Bidpaa
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

from copy import copy
from ctypes import *
from math import inf
from operator import *

SELF_INSTANCE = 0
TYPE_INSTANCE = 1
INTG_INSTANCE = 2
OTHR_INSTANCE = -1


__TYPES_MAP = {
    "ZUint8": c_uint8,
    "ZUint16": c_uint16,
    "ZUint32": c_uint32,
    "ZUint64": c_uint64,
    "ZInt8": c_int8,
    "ZInt16": c_int16,
    "ZInt32": c_int32,
    "ZInt64": c_int64,
}

__TYPES_MAX = {
    "ZUint8": 0xff,
    "ZUint16": 0xffff,
    "ZUint32": 0xffffffff,
    "ZUint64": 0xffffffffffffffff,
    "ZInt8": 0x7f,
    "ZInt16": 0x7fff,
    "ZInt32": 0x7fffffff,
    "ZInt64": 0x7fffffffffffffff,
}

__TYPES_MIN = {
    "ZUint8": 0,
    "ZUint16": 0,
    "ZUint32": 0,
    "ZUint64": 0,
    "ZInt8": -0x80,
    "ZInt16": -0x80000,
    "ZInt32": -0x80000000,
    "ZInt64": -0x8000000000000000,
}


__DUNDERS_UNARY = {
    "__hash__": hash,
    "__hex__": hex,
    "__bin__": bin,
    "__oct__": oct,
    "__not__": not_,
    "__neg__": neg,
    "__pos__": pos,
}

__DUNDERS_BINARY = {
    "__add__": add,
    "__sub__": sub,
    "__mul__": mul,
    "__trudiv__": truediv,
    "__floordiv__": floordiv,
    "__mod__": mod,
    "__pow__": pow,
    "__and__": and_,
    "__or__": or_,
    "__xor__": xor,
    "__lshift__": lshift,
    "__rshift__": rshift,
    "__lt__": lt,
    "__le__": le,
    "__eq__": eq,
    "__ne__": ne,
    "__ge__": ge,
    "__gt__": gt,
}

__DUNDERS_INPLACE = {
    "__iadd__": iadd,
    "__isub__": isub,
    "__imul__": imul,
    "__itrudiv__": itruediv,
    "__ifloordiv__": ifloordiv,
    "__imod__": imod,
    "__ipow__": ipow,
    "__iand__": iand,
    "__ior__": ior,
    "__ixor__": ixor,
    "__ilshift__": ilshift,
    "__irshift__": irshift,
}


__FMT = {
    'b': bin,
    'o': oct,
    'x': hex,
}


def __exec(func):
    globals()[func.__defaults__[0]] = func()
    return func


@__exec
def __generate_unary_dunders(_="UNARY_DUNDERS"):
    unary_dunderfuncs = {}

    for unary_dunder, unary_op in __DUNDERS_UNARY.items():

        def dunder_factory(unary_dunder, unary_op):
            def dunderfn(self):
                op = dunderfn.__dict__['op']
                return op(self.val)

            dunderfn.__dict__['op'] = unary_op
            dunderfn.__name__ = unary_dunder
            dunderfn.__qualname__ = unary_dunder

            return dunderfn

        unary_dunderfuncs[unary_dunder] = dunder_factory(
            unary_dunder, unary_op)

    return unary_dunderfuncs


@__exec
def __generate_binary_dunders(_="BINARY_DUNDERS"):
    binary_dunderfuncs = {}

    for binary_dunder, binary_op in __DUNDERS_BINARY.items():

        def dunder_factory(binary_dunder, binary_op):
            def dunderfn(self, other):
                op = dunderfn.__dict__['op']

                instance_check = self.__instancecheck__(other)
                if instance_check == SELF_INSTANCE:
                    res = op(self.__maxand__(0), other.__maxand__(0))
                elif instance_check == TYPE_INSTANCE:
                    res = op(self.__maxand__(0), self.__maxand__(other.value, on_self=False))
                elif instance_check == INTG_INSTANCE:
                    res = op(self.__maxand__(0), self.__maxand__(other, on_self=False))
                else:
                    raise TypeError(
                        f"Ilegal operation {op.__name__} between {self.clsname} and {other.__class__.__name__}")

                return self.clstype(res) if type(res) != bool else res

            dunderfn.__dict__['op'] = binary_op
            dunderfn.__name__ = binary_dunder
            dunderfn.__qualname__ = binary_dunder

            return dunderfn

        binary_dunderfuncs[binary_dunder] = dunder_factory(
            binary_dunder, binary_op)

    return binary_dunderfuncs


@__exec
def __generate_inplace_dunders(_="INPLACE_DUNDERS"):
    inplace_dunderfuncs = {}

    for inplace_dunder, inplace_op in __DUNDERS_INPLACE.items():

        def dunder_factory(inplace_dunder, inplace_op):
            def dunderfn(self, other):
                op = dunderfn.__dict__['op']

                instance_check = self.__instancecheck__(other)
                if instance_check == SELF_INSTANCE:
                    res = op(self.__maxand__(0), other.__maxand__(0))
                elif instance_check == TYPE_INSTANCE:
                    res = op(self.__maxand__(0), self.__maxand__(other.value, on_self=False))
                elif instance_check == INTG_INSTANCE:
                    res = op(self.__maxand__(0), self.__maxand__(other, on_self=False))
                else:
                    raise TypeError(
                        f"Ilegal operation {op.__name__} between {self.clsname} and {other.__class__.__name__}")

                self.__integer = self.__tyy(res)
                return self

            dunderfn.__dict__['op'] = inplace_op
            dunderfn.__name__ = inplace_dunder
            dunderfn.__qualname__ = inplace_dunder

            return dunderfn

        inplace_dunderfuncs[inplace_dunder] = dunder_factory(
            inplace_dunder, inplace_op)

    return inplace_dunderfuncs


def __byte_zinteger(self, msb_leftmost=True) -> bytearray:
    if self.min < 0:
        raise OverflowError("Negative numbers cannot be decomposed to bytes")
    value = self.val
    decomposed_bytes = bytearray([])
    shrn = 0
    while True:
        value >>= shrn
        if value == 0:
            break
        byte = value & 255
        decomposed_bytes += bytearray([byte])
        shrn += 8
    if msb_leftmost:
        decomposed_bytes.reverse()
    return bytes(decomposed_bytes)


def __maxd_zinteger(self, val: int, on_self=True) -> int:
    val = self.val if on_self else val
    if val < 0:
        return val

    return val & self.__max


def __init_zinteger(self, integer: int, mask=None, warn=True, abort=False):
    if type(integer) not in [self.cty, self.sty, int]:
        raise TypeError(f"Unallowed type for {self.clsname}: {type(integer)}")

    integer &= mask if integer >= 0 else integer

    if warn or abort:
        do_exit = 0
        if integer > self.max:
            print(f'\033[1;33mWarning:\033[0m Overflow for {self.clsname}')
            do_exit = "overflow"
        if integer < self.min:
            print(f'\033[1;33mWarning:\033[0m Underflow for {self.clsname}')
            do_exit = "underflow"
        if do_exit:
            print(f"Exiting due to {do_exit}...")
            exit(1)

    self.__integer = self.cty(integer.value) if type(integer) in [
        self.cty, self.sty] else self.cty(integer)


def __inst_zinteger(self, other) -> bool:
    cls_other = other.__class__
    cls_self = self.clstype
    cls_type = self.ctype
    cls_int = int(0).__class__

    if cls_self == cls_other:
        return SELF_INSTANCE
    elif cls_type == cls_other:
        return TYPE_INSTANCE
    elif cls_int == cls_other:
        return INTG_INSTANCE

    return OTHR_INSTANCE


def __geti_zinteger(self, cls, instance=None):
    return self.__integer


def __seti_zinteger(self, instance, value):
    self.__integer = value


def __repr_zinteger(self) -> str:
    return str(self.val)


def __strn_zinteger(self) -> str:
    return str(self.val)


def __frmt_zinteger(self, spec: str) -> str:
    fmtfn = __frmt_zinteger.__dict__['frmt_zintegers']
    if any([spec.endswith(c) for c in ['b', 'o', 'x']]):
        fmt = spec[-1]
        prefix = "0" + fmt
        spec = spec.rstrip(fmt)
        if spec.startswith('0'):
            spec = spec.lstrip('0')
            numpad = int(spec)
            return ('0' * numpad) + fmtfn[fmt](self).lstrip(prefix)
        elif spec == '':
            return fmtfn[fmt](self).lstrip(prefix)
        else:
            return ""
    return ""


def __gtvl_zinteger(self, name: str) -> int:
    if name.startswith("val") or name.startswith("int"):
        return self.__integer.value
    elif name.startswith("cint") or name.startswith("cval"):
        return self.__integer
    elif name.startswith("cty"):
        return self.__tyy
    elif name.startswith("ctyname"):
        return self.__tyy.__name__
    elif name.startswith("clsn") or name == "cnm":
        return self.__class__.__name__
    elif name.startswith("clst") or name == "clt":
        return self.__class__
    elif name.startswith("min"):
        return self.__min
    elif name.startswith("max"):
        return self.__max
    elif name.startswith("sty"):
        return type(self)
    elif name.startswith("nb"):
        return len(format(self.__max, "b"))
    elif name.startswith("test"):
        return self(0).__test__
    else:
        raise AttributeError


def __docs_zinteger(ident_tyy, ident_slf, ident_min, ident_max):
    docs_tynm = ident_tyy.__name__
    docs_sign = "an unsigned" if ident_slf.startswith("ZUint") else "a signed"
    docs_bitw = ident_slf.lstrip("Z").lstrip("U").lstrip("I").lstrip("int")
    docs_strt = f"{ident_slf} takes {docs_sign}, {docs_bitw}-bit value of range [{ident_min}, {ident_max})"
    docs_ctyy = f"This class maps to ctype {docs_tynm} on your system. It may differ on other systems."
    docs_dndr = "ZIntmplements: " + ", ".join(__DUNDERS_BINARY.keys()) + ""
    docs_fmtt = "__format__ is implemented for [padding-enabled] b, o, x flags"
    docs_args = "Parameters:"
    docs_arg1 = ";integer: the given in-range integer"
    docs_arg2 = ";mask: the pre-masking bitmask (meaningless for negative integers)"
    docs_retr = "Returns:"
    docs_rett = "An instance of {}"
    docs_sepr = "-------"

    return "\n".join([
        docs_strt, docs_sepr,
        docs_ctyy, docs_dndr,
        docs_fmtt, docs_sepr,
        docs_args, docs_arg1,
        docs_arg2, docs_sepr,
        docs_retr, docs_rett
    ])


def __zinteger(cls: type):
    dunderfuncs = {
        **globals()['UNARY_DUNDERS'],
        **globals()['BINARY_DUNDERS'],
        **globals()['INPLACE_DUNDERS'],
    }

    ident_slf = cls.__name__
    ident_min = __TYPES_MIN[ident_slf]
    ident_max = __TYPES_MAX[ident_slf]
    ident_tyy = __TYPES_MAP[ident_slf]

    this_init = copy(__init_zinteger)
    this_frmt = copy(__frmt_zinteger)
    this_inst = copy(__inst_zinteger)
    this_repr = copy(__repr_zinteger)
    this_strn = copy(__strn_zinteger)
    this_gtvl = copy(__gtvl_zinteger)
    this_geti = copy(__geti_zinteger)
    this_seti = copy(__seti_zinteger)
    this_maxd = copy(__maxd_zinteger)
    this_byte = copy(__byte_zinteger)

    this_docs = __docs_zinteger(ident_tyy, ident_slf, ident_min, ident_max)

    this_init.__defaults__ = (ident_max, *this_init.__defaults__[1:])
    this_frmt.__dict__['frmt_zintegers'] = __FMT

    cls = type(
        ident_slf,
        (), {
            "__tyy": ident_tyy,
            "__min": ident_min,
            "__max": ident_max,
            "__idt": ident_slf,
            "__get__": this_geti,
            "__set__": this_seti,
            "__str__": this_strn,
            "__init__": this_init,
            "__repr__": this_repr,
            "__bytes__": this_byte,
            "__maxand__": this_maxd,
            "__format__": this_frmt,
            "__getattr__": this_gtvl,
            "__instancecheck__": this_inst,
            **dunderfuncs
        }
    )

    def set_method_name(method: callable, name: str):
        method.__name__ = name
        method.__qualname__ = name

    list(
        map(
            lambda args: set_method_name(*args),
            [
                (cls.__str__, "__str__"),
                (cls.__get__, "__get__"),
                (cls.__set__, "__set__"),
                (cls.__init__, "__init__"),
                (cls.__repr__, "__repr__"),
                (cls.__bytes__, "__bytes__"),
                (cls.__format__, "__format__"),
                (cls.__maxand__, "__maxand__"),
                (cls.__instancecheck__, "__instancecheck__"),
            ]
        )
    )
    cls.__doc__ = this_docs

    return cls


@__zinteger
class ZUint8:
    pass


@__zinteger
class ZUint16:
    pass


@__zinteger
class ZUint32:
    pass


@__zinteger
class ZUint64:
    pass


@__zinteger
class ZInt8:
    pass


@__zinteger
class ZInt16:
    pass


@__zinteger
class ZInt32:
    pass


@__zinteger
class ZInt64:
    pass


__all__ = [
    ZUint8,
    ZUint16,
    ZUint32,
    ZUint64,
    ZInt8,
    ZInt16,
    ZInt32,
    ZInt64,
]
