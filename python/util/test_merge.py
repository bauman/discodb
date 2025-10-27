__author__ = 'dan'
BIG_VAL = 2000
import discodb
from time import sleep

def test_merging():
    a = discodb.DiscoDBConstructor()
    a.add(b"k", b"v")
    a.add(b"db1", b"a value")
    for x in range(BIG_VAL):
        a.add(b"special", str(x).encode())

    o = a.finalize(unique_items=True)

    print("DB1 Keys and Values")
    print([k for k in o.keys()])
    print([v for v in o.values()])

    # dump the question-focused dataset.
    with open("/tmp/qfd1", 'wb') as f:
        o.dump(f)
    # need to re-open it to query it
    with open("/tmp/qfd1", 'rb') as f:
        b = discodb.DiscoDB.load(f)

    print("DB1 Keys and Values read from disk")
    first_keys = [k for k in b.keys()]
    first_values = [v for v in b.values()]
    type(b)

    c = discodb.DiscoDBConstructor()
    c.add(b"k", b"2")
    c.add(b"k", b"3")
    c.add(b"db2", b"another value")
    c.add(b"k", b"2")
    for x in range(BIG_VAL):
        c.add(b"special", str(x).encode())


    c.merge(b)
    oo = c.finalize(unique_items=True)

    print("DB2 Keys and Values")
    second_keys = [k for k in oo.keys()]
    second_values = [v for v in oo.values()]

    assert set(first_keys).issubset( second_keys )
    assert set(first_values).issubset( second_values )

    with open("/tmp/qfd2", 'wb') as f:
        oo.dump(f)

    d = discodb.DiscoDBConstructor()
    with open("/tmp/qfd2", 'rb') as f:
        e = discodb.DiscoDB.load(f)


    d.merge_with_explicit_value(e, b"new_value_for_all_keys") #one of these must be commented
    #d.merge(e, False) #one of these must be commented

    oo2 = d.finalize(unique_items=True)
    print ("testing the final merge")
    print ("values before reload ")
    third_keys = [v for v in oo2.keys()]
    third_values = [v for v in oo2.values()]
    assert set(third_keys) == set(second_keys)
    assert set(third_values) == set([b"new_value_for_all_keys"])

    with open("/tmp/qfdfinal", 'wb') as f:
        oo2.dump(f)

    with open("/tmp/qfdfinal", 'rb') as f:
        oofinal = discodb.DiscoDB.load(f)

    print("key/values after reload ")
    for k in oofinal.keys():
        for v in oofinal.query(discodb.Q.parse(k.decode())):
            print(k, "/", v)

