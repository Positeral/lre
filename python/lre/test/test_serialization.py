import unittest
import lre


class TestOrder(unittest.TestCase):
    def testNumeric(self):
        self.assertGreater(lre.dumps(1), lre.dumps(0),
                           'invalid order')
        
        self.assertGreater(lre.dumps(0), lre.dumps(-1),
                           'invalid order')

        self.assertGreater(lre.dumps(10), lre.dumps(1),
                           'invalid order')
        
        self.assertGreater(lre.dumps(10.1), lre.dumps(10),
                           'invalid order')

        self.assertGreater(lre.dumps(0.10), lre.dumps(0.09),
                           'invalid order')

        self.assertGreater(lre.dumps(0), lre.dumps(-0.5),
                           'invalid order')

        self.assertGreater(lre.dumps(-123), lre.dumps(-1234),
                           'invalid order')

    def testBigNumeric(self):
        self.assertGreater(lre.dumps(2**128), lre.dumps(2**127),
                           'invalid order for big number')
        
        self.assertGreater(lre.dumps(2**128), lre.dumps(2**127),
                           'invalid order for big number')

        self.assertGreater(lre.dumps(-2**100), lre.dumps(-2**101),
                           'invalid order for big number')

        self.assertGreater(lre.dumps(-2**100), lre.dumps(-2**300),
                           'invalid order for big number')

    def testString(self):
        self.assertGreater(lre.dumps('124'), lre.dumps('123'),
                           'invalid order')

        self.assertGreater(lre.dumps('91221'), lre.dumps('912200000'),
                           'invalid order')

    def testSorting(self):
        l1 = [-11, -10.99, -10.9, -10, -1.01, -1.001, -1, -0.51, -0.5, -0.05]
        l2 = sorted(l1, key=lre.dumps)
        self.assertEqual(l1, l2, 'invalid order')

        l1 = [-1, 0, 1, float('inf'), b'bytes', u'unicode']
        l2 = sorted(l1, key=lre.dumps)
        self.assertEqual(l1, l2, 'invalid order')

        l1 = [float('-inf'), -100, -2, 1, float('inf'), b'', u'']
        l2 = sorted(l1, key=lre.dumps)
        self.assertEqual(l1, l2, 'invalid order')

        l1 = [
            -0xffffffffffffff,
            -0xffffffffffff,
            -0xffffffffff,
            -0xffffffff,
            -0xffffff,
            -0xffff,
            -0xff,
            -1,
            0,
            1,
            0xff,
            0xffff,
            0xffffff,
            0xffffffff,
            0xffffffffff,
            0xffffffffffff,
            0xffffffffffffff
        ]
        l2 = sorted(l1, key=lre.dumps)
        self.assertEqual(l1, l2, 'invalid order')


class TestFormatEmptyValues(unittest.TestCase):
    def testEmpty(self):
        self.assertEqual(lre.dumps([]), b'',
                         'unexpected empty serialization')

        self.assertEqual(lre.dumps([[]]), b'',
                         'unexpected empty serialization')

        self.assertEqual(lre.dumps([[], [], [[[], []]]]), b'',
                         'unexpected empty serialization')

    def testEmptyString(self):
        v = u''
        self.assertEqual(lre.dumps(v), b'XL+',
                         'unexpected serialization of %r' % v)

        v = b''
        self.assertEqual(lre.dumps(v), b'XH+',
                         'unexpected serialization of %r' % v)

        v = [u'']
        self.assertEqual(lre.dumps(v), b'XL+',
                         'unexpected serialization of %r' % v)

        v = [b'']
        self.assertEqual(lre.dumps(v), b'XH+',
                         'unexpected serialization of %r' % v)

        v = [[], u'']
        self.assertEqual(lre.dumps(v), b'XL+',
                         'unexpected serialization of %r' % v)

        v = [[], b'']
        self.assertEqual(lre.dumps(v), b'XH+',
                         'unexpected serialization of %r' % v)

    def testEmptyInteger(self):
        v = 0
        self.assertEqual(lre.dumps(v), b'M00+',
                         'unexpected serialization of %r' % v)

        v = 0.0
        self.assertEqual(lre.dumps(v), b'M00+',
                         'unexpected serialization of %r' % v)

        v = -0.0
        self.assertEqual(lre.dumps(v), b'M00+',
                         'unexpected serialization of %r' % v)


class TestFormatString(unittest.TestCase):
    def testUnicode(self):
        v = u'abcdef'
        self.assertEqual(lre.dumps(v), b'X616263646566L+',
                         'unexpected serialization of %r' % v)

        v = u'china\u6123!'
        self.assertEqual(lre.dumps(v), b'X6368696e61e684a321L+',
                         'unexpected serialization of %r' % v)

        v = [u'china\u6123!', u'china\u6123!']
        self.assertEqual(lre.dumps(v), b'X6368696e61e684a321L+X6368696e61e684a321L+',
                         'unexpected serialization of %r' % v)

    def testBytes(self):
        v = b'\x01\x02'
        self.assertEqual(lre.dumps(v), b'X0102H+',
                         'unexpected serialization of %r' % v)        

        v = b'\x01\x02'
        self.assertEqual(lre.dumps([v,v]), b'X0102H+X0102H+',
                         'unexpected serialization of %r' % v)


class TestLimits(unittest.TestCase):
    def testNan(self):
        with self.assertRaises(ValueError):
            lre.dumps(float('nan'))

    def testBigintOverflow(self):
        with self.assertRaises(OverflowError):
            lre.dumps(2**524280)


class TestBignum(unittest.TestCase):
    def testBigBegin(self):
        v = 0xffffffffffffffff
        self.assertEqual(lre.dumps(v), b'U0008ffffffffffffffff+',
                         'unexpected serialization of %r' % v)

        v = -0xffffffffffffffff
        self.assertEqual(lre.dumps(v), b'Dfff70000000000000000~',
                         'unexpected serialization of %r' % v)

        v = 9223372036854775808
        self.assertEqual(lre.dumps(v), b'U00088000000000000000+',
                         'unexpected serialization of %r' % v)

        v = -9223372036854775809
        self.assertEqual(lre.dumps(v), b'Dfff77ffffffffffffffe~',
                         'unexpected serialization of %r' % v)

    def testVariative(self):
        v = [2**70, 1, []]
        self.assertEqual(lre.dumps(v), b'U0009400000000000000000+M01+',
                         'unexpected serialization of %r' % v)


