"""
DiscoDB python wrapper to libdiscodb's handling


This module wraps the python c extension
at discodb._discodb
that does the heavy lifting for this module.

discodb requires a system wide (or otherwise appropriately installed)
    libcmph (licensed LGPL)


"""

import six
try:
    from ._discodb import _DiscoDB, DiscoDBConstructor, DiscoDBError, DiscoDBIter, DiscoDBView
except ImportError as imp_err:
    from sys import stderr
    RED = '\033[0;31m'
    NC = '\033[0m' # No Color
    if "libcmph" in str(imp_err):
        stderr.write("----------------------------------------------------\n"
                     f"{RED}DiscoDB import Error{NC}:\n"
                     f"       {RED}libcmph{NC} not found on system, \n"
                     f"       please obtain system-wide libcmph\n"
                     f"       to comply with LGPL License\n"
                     "            apt-get install libcmph0\n"
                     "            apk add libcmph\n"
                     "            dnf/yum install cmph (third party)\n"
                     "            brew install libcmph (third party)\n"
                     "----------------------------------------------------\n")
    elif "libdisco" in str(imp_err):
        stderr.write("-----------------------------------------------------------\n"
                     f"{RED}DiscoDB import Error{NC}:\n"
                     f"       {RED}libdiscodb{NC} not found on system, \n"
                     f"       please obtain system-wide libdiscodb and libcmph\n"
                     f"       to comply with LGPL License\n"
                     "-----------------------------------------------------------\n")
    raise


from .query import Q
from .tools import kvgroup


def discodb_unpickle(string):
    return DiscoDB.loads(string)


class DiscoDBInquiry(object):
    def __init__(self, iterfunc):
        self.iterfunc = iterfunc

    def __iter__(self):
        return self.iterfunc()

    def __len__(self):
        return iter(self).size()

    def __nonzero__(self):
        try:
            iter(self).next()
        except StopIteration:
            return False
        return True

    def __format__(self, format_spec='%s.3'):
        format_str, precision = format_spec.rsplit('.', 1)

        def firstN(N):
            for n, item in enumerate(self):
                if n == N:
                    yield '...'
                    return
                yield format_str % item
        return ', '.join(firstN(int(precision)))

    def __str__(self):
        return '%s([%s])' % (self.__class__.__name__, self.__format__())


class DiscoDBLazyInquiry(DiscoDBInquiry):
    def __len__(self):
        return iter(self).count()


class DiscoDBItemInquiry(DiscoDBLazyInquiry):
    def __len__(self):
        return sum(1 for i in self)

    def __format__(self, format_spec='(%s, %s).3'):
        return super(DiscoDBItemInquiry, self).__format__(format_spec)


class DiscoDB(_DiscoDB):
    """DiscoDB(iter[, flags]) -> new DiscoDB from k, v[s] in iter."""
    def __len__(self):
        return len(self.keys())

    def __reduce__(self):
        return discodb_unpickle, (self.dumps(), )

    def __str__(self):
        return '%s({%s})' % (self.__class__.__name__,
                             self.items().__format__('%s: %s.3'))

    def __getitem__(self, key):
        return DiscoDBInquiry(lambda: super(DiscoDB, self).__getitem__(key))

    def get(self, key, default=None):
        """self[key] if key in self, else default.
        keys are bytes.
        """
        if key in self:
            return self[key]
        return default

    def items(self):
        """an inquiry over the items of self."""
        return DiscoDBItemInquiry(lambda: ((k, self[k]) for k in self))

    def keys(self):
        """an inquiry over the keys of self."""
        return DiscoDBInquiry(super(DiscoDB, self).keys)

    def values(self):
        """an inquiry over the values of self."""
        return DiscoDBInquiry(super(DiscoDB, self).values)

    def unique_values(self):
        """an inquiry over the unique values of self."""
        return DiscoDBInquiry(super(DiscoDB, self).unique_values)

    def metaquery(self, query):
        """
        an inquiry over the (q, values) of self in the expansion of the query.

        See :mod:`discodb.query` for more information.
        """
        if isinstance(query, six.string_types):
            query = Q.parse(query)
        return DiscoDBItemInquiry(lambda: query.metaquery(self))

    def query(self, query, view=None):
        """
        an inquiry over the values of self whose keys satisfy the query.

        The query can be either a :class:`Q` object, or text.
        If text, it is transformed into a :class:`Q` via :meth:`Q.parse`.
        """
        if isinstance(query, six.string_types):
            query = Q.parse(query)
        if view is None:
            l = lambda: super(DiscoDB, self).query(query)
        else:
            if not isinstance(view, DiscoDBView):
                view = self.make_view(view)
            l = lambda: super(DiscoDB, self).query(query, view=view)
        return DiscoDBLazyInquiry(l)

    def peek(self, key, default=None):
        """first element of self[key] or else default."""
        try:
            return iter(self.get(key, [])).next()
        except StopIteration:
            return default

    def make_view(self, data):
        return DiscoDBView(self, data)


__all__ = ['DiscoDB',
           'DiscoDBConstructor',
           'DiscoDBError',
           'DiscoDBInquiry',
           'DiscoDBIter',
           'DiscoDBView',
           'Q',
           'kvgroup']
