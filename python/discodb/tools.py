"""
Tools to help in constructing DiscoDB objects.

>>> kvlist = [(b'k', b'v'), (b'k', b'v2'), (b'k2', b'v2')]
>>> [(k, list(v)) for k, v in kvgroup(kvlist)]
[(b'k', [b'v', b'v2']), (b'k2', [b'v2'])]
"""

from itertools import chain, groupby


def iterify(object):
    if hasattr(object, '__iter__'):
        return object
    return object,


def key(kv):
    k, v = kv
    return k


def kvgroup(kviter):
    """Like itertools.groupby, but iterates over (k, vs) instead of (k, k-vs).

    The result can be used to construct a :class:`discodb.DiscoDB`,
    iff ``kviter`` is in sorted order.
    """
    for k, kvs in groupby(kviter, key):
        yield k, (v for _k, v in kvs)


def normalize(iter):
    for k, vss in kvgroup(sorted(iter)):
        yield k, chain(*(iterify(vs) for vs in vss))

