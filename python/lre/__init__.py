from .clre import LRE

lre_object = LRE(2048)
dumps = lre_object.pack
loads = lre_object.load
