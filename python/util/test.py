import doctest
import unittest
from random import randint

from discodb import DiscoDB, Q
from discodb import DiscoDBConstructor
from discodb import query
from discodb import tools

def k_vs_iter(N, max_values=100):
    for x in range(N):
        # Force every key to have at least ONE value so that the test in
        # test_query_noresults doesn't erroneously pass. The problem there is
        # that queries that should return no results were returning the very
        # first key's value. So, if the first key has no value, then the test
        # could pass but we wouldn't know if it passed because the query truly
        # returned no results (as it should) or if it actually returned the
        # first key's value, which was also empty.
        yield ('%s' % x).encode(), (('%s' % v).encode() for v in range(randint(1, max_values)))

class TestConstructor(unittest.TestCase):
    def test_null_constructor(self):
        discodb = DiscoDB()

    def test_dict_constructor(self):
        discodb = DiscoDB(dict(k_vs_iter(1000)))

    def test_list_constructor(self):
        return
        discodb = DiscoDB(list(k_vs_iter(1000)))

    def test_iter_constructor(self):
        discodb = DiscoDB(k_vs_iter(1000))

    def test_kviter_constructor(self):
        discodb = DiscoDB((k, v) for k, vs in k_vs_iter(1000) for v in vs)

    def test_flags_constructor(self):
        discodb = DiscoDB(k_vs_iter(1000), disable_compression=True)
        discodb = DiscoDB(k_vs_iter(1000), unique_items=True)

    def test_external_constructor(self):
        discodb_constructor = DiscoDBConstructor()
        for k, vs in k_vs_iter(1000):
            discodb_constructor.add(k, vs)
        discodb = discodb_constructor.finalize()

class TestMappingProtocol(unittest.TestCase):
    numkeys = 1000

    def setUp(self):
        self.discodb = DiscoDB(k_vs_iter(self.numkeys))

    def test_contains(self):
        assert b"0" in self.discodb
        assert b"key" not in self.discodb

    def test_nonzero(self):
        # self.assertFalse(self.discodb.query('NONKEY'))
        # self.assertTrue(self.discodb.query(b'0'))
        self.assertTrue(self.discodb.values())
        self.assertTrue(self.discodb.keys())

    def test_length(self):
        self.assertEqual(len(self.discodb), self.numkeys)

    def test_get(self):
        len(list(self.discodb.get(b'0')))
        self.assertEqual(self.discodb.get(b'X'), None)
        self.assertEqual(self.discodb.get(b'X', b'Y'), b'Y')

    def test_getitem(self):
        for x in range(self.numkeys):
            try:
                list(self.discodb[str(x).encode()])
            except KeyError:
                self.assertEqual(x, self.numkeys)

    def test_iter(self):
        self.assertEqual(list(self.discodb), list(self.discodb.keys()))

    def test_items(self):
        for key, values in self.discodb.items():
            key, list(values)

    def test_keys(self):
        len(list(self.discodb.keys()))

    def test_values(self):
        len(list(self.discodb.values()))

    def test_unique_values(self):
        len(list(self.discodb.unique_values()))

    def test_peek(self):
        self.assertNotEqual(self.discodb.peek(b'0'), None)
        self.assertEqual(self.discodb.peek(b'X'), None)
        self.assertTrue(int(self.discodb.peek(b'0', b'1')) >= 0)

    def test_query(self):
        q = Q.parse('5 & 10 & (15 | 30)')
        list(self.discodb.query(q))

    def test_query_results(self):
        q = Q.parse('5')
        self.assertEqual(list(self.discodb.query(q)),
                          list(self.discodb.get(b'5')))

    def test_query_results_nonkey(self):
        q = Q.parse('nonkey')
        self.assertEqual(list(self.discodb.query(q)), [])

    def test_str(self):
        repr(self.discodb)
        str(self.discodb)

class TestLargeMappingProtocol(TestMappingProtocol):
    numkeys = 10000

class TestSerializationProtocol(unittest.TestCase):
    numkeys = 10000

    def setUp(self):
        self.discodb = DiscoDB(k_vs_iter(self.numkeys))

    def test_dumps_loads(self):
        dbuffer = self.discodb.dumps()
        self.assertEqual(dbuffer, DiscoDB.loads(dbuffer).dumps())

    def test_dump_load(self):
        from tempfile import NamedTemporaryFile
        handle = NamedTemporaryFile()
        self.discodb.dump(handle)
        handle.seek(0)
        discodb = DiscoDB.load(handle)
        self.assertEqual(discodb.dumps(), self.discodb.dumps())

