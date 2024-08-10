import numpy as np

# used to translate int8 to uint8 which is handy to port AndroidAPS code to C+=

def convert_int8_to_uint8(int8_array):
    # Ensure the input is of type int8
    int8_array = int8_array.astype(np.int8)
    
    # Initialize a uint8 array
    uint8_array = np.zeros_like(int8_array, dtype=np.uint8)
    
    # Convert negative values
    uint8_array[int8_array < 0] = (int8_array[int8_array < 0] + 256).astype(np.uint8)
    
    # Keep non-negative values as they are
    uint8_array[int8_array >= 0] = int8_array[int8_array >= 0].astype(np.uint8)
    
    return uint8_array

# Example usage
int8_array = np.array([7, 21, 5, 0, 8, 0, -110, 0], dtype=np.int8)
print("Original int8 array:", int8_array)

uint8_array = convert_int8_to_uint8(int8_array)
print("Converted uint8 array:", uint8_array)
