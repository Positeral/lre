import os
import unittest

testsuite = unittest.TestLoader().discover(os.path.dirname(__file__))
unittest.TextTestRunner(verbosity=1).run(testsuite)
