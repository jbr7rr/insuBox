import numpy as np

def convert_int8_to_uint8(int8_array):
    uint8_array = int8_array.astype(np.uint8)
    
    return uint8_array

int8_array = np.array([18, 24, 12, 0, 0, 0, 6, 25, 0, 2, 0, -110, 0, -56, -19, 88], dtype=np.int8)
print("Original int8 array:", ', '.join(map(str, int8_array)))

uint8_array = convert_int8_to_uint8(int8_array)
print("Converted uint8 array:", ', '.join(map(str, uint8_array)))
