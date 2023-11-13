import unittest
import BlueExposureTest


class TestMetaclass(type):
    __persistvars__ = []
    __nonpersistvars__ = []

    def __new__(cls, name, bases, dict):
        return type.__new__(cls, name, bases, dict)

    def __call__(self, *args, **kwargs):
        inst = BlueExposureTest.classes.CreateInstance(self.__cid__)
        inst.__klass__ = self
        return inst


class TestClassWithMetaclass(object, metaclass=TestMetaclass):
    __cid__ = "BlueExposureTest.TestMethods"


class TestDir(unittest.TestCase):
    """
    Test that the dir() function returns
    sensible values for Blue-Exposed things.
    """
    def test_function_in_module_dir(self):
        self.assertIn("FunctionReturningBool", dir(BlueExposureTest))

    def test_enum_in_module_dir(self):
        self.assertIn("TEST_ENUM", dir(BlueExposureTest))

    def test_attributes_in_enum_dir(self):
        self.assertIn("ONE", dir(BlueExposureTest.TEST_ENUM))

    def test_class_in_module_dir(self):
        self.assertIn("TestMethods", dir(BlueExposureTest))

    def test_thunker_in_blue_list_dir(self):
        obj = BlueExposureTest.TestAttributes()
        self.assertIn("CloneTo", dir(obj.myVector))

    def test_method_in_singleton_dir(self):
        self.assertIn("LiveCount", dir(BlueExposureTest.classes))

    def test_method_in_class_type_dir(self):
        self.assertIn("MethodReturningInt", dir(BlueExposureTest.TestMethods))

    def test_method_in_instance_dir(self):
        tm = BlueExposureTest.TestMethods()
        self.assertIn("MethodReturningInt", dir(tm))

    def test_variable_in_instance_dir(self):
        tm = BlueExposureTest.TestMethods()
        self.assertIn("returnError", dir(tm))

    def test_baseclass_method_in_subclass_dir(self):
        class Foo(TestClassWithMetaclass):
            pass
        self.assertIn("MethodReturningInt", dir(Foo()))

    def test_baseclass_variable_in_subclass_dir(self):
        class Foo(TestClassWithMetaclass):
            pass
        self.assertIn("returnError", dir(Foo()))

    def test_subclass_variable_in_subclass_dir(self):
        class Foo(TestClassWithMetaclass):
            bar = 1
        self.assertIn("bar", dir(Foo()))

    def test_subclass_method_in_subclass_dir(self):
        class Foo(TestClassWithMetaclass):
            def DoStuff(self):
                pass
        self.assertIn("DoStuff", dir(Foo()))
