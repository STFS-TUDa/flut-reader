import ast

import h5py
import numpy as np
from six import string_types


def write_dictionary_to_hdf5(h5, dictionary, encode_list_as_string=False):
    """
    Writes the entries of dictionary to the h5-file or group.

    Parameters
    ----------
    h5 : HDF5-File or Group
    dictionary : dict
        Dictionary of key-value pairs to write.
    encode_list_as_string : bool, optional
        if true lists are converted to str(list) before write-out, by default False

    Example
    -------
    dictionary = {'data': [0, 1, 2], 'pos': {'x': 2, 'y': 1}}
    HDF5-file structure:
        - data : [0, 1, 2]
        - Pos
            - x: 2
            - y: 1

    """
    for key, value in dictionary.items():
        if isinstance(value, dict):
            write_dictionary_to_hdf5(
                h5.require_group(key),
                value,
                encode_list_as_string=encode_list_as_string,
            )
        else:
            data = encode_h5_entry(value, encode_list_as_string=encode_list_as_string)
            h5.create_dataset(key, data=data)


def encode_h5_entry(item, encode_list_as_string=False):
    """
    Encode a h5 entry.
    * strings are encoded in utf-8
    * lists are encoded as str(list) in utf-8
        if encode_list_as_string is set to True

    Parameters
    ----------
    item : object
    encode_list_as_string : bool, optional
        if true lists are converted to str(list), by default False
    """
    # None values are stored as an empty string
    if item is None:
        return "".encode(encoding="utf-8")
    # Strings are encoded as utf-8
    if isinstance(item, string_types):
        return item.encode(encoding="utf-8")
    if isinstance(item, list) and encode_list_as_string:
        return str(item).encode(encoding="utf-8")
    return item


def read_hdf5_as_dictionary(h5):
    """
    Read content of a HDF5-file or group to dictionary.

    Parameters
    ----------
    h5 : HDF5-File or Group
    """

    def loop_groups(group, dictionary):
        for key, value in group.items():
            if isinstance(value, h5py.Group):
                dictionary[key] = {}
                loop_groups(value, dictionary[key])
            else:
                dictionary[key] = decode_h5_entry(value)
        return dictionary

    return loop_groups(h5, {})


def decode_h5_entry(item):
    """
    Decode h5 entry corresponding to encode_h5_entry.
    * strings are decoded from utf-8
    * Lists that are stored as encoded strings are converted back to lists

    Parameters
    ----------
    item : HDF5-entry
        HDF5 value to decode
    """

    def stringToList(string):
        if string == "":
            return None
        else:
            return (
                ast.literal_eval(string)
                if string[0] == "[" and string[-1] == "]"
                else string
            )

    datatype = item.dtype
    item = item[()]
    if h5py.check_string_dtype(datatype):
        # Special treatment of strings
        if isinstance(item, np.ndarray):
            item = item.astype("str").tolist()
        else:
            item = stringToList(item.decode(encoding="utf-8"))
    return item
