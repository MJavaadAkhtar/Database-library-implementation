import pickle
import subprocess
import numpy as np
import re

data = np.zeros((4001,4))

for i in range(1000,5001,100):
    result = subprocess.check_output(["./write_fixed_len_pages","test.csv","myPage",str(i)])
    m = re.search("NUMBER OF RECORDS: (\d+)\nNUMBER OF PAGES: (\d+)\nTOTAL TIME: (\d+) milliseconds\n",result)
    if m:
        print(m.group(1),m.group(2),m.group(3))
