"""Path hack to make PEP8/FLAKE happy"""

import os
import sys

sys.path.append(os.path.join(os.getcwd(), "..", "..", "lib"))