class TestLargeSerializationProtocol(TestSerializationProtocol):
    numkeys = 10000

class TestUncompressed(TestMappingProtocol, TestSerializationProtocol):
    def setUp(self):
        self.discodb = DiscoDB(k_vs_iter(self.numkeys),
                               disable_compression=True)
        self.discodb_c = DiscoDB(self.discodb)

    def test_compression(self):
        self.assertEqual(dict((k, list(vs)) for k, vs in self.discodb.items()),
                         dict((k, list(vs)) for k, vs in self.discodb_c.items()))

class TestUniqueItems(TestMappingProtocol, TestSerializationProtocol):
    def setUp(self):
        base = dict(k_vs_iter(self.numkeys))
        base[b'0'] = [b'1', b'1', b'2']
        self.discodb = DiscoDB(base, unique_items=True)

    def test_uniq(self):
        self.assertEqual(list(self.discodb[b'0']), [b'1', b'2'])

class TestQuery(unittest.TestCase):
    def setUp(self):
        self.discodb = DiscoDB(
            {
                b"alice": [b"blue"],
                b"bob": [b"red"],
                b"carol": [b"blue", b"red"]
            }
        )

    def q(self, s):
        return self.discodb.query(Q.parse(s))

    def test_empty(self):

        self.assertEqual(list(self.q('')), [])
        self.assertEqual(len(self.q('')), 0)

    def test_get_len(self):
        self.assertEqual(len(self.discodb.get(b'alice')), 1)
        self.assertEqual(len(self.discodb.get(b'bob')), 1)
        self.assertEqual(len(self.discodb.get(b'carol')), 2)

    def test_query_len(self):
        self.assertEqual(len(self.q('alice')), 1)
        self.assertEqual(len(self.q('bob')), 1)
        self.assertEqual(len(self.q('carol')), 2)
        self.assertEqual(len(self.q('alice & bob')), 0)
        self.assertEqual(len(self.q('alice | bob')), 2)
        self.assertEqual(len(self.q('alice & carol')), 1)
        self.assertEqual(len(self.q('alice | carol')), 2)
        self.assertEqual(len(self.q('alice|bob|carol')), 2)
        self.assertEqual(len(self.q('alice&bob&carol')), 0)

    def test_query_len_doesnt_advance_iter(self):
        # check that calling len() doesn't advance the iterator
        res = self.q('alice')
        self.assertEqual(len(res), 1)
        self.assertEqual(len(res), 1)

    def test_query_results(self):
        self.assertEqual(set(self.q('alice')), set([b'blue']))
        self.assertEqual(set(self.q('bob')), set([b'red']))
        self.assertEqual(set(self.q('carol')), set([b'blue', b'red']))
        self.assertEqual(set(self.q('alice & bob')), set())
        self.assertEqual(set(self.q('alice | bob')), set([b'blue', b'red']))
        self.assertEqual(set(self.q('alice & carol')), set([b'blue']))
        self.assertEqual(set(self.q('alice | carol')), set([b'blue', b'red']))
        self.assertEqual(set(self.q('alice|bob|carol')), set([b'blue', b'red']))
        self.assertEqual(set(self.q('alice&bob&carol')), set())

    def test_query_len_nonkey(self):
        self.assertEqual(len(self.q('nonkey')), 0)
        self.assertEqual(len(self.q('~nonkey')), 2)
        self.assertEqual(len(self.q('nonkey & alice')), 0)
        self.assertEqual(len(self.q('nonkey | alice')), 1)

    def test_query_results_nonkey(self):
        self.assertEqual(set(self.q('nonkey')), set())
        self.assertEqual(set(self.q('~nonkey')), set([b'blue', b'red']))
        self.assertEqual(set(self.q('nonkey & alice')), set())
        self.assertEqual(set(self.q('nonkey | alice')), set([b'blue']))


if __name__ == '__main__':
    unittest.TextTestRunner().run(doctest.DocTestSuite(query))
    unittest.TextTestRunner().run(doctest.DocTestSuite(tools))
    unittest.main()
