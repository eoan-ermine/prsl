import os
import lit.formats

config.name = "Prsl"

config.suffixes = [
    ".prsl"
]

config.test_format = lit.formats.ShTest(True)

config.test_source_root = os.path.dirname(__file__)