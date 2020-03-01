import unittest
import lre

# Disable preallocated buffer
newlre = lre.LRE(0)
lre.dumps = newlre.pack
lre.loads = newlre.load


class TestOrder(unittest.TestCase):
    def testTypes(self):
        l1 = [float('-inf'), -10.5, -1, 0, 1, 10.5, float('inf'), b'', u'']
        l2 = sorted(l1, key=lre.dumps)
        self.assertEqual(l1, l2, 'invalid order')

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

    def testSortingFractional(self):
        l1 = [-0.999, -0.995, -0.8001, -0.1234, 0, 0.1, 0.1, 0.101, 0.801]
        l2 = sorted(l1, key=lre.dumps)
        self.assertEqual(l1, l2, 'invalid order')

    def testSortingString(self):
        l1 = [['00', 2], ['000', 3]]
        l2 = sorted(l1, key=lre.dumps)
        self.assertEqual(l1, l2, 'invalid order')


class TestLimits(unittest.TestCase):
    def testNan(self):
        with self.assertRaises(ValueError):
            lre.dumps(float('nan'))

    def testBigintOverflow(self):
        with self.assertRaises(OverflowError):
            lre.dumps(2**524280)

    def testDepthLimit(self):
        l = []
        l.append(l)

        with self.assertRaises(ValueError):
            lre.dumps(l)



